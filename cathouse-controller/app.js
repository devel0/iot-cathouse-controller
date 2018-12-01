//==============================================================================
//
//-------------------- PLEASE REVIEW FOLLOW VARIABLES ------------------
//

//
// add a description to your sensors here
//
var sensorDesc = [{
        id: "28b03724070000c8",
        description: "bottom"
    },
    {
        id: "28f00a3b05000038",
        description: "ambient"
    },
    {
        id: "28e2cc23070000d8",
        description: "wood"
    },
    {
        id: "28d12b5b0500001c",
        description: "extern"
    }
];

// automatic replaced to debug = false during compilation
// this is used to debug page index.htm locally
var debug = true;

//==============================================================================

requirejs.config({
    "moment": "://cdnjs.cloudflare.com/ajax/libs/require.js/2.3.6/require.min.js"
});

// updated from /info api
var history_interval_sec = 10;

var baseurl = '';
if (debug) baseurl = 'http://10.10.3.9';

async function reloadTemp(addr) {
    $('.j-spin').removeClass('collapse');
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
    $('.j-spin').addClass('collapse');
    $('#t' + addr)[0].innerText = res;
}

async function reloadInfo() {
    $('.j-spin').removeClass('collapse');
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
    $('.j-spin').addClass('collapse');
    $('#info')[0].innerHTML = JSON.stringify(res, null, 2);

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
    console.log('moment now = ' + dtnow.format());

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

    $('.j-spin').removeClass('collapse');
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

    history_interval_sec = resnfo.history_interval_sec;
    $('.j-spin').addClass('collapse');

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