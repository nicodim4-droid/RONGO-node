const isEnglish = document.documentElement.lang === 'en';
const locale = isEnglish ? 'en-US' : 'ru-RU';
const text = {
  noData: isEnglish ? 'NO DATA' : 'Нет данных',
  online: isEnglish ? 'ONLINE' : 'В ЭФИРЕ',
  offline: isEnglish ? 'OFFLINE' : 'НЕ В СЕТИ',
  nodeOnline: isEnglish ? 'NODE ONLINE' : 'УЗЕЛ В ЭФИРЕ',
  nodeOffline: isEnglish ? 'УЗЕЛ НЕ В СЕТИ' : 'NODE OFFLINE',
  sensorOnline: isEnglish ? 'SENSOR ONLINE' : 'ДАТЧИК В ЭФИРЕ',
  sensorOffline: isEnglish ? 'SENSOR OFFLINE' : 'ДАТЧИК ОТКЛЮЧЕН',
  sensorError: isEnglish ? 'SENSOR ERROR' : 'ОШИБКА ДАТЧИКА',
  controllerUnavailable: isEnglish ? 'Controller unavailable' : 'Нет связи с контроллером',
  waiting: isEnglish ? 'WAITING' : 'ОЖИДАНИЕ',
  active: isEnglish ? 'ACTIVE' : 'АКТИВНА',
  canOffline: isEnglish ? 'OFFLINE' : 'НЕ В СЕТИ',
  temperatureNode: isEnglish ? 'Temperature node' : 'Температурный узел',
  apiError: isEnglish ? 'API request failed' : 'Ошибка запроса API'
};

const elements = {
  clock: document.getElementById('clock'),
  deviceTitle: document.getElementById('device-title'),
  controllerStatus: document.getElementById('controller-status'),
  controllerStatusText: document.getElementById('controller-status-text'),
  nodeStatus: document.getElementById('node-status'),
  nodeName: document.getElementById('node-name'),
  nodeArticle: document.getElementById('node-article'),
  deviceType: document.getElementById('device-type'),
  deviceIp: document.getElementById('device-ip'),
  uptime: document.getElementById('uptime'),
  wifiRssi: document.getElementById('wifi-rssi'),
  sensorType: document.getElementById('sensor-type'),
  sensorStatus: document.getElementById('sensor-status'),
  temperature: document.getElementById('temperature'),
  humidity: document.getElementById('humidity'),
  canStatus: document.getElementById('can-status'),
  canNodes: document.getElementById('can-nodes'),
  canExchange: document.getElementById('can-exchange'),
  apiError: document.getElementById('api-error')
};

const modeButtons = Array.from(document.querySelectorAll('.mode-button'));
let currentMode = 'node';
let updateInProgress = false;

function setBadge(element, label, state) {
  element.className = `state-badge status-${state}`;
  element.textContent = label;
}

function setMode(mode) {
  currentMode = mode === 'master' ? 'master' : 'node';
  document.documentElement.dataset.mode = currentMode;
  const name = currentMode === 'master' ? 'RONGO-master' : 'RONGO-node';
  elements.deviceTitle.textContent = name;
  elements.nodeName.textContent = name;
  modeButtons.forEach((button) => {
    button.setAttribute('aria-pressed', String(button.dataset.mode === currentMode));
  });
}

function formatNumber(value, decimals = 1) {
  return typeof value === 'number' && Number.isFinite(value)
    ? value.toFixed(decimals)
    : text.noData;
}

function formatUptime(seconds) {
  if (typeof seconds !== 'number' || !Number.isFinite(seconds) || seconds < 0) {
    return text.noData;
  }

  const totalMinutes = Math.floor(seconds / 60);
  const days = Math.floor(totalMinutes / 1440);
  const hours = Math.floor((totalMinutes % 1440) / 60);
  const minutes = totalMinutes % 60;

  if (isEnglish) {
    if (days > 0) return `${days} d ${hours} h`;
    if (hours > 0) return `${hours} h ${minutes} min`;
    if (minutes > 0) return `${minutes} min`;
    return `${Math.floor(seconds)} s`;
  }
  if (days > 0) return `${days} д ${hours} ч`;
  if (hours > 0) return `${hours} ч ${minutes} мин`;
  if (minutes > 0) return `${minutes} мин`;
  return `${Math.floor(seconds)} с`;
}

function updateClock() {
  elements.clock.textContent = new Date().toLocaleTimeString(locale, {
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit'
  });
}

function formatCanState(state) {
  if (state === 'active') return { label: text.active, className: 'online' };
  if (state === 'offline') return { label: text.canOffline, className: 'offline' };
  return { label: text.waiting, className: 'pending' };
}

function renderSensor(sensor) {
  elements.sensorType.textContent = sensor && typeof sensor.type === 'string'
    ? sensor.type
    : text.noData;

  if (!sensor || sensor.status === 'offline') {
    elements.sensorStatus.className = 'sensor-state status-offline';
    elements.sensorStatus.textContent = text.sensorOffline;
    elements.temperature.textContent = text.noData;
    elements.humidity.textContent = text.noData;
    return;
  }

  if (sensor.status === 'error') {
    elements.sensorStatus.className = 'sensor-state status-error';
    elements.sensorStatus.textContent = text.sensorError;
    elements.temperature.textContent = text.noData;
    elements.humidity.textContent = text.noData;
    return;
  }

  if (sensor.status === 'online') {
    elements.sensorStatus.className = 'sensor-state status-online';
    elements.sensorStatus.textContent = text.sensorOnline;
  } else {
    elements.sensorStatus.className = 'sensor-state status-pending';
    elements.sensorStatus.textContent = text.noData;
  }

  elements.temperature.textContent = formatNumber(sensor.temperatureC);
  elements.humidity.textContent = formatNumber(sensor.humidityPercent);
}

function renderCan(can) {
  const state = formatCanState(can && can.status);
  setBadge(elements.canStatus, state.label, state.className);
  elements.canNodes.textContent = Number.isInteger(can && can.discoveredNodes)
    ? String(can.discoveredNodes)
    : text.waiting;
  elements.canExchange.textContent = formatCanState(can && can.exchange).label;
}

function renderDashboard(data) {
  const online = data.status === 'online';
  elements.controllerStatus.className = `controller-status status-${online ? 'online' : 'offline'}`;
  elements.controllerStatusText.textContent = online ? text.online : text.offline;
  setBadge(elements.nodeStatus, online ? text.nodeOnline : text.nodeOffline, online ? 'online' : 'offline');

  const name = currentMode === 'master'
    ? 'RONGO-master'
    : (typeof data.name === 'string' && data.name.length > 0 ? data.name : 'RONGO-node');
  elements.deviceTitle.textContent = name;
  elements.nodeName.textContent = name;

  elements.deviceType.textContent = data.deviceType === 'temperature'
    ? text.temperatureNode
    : (typeof data.deviceType === 'string' ? data.deviceType : text.noData);
  elements.nodeArticle.textContent = typeof data.article === 'string' && data.article.length > 0
    ? data.article
    : text.noData;
  elements.deviceIp.textContent = typeof data.ip === 'string' && data.ip.length > 0
    ? data.ip
    : text.noData;
  elements.uptime.textContent = formatUptime(data.uptimeSeconds);
  elements.wifiRssi.textContent = typeof data.wifiRssiDbm === 'number'
    ? `${formatNumber(data.wifiRssiDbm, 0)} dBm`
    : text.noData;
  renderSensor(data.sensor);
  renderCan(data.can);
}

function renderUnavailable(error) {
  elements.controllerStatus.className = 'controller-status status-offline';
  elements.controllerStatusText.textContent = text.offline;
  setBadge(elements.nodeStatus, text.nodeOffline, 'offline');
  elements.deviceTitle.textContent = currentMode === 'master' ? 'RONGO-master' : 'RONGO-node';
  elements.nodeName.textContent = elements.deviceTitle.textContent;
  elements.deviceType.textContent = text.noData;
  elements.nodeArticle.textContent = text.noData;
  elements.deviceIp.textContent = text.noData;
  elements.uptime.textContent = text.noData;
  elements.wifiRssi.textContent = text.noData;
  renderSensor(null);
  renderCan(null);
  elements.apiError.textContent = `${text.apiError}: ${error.message}`;
  elements.apiError.hidden = false;
}

async function updateDashboard() {
  if (updateInProgress) return;
  updateInProgress = true;
  try {
    const response = await fetch('/api/device', { cache: 'no-store' });
    if (!response.ok) {
      throw new Error(`HTTP ${response.status}`);
    }
    renderDashboard(await response.json());
    elements.apiError.hidden = true;
  } catch (error) {
    renderUnavailable(error);
    console.error(error);
  } finally {
    updateInProgress = false;
  }
}

modeButtons.forEach((button) => {
  button.addEventListener('click', () => setMode(button.dataset.mode));
});

setMode('node');
updateClock();
updateDashboard();
window.setInterval(updateDashboard, 2000);
window.setInterval(updateClock, 1000);
