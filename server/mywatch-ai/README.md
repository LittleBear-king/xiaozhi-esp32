# MyWatch AI Server

这是手表自有 AI 服务的第一阶段：用本地 Ollama 模型提供对话能力，保留
OpenAI 风格的 HTTP 接口，后续可以接入 ASR、TTS 和 XiaoZhi MQTT 网关。

## 启动本地模型

先安装并启动 Ollama，然后下载一个适合本机的模型：

```bash
ollama serve
ollama pull qwen2.5:3b
```

启动网关：

```bash
cd /home/bear/work/xiaozhi/server/mywatch-ai
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
python app.py
```

默认监听 `0.0.0.0:8088`，模型由 `MYWATCH_MODEL` 指定。
网关默认让模型驻留 30 分钟，避免每次对话重新加载；设置
`MYWATCH_KEEP_ALIVE=-1` 可让模型一直驻留，直到 Ollama 重启。

## 测试

```bash
curl http://127.0.0.1:8088/health
curl -X POST http://127.0.0.1:8088/v1/chat \
  -H 'Content-Type: application/json' \
  -d '{"messages":[{"role":"user","content":"你好，请介绍一下自己"}]}'
```

语音识别需要额外安装 `faster-whisper`，默认使用 RTX GPU 和 `small` 模型：

```bash
pip install -r requirements.txt
curl -X POST http://127.0.0.1:8088/v1/transcribe \
  -F "audio=@hello.wav" -F "language=zh"
```

语音对话接口会先识别录音，再调用 Ollama：

```bash
curl -X POST http://127.0.0.1:8088/v1/voice-chat \
  -F "audio=@speech.wav" -F "language=zh"
```

成功后返回 `input_text`（识别结果）和 `reply_text`（模型回答）。没有检测到
人声时返回 `422`；Ollama 不可用时返回 `502`。

## TTS 语音合成

TTS 使用本地 Piper。先安装 Piper，并下载一个中文 `.onnx` 语音模型，然后设置：

```bash
export MYWATCH_TTS_BIN=piper
export MYWATCH_TTS_MODEL=/绝对路径/中文语音模型.onnx
```

直接测试文字转语音：

```bash
curl -X POST http://127.0.0.1:8088/v1/synthesize \
  -F "text=你好，我是 MyWatch"
```

`/v1/synthesize` 和默认的 `/v1/voice-chat` 返回 `audio_base64`，解码后是 WAV
音频。暂时只测试 ASR 或聊天时，可在语音对话请求中加入
`-F "include_audio=false"`。

## WebSocket 实时通道

手表可以通过 `/ws/voice` 持续上传二进制音频块。第一版使用实时连接和二进制音频返回；模型推理在收到 `end` 后处理完整一句话，后续可再替换为流式 ASR。

客户端消息顺序：

```text
文本：{"type":"start","language":"zh"}
二进制：WAV 文件分片（第一版要求内容最终组成一个有效 WAV）
文本：{"type":"end"}
```

服务器会发送 `ready`、`audio_received`、`transcribing`、`transcript`、`thinking`、`reply`、`synthesizing`、`audio_start`、多个二进制音频块、`audio_end` 和 `done`。发生问题时发送 `error`。

电脑端联调脚本：

```bash
pip install websockets
python tools/test-mywatch-voice-ws.py \
  --url ws://127.0.0.1:8088/ws/voice \
  --audio speech.wav \
  --output reply.wav
```

脚本会打印每个协议事件，并将服务器返回的 TTS 音频保存为 `reply.wav`。

第一次调用会加载 ASR 模型，后续请求复用同一个模型。没有 NVIDIA GPU 时，
可设置 `MYWATCH_ASR_DEVICE=cpu` 和 `MYWATCH_ASR_COMPUTE_TYPE=int8`。

返回内容包含 `message` 和 `model`。这个服务不依赖小智云端账号，模型和会话
提示词由我们自己控制。
