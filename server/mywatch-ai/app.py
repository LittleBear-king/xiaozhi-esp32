"""Small provider-neutral chat gateway for the MyWatch project."""

from __future__ import annotations

import json
import os
import tempfile
from threading import Lock
from typing import Any
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen
from fastapi import File, UploadFile
from fastapi import FastAPI, HTTPException
from pydantic import BaseModel, Field


class Message(BaseModel):
    role: str = Field(pattern="^(system|user|assistant)$")
    content: str = Field(min_length=1, max_length=8000)


class ChatRequest(BaseModel):
    messages: list[Message] = Field(min_length=1, max_length=32)
    model: str | None = None
    temperature: float = Field(default=0.7, ge=0, le=2)


class ChatResponse(BaseModel):
    model: str
    message: Message


app = FastAPI(title="MyWatch AI", version="0.1.0")
_asr_model = None
_asr_lock = Lock()


def _ollama_url() -> str:
    return os.getenv("MYWATCH_OLLAMA_URL", "http://127.0.0.1:11434").rstrip("/")


def _model_name(requested: str | None) -> str:
    return requested or os.getenv("MYWATCH_MODEL", "qwen2.5:3b")


def _messages(messages: list[Message]) -> list[dict[str, str]]:
    result = [message.model_dump() for message in messages]
    if not any(message["role"] == "system" for message in result):
        result.insert(
            0,
            {
                "role": "system",
                "content": os.getenv(
                    "MYWATCH_SYSTEM_PROMPT",
                    "你是 MyWatch 手表上的中文语音助手，回答简洁、自然、适合语音播放。",
                ),
            },
        )
    return result


def _keep_alive() -> str | int:
    value = os.getenv("MYWATCH_KEEP_ALIVE", "30m")
    return -1 if value.strip() == "-1" else value


def _get_asr_model():
    """Load Whisper only on the first transcription request and reuse it."""
    global _asr_model
    if _asr_model is not None:
        return _asr_model
    with _asr_lock:
        if _asr_model is not None:
            return _asr_model
        try:
            from faster_whisper import WhisperModel
        except ImportError as error:
            raise HTTPException(
                status_code=503,
                detail="ASR is not installed; run: pip install faster-whisper",
            ) from error
        device = os.getenv("MYWATCH_ASR_DEVICE", "cuda")
        compute_type = os.getenv("MYWATCH_ASR_COMPUTE_TYPE", "float16")
        try:
            _asr_model = WhisperModel(
                os.getenv("MYWATCH_ASR_MODEL", "small"),
                device=device,
                compute_type=compute_type,
            )
        except Exception as error:
            raise HTTPException(status_code=503, detail=f"ASR model could not start: {error}") from error
    return _asr_model


def _call_ollama(request: ChatRequest) -> ChatResponse:
    model = _model_name(request.model)
    payload = json.dumps(
        {"model": model, "messages": _messages(request.messages), "stream": False,
         "options": {"temperature": request.temperature},
         "keep_alive": _keep_alive()}
    ).encode("utf-8")
    http_request = Request(
        f"{_ollama_url()}/api/chat",
        data=payload,
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    try:
        with urlopen(http_request, timeout=120) as response:
            data: dict[str, Any] = json.loads(response.read())
    except (HTTPError, URLError, TimeoutError) as error:
        raise HTTPException(status_code=502, detail=f"Ollama unavailable: {error}") from error
    message = data.get("message")
    if not isinstance(message, dict) or not isinstance(message.get("content"), str):
        raise HTTPException(status_code=502, detail="Ollama returned an invalid response")
    return ChatResponse(model=model, message=Message(role="assistant", content=message["content"]))


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok", "model": _model_name(None)}


@app.post("/v1/chat", response_model=ChatResponse)
def chat(request: ChatRequest) -> ChatResponse:
    return _call_ollama(request)

@app.post("/v1/upload-test")
async def upload_test(audio: UploadFile = File(...)) -> dict[str, object]:
    data = await audio.read()
    return {
        "filename": audio.filename,
        "content_type": audio.content_type,
        "size": len(data),
    }


@app.post("/v1/transcribe")
async def transcribe(
    audio: UploadFile = File(...),
    language: str | None = None,
) -> dict[str, object]:
    """Transcribe one uploaded recording into text."""
    suffix = os.path.splitext(audio.filename or "audio.wav")[1] or ".wav"
    with tempfile.NamedTemporaryFile(suffix=suffix) as temporary:
        temporary.write(await audio.read())
        temporary.flush()
        model = _get_asr_model()
        segments, info = model.transcribe(
            temporary.name,
            language=language or None,
            vad_filter=True,
            beam_size=5,
        )
        text = "".join(segment.text for segment in segments).strip()
    return {
        "text": text,
        "language": info.language,
        "language_probability": round(info.language_probability, 4),
    }

if __name__ == "__main__":
    import uvicorn

    uvicorn.run(
        app,
        host=os.getenv("MYWATCH_HOST", "0.0.0.0"),
        port=int(os.getenv("MYWATCH_PORT", "8088")),
    )
