const states = {
  idle: {
    label: "READY",
    message: "",
    caption: "",
    surface: "watch_face",
    network: "Wi-Fi"
  },
  connecting: {
    label: "CONNECTING",
    message: "Opening secure channel",
    caption: "Please wait",
    surface: "assistant",
    network: "Linking"
  },
  listening: {
    label: "LISTENING",
    message: "Listening...",
    caption: "Speak naturally",
    surface: "assistant",
    network: "Online"
  },
  speaking: {
    label: "SPEAKING",
    message: "I am here. What would you like to do?",
    caption: "Tap the crown to close",
    surface: "assistant",
    network: "Online"
  },
  notification: {
    label: "NOTIFICATION",
    message: "Your reminder starts in 10 minutes",
    caption: "Calendar",
    surface: "assistant",
    network: "Wi-Fi"
  },
  error: {
    label: "CONNECTION ERROR",
    message: "Unable to reach the assistant",
    caption: "Check Wi-Fi and try again",
    surface: "assistant",
    network: "Offline"
  }
};

const elements = {
  watchScreen: document.querySelector("#watchScreen"),
  watchFace: document.querySelector("#watchFace"),
  assistantView: document.querySelector("#assistantView"),
  assistantState: document.querySelector("#assistantState"),
  assistantMessage: document.querySelector("#assistantMessage"),
  assistantCaption: document.querySelector("#assistantCaption"),
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

function updateClock() {
  const now = new Date();
  elements.clock.textContent = now.toLocaleTimeString([], {
    hour: "2-digit",
    minute: "2-digit",
    hour12: false
  });
  elements.date.textContent = [
    now.getFullYear(),
    String(now.getMonth() + 1).padStart(2, "0"),
    String(now.getDate()).padStart(2, "0")
  ].join("-");
}

function setState(nextState) {
  if (!states[nextState]) return;
  window.clearTimeout(pendingTransition);
  currentState = nextState;

  const state = states[nextState];
  const isIdle = nextState === "idle";
  const message = nextState === "notification" ? elements.messageInput.value : state.message;

  elements.watchScreen.dataset.state = nextState;
  elements.watchFace.classList.toggle("is-hidden", !isIdle);
  elements.assistantView.classList.toggle("is-hidden", isIdle);
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

document.querySelector("#talkButton").addEventListener("click", () => {
  setState("connecting");
  pendingTransition = window.setTimeout(() => setState("listening"), 650);
});

document.querySelector("#crownButton").addEventListener("click", () => setState("idle"));
elements.batteryRange.addEventListener("input", updateBattery);
elements.chargingToggle.addEventListener("change", updateBattery);
elements.messageInput.addEventListener("input", () => {
  if (currentState === "notification") setState("notification");
});

updateClock();
updateBattery();
setState("idle");
window.setInterval(updateClock, 1000);
