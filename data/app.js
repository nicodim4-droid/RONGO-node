const elements = {
  clock: document.getElementById('clock'),
  systemStatus: document.getElementById('system-status'),
  temperature: document.getElementById('temperature'),
  temperatureCaption: document.getElementById('temperature-caption'),
  humidity: document.getElementById('humidity'),
  humidityCaption: document.getElementById('humidity-caption'),
  uptime: document.getElementById('uptime'),
  rssi: document.getElementById('rssi'),
  sensorStatus: document.getElementById('sensor-status'),
  deviceName: document.getElementById('device-name'),
  deviceIp: document.getElementById('device-ip'),
  firmware: document.getElementById('firmware'),
  sensorDetail: document.getElementById('sensor-detail'),
  lastUpdate: document.getElementById('last-update'),
  connectionError: document.getElementById('connection-error')
};

let updateInProgress = false;

function setStatus(element, text, state) {
  element.className = state;
  const label = element.querySelector('.status-text');
  if (label) {
    label.textContent = text;
  } else {
    element.textContent = text;
  }
}

function formatNumber(value, decimals) {
  return typeof value === 'number' && Number.isFinite(value)
    ? value.toFixed(decimals)
    : '—';
}

function formatUptime(seconds) {
  if (typeof seconds !== 'number' || !Number.isFinite(seconds) || seconds < 0) {
    return '—';
  }

  const totalMinutes = Math.floor(seconds / 60);
  const days = Math.floor(totalMinutes / 1440);
  const hours = Math.floor((totalMinutes % 1440) / 60);
  const minutes = totalMinutes % 60;

  if (days > 0) return `${days} д ${hours} ч`;
  if (hours > 0) return `${hours} ч ${minutes} мин`;
  return `${minutes} мин`;
}

function updateClock() {
  elements.clock.textContent = new Date().toLocaleTimeString('ru-RU', {
    hour: '2-digit',
    minute: '2-digit',
    second: '2-digit'
  });
}

function setTelemetryUnavailable(message) {
  elements.temperature.textContent = '—';
  elements.humidity.textContent = '—';
  elements.uptime.textContent = '—';
  elements.rssi.textContent = '—';
  elements.temperatureCaption.textContent = 'Данные недоступны';
  elements.humidityCaption.textContent = 'Данные недоступны';
  elements.sensorDetail.textContent = 'Нет связи с датчиком';
  setStatus(elements.sensorStatus, 'Нет данных', 'state-badge status-warning');
  elements.connectionError.textContent = message;
  elements.connectionError.hidden = false;
}

function renderTelemetry(data) {
  elements.temperature.textContent = formatNumber(data.temp, 1);
  elements.humidity.textContent = formatNumber(data.hum, 1);
  elements.uptime.textContent = formatUptime(data.uptime);
  elements.rssi.textContent = formatNumber(data.rssi, 0);

  const sensorAvailable = data.sensor === true;
  const sensorFailed = data.sensor === false;

  elements.temperatureCaption.textContent = sensorAvailable
    ? 'Датчик работает штатно'
    : 'Нет показаний датчика';
  elements.humidityCaption.textContent = sensorAvailable
    ? 'Датчик работает штатно'
    : 'Нет показаний датчика';
  elements.sensorDetail.textContent = sensorAvailable
    ? 'Подключён'
    : sensorFailed
      ? 'Не отвечает'
      : 'Нет данных';

  if (sensorAvailable) {
    setStatus(elements.sensorStatus, 'В норме', 'state-badge status-online');
  } else if (sensorFailed) {
    setStatus(elements.sensorStatus, 'Ошибка', 'state-badge status-error');
  } else {
    setStatus(elements.sensorStatus, 'Нет данных', 'state-badge status-warning');
  }

  if (typeof data.ip === 'string' && data.ip.length > 0) {
    elements.deviceIp.textContent = data.ip;
  }
  if (typeof data.fw === 'string' && data.fw.length > 0) {
    elements.firmware.textContent = data.fw;
  }
  if (typeof data.device === 'string' && data.device.length > 0) {
    elements.deviceName.textContent = data.device;
  }
}

async function fetchJson(url) {
  const response = await fetch(url, { cache: 'no-store' });
  if (!response.ok) {
    throw new Error(`HTTP ${response.status}`);
  }
  return response.json();
}

async function updateDashboard() {
  if (updateInProgress) return;
  updateInProgress = true;
  updateClock();

  try {
    const status = await fetchJson('/status');
    const isOnline = status.status === 'online';
    setStatus(
      elements.systemStatus,
      isOnline ? 'Устройство в сети' : 'Неизвестный статус',
      isOnline ? 'system-status status-online' : 'system-status status-warning'
    );
    if (typeof status.device === 'string' && status.device.length > 0) {
      elements.deviceName.textContent = status.device;
    }

    try {
      const telemetry = await fetchJson('/data');
      renderTelemetry(telemetry);
      elements.connectionError.hidden = true;
      elements.lastUpdate.textContent = new Date().toLocaleTimeString('ru-RU', {
        hour: '2-digit',
        minute: '2-digit',
        second: '2-digit'
      });
    } catch (error) {
      setTelemetryUnavailable(`Не удалось получить телеметрию: ${error.message}`);
    }
  } catch (error) {
    setStatus(elements.systemStatus, 'Нет связи с устройством', 'system-status status-offline');
    setTelemetryUnavailable(`Ошибка подключения к устройству: ${error.message}`);
  } finally {
    updateInProgress = false;
  }
}

updateClock();
updateDashboard();
window.setInterval(updateDashboard, 5000);
window.setInterval(updateClock, 1000);
