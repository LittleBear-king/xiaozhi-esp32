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

返回内容包含 `message` 和 `model`。这个服务不依赖小智云端账号，模型和会话
提示词由我们自己控制。
