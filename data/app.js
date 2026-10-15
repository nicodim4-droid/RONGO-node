async function update() {
    try {
        const r = await fetch('/data');
        if (!r.ok) throw new Error('HTTP ' + r.status);
        const j = await r.json();

        document.getElementById('temp').textContent   = (j.temp !== null) ? j.temp.toFixed(2) : '—';
        document.getElementById('hum').textContent    = (j.hum  !== null) ? j.hum.toFixed(2)  : '—';
        document.getElementById('uptime').textContent = j.uptime;
        document.getElementById('ip').textContent     = j.ip;
        document.getElementById('rssi').textContent   = j.rssi;
        document.getElementById('fw').textContent     = j.fw;

        const s = document.getElementById('sensor');
        if (j.sensor) {
            s.textContent = 'OK';
            s.className   = 'ok';
        } else {
            s.textContent = 'ERROR';
            s.className   = 'error';
        }
    } catch (e) {
        console.log('update failed', e);
    }
}

setInterval(update, 1000);   // раз в секунду
update();                    // и сразу при загрузке