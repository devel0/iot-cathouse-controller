//==============================================================================
//
//-------------------- PLEASE REVIEW FOLLOW VARIABLES ------------------
//

// automatic replaced to debug = false during compilation
// this is used to debug page index.htm locally
var debug = true;

//==============================================================================

var sensorDesc = [];

requirejs.config({
    "moment": "://cdnjs.cloudflare.com/ajax/libs/require.js/2.3.6/require.min.js"
});

function showSpin() {
    $('.j-spin').removeClass("collapse");
}

function hideSpin() {
    $('.j-spin').addClass("collapse");
}

function showHome() {
    $('.j-containers').addClass('collapse');
    $('.j-home').removeClass('collapse');
}

function showConfig() {
    $('.j-containers').addClass('collapse');
    $('.j-config').removeClass('collapse');
}

// updated from /info api
var history_interval_sec = 10;

var baseurl = '';
if (debug) baseurl = 'http://10.10.3.9';
//if (debug) baseurl = 'http://10.10.3.11';

async function reloadTemp(addr) {
    showSpin();
    let finished = false;
    let res = null;
    while (!finished) {
        try {
            res = await $.ajax({
                url: baseurl + '/temp/' + addr,
                type: 'GET'
            });
            finished = true;
        } catch (e) {
            sleep(1000);
        }
    }
    hideSpin();
    $('#t' + addr)[0].innerText = res;
}

async function reloadInfo() {
    showSpin();
    let finished = false;
    let res = null;
    while (!finished) {
        try {
            res = await $.ajax({
                url: baseurl + '/info',
                type: 'GET'
            });
            finished = true;
        } catch (e) {
            sleep(1000);
        }
    }
    hideSpin();
    //$('#info')[0].innerHTML = JSON.stringify(res, null, 2);

    if (res["p1"] == true)
        $('.port-p1').addClass('port-on');
    else
        $('.port-p1').removeClass('port-on');

    if (res["p2"] == true)
        $('.port-p2').addClass('port-on');
    else
        $('.port-p2').removeClass('port-on');

    if (res["p3"] == true)
        $('.port-p3').addClass('port-on');
    else
        $('.port-p3').removeClass('port-on');

    if (res["p4"] == true)
        $('.port-p4').addClass('port-on');
    else
        $('.port-p4').removeClass('port-on');

    if (res["led"] == true)
        $('.port-led').addClass('port-on');
    else
        $('.port-led').removeClass('port-on');

    if (res["fan"] == true)
        $('.port-fan').addClass('port-on');
    else
        $('.port-fan').removeClass('port-on');

    let Wh = res.Wh;
    let runtime_hr = res.runtime_hr;

    $('.mean-power')[0].innerText = (Wh / runtime_hr).toFixed(0);
}

async function reloadConfig() {
    showSpin();
    let finished = false;
    let res = null;
    while (!finished) {
        try {
            res = await $.ajax({
                url: baseurl + '/getconfig',
                type: 'GET'
            });
            finished = true;
        } catch (e) {
            sleep(1000);
            finished=true;
        }
    }
    hideSpin();

    sensorDesc = [{
            id: res["tbottomId"],
            description: "bottom"
        },
        {
            id: res["twoodId"],
            description: "wood"
        },
        {
            id: res["tambientId"],
            description: "ambient"
        },
        {
            id: res["texternId"],
            description: "extern"
        }
    ];

    $('#config-firmwareVersion')[0].innerText = res["firmwareVersion"];
    $('#config-wifiSSID')[0].innerText = res["wifiSSID"];
    $('#config-tbottomId')[0].value = res["tbottomId"];
    $('#config-twoodId')[0].value = res["twoodId"];
    $('#config-tambientId')[0].value = res["tambientId"];
    $('#config-texternId')[0].value = res["texternId"];
    $('#config-temperatureHistoryFreeramThreshold-kb')[0].value = res["temperatureHistoryFreeramThreshold"] / 1024.0;
    $('#config-temperatureHistoryBacklogHours')[0].value = res["temperatureHistoryBacklogHours"];
    $('#config-updateConsumptionIntervalMs-sec')[0].value = res["updateConsumptionIntervalMs"] / 1000.0;
    $('#config-updateFreeramIntervalMs-sec')[0].value = res["updateFreeramIntervalMs"] / 1000.0;
    $('#config-updateTemperatureIntervalMs-sec')[0].value = res["updateTemperatureIntervalMs"] / 1000.0;
    $('#config-tbottomLimit')[0].value = res["tbottomLimit"];
    $('#config-twoodLimit')[0].value = res["twoodLimit"];
    $('#config-tambientLimit')[0].value = res["tambientLimit"];
    $('#config-cooldownTimeMs-min')[0].value = res["cooldownTimeMs"] / 1000.0 / 60.0;
    $('#config-tambientVsExternGTESysOff')[0].value = res["tambientVsExternGTESysOff"];
    $('#config-tambientVsExternLTESysOn')[0].value = res["tambientVsExternLTESysOn"];
    $('#config-tbottomGTEFanOn')[0].value = res["tbottomGTEFanOn"];
    $('#config-tbottomLTEFanOff')[0].value = res["tbottomLTEFanOff"];
    $('#config-autoactivateWoodBottomDeltaGTESysOn')[0].value = res["autoactivateWoodBottomDeltaGTESysOn"];
    $('#config-autodeactivateWoodDeltaLT')[0].value = res["autodeactivateWoodDeltaLT"];
    $('#config-autodeactivateInhibitAutoactivateMinMs-min')[0].value = res["autodeactivateInhibitAutoactivateMinMs"] / 1000.0 / 60.0;
    $('#config-autodeactivateExcursionSampleCount')[0].value = res["autodeactivateExcursionSampleCount"];
    $('#config-autodeactivateExcursionSampleTotalMs-min')[0].value = res["autodeactivateExcursionSampleTotalMs"] / 1000.0 / 60.0;
    $('#config-texternGTESysOff')[0].value = res["texternGTESysOff"];
}

async function saveConfig() {
    showSpin();
    let finished = false;
    let res = null;

    let config = {
        tbottomId: $('#config-tbottomId')[0].value,
        twoodId: $('#config-twoodId')[0].value,
        tambientId: $('#config-tambientId')[0].value,
        texternId: $('#config-texternId')[0].value,
        temperatureHistoryFreeramThreshold: parseFloat($('#config-temperatureHistoryFreeramThreshold-kb')[0].value) * 1024,
        temperatureHistoryBacklogHours: parseInt($('#config-temperatureHistoryBacklogHours')[0].value),
        updateConsumptionIntervalMs: parseFloat($('#config-updateConsumptionIntervalMs-sec')[0].value) * 1000,
        updateFreeramIntervalMs: parseFloat($('#config-updateFreeramIntervalMs-sec')[0].value) * 1000,
        updateTemperatureIntervalMs: parseFloat($('#config-updateTemperatureIntervalMs-sec')[0].value) * 1000,
        tbottomLimit: parseFloat($('#config-tbottomLimit')[0].value),
        twoodLimit: parseFloat($('#config-twoodLimit')[0].value),
        tambientLimitxx: parseFloat($('#config-tambientLimit')[0].value),
        cooldownTimeMs: parseFloat($('#config-cooldownTimeMs-min')[0].value) * 1000 * 60,
        tambientVsExternGTESysOff: parseFloat($('#config-tambientVsExternGTESysOff')[0].value),
        tambientVsExternLTESysOn: parseFloat($('#config-tambientVsExternLTESysOn')[0].value),
        tbottomGTEFanOn: parseFloat($('#config-tbottomGTEFanOn')[0].value),
        tbottomLTEFanOff: parseFloat($('#config-tbottomLTEFanOff')[0].value),
        autoactivateWoodBottomDeltaGTESysOn: parseFloat($('#config-autoactivateWoodBottomDeltaGTESysOn')[0].value),
        autodeactivateWoodDeltaLT: parseFloat($('#config-autodeactivateWoodDeltaLT')[0].value),
        autodeactivateInhibitAutoactivateMinMs: parseFloat($('#config-autodeactivateInhibitAutoactivateMinMs-min')[0].value) * 1000 * 60,
        autodeactivateExcursionSampleCount: parseInt($('#config-autodeactivateExcursionSampleCount')[0].value),
        autodeactivateExcursionSampleTotalMs: parseFloat($('#config-autodeactivateExcursionSampleTotalMs-min')[0].value) * 1000 * 60,
        texternGTESysOff: parseFloat($('#config-texternGTESysOff')[0].value)
    };

    while (!finished) {
        try {
            res = await $.ajax({
                url: baseurl + '/saveconfig',
                type: 'POST',
                data: JSON.stringify(config),
                dataType: 'JSON',
                error: function (e) {
                    if (e.statusText == "OK") {
                        hideSpin();
                        finished = true;
                        alert('config saved');
                        return 0;
                    } else if (e.statusText == "error") {
                        hideSpin();
                        alert('failed');
                        finished = true;
                        return 0;
                    }
                }
            });
            finished = true;
        } catch (e) {
            sleep(1000);
        }
    }
    hideSpin();
}

var reload_enabled = false;
setInterval(autoreload, 10000);

function autoreload() {
    if (!reload_enabled) return;
    reloadall();
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function reloadall() {
    $('.tempdev').each(async function (idx) {
        let v = this.innerText;
        await reloadTemp(v);
    });
    await reloadInfo();

    let finished = false;

    let res = null;
    while (!finished) {
        try {
            res = await $.ajax({
                url: baseurl + "/temphistory",
                type: 'GET'
            });
            finished = true;
        } catch (e) {
            sleep(1000);
        }
    }

    var colors = ['orange', 'yellow', 'green', 'blue', 'violet', 'black', 'red'];
    var ctx = document.getElementById("myChart").getContext('2d');

    var dtnow = moment();

    var i = 0;
    var dss = [];
    $.each(res, function (idx, data) {
        id = Object.keys(data)[0];
        desc = id;
        q = $.grep(sensorDesc, (el, idx) => el.id == id);
        if (q.length > 0) desc = q[0].description;

        if (i > colors.length - 1) color = 'brown';
        else color = colors[i];

        valcnt = data[id].length;

        let trend = '<i class="fas fa-equals"></i>';
        if (valcnt > 1) {
            var last = data[id][valcnt - 1];
            var lastbut1 = data[id][valcnt - 2];
            if (last > lastbut1)
                trend = '<i style="color:red" class="fas fa-arrow-up"></i>';
            else if (last < lastbut1)
                trend = '<i style="color:blue" class="fas fa-arrow-down"></i>';
        }
        $('#trend' + id)[0].innerHTML = trend;


        dts = [];
        $.each(data[id], function (idx, val) {
            secbefore = (valcnt - idx - 1) * history_interval_sec;
            tt = moment(dtnow).subtract(secbefore, 'seconds');
            dts.push({
                t: tt,
                y: val
            });
        });

        dss.push({
            borderColor: color,
            label: desc,
            data: dts,
            pointRadius: 0
        });

        ++i;
    });

    var myChart = new Chart(ctx, {
        type: 'line',
        data: {
            datasets: dss
        },
        options: {
            scales: {
                xAxes: [{
                    type: 'time',
                    time: {
                        displayFormats: {
                            'hour': 'HH:mm'
                        }
                    },
                    position: 'bottom'
                }]
            }
        }
    });
}

async function myfn() {
    // retrieve temperature devices and populate table

    hideSpin();
    let res = null;
    let finished = false;
    while (!finished) {
        try {
            res = await $.ajax({
                url: baseurl + '/tempdevices',
                type: 'GET'
            });
            finished = true;
        } catch (e) {
            sleep(1000);
        }
    }

    finished = false;
    let resnfo = null;
    while (!finished) {
        try {
            resnfo = await $.ajax({
                url: baseurl + '/info',
                type: 'GET'
            });
            finished = true;
        } catch (e) {
            sleep(1000);
        }
    }

    await reloadConfig();

    history_interval_sec = resnfo.history_interval_sec;
    hideSpin();

    var h = "";

    for (i = 0; i < res.tempdevices.length; ++i) {
        let tempId = res.tempdevices[i];

        h += "<tr>";

        // address
        h += "<td><span class='tempdev'>";
        h += tempId;
        h += "</span></td>";

        // description
        h += "<td>";
        q = $.grep(sensorDesc, (el, idx) => el.id == tempId);
        if (q.length > 0) h += q[0].description;
        h += "</td>";

        // value
        h += "<td><span id='t" + tempId + "'>";
        h += "</span></td>";

        // trend
        h += "<td><span id='trend" + tempId + "'>";
        h += "</span></td>";

        // action
        //h += "<td><button class='btn btn-primary' onclick='reloadTemp(\"" + res.tempdevices[i] + "\")'>reload</button></td>";

        h += "</tr>";
    }

    $('#tbody-temp')[0].innerHTML = h;

    await reloadall();
}

myfn();