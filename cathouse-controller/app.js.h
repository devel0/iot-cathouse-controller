F(" \
var debug = false; \
 \
var infoUpdateIntervalMs = 5000; \
var tempUpdateIntervalMs = 10000; \
 \
 \
var sensorDesc = []; \
 \
requirejs.config({ \
\"moment\": \"://cdnjs.cloudflare.com/ajax/libs/require.js/2.3.6/require.min.js\" \
}); \
 \
function showSpin() { \
$('.j-spin').removeClass(\"collapse\"); \
} \
 \
function hideSpin() { \
$('.j-spin').addClass(\"collapse\"); \
} \
 \
function showHome() { \
$('.j-containers').addClass('collapse'); \
$('.j-home').removeClass('collapse'); \
$('.j-menu').removeClass('fstrong'); \
$('.menu-home').addClass('fstrong'); \
} \
 \
function showConfig() { \
$('.j-containers').addClass('collapse'); \
$('.j-config').removeClass('collapse'); \
$('.j-menu').removeClass('fstrong'); \
$('.menu-config').addClass('fstrong'); \
} \
 \
var history_interval_sec = 10; \
var manualMode = false; \
 \
var baseurl = ''; \
if (debug) baseurl = 'http://10.10.3.9'; \
 \
async function reloadTemp(addr) { \
showSpin(); \
let finished = false; \
let res = null; \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + '/temp/' + addr, \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
await sleep(1000); \
} \
} \
hideSpin(); \
$('#t' + addr)[0].innerText = res; \
} \
 \
function human_readable_filesize(bytes, onlyBytesUnit = true, bytesMultiple = 1, decimals = 1) { \
let k = 1024.0; \
let m = k * 1024.0; \
let g = m * 1024.0; \
let t = g * 1024.0; \
 \
if (bytesMultiple != 1) bytes = Math.trunc(mround(bytes, bytesMultiple)); \
 \
if (bytes < k) { \
if (onlyBytesUnit) return bytes; \
else return bytes + ' b'; \
} else if (bytes >= k && bytes < m) return (bytes / k).toFixed(decimals) + ' Kb'; \
else if (bytes >= m && bytes < g) return (bytes / m).toFixed(decimals) + ' Mb'; \
else if (bytes >= g && bytes < t) return (bytes / g).toFixed(decimals) + ' Gb'; \
else return (bytes / t).toFixed(decimals); \
} \
 \
function toggleCatInThere() { \
if (confirm('sure to force toggling of cathouse in there?') != true) \
return; \
_toggleCatInThere(); \
} \
 \
async function _toggleCatInThere() { \
showSpin(); \
let finished = false; \
let res = null; \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + '/info', \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
await sleep(1000); \
} \
} \
 \
let catIsInThere = res.catIsInThere ? false : true; \
finished = false; \
res = null; \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + '/setcatinthere/' + (catIsInThere ? '1' : '0'), \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
await sleep(1000); \
} \
} \
hideSpin(); \
 \
} \
 \
async function togglePort(p) { \
if (!manualMode) { \
alert(\"can't toggle port in automatic mode\"); \
} else { \
showSpin(); \
let finished = false; \
let res = null; \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + '/port/toggle/' + p, \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
await sleep(1000); \
} \
} \
hideSpin(); \
 \
await reloadInfo(); \
} \
} \
 \
async function reloadInfo() { \
showSpin(); \
let finished = false; \
let res = null; \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + '/info', \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
await sleep(1000); \
} \
} \
hideSpin(); \
 \
manualMode = res[\"manualMode\"]; \
 \
if (res[\"p1\"] == true) \
$('.port-p1').addClass('port-on'); \
else \
$('.port-p1').removeClass('port-on'); \
 \
if (res[\"p2\"] == true) \
$('.port-p2').addClass('port-on'); \
else \
$('.port-p2').removeClass('port-on'); \
 \
if (res[\"p3\"] == true) \
$('.port-p3').addClass('port-on'); \
else \
$('.port-p3').removeClass('port-on'); \
 \
if (res[\"p4\"] == true) \
$('.port-p4').addClass('port-on'); \
else \
$('.port-p4').removeClass('port-on'); \
 \
if (res[\"led\"] == true) \
$('.port-led').addClass('port-on'); \
else \
$('.port-led').removeClass('port-on'); \
 \
if (res[\"fan\"] == true) \
$('.port-fan').addClass('port-on'); \
else \
$('.port-fan').removeClass('port-on'); \
 \
let Wh = res.Wh; \
let runtime_hr = res.runtime_hr; \
 \
$('.mean-power')[0].innerText = (Wh / runtime_hr).toFixed(0); \
$('.free-ram')[0].innerText = human_readable_filesize(res.freeram); \
$('.temp-history-interval-min')[0].innerText = res.temp_history_interval_min.toFixed(1); \
$('.current-state')[0].innerText = res.current_state; \
$('.cycle-prev-current')[0].innerText = res.prev_cycle + \"/\" + res.current_cycle; \
$('.cat-is-in-there')[0].innerText = res.catIsInThere ? 'yes' : 'no'; \
if (res.catIsInThere) \
$('.cat-is-in-there').addClass('port-on'); \
else \
$('.cat-is-in-there').removeClass('port-on'); \
 \
{ \
var ctx = document.getElementById(\"weightChart\").getContext('2d'); \
 \
var dtnow = moment(); \
let ary = res[\"adcWeightArray\"]; \
let interval_sec = res[\"statIntervalSec\"]; \
 \
var i = 0; \
var dss = []; { \
dts = []; \
let valcnt = ary.length; \
$.each(ary, function (idx, val) { \
secbefore = (valcnt - idx - 1) * interval_sec; \
tt = moment(dtnow).subtract(secbefore, 'seconds'); \
dts.push({ \
t: tt, \
y: val \
}); \
}); \
let meanw = _.reduce(ary, function (memo, num) { \
return memo + num; \
}, 0) / valcnt; \
let meanwlatest = 0.0; { \
let i = valcnt - 1; \
let c = 0; \
while (i >= 0 && c < 20) { \
meanwlatest += ary[i]; \
++c; \
} \
meanwlatest /= 20; \
} \
$('.adc-weight')[0].innerText = meanw.toFixed(0); \
$('.adc-weight-latest')[0].innerText = meanwlatest.toFixed(0); \
dtsmean = []; \
dtsmean.push({ \
t: moment(dtnow).subtract((valcnt - 1) * interval_sec, 'seconds'), \
y: meanw \
}); \
dtsmean.push({ \
t: moment(dtnow), \
y: meanw \
}); \
 \
dss.push({ \
borderColor: '#00ff00', \
fill: false, \
label: 'mean', \
data: dtsmean, \
pointRadius: 0 \
}, { \
borderColor: '#00aa00', \
fill: true, \
label: 'adc Weight', \
data: dts, \
pointRadius: 0 \
}); \
 \
++i; \
} \
 \
var myChart = new Chart(ctx, { \
type: 'line', \
data: { \
datasets: dss \
}, \
options: { \
maintainAspectRatio: false, \
animation: false, \
scales: { \
xAxes: [{ \
type: 'time' \
}] \
} \
} \
}); \
} \
} \
 \
async function reloadAllTemp() { \
$('.tempdev').each(async function (idx) { \
let v = this.innerText; \
await reloadTemp(v); \
}); \
} \
 \
async function reloadCharts() { \
{ \
let finished = false; \
let res = null; \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + \"/temphistory\", \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
await sleep(1000); \
} \
} \
 \
var colors = ['orange', 'yellow', 'green', 'blue', 'violet', 'black', 'red']; \
var ctx = document.getElementById(\"tempChart\").getContext('2d'); \
 \
var dtnow = moment(); \
 \
var i = 0; \
var dss = []; \
$.each(res, function (idx, data) { \
id = Object.keys(data)[0]; \
desc = id; \
q = $.grep(sensorDesc, (el, idx) => el.id == id); \
if (q.length > 0) desc = q[0].description; \
 \
if (i > colors.length - 1) color = 'brown'; \
else color = colors[i]; \
 \
valcnt = data[id].length; \
 \
let trend = '<i class=\"fas fa-equals\"></i>'; \
if (valcnt > 1) { \
var last = data[id][valcnt - 1]; \
var lastbut1 = data[id][valcnt - 2]; \
if (last > lastbut1) \
trend = '<i style=\"color:red\" class=\"fas fa-arrow-up\"></i>'; \
else if (last < lastbut1) \
trend = '<i style=\"color:blue\" class=\"fas fa-arrow-down\"></i>'; \
} \
$('#trend' + id)[0].innerHTML = trend; \
 \
 \
dts = []; \
$.each(data[id], function (idx, val) { \
secbefore = (valcnt - idx - 1) * history_interval_sec; \
tt = moment(dtnow).subtract(secbefore, 'seconds'); \
dts.push({ \
t: tt, \
y: val \
}); \
}); \
 \
dss.push({ \
borderColor: color, \
label: desc, \
data: dts, \
pointRadius: 0 \
}); \
 \
++i; \
}); \
 \
var myChart = new Chart(ctx, { \
type: 'line', \
data: { \
datasets: dss \
}, \
options: { \
animation: false, \
scales: { \
xAxes: [{ \
type: 'time', \
time: { \
displayFormats: { \
'hour': 'HH:mm' \
} \
}, \
position: 'bottom' \
}] \
} \
} \
}); \
} \
 \
{ \
let finished = false; \
let res = null; \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + \"/catinhistory\", \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
await sleep(1000); \
} \
} \
 \
var ctx = document.getElementById(\"catinChart\").getContext('2d'); \
 \
var dtnow = moment(); \
 \
var i = 0; \
var dss = []; { \
dts = []; \
let valcnt = res.length; \
$.each(res, function (idx, val) { \
secbefore = (valcnt - idx - 1) * history_interval_sec; \
tt = moment(dtnow).subtract(secbefore, 'seconds'); \
dts.push({ \
t: tt, \
y: val ? 1 : 0 \
}); \
}); \
 \
dss.push({ \
borderColor: '#F0B8FF', \
fill: true, \
label: 'cat in there', \
data: dts, \
pointRadius: 0 \
}); \
 \
++i; \
}; \
 \
var myChart = new Chart(ctx, { \
type: 'line', \
data: { \
datasets: dss \
}, \
options: { \
maintainAspectRatio: false, \
animation: false, \
scales: { \
xAxes: [{ \
type: 'time' \
}], \
yAxes: [{ \
ticks: { \
min: 0, \
max: 1 \
} \
}] \
} \
} \
}); \
} \
} \
 \
async function reloadConfig() { \
showSpin(); \
let finished = false; \
let res = null; \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + '/getconfig', \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
await sleep(1000); \
finished = true; \
} \
} \
hideSpin(); \
 \
sensorDesc = [{ \
id: res[\"tbottomId\"], \
description: \"bottom\" \
}, \
{ \
id: res[\"twoodId\"], \
description: \"wood\" \
}, \
{ \
id: res[\"tambientId\"], \
description: \"ambient\" \
}, \
{ \
id: res[\"texternId\"], \
description: \"extern\" \
} \
]; \
 \
$('#config-firmwareVersion')[0].innerText = res[\"firmwareVersion\"]; \
$('#config-wifiSSID')[0].innerText = res[\"wifiSSID\"]; \
$('#config-tbottomId')[0].value = res[\"tbottomId\"]; \
$('#config-twoodId')[0].value = res[\"twoodId\"]; \
$('#config-tambientId')[0].value = res[\"tambientId\"]; \
$('#config-texternId')[0].value = res[\"texternId\"]; \
$('#config-manualMode').prop('checked', res[\"manualMode\"]); \
$('#config-adcWeightDeltaCat')[0].value = res[\"adcWeightDeltaCat\"]; \
$('#config-tbottomLimit')[0].value = res[\"tbottomLimit\"]; \
$('#config-twoodLimit')[0].value = res[\"twoodLimit\"]; \
$('#config-tambientLimit')[0].value = res[\"tambientLimit\"]; \
$('#config-cooldownTimeMs-min')[0].value = res[\"cooldownTimeMs\"] / 1000.0 / 60.0; \
$('#config-standbyPort')[0].value = res[\"standbyPort\"]; \
$('#config-standbyDuration-min')[0].value = res[\"standbyDurationMs\"] / 1000.0 / 60.0; \
$('#config-fullpowerDuration-min')[0].value = res[\"fullpowerDurationMs\"] / 1000.0 / 60.0; \
$('#config-texternGTESysOff')[0].value = res[\"texternGTESysOff\"]; \
$('#config-tbottomGTEFanOn')[0].value = res[\"tbottomGTEFanOn\"]; \
$('#config-fanlessMode').prop('checked', res[\"fanlessMode\"]); \
$('#config-portDurationMs-min')[0].value = res[\"portDurationMs\"] / 1000.0 / 60.0; \
$('#config-portOverlapDurationMs-min')[0].value = res[\"portOverlapDurationMs\"] / 1000.0 / 60.0; \
} \
 \
var infoLastLoad; \
var tempLastLoad; \
var chartLastLoad; \
var autorefreshInProgress = false; \
 \
async function autorefresh() { \
if (autorefreshInProgress) return; \
 \
autorefreshInProgress = true; \
var dtnow = new Date(); \
if (infoLastLoad === undefined || (dtnow - infoLastLoad) > infoUpdateIntervalMs) { \
await reloadInfo(); \
infoLastLoad = new Date(); \
} \
if (tempLastLoad === undefined || (dtnow - tempLastLoad) > tempUpdateIntervalMs) { \
await reloadAllTemp(); \
tempLastLoad = new Date(); \
} \
if (chartLastLoad === undefined || (dtnow - chartLastLoad) > history_interval_sec * 1000) { \
await reloadCharts(); \
chartLastLoad = new Date(); \
} \
autorefreshInProgress = false; \
} \
 \
function sleep(ms) { \
return new Promise(resolve => setTimeout(resolve, ms)); \
} \
 \
function manageResize() {} \
 \
async function myfn() { \
 \
setInterval(autorefresh, 1000); \
 \
manageResize(); \
$(window).resize(function () { \
manageResize(); \
}); \
 \
autorefreshInProgress = true; \
 \
showSpin(); \
let res = null; \
let finished = false; \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + '/tempdevices', \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
await sleep(1000); \
} \
} \
 \
finished = false; \
let resnfo = null; \
while (!finished) { \
try { \
resnfo = await $.ajax({ \
url: baseurl + '/info', \
type: 'GET' \
}); \
finished = true; \
} catch (e) { \
await sleep(1000); \
} \
} \
 \
await reloadConfig(); \
 \
history_interval_sec = resnfo.history_interval_sec; \
hideSpin(); \
 \
var h = \"\"; \
 \
for (i = 0; i < res.tempdevices.length; ++i) { \
let tempId = res.tempdevices[i]; \
 \
h += \"<tr>\"; \
 \
h += \"<td><span class='tempdev'>\"; \
h += tempId; \
h += \"</span></td>\"; \
 \
h += \"<td>\"; \
q = $.grep(sensorDesc, (el, idx) => el.id == tempId); \
if (q.length > 0) h += q[0].description; \
h += \"</td>\"; \
 \
h += \"<td><span id='t\" + tempId + \"'>\"; \
h += \"</span></td>\"; \
 \
h += \"<td><span id='trend\" + tempId + \"'>\"; \
h += \"</span></td>\"; \
 \
 \
h += \"</tr>\"; \
} \
 \
$('#tbody-temp')[0].innerHTML = h; \
 \
autorefreshInProgress = false; \
} \
 \
$(document).ready(function () { \
myfn(); \
}); \
 \
async function saveConfig() { \
showSpin(); \
let finished = false; \
let res = null; \
 \
let config = { \
tbottomId: $('#config-tbottomId')[0].value, \
twoodId: $('#config-twoodId')[0].value, \
tambientId: $('#config-tambientId')[0].value, \
texternId: $('#config-texternId')[0].value, \
manualMode: $('#config-manualMode').is(\":checked\"), \
adcWeightDeltaCat: $('#config-adcWeightDeltaCat')[0].value, \
tbottomLimit: parseFloat($('#config-tbottomLimit')[0].value), \
twoodLimit: parseFloat($('#config-twoodLimit')[0].value), \
tambientLimitxx: parseFloat($('#config-tambientLimit')[0].value), \
cooldownTimeMs: parseFloat($('#config-cooldownTimeMs-min')[0].value) * 1000 * 60, \
standbyPort: parseInt($('#config-standbyPort')[0].value), \
fullpowerDurationMs: parseFloat($('#config-fullpowerDuration-min')[0].value) * 1000 * 60, \
standbyDurationMs: parseFloat($('#config-standbyDuration-min')[0].value) * 1000 * 60, \
texternGTESysOff: parseFloat($('#config-texternGTESysOff')[0].value), \
tbottomGTEFanOn: parseFloat($('#config-tbottomGTEFanOn')[0].value), \
portDurationMs: parseFloat($('#config-portDurationMs-min')[0].value) * 1000 * 60, \
portOverlapDurationMs: parseFloat($('#config-portOverlapDurationMs-min')[0].value) * 1000 * 60, \
fanlessMode: $('#config-fanlessMode').is(\":checked\") \
}; \
 \
while (!finished) { \
try { \
res = await $.ajax({ \
url: baseurl + '/saveconfig', \
type: 'POST', \
data: JSON.stringify(config), \
dataType: 'JSON', \
error: function (e) { \
if (e.statusText == \"OK\") { \
hideSpin(); \
finished = true; \
alert('config saved'); \
return 0; \
} else if (e.statusText == \"error\") { \
hideSpin(); \
alert('failed'); \
finished = true; \
return 0; \
} \
} \
}); \
finished = true; \
} catch (e) { \
await sleep(1000); \
} \
} \
hideSpin(); \
} \
")
