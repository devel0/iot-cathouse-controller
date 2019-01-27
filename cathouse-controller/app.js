//==============================================================================
//
//-------------------- PLEASE REVIEW FOLLOW VARIABLES ------------------
//

// automatic replaced to debug = false during compilation
// this is used to debug page index.htm locally
var debug = true;

var infoUpdateIntervalMs = 5000;
var tempUpdateIntervalMs = 10000;

//==============================================================================

var LED_PORT = 5;
var sensorDesc = [];

var decimalSep = '.';
var adcWeightLastSampleCnt = 0;

requirejs.config({
    "moment": "://cdnjs.cloudflare.com/ajax/libs/require.js/2.3.6/require.min.js"
});

function showSpin() {
    $('.j-spin').removeClass("collapse");
}

function hideSpin() {
    $('.j-spin').addClass("collapse");
}

function showSpinInfo() {
    //    $('.j-spin-info').removeClass("collapse");
}

function hideSpinInfo() {
    //    $('.j-spin-info').addClass("collapse");
}

function showSpinChart() {
    $('.j-spin-chart').removeClass("collapse");
}

function hideSpinChart() {
    $('.j-spin-chart').addClass("collapse");
}

function showSpinTemp() {
    //    $('.j-spin-temp').removeClass("collapse");
}

function hideSpinTemp() {
    //    $('.j-spin-temp').addClass("collapse");
}

function showHome() {
    $('.j-containers').addClass('collapse');
    $('.j-home').removeClass('collapse');
    $('.j-menu').removeClass('fstrong');
    $('.menu-home').addClass('fstrong');
}

function showConfig() {
    $('.j-containers').addClass('collapse');
    $('.j-config').removeClass('collapse');
    $('.j-menu').removeClass('fstrong');
    $('.menu-config').addClass('fstrong');
}

// updated from /info api
var history_interval_sec = 10;
var manualMode = false;

var baseurl = '';
if (debug) baseurl = 'http://10.10.3.9';
//if (debug) baseurl = 'http://10.10.3.11';

async function reloadTemp(addr) {
    showSpinTemp();
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
            await sleep(1000);
        }
    }
    hideSpinTemp();
    $('#t' + addr)[0].innerText = res;
}

// https://github.com/devel0/js-util/blob/986f314207ebd1b27db3a937a552b30f1b1392fa/src/js-util.js
function human_readable_filesize(bytes, onlyBytesUnit = true, bytesMultiple = 1, decimals = 1) {
    let k = 1024.0;
    let m = k * 1024.0;
    let g = m * 1024.0;
    let t = g * 1024.0;

    if (bytesMultiple != 1) bytes = Math.trunc(mround(bytes, bytesMultiple));

    if (bytes < k) {
        if (onlyBytesUnit) return bytes;
        else return bytes + ' b';
    } else if (bytes >= k && bytes < m) return (bytes / k).toFixed(decimals) + ' Kb';
    else if (bytes >= m && bytes < g) return (bytes / m).toFixed(decimals) + ' Mb';
    else if (bytes >= g && bytes < t) return (bytes / g).toFixed(decimals) + ' Gb';
    else return (bytes / t).toFixed(decimals);
}

function toggleCatInThere() {
    if (confirm('sure to force toggling of cathouse in there?') != true)
        return;
    _toggleCatInThere();
}

async function _toggleCatInThere() {
    //console.log("processing toggling");
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
            await sleep(1000);
        }
    }

    let catIsInThere = res.catIsInThere ? false : true;
    finished = false;
    res = null;
    while (!finished) {
        try {
            res = await $.ajax({
                url: baseurl + '/setcatinthere/' + (catIsInThere ? '1' : '0'),
                type: 'GET'
            });
            finished = true;
        } catch (e) {
            await sleep(1000);
        }
    }
    hideSpin();

    //alert('toggled to ' + ((catIsInThere ? '1' : '0')));
}

async function togglePort(p) {
    if (!manualMode && p != LED_PORT) {
        alert("can't toggle port in automatic mode");
    } else {
        showSpin();
        let finished = false;
        let res = null;
        while (!finished) {
            try {
                res = await $.ajax({
                    url: baseurl + '/port/toggle/' + p,
                    type: 'GET'
                });
                finished = true;
            } catch (e) {
                await sleep(1000);
            }
        }
        hideSpin();

        await reloadInfo();
    }
}

async function reloadInfo() {
    //console.log("--> reloadInfo");
    showSpinInfo();
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
            await sleep(1000);
        }
    }
    hideSpinInfo();
    //$('#info')[0].innerHTML = JSON.stringify(res, null, 2);

    manualMode = res["manualMode"];

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
    $('.runtime-hr')[0].innerText = runtime_hr.toFixed(1);
    $('.free-ram')[0].innerText = human_readable_filesize(res.freeram);
    $('.free-ram-min')[0].innerText = human_readable_filesize(res.freeram_min);
    $('.temp-history-interval-min')[0].innerText = res.temp_history_interval_min.toFixed(1);
    $('.temp-read-failure')[0].innerText = res.temp_read_failure;
    if (res.temp_read_failure > 0)
        $('.temp-read-failure').addClass('f-danger');
    $('.cycle-prev-current')[0].innerText = res.prev_cycle + "/" + res.current_cycle;
    $('.cat-is-in-there')[0].innerText = res.catIsInThere ? 'yes' : 'no';
    if (res.catIsInThere)
        $('.cat-is-in-there').addClass('port-on');
    else
        $('.cat-is-in-there').removeClass('port-on');
    adcWeightLastSampleCnt = res.adcWeightLastSampleCnt;

    // adc weight array chart
    // cat in there chart
    {
        var ctx = document.getElementById("weightChart").getContext('2d');

        var dtnow = moment();
        let ary = res["adcWeightArray"];
        let interval_sec = res["statIntervalSec"];

        var i = 0;
        var dss = []; {
            dts = [];
            let valcnt = ary.length;
            $.each(ary, function (idx, val) {
                secbefore = (valcnt - idx - 1) * interval_sec;
                tt = moment(dtnow).subtract(secbefore, 'seconds');
                dts.push({
                    t: tt,
                    y: val
                });
            });

            dtsmeanLastBut = [];
            dtsmeanLast = [];

            if (adcWeightLastSampleCnt > 0 && valcnt > adcWeightLastSampleCnt * 2) {
                let meanwlatestbut = 0.0; {
                    let i = valcnt - adcWeightLastSampleCnt - 1;
                    let c = 0;
                    while (i >= 0 && c < adcWeightLastSampleCnt) {
                        meanwlatestbut += ary[i--];
                        ++c;
                    }
                    meanwlatestbut /= adcWeightLastSampleCnt;
                }
                let meanwlatest = 0.0; {
                    let i = valcnt - 1;
                    let c = 0;
                    while (i >= 0 && c < adcWeightLastSampleCnt) {
                        meanwlatest += ary[i--];
                        ++c;
                    }
                    meanwlatest /= adcWeightLastSampleCnt;
                }
                $('.adc-weight-latest-but')[0].innerText = meanwlatestbut.toFixed(0);
                $('.adc-weight-latest')[0].innerText = meanwlatest.toFixed(0) + ' ( ' +
                    ((meanwlatest > meanwlatestbut) ? '+' : '') + (meanwlatest - meanwlatestbut).toFixed(0) + ' )';

                dtsmeanLastBut.push({
                    t: moment(dtnow).subtract(adcWeightLastSampleCnt * 2 * interval_sec, 'seconds'),
                    y: meanwlatestbut
                });
                dtsmeanLastBut.push({
                    t: moment(dtnow).subtract(adcWeightLastSampleCnt * interval_sec, 'seconds'),
                    y: meanwlatestbut
                });

                dtsmeanLast.push({
                    t: moment(dtnow).subtract(adcWeightLastSampleCnt * interval_sec, 'seconds'),
                    y: meanwlatest
                });
                dtsmeanLast.push({
                    t: moment(dtnow),
                    y: meanwlatest
                });
            }

            dss.push({
                borderColor: 'blue',
                fill: false,
                label: 'meanLastBut',
                data: dtsmeanLastBut,
                pointRadius: 0
            }, {
                borderColor: 'red',
                fill: false,
                label: 'meanLast',
                data: dtsmeanLast,
                pointRadius: 0
            }, {
                borderColor: '#00aa00',
                fill: true,
                label: 'adc Weight',
                data: dts,
                pointRadius: 0
            });

            ++i;
        }

        var myChart = new Chart(ctx, {
            type: 'line',
            data: {
                datasets: dss
            },
            options: {
                maintainAspectRatio: false,
                animation: false,
                scales: {
                    xAxes: [{
                        type: 'time'
                    }]
                }
            }
        });
    }
}

async function reloadAllTemp() {
    //console.log("--> reloadAllTemp");
    $('.tempdev').each(async function (idx) {
        let v = this.innerText;
        await reloadTemp(v);
    });
}

async function getBitHistoriesDataSource() {
    var dtnow = moment();

    showSpinChart();
    let finished = false;
    var res = null;
    while (!finished) {
        try {
            res = await $.ajax({
                url: baseurl + "/bithistories",
                type: 'GET'
            });
            finished = true;
        } catch (e) {
            await sleep(1000);
        }
    }
    hideSpinChart();

    var dss = []; {
        let rr = res.catInThereHistory;
        var i = 0; {
            dts = [];
            let valcnt = rr.length;
            $.each(rr, function (idx, val) {
                secbefore = (valcnt - idx - 1) * history_interval_sec;
                tt = moment(dtnow).subtract(secbefore, 'seconds');
                dts.push({
                    t: tt,
                    y: val
                });
            });

            dss.push({
                borderColor: '#3b9004',
                backgroundColor: '#3b9004',
                fill: true,
                label: 'cat in there',
                data: dts,
                pointRadius: 0
            });

            ++i;
        };
    }

    {
        let rr = res.p1History;
        var i = 0; {
            dts = [];
            let valcnt = rr.length;
            $.each(rr, function (idx, val) {
                secbefore = (valcnt - idx - 1) * history_interval_sec;
                tt = moment(dtnow).subtract(secbefore, 'seconds');
                dts.push({
                    t: tt,
                    y: val
                });
            });

            dss.push({
                borderColor: '#b70013',
                backgroundColor: '#b70013',
                fill: true,
                label: 'P1',
                data: dts,
                pointRadius: 0
            });

            ++i;
        };
    }

    {
        let rr = res.p2History;
        var i = 0; {
            dts = [];
            let valcnt = rr.length;
            $.each(rr, function (idx, val) {
                secbefore = (valcnt - idx - 1) * history_interval_sec;
                tt = moment(dtnow).subtract(secbefore, 'seconds');
                dts.push({
                    t: tt,
                    y: val
                });
            });

            dss.push({
                borderColor: '#df263c',
                backgroundColor: '#df263c',
                fill: true,
                label: 'P2',
                data: dts,
                pointRadius: 0
            });

            ++i;
        };
    }

    {
        let rr = res.p3History;
        var i = 0; {
            dts = [];
            let valcnt = rr.length;
            $.each(rr, function (idx, val) {
                secbefore = (valcnt - idx - 1) * history_interval_sec;
                tt = moment(dtnow).subtract(secbefore, 'seconds');
                dts.push({
                    t: tt,
                    y: val
                });
            });

            dss.push({
                borderColor: '#ff4558',
                backgroundColor: '#ff4558',
                fill: true,
                label: 'P3',
                data: dts,
                pointRadius: 0
            });

            ++i;
        };
    }

    {
        let rr = res.p4History;
        var i = 0; {
            dts = [];
            let valcnt = rr.length;
            $.each(rr, function (idx, val) {
                secbefore = (valcnt - idx - 1) * history_interval_sec;
                tt = moment(dtnow).subtract(secbefore, 'seconds');
                dts.push({
                    t: tt,
                    y: val
                });
            });

            dss.push({
                borderColor: '#ff6a79',
                backgroundColor: '#ff6a79',
                fill: true,
                label: 'P4',
                data: dts,
                pointRadius: 0
            });

            ++i;
        };
    }

    {
        let rr = res.fanHistory;
        var i = 0; {
            dts = [];
            let valcnt = rr.length;
            $.each(rr, function (idx, val) {
                secbefore = (valcnt - idx - 1) * history_interval_sec;
                tt = moment(dtnow).subtract(secbefore, 'seconds');
                dts.push({
                    t: tt,
                    y: val
                });
            });

            dss.push({
                borderColor: '#fff06b',
                backgroundColor: '#fff06b',
                fill: true,
                label: 'fan',
                data: dts,
                pointRadius: 0
            });

            ++i;
        };
    }

    {
        let rr = res.disabledHistory;
        var i = 0; {
            dts = [];
            let valcnt = rr.length;
            $.each(rr, function (idx, val) {
                secbefore = (valcnt - idx - 1) * history_interval_sec;
                tt = moment(dtnow).subtract(secbefore, 'seconds');
                dts.push({
                    t: tt,
                    y: val
                });
            });

            dss.push({
                borderColor: '#989898',
                backgroundColor: '#989898',
                fill: true,
                label: 'disabled',
                data: dts,
                pointRadius: 0
            });

            ++i;
        };
    }

    {
        let rr = res.cooldownHistory;
        var i = 0; {
            dts = [];
            let valcnt = rr.length;
            $.each(rr, function (idx, val) {
                secbefore = (valcnt - idx - 1) * history_interval_sec;
                tt = moment(dtnow).subtract(secbefore, 'seconds');
                dts.push({
                    t: tt,
                    y: val
                });
            });

            dss.push({
                borderColor: '#0abbda',
                backgroundColor: '#0abbda',
                fill: true,
                label: 'cooldown',
                data: dts,
                pointRadius: 0
            });

            ++i;
        };
    }

    return dss;
}

async function getTempHistoryDataSource() {
    var dtnow = moment();

    var dss = [];

    showSpinChart();
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
            await sleep(1000);
        }
    }
    hideSpinChart();

    var colors = ['orange', 'yellow', 'green', 'blue', 'violet', 'black', 'red'];

    var i = 0;

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

    return dss;
}

function getTempRelDataSource(dss) {
    // add ambient-extern dataset     

    let ambientDss = null;
    let externDss = null;
    for (j = 0; j < dss.length; ++j) {
        if (dss[j].label == 'ambient')
            ambientDss = dss[j];
        else if (dss[j].label == 'extern')
            externDss = dss[j];
    }

    var dssRel = [];
    if (ambientDss != null && externDss != null) {
        dts = [];
        for (w = 0; w < ambientDss.data.length; ++w) {
            dts.push({
                t: ambientDss.data[w].t,
                y: ambientDss.data[w].y - externDss.data[w].y
            });
        }
        dssRel.push({
            borderColor: 'red',
            label: 'ambient - extern',
            data: dts,
            pointRadius: 0
        });
    }

    return dssRel;
}

async function reloadCharts() {
    var dtnow = moment();

    // temperature charts
    {
        var ctx = document.getElementById("tempChart").getContext('2d');
        var dss = await getTempHistoryDataSource();

        var myChart = new Chart(ctx, {
            type: 'line',
            data: {
                datasets: dss
            },
            options: {
                maintainAspectRatio: false,
                animation: false,
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

        dssRel = getTempRelDataSource(dss);
        var relCtx = document.getElementById("tempRelChart").getContext('2d');
        var myChart2 = new Chart(relCtx, {
            type: 'line',
            data: {
                datasets: dssRel
            },
            options: {
                maintainAspectRatio: false,
                animation: false,
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

    // bit charts
    {
        var ctx = document.getElementById("bitChart").getContext('2d');

        var dss = await getBitHistoriesDataSource();

        var myChart = new Chart(ctx, {
            type: 'line',
            data: {
                datasets: dss
            },
            options: {
                maintainAspectRatio: false,
                animation: false,
                scales: {
                    xAxes: [{
                        type: 'time',
                        stacked: true
                    }],
                    yAxes: [{
                        stacked: true
                    }]
                }
            }
        });
    }
}

async function exportDataSet(bitdss, filename) {
    var csv = '';
    var sep = (decimalSep == '.') ? ',' : ';';

    if (bitdss.length > 0) {
        var bitdssl = bitdss.length;
        var datadepth = bitdss[0].data.length;
        csv = csv.concat('"time",');
        for (i = 0; i < bitdssl; ++i) {
            csv = csv.concat('"' + bitdss[i].label + '"');
            if (i != bitdssl - 1)
                csv = csv.concat(sep);
            else
                csv = csv.concat(String.fromCharCode(13, 10));
        }
        let j = 0;
        while (j < datadepth) {
            for (i = 0; i < bitdssl; ++i) {
                if (i == 0) {
                    let t = moment(bitdss[i].data[j].t).format("HH:mm");
                    csv = csv.concat('"' + t + '",');
                }
                csv = csv.concat(bitdss[i].data[j].y);

                if (i != bitdssl - 1)
                    csv = csv.concat(sep);
                else
                    csv = csv.concat(String.fromCharCode(13, 10));
            }
            ++j;
        }
    }

    var blob = new Blob([csv], {
        type: "text/plain;charset=utf-8"
    });
    saveAs(blob, filename);
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
            await sleep(1000);
            finished = true;
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
    $('#config-manualMode').prop('checked', res["manualMode"]);
    $('#config-adcWeightDeltaCat')[0].value = res["adcWeightDeltaCat"];
    $('#config-adcWeightDeltaFullpower')[0].value = res["adcWeightDeltaFullpower"];
    $('#config-tbottomLimit')[0].value = res["tbottomLimit"];
    $('#config-twoodLimit')[0].value = res["twoodLimit"];
    $('#config-tambientLimit')[0].value = res["tambientLimit"];
    $('#config-cooldownTimeMs-min')[0].value = res["cooldownTimeMs"] / 1000.0 / 60.0;
    $('#config-texternGTESysOff')[0].value = res["texternGTESysOff"];
    $('#config-tbottomGTEFanOn')[0].value = res["tbottomGTEFanOn"];
}

var infoLastLoad;
var tempLastLoad;
var chartLastLoad;
var autorefreshInProgress = false;

async function autorefresh() {
    if (autorefreshInProgress) return;

    autorefreshInProgress = true;
    var dtnow = new Date();
    if (infoLastLoad === undefined || (dtnow - infoLastLoad) > infoUpdateIntervalMs) {
        await reloadInfo();
        infoLastLoad = new Date();
    }
    if (tempLastLoad === undefined || (dtnow - tempLastLoad) > tempUpdateIntervalMs) {
        await reloadAllTemp();
        tempLastLoad = new Date();
    }
    if (chartLastLoad === undefined || (dtnow - chartLastLoad) > history_interval_sec * 1000) {
        await reloadCharts();
        chartLastLoad = new Date();
    }
    autorefreshInProgress = false;
}

function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

function manageResize() {}

async function myfn() {

    decimalSep = 0.1.toLocaleString().replace(/\d/g, '');

    setInterval(autorefresh, 1000);

    manageResize();
    $(window).resize(function () {
        manageResize();
    });

    autorefreshInProgress = true;
    // retrieve temperature devices and populate table

    showSpin();
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
            await sleep(1000);
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
            await sleep(1000);
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


    $('#bitExport').click(async function () {
        let dss1 = await getBitHistoriesDataSource();
        //      exportDataSet(dss1, 'bithistory.csv');
        dss2 = await getTempHistoryDataSource();
        //        exportDataSet(dss2, 'temphistory.csv');

        let dss = dss1.concat(dss2);
        exportDataSet(dss, 'full.csv');
    });

    autorefreshInProgress = false;
}

$(document).ready(function () {
    myfn();
});

async function saveConfig() {
    showSpin();
    let finished = false;
    let res = null;

    let config = {
        tbottomId: $('#config-tbottomId')[0].value,
        twoodId: $('#config-twoodId')[0].value,
        tambientId: $('#config-tambientId')[0].value,
        texternId: $('#config-texternId')[0].value,
        manualMode: $('#config-manualMode').is(":checked"),
        adcWeightDeltaCat: $('#config-adcWeightDeltaCat')[0].value,
        adcWeightDeltaFullpower: $('#config-adcWeightDeltaFullpower')[0].value,
        tbottomLimit: parseFloat($('#config-tbottomLimit')[0].value),
        twoodLimit: parseFloat($('#config-twoodLimit')[0].value),
        tambientLimit: parseFloat($('#config-tambientLimit')[0].value),
        cooldownTimeMs: parseFloat($('#config-cooldownTimeMs-min')[0].value) * 1000 * 60,
        texternGTESysOff: parseFloat($('#config-texternGTESysOff')[0].value),
        tbottomGTEFanOn: parseFloat($('#config-tbottomGTEFanOn')[0].value)
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
            await sleep(1000);
        }
    }
    hideSpin();
}