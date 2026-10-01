const states = {
  idle: { label: "就绪", message: "", caption: "", surface: "watch_face", network: "在线" },
  sleeping: { label: "息屏", message: "", caption: "", surface: "sleep", network: "" },
  connecting: {
    label: "正在连接",
    message: "正在建立安全连接",
    caption: "请稍候",
    surface: "assistant",
    network: "连接中"
  },
  listening: {
    label: "倾听中",
    message: "我在听……",
    caption: "请自然说话",
    surface: "assistant",
    network: "在线"
  },
  speaking: {
    label: "正在回复",
    message: "我在，你想做什么？",
    caption: "按侧键结束",
    surface: "assistant",
    network: "在线"
  },
  notification: {
    label: "通知",
    message: "你的日程将在 10 分钟后开始",
    caption: "日历",
    surface: "assistant",
    network: "在线"
  },
  error: {
    label: "连接失败",
    message: "暂时无法连接语音助手",
    caption: "请检查 Wi-Fi 后重试",
    surface: "assistant",
    network: "离线"
  },
  launcher: { label: "应用", message: "", caption: "", surface: "apps", network: "在线" },
  activity: { label: "运动", message: "", caption: "", surface: "apps", network: "在线" },
  notifications: { label: "通知", message: "", caption: "", surface: "apps", network: "在线" },
  settings: { label: "设置", message: "", caption: "", surface: "apps", network: "在线" },
  tools: { label: "工具", message: "", caption: "", surface: "apps", network: "在线" }
};

const elements = {
  watchScreen: document.querySelector("#watchScreen"),
  watchFace: document.querySelector("#watchFace"),
  assistantView: document.querySelector("#assistantView"),
  assistantState: document.querySelector("#assistantState"),
  assistantMessage: document.querySelector("#assistantMessage"),
  assistantCaption: document.querySelector("#assistantCaption"),
  appView: document.querySelector("#appView"),
  appTitle: document.querySelector("#appTitle"),
  appContent: document.querySelector("#appContent"),
  networkStatus: document.querySelector("#networkStatus"),
  clock: document.querySelector("#clock"),
  date: document.querySelector("#date"),
  batteryFill: document.querySelector("#batteryFill"),
  batteryText: document.querySelector("#batteryText"),
  batteryRange: document.querySelector("#batteryRange"),
  batteryOutput: document.querySelector("#batteryOutput"),
  chargingToggle: document.querySelector("#chargingToggle"),
  messageInput: document.querySelector("#messageInput"),
  stateReadout: document.querySelector("#stateReadout"),
  surfaceReadout: document.querySelector("#surfaceReadout")
};

let currentState = "idle";
let pendingTransition = null;
let countdownSeconds = 300;
let countdownRunning = false;
let countdownLastTick = performance.now();

function updateClock() {
  const now = new Date();
  elements.clock.textContent = now.toLocaleTimeString([], {
    hour: "2-digit",
    minute: "2-digit",
    hour12: false
  });
  const weekdays = ["周日", "周一", "周二", "周三", "周四", "周五", "周六"];
  const calendarDate =
    `${now.getFullYear()}年${String(now.getMonth() + 1).padStart(2, "0")}月` +
    `${String(now.getDate()).padStart(2, "0")}日`;
  elements.date.textContent = `${calendarDate}  ${weekdays[now.getDay()]}`;
}

function renderApp(name) {
  const pages = {
    launcher: `<div class="app-grid">
      <button class="app-tile" data-app="activity">运动</button>
      <button class="app-tile" data-app="notifications">通知</button>
      <button class="app-tile" data-app="settings">设置</button>
      <button class="app-tile" data-app="tools">工具</button>
      <button class="app-tile" data-app="connecting">AI</button>
      <button class="app-tile" data-app="idle">表盘</button>
    </div>`,
    activity: `<div class="metric">今日<strong>6,842 步</strong></div>
      <div class="metric"><strong>4.78 km</strong>距离</div>
      <div class="metric"><strong>273.6 kcal</strong>消耗</div>
      <div class="metric"><strong>活跃 46 分钟</strong>运动传感器正常</div>`,
    notifications: `<article class="notice-card"><strong>日历 · 设计评审</strong>将在 10 分钟后开始</article>
      <article class="notice-card"><strong>手机 · 已连接</strong>配套应用正在同步</article>`,
    settings: `<div class="settings-list">
      <button class="setting-row"><span>抬腕亮屏</span><strong>开</strong></button>
      <button class="setting-row"><span>勿扰模式</span><strong>关</strong></button>
      <button class="setting-row"><span>时间格式</span><strong>24小时</strong></button>
      <button class="setting-row"><span>屏幕亮度</span><strong>75%</strong></button>
    </div>`,
    tools: `<div class="tool-inline"><button class="tool-action" data-tool="countdown">${formatCountdown()}</button><button class="tool-reset" data-tool="countdown-reset">复位</button></div>
      <div class="system-row"><strong>秒表 · 00:00.00</strong>点击设备开始计时</div>
      <div class="system-row"><strong>RTC 正常</strong>系统时间已同步</div>
      <div class="system-row"><strong>手机未连接</strong>天气、音乐和查找手机需要先配对</div>
      <div class="system-row"><strong>WDT 开启</strong>OTA 回滚保护已启用</div>`
  };
  elements.appTitle.textContent = states[name].label;
  elements.appContent.innerHTML = pages[name] || pages.launcher;
}

function formatCountdown() {
  const minutes = Math.floor(countdownSeconds / 60).toString().padStart(2, "0");
  const seconds = Math.floor(countdownSeconds % 60).toString().padStart(2, "0");
  return `${countdownRunning ? "暂停" : "开始"}  ${minutes}:${seconds}`;
}

function updateCountdown(now) {
  const elapsed = (now - countdownLastTick) / 1000;
  countdownLastTick = now;
  if (currentState === "tools" && countdownRunning) {
    countdownSeconds = Math.max(0, countdownSeconds - elapsed);
    if (countdownSeconds === 0) countdownRunning = false;
    const button = elements.appContent.querySelector('[data-tool="countdown"]');
    if (button) button.textContent = formatCountdown();
  }
}

function setState(nextState) {
  if (!states[nextState]) return;
  window.clearTimeout(pendingTransition);
  currentState = nextState;

  const state = states[nextState];
  const isIdle = nextState === "idle";
  const isSleeping = nextState === "sleeping";
  const isApp = state.surface === "apps";
  const message = nextState === "notification" ? elements.messageInput.value : state.message;

  elements.watchScreen.dataset.state = nextState;
  elements.watchFace.classList.toggle("is-hidden", !isIdle);
  elements.assistantView.classList.toggle("is-hidden", isIdle || isApp || isSleeping);
  elements.appView.classList.toggle("is-hidden", !isApp);
  document.querySelector(".status-strip").classList.toggle("is-hidden", isSleeping);
  if (isApp) renderApp(nextState);
  elements.assistantState.textContent = state.label;
  elements.assistantMessage.textContent = message;
  elements.assistantCaption.textContent = state.caption;
  elements.networkStatus.textContent = state.network;
  elements.stateReadout.textContent = nextState;
  elements.surfaceReadout.textContent = state.surface;

  document.querySelectorAll("[data-state]").forEach((button) => {
    button.classList.toggle("is-active", button.dataset.state === nextState);
  });
}

function updateBattery() {
  const level = Number(elements.batteryRange.value);
  const charging = elements.chargingToggle.checked;
  const color = charging ? "#2fd17b" : level <= 15 ? "#ff5f68" : "#dce2e8";
  elements.batteryFill.style.width = `${level}%`;
  elements.batteryFill.style.backgroundColor = color;
  elements.batteryText.textContent = charging ? `${level}% +` : `${level}%`;
  elements.batteryOutput.textContent = `${level}%`;
}

document.querySelector("#stateGrid").addEventListener("click", (event) => {
  const button = event.target.closest("[data-state]");
  if (button) setState(button.dataset.state);
});

document.querySelector("#appsButton").addEventListener("click", () => setState("launcher"));
document.querySelector("#appBack").addEventListener("click", () => {
  setState(currentState === "launcher" ? "idle" : "launcher");
});
elements.appContent.addEventListener("click", (event) => {
  const target = event.target.closest("[data-app]");
  if (target) setState(target.dataset.app);
  const tool = event.target.closest("[data-tool]");
  if (!tool) return;
  if (tool.dataset.tool === "countdown") {
    countdownRunning = countdownSeconds > 0 && !countdownRunning;
    countdownLastTick = performance.now();
    tool.textContent = formatCountdown();
  } else if (tool.dataset.tool === "countdown-reset") {
    countdownRunning = false;
    countdownSeconds = 300;
    const countdown = elements.appContent.querySelector('[data-tool="countdown"]');
    if (countdown) countdown.textContent = formatCountdown();
  }
});

document.querySelector("#talkButton").addEventListener("click", () => {
  setState("connecting");
  pendingTransition = window.setTimeout(() => setState("listening"), 650);
});

document.querySelector("#crownButton").addEventListener("click", () => setState("idle"));
document.querySelector("#raiseWristButton").addEventListener("click", () => {
  if (currentState === "sleeping") setState("idle");
});
elements.batteryRange.addEventListener("input", updateBattery);
elements.chargingToggle.addEventListener("change", updateBattery);
elements.messageInput.addEventListener("input", () => {
  if (currentState === "notification") setState("notification");
});

updateClock();
updateBattery();
setState("idle");
window.setInterval(updateClock, 1000);
window.setInterval(() => updateCountdown(performance.now()), 100);
