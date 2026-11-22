#pragma once
#include <Arduino.h>

// Static pages live in flash and are sent with send_P, so building a page
// does not allocate heap. Live values are loaded by JavaScript from /sensors.

// Shared stylesheet, cached by the browser (bump ?v= after changing it).
const char STYLE_CSS[] PROGMEM = R"rawliteral(
:root{--bg:#eef2f7;--card:#fff;--text:#1d2633;--muted:#6b7686;--line:#e3e8ef;
--accent:#2b7fe8;--soft:#e8f1fd;--ok:#1f9d55;--warn:#d98a00;--bad:#d64545}
@media (prefers-color-scheme:dark){:root{--bg:#10151c;--card:#19212b;--text:#e6ebf2;
--muted:#8d98a8;--line:#2a3441;--soft:#1d2d44}}
*{box-sizing:border-box}
body{margin:0;min-height:100vh;display:flex;justify-content:center;align-items:flex-start;
padding:24px 16px;background:var(--bg);color:var(--text);
font:15px/1.45 system-ui,-apple-system,"Segoe UI",Roboto,Arial,sans-serif}
.card{width:100%;max-width:420px;background:var(--card);border-radius:16px;padding:20px;
box-shadow:0 6px 24px rgba(16,24,40,.08)}
.logo{display:block;height:44px;max-width:100%;margin:0 auto 18px}
.head{display:flex;align-items:center;justify-content:space-between;gap:8px;margin-bottom:12px}
h1{font-size:20px;margin:0}
.chip{font-size:12px;padding:3px 10px;border-radius:99px;background:var(--soft);
color:var(--accent);font-weight:600;white-space:nowrap}
.stats{display:grid;grid-template-columns:1fr 1fr;gap:12px;margin-bottom:8px}
.stat{border:1px solid var(--line);border-radius:12px;padding:12px 14px}
.stat small{display:block;color:var(--muted)}
.stat b{font-size:28px;font-variant-numeric:tabular-nums}
.stat em{font-style:normal;font-size:14px;color:var(--muted);margin-left:2px}
.row{display:flex;align-items:center;justify-content:space-between;padding:12px 0;
border-top:1px solid var(--line)}
.row small{color:var(--muted);margin-left:6px}
.live{display:flex;align-items:center;flex-wrap:wrap;gap:6px 8px;font-size:13px;color:var(--muted);margin:-4px 0 12px}
.dot{width:8px;height:8px;border-radius:50%;background:var(--muted);flex:none}
.dot.ok{background:var(--ok);box-shadow:0 0 0 3px rgba(31,157,85,.25)}
.dot.bad{background:var(--bad)}
.cloud{margin-left:auto;font-weight:600}
.cloud.up{color:var(--ok)}
.spark{display:block;width:100%;height:30px;margin-top:6px;overflow:visible}
.spark polyline{fill:none;stroke:var(--accent);stroke-width:2;vector-effect:non-scaling-stroke;
stroke-linejoin:round;stroke-linecap:round}
.spark.hum polyline{stroke:#14a3b8}
.lbl{display:flex;align-items:center;gap:10px}
.bulb{width:18px;height:18px;border-radius:50%;background:var(--line);flex:none;
transition:background .25s,box-shadow .25s}
.bulb.on{background:var(--c);box-shadow:0 0 12px 3px var(--c)}
.bulb.plain.on{background:var(--accent);box-shadow:none}
.swatches{display:flex;flex-wrap:wrap;gap:8px;padding:2px 0 14px}
.dotc{width:28px;height:28px;border-radius:50%;border:3px solid var(--card);padding:0;cursor:pointer;
background:var(--c);box-shadow:0 0 0 1px var(--line);position:relative;display:grid;place-items:center;
font-size:16px;color:#fff;transition:transform .15s}
.dotc:hover{transform:scale(1.1)}
.dotc.sel{box-shadow:0 0 0 2px var(--text)}
.dotc.more{background:conic-gradient(#f33,#fc0,#3c3,#0cf,#36f,#c3f,#f33);margin:0}
.dotc.more input{position:absolute;inset:0;width:100%;height:100%;opacity:0;cursor:pointer;padding:0;border:0}
.sw{width:46px;height:26px;border-radius:99px;border:0;padding:0;background:var(--line);
position:relative;cursor:pointer;transition:background .2s}
.sw::after{content:"";position:absolute;top:3px;left:3px;width:20px;height:20px;
border-radius:50%;background:#fff;transition:transform .2s}
.sw[aria-pressed=true]{background:var(--accent)}
.sw[aria-pressed=true]::after{transform:translateX(20px)}
.btn{display:block;width:100%;padding:12px;border:0;border-radius:10px;background:var(--accent);
color:#fff;font:inherit;font-weight:600;text-align:center;text-decoration:none;cursor:pointer}
.btn.ghost{background:var(--soft);color:var(--accent)}
.btn:disabled,.link:disabled{opacity:.6;cursor:wait}
.link{border:0;background:none;color:var(--accent);font:inherit;font-weight:600;cursor:pointer;padding:4px}
.actions{display:grid;gap:8px;margin-top:16px}
label{display:block;font-size:13px;color:var(--muted);margin:12px 0 4px}
input{width:100%;padding:11px 12px;border:1px solid var(--line);border-radius:10px;
background:var(--card);color:var(--text);font:inherit}
input:focus{outline:2px solid var(--accent);outline-offset:-1px}
input:disabled{opacity:.6}
.pw{position:relative}
.pw input{padding-right:60px}
.pw button{position:absolute;right:4px;top:50%;transform:translateY(-50%)}
.status,.msg{font-size:13px;color:var(--muted);min-height:20px}
.msg{margin-top:12px}
.spin{display:inline-block;width:12px;height:12px;margin-right:6px;border:2px solid currentColor;
border-right-color:transparent;border-radius:50%;animation:r .8s linear infinite;vertical-align:-1px}
@keyframes r{to{transform:rotate(360deg)}}
.list{list-style:none;margin:8px 0 4px;padding:0;max-height:300px;overflow:auto;
border:1px solid var(--line);border-radius:12px}
.net{display:flex;align-items:center;gap:12px;padding:10px 12px;cursor:pointer;
border-top:1px solid var(--line)}
.net:first-child{border-top:0}
.net:hover,.net:focus,.net.sel{background:var(--soft);outline:0}
.net .name{flex:1;min-width:0;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;font-weight:500}
.net .meta{font-size:12px;color:var(--muted);text-align:right;white-space:nowrap;
font-variant-numeric:tabular-nums}
.bars{display:inline-flex;align-items:flex-end;gap:2px;height:16px;flex:none}
.bars i{width:4px;border-radius:1px;background:var(--line)}
.bars i:nth-child(1){height:25%}.bars i:nth-child(2){height:50%}
.bars i:nth-child(3){height:75%}.bars i:nth-child(4){height:100%}
.l4,.l3{--c:var(--ok)}.l2{--c:var(--warn)}.l1{--c:var(--bad)}
.l4 i,.l3 i:nth-child(-n+3),.l2 i:nth-child(-n+2),.l1 i:first-child{background:var(--c)}
)rawliteral";

const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="vi"><head><meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32 Dashboard</title><link rel="stylesheet" href="/style.css?v=4"></head>
<body><main class="card">
<img class="logo" src="/logo.png" alt="Core IoT">
<div class="head"><h1>Bảng điều khiển</h1><span class="chip" id="mode">...</span></div>
<div class="live"><i class="dot" id="dot"></i><span id="ago">Đang kết nối...</span>
<span class="cloud" id="cloud"></span></div>
<div class="stats">
<div class="stat"><small>🌡️ Nhiệt độ</small><b id="temp">--</b><em>°C</em>
<svg class="spark" id="st" viewBox="0 0 100 30" preserveAspectRatio="none"><polyline/></svg></div>
<div class="stat"><small>💧 Độ ẩm</small><b id="hum">--</b><em>%</em>
<svg class="spark hum" id="sh" viewBox="0 0 100 30" preserveAspectRatio="none"><polyline/></svg></div>
</div>
<div class="row"><span class="lbl"><i class="bulb plain" id="b1"></i>LED 1<small>D13 · GPIO48</small></span>
<button class="sw" id="l1" aria-pressed="false" aria-label="LED 1" onclick="toggleLED(1)"></button></div>
<div class="row"><span class="lbl"><i class="bulb" id="b2"></i>LED RGB<small>NeoPixel GPIO45</small></span>
<button class="sw" id="l2" aria-pressed="false" aria-label="LED RGB" onclick="toggleLED(2)"></button></div>
<div class="swatches" id="pal">
<label class="dotc more" title="Chọn màu khác">＋<input type="color" id="c2" value="#00ff00"
 aria-label="Chọn màu LED RGB" onchange="setColor(this.value.slice(1))"></label></div>
<div class="actions"><a class="btn ghost" href="/settings">⚙️ Cấu hình Wi-Fi</a></div>
</main>
<script>
const $ = id => document.getElementById(id);
const N = 20;                                   // sparkline points (~1 min at 3 s)
const hist = {t: [], h: []};
const COLORS = ['ff3b30', 'ff9500', 'ffcc00', '00ff00', '007aff', 'af52de', 'ffffff'];
let lastOk = 0, online = false;

// Colour presets, inserted before the custom picker
COLORS.forEach(c => {
  const b = document.createElement('button');
  b.className = 'dotc'; b.style.setProperty('--c', '#' + c);
  b.dataset.c = c; b.setAttribute('aria-label', 'Màu #' + c);
  b.onclick = () => setColor(c);
  $('pal').insertBefore(b, $('pal').lastElementChild);
});

function setLeds(j) {
  const on1 = j.led1 === 'ON', on2 = j.led2 === 'ON';
  $('l1').setAttribute('aria-pressed', on1);
  $('l2').setAttribute('aria-pressed', on2);
  $('b1').classList.toggle('on', on1);
  $('b2').classList.toggle('on', on2);
  if (j.color) {
    $('b2').style.setProperty('--c', '#' + j.color);
    document.querySelectorAll('.dotc[data-c]').forEach(b => b.classList.toggle('sel', b.dataset.c === j.color));
    if (document.activeElement !== $('c2')) $('c2').value = '#' + j.color;
  }
}
function toggleLED(id) {
  fetch('/toggle?led=' + id).then(r => r.json()).then(setLeds).catch(() => {});
}
function setColor(hex) {
  fetch('/color?hex=' + hex).then(r => r.json()).then(setLeds).catch(() => {});
}
// Sparkline scaled to the min/max of the buffered values
function spark(id, arr) {
  const pl = $(id).firstElementChild;
  if (arr.length < 2) { pl.setAttribute('points', ''); return; }
  const lo = Math.min(...arr), hi = Math.max(...arr), span = Math.max(hi - lo, 0.5);
  pl.setAttribute('points', arr.map((v, i) =>
    (i * 100 / (N - 1)).toFixed(1) + ',' + (27 - (v - lo) / span * 24).toFixed(1)).join(' '));
}
function push(arr, v) { if (v !== null) { arr.push(v); if (arr.length > N) arr.shift(); } }

function refresh() {
  fetch('/sensors', {cache: 'no-store'}).then(r => r.json()).then(d => {
    lastOk = Date.now(); online = true;
    $('temp').textContent = d.temp === null ? '--' : d.temp.toFixed(1);
    $('hum').textContent = d.hum === null ? '--' : d.hum.toFixed(1);
    push(hist.t, d.temp); push(hist.h, d.hum);
    spark('st', hist.t); spark('sh', hist.h);
    setLeds(d);
    $('mode').textContent = d.mode + ' · ' + d.ip + (d.rssi ? ' · ' + d.rssi + ' dBm' : '');
    $('cloud').textContent = d.mode === 'STA'
      ? (d.cloud === 'ON' ? '☁️ CoreIOT: đã kết nối' : '☁️ CoreIOT: đang kết nối...')
      : '☁️ CoreIOT: cần Wi-Fi router';
    $('cloud').classList.toggle('up', d.cloud === 'ON');
    tick();
  }).catch(() => { online = false; tick(); });
}
// Connection indicator: stale after 10 s without a successful poll
function tick() {
  const s = Math.round((Date.now() - lastOk) / 1000);
  const ok = lastOk && online && s < 10;
  $('dot').className = 'dot ' + (ok ? 'ok' : lastOk ? 'bad' : '');
  $('ago').textContent = !lastOk ? 'Đang kết nối...' : ok ? 'Cập nhật ' + s + ' giây trước' : 'Mất kết nối với board';
}
refresh();
setInterval(refresh, 3000);
setInterval(tick, 1000);
</script></body></html>
)rawliteral";

const char SETTINGS_HTML[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html lang="vi"><head><meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Wi-Fi Settings</title><link rel="stylesheet" href="/style.css?v=4"></head>
<body><main class="card">
<img class="logo" src="/logo.png" alt="Core IoT">
<div class="head"><h1>Cấu hình Wi-Fi</h1><button type="button" class="link" id="scan">Quét lại</button></div>
<div class="status" id="scanStatus" role="status"></div>
<ul class="list" id="networks" aria-label="Mạng Wi-Fi tìm thấy" hidden></ul>
<form id="wifiForm">
<label for="ssid">Tên mạng (SSID)</label>
<input id="ssid" maxlength="32" required autocomplete="off" placeholder="Chọn ở trên hoặc nhập tay">
<label for="pass">Mật khẩu</label>
<div class="pw"><input id="pass" type="password" maxlength="64" autocomplete="new-password"
placeholder="Bỏ trống nếu là mạng mở"><button type="button" class="link" id="eye">Hiện</button></div>
<div class="actions"><button class="btn" id="connect" type="submit">Kết nối</button>
<a class="btn ghost" href="/">Quay lại</a></div>
</form>
<div class="msg" id="msg" role="status"></div>
</main>
<script>
const $ = id => document.getElementById(id);
const scanButton = $('scan'), connectButton = $('connect'), list = $('networks');
const ssid = $('ssid'), pass = $('pass');
const LABELS = ['', 'Yếu', 'Trung bình', 'Mạnh', 'Rất mạnh'];
const level = rssi => rssi >= -55 ? 4 : rssi >= -67 ? 3 : rssi >= -75 ? 2 : 1;
const quality = rssi => Math.max(0, Math.min(100, 2 * (rssi + 100)));

function setBusy(busy) { scanButton.disabled = connectButton.disabled = busy; }

function choose(item, network) {
  for (const el of list.children) el.classList.remove('sel');
  item.classList.add('sel');
  ssid.value = network.ssid;
  pass.value = '';
  pass.disabled = !network.secure;
  pass.placeholder = network.secure ? 'Nhập mật khẩu' : 'Mạng mở, không cần mật khẩu';
  if (network.secure) pass.focus();
}

function render(networks) {
  list.replaceChildren();
  for (const network of networks) {
    const lv = level(network.rssi);
    const item = document.createElement('li');
    item.className = 'net';
    item.tabIndex = 0;
    item.innerHTML = '<span class="bars l' + lv + '"><i></i><i></i><i></i><i></i></span>' +
      '<span class="name"></span><span class="meta">' + network.rssi + ' dBm<br>' +
      LABELS[lv] + ' · ' + quality(network.rssi) + '%</span>';
    // SSID is untrusted text: never put it into innerHTML.
    item.querySelector('.name').textContent = (network.secure ? '🔒 ' : '') + network.ssid;
    item.title = network.ssid;
    item.onclick = () => choose(item, network);
    item.onkeydown = e => { if (e.key === 'Enter' || e.key === ' ') { e.preventDefault(); choose(item, network); } };
    list.appendChild(item);
  }
  list.hidden = !networks.length;
}

async function scan() {
  setBusy(true);
  $('scanStatus').innerHTML = '<span class="spin"></span>Đang quét Wi-Fi...';
  try {
    const started = await fetch('/scan', {method: 'POST'});
    if (!started.ok) throw new Error(await started.text());
    let result;
    for (let attempt = 0; attempt < 60; ++attempt) {
      await new Promise(resolve => setTimeout(resolve, 500));
      const response = await fetch('/scan', {cache: 'no-store'});
      if (!response.ok) throw new Error('Không lấy được kết quả quét.');
      result = await response.json();
      if (result.status !== 'scanning') break;
    }
    if (!result || result.status !== 'done') throw new Error('Quét thất bại hoặc quá thời gian. Hãy thử lại.');
    // The ESP32 already returns the strongest networks, unique and sorted.
    const found = result.networks;
    render(found);
    $('scanStatus').textContent = found.length ? found.length + ' mạng mạnh nhất. Chọn một mạng hoặc nhập SSID.' :
                                                 'Không tìm thấy mạng. Bạn có thể nhập SSID trực tiếp.';
  } catch (error) {
    $('scanStatus').textContent = error.message;
  } finally {
    setBusy(false);
  }
}

scanButton.onclick = scan;
ssid.oninput = () => {
  for (const el of list.children) el.classList.remove('sel');
  pass.disabled = false;
  pass.placeholder = 'Bỏ trống nếu là mạng mở';
};
$('eye').onclick = () => {
  const hidden = pass.type === 'password';
  pass.type = hidden ? 'text' : 'password';
  $('eye').textContent = hidden ? 'Ẩn' : 'Hiện';
};

$('wifiForm').onsubmit = async e => {
  e.preventDefault();
  if (connectButton.disabled) return;
  setBusy(true);
  const msg = $('msg');
  msg.innerHTML = '<span class="spin"></span>Đang gửi thông tin kết nối...';
  try {
    const response = await fetch('/connect', {
      method: 'POST',
      headers: {'Content-Type': 'application/x-www-form-urlencoded'},
      body: new URLSearchParams({ssid: ssid.value, pass: pass.disabled ? '' : pass.value})
    });
    const text = await response.text();
    if (!response.ok) throw new Error(text);
    pass.value = '';
    msg.textContent = text + ' Nếu thành công, AP sẽ tắt; xem IP mới trên Serial Monitor.';
  } catch (error) {
    msg.textContent = 'Không xác nhận được kết nối: ' + error.message +
                      '. Nếu AP đã tắt, xem IP mới trên Serial Monitor.';
  } finally {
    setBusy(false);
  }
};

scan();
</script></body></html>
)rawliteral";
