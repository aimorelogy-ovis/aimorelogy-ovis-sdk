const state = { authorization: "" };
const form = document.querySelector("#configForm");
const toast = document.querySelector("#toast");
const dialog = document.querySelector("#loginDialog");

function notify(message) {
  toast.textContent = message;
  toast.classList.add("visible");
  window.setTimeout(() => toast.classList.remove("visible"), 3500);
}

async function api(path, options = {}) {
  const headers = { ...(options.headers || {}) };
  if (state.authorization) headers.Authorization = state.authorization;
  if (options.method && options.method !== "GET") headers["X-OVIS-CSRF"] = "1";
  if (options.body) headers["Content-Type"] = "application/json";
  const response = await fetch(path, { ...options, headers });
  const data = await response.json();
  if (!response.ok) throw new Error(data.error || `请求失败：${response.status}`);
  return data;
}

async function refreshStatus() {
  try {
    const data = await api("/api/v1/ipcamera/status");
    document.querySelector("#serviceState").textContent = data.running ? "运行中" : "已停止";
    document.querySelector("#serviceDetail").textContent = data.pid ? `PID ${data.pid}` : (data.detail || "未检测到进程");
  } catch (error) {
    document.querySelector("#serviceState").textContent = "状态不可用";
    document.querySelector("#serviceDetail").textContent = error.message;
  }
}

async function waitTask(id) {
  for (;;) {
    const task = await api(`/api/v1/tasks/${id}`);
    if (task.state !== "running") {
      if (task.state === "failed") throw new Error(task.detail || "服务操作失败");
      return task;
    }
    await new Promise(resolve => window.setTimeout(resolve, 700));
  }
}

async function runAction(action, button) {
  if (!state.authorization) return dialog.showModal();
  button.disabled = true;
  try {
    const task = await api(`/api/v1/ipcamera/${action}`, { method: "POST" });
    await waitTask(task.task_id);
    notify("服务操作已完成");
    await refreshStatus();
  } catch (error) { notify(error.message); }
  finally { button.disabled = false; }
}

async function loadConfig() {
  if (!state.authorization) return dialog.showModal();
  try {
    const data = await api("/api/v1/ipcamera/config");
    Object.entries(data.values).forEach(([name, value]) => {
      const input = form.elements[name];
      if (!input) return;
      if (input.type === "checkbox") input.checked = Boolean(value);
      else input.value = value;
    });
  } catch (error) { notify(error.message); }
}

async function saveConfig() {
  if (!form.reportValidity()) return;
  const values = {};
  [...form.elements].forEach(input => {
    if (!input.name) return;
    values[input.name] = input.type === "checkbox" ? (input.checked ? 1 : 0) : Number(input.value);
  });
  try {
    await api("/api/v1/ipcamera/config", { method: "PUT", body: JSON.stringify(values) });
    notify("配置已保存，重启 ipcamera 后生效");
  } catch (error) { notify(error.message); }
}

document.querySelectorAll("[data-action]").forEach(button => button.addEventListener("click", () => runAction(button.dataset.action, button)));
document.querySelector("#reloadConfig").addEventListener("click", loadConfig);
document.querySelector("#saveConfig").addEventListener("click", saveConfig);
document.querySelector("#resetConfig").addEventListener("click", async () => {
  if (!window.confirm("确定恢复出厂配置？")) return;
  try { await api("/api/v1/ipcamera/config/reset", { method: "POST" }); await loadConfig(); notify("已恢复出厂配置，重启后生效"); }
  catch (error) { notify(error.message); }
});
document.querySelector("#loginButton").addEventListener("click", () => dialog.showModal());
document.querySelector("#loginForm").addEventListener("submit", async event => {
  event.preventDefault();
  state.authorization = `Basic ${btoa(`${document.querySelector("#username").value}:${document.querySelector("#password").value}`)}`;
  try {
    await api("/api/v1/ipcamera/config");
    document.querySelector("#loginError").textContent = "";
    document.querySelector("#loginButton").textContent = "已登录";
    dialog.close();
    await loadConfig();
  } catch (error) {
    state.authorization = "";
    document.querySelector("#loginError").textContent = error.message;
  }
});

api("/api/v1/system/version").then(data => { document.querySelector("#version").textContent = `v${data.manager}`; });
refreshStatus();
window.setInterval(refreshStatus, 5000);
