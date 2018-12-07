/* GENERATED FILE BY gen-h UTIL - DO NOT EDIT THIS */
F("<html> \
 \
<head> \
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"> \
<style> \
.port-on { \
background-color: yellow; \
} \
.cfg-lbl { \
color: gray; \
font-style: italic; \
text-align: right; \
} \
</style> \
</head> \
 \
<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/4.1.3/css/bootstrap.min.css\" \
integrity=\"sha256-eSi1q2PG6J7g7ib17yAaWMcrr5GrtohYChqibrV7PBE=\" crossorigin=\"anonymous\" /> \
<link rel=\"stylesheet\" href=\"https://use.fontawesome.com/releases/v5.5.0/css/all.css\" integrity=\"sha384-B4dIYHKNBt8Bc12p+WXckhzcICo0wtJAoU8YZTY5qE0Id1GSseTk6S+L3BlXeVIU\" \
crossorigin=\"anonymous\"> \
 \
<body> \
<div class=\"container\"> \
<div class=\"row\"> \
<div class=\"col-auto\"> \
<div class=\"btn-group\" role=\"group\"> \
<button class=\"btn btn-link\" onclick='showHome()'>Home</button> \
<button class=\"btn btn-link\" onclick='showConfig()'>Config</button> \
<div class=\"col\"><i class=\"fas fa-spin fa-spinner j-spin collapse\"></i></div> \
</div> \
</div> \
</div> \
</div> \
 \
<!-- HOME --> \
<div class=\"container j-containers j-home\"> \
<div class=\"row mt-3\"> \
<div class=\"col-auto\"> \
<h1>Temperature sensors</h1> \
</div> \
</div> \
<div class=\"row\"> \
<div class=\"col\"> \
<h2>Chart</h2> \
<canvas id=\"myChart\" height=\"80\"></canvas> \
</div> \
<div class=\"col-auto\"> \
<div class=\"row\"> \
<div class=\"col-auto\"> \
<h2>Ports</h2> \
<div class=\"port-p1\">P1</div> \
<div class=\"port-p2\">P2</div> \
<div class=\"port-p3\">P3</div> \
<div class=\"port-p4\">P4</div> \
<div class=\"port-led\">LED</div> \
<div class=\"port-fan\">FAN</div> \
</div> \
</div> \
<div class=\"row mt-3\"> \
<div class=\"col-auto\"> \
<span class='h2 mean-power'></span><span class='h2'> W</span> \
</div> \
</div> \
</div> \
</div> \
<div class=\"row\"> \
<div class=\"col\"> \
<h2>Tabular</h2> \
<div class=\"table-container\"> \
<div class=\"table table-striped\"> \
<table class=\"table\"> \
<thead> \
<tr> \
<th scope=\"col\"><b>Address</b></th> \
<th scope=\"col\"><b>Description</b></th> \
<th scope=\"col\"><b>Value (C)</b></th> \
<th scope=\"col\"><b>Trend</b></th> \
</tr> \
</thead> \
<tbody id=\"tbody-temp\"></tbody> \
</table> \
</div> \
</div> \
</div> \
</div> \
<div class=\"row\"> \
<div class=\"col-auto\"> \
<button class=\"btn btn-primary\" onclick='reloadall()'>reload all</button> \
</div> \
<div class=\"col\"> \
<button class=\"btn btn-default\" onclick='reload_enabled=true;'>autoreload</button> \
</div> \
</div> \
<div class=\"row mt-3\"> \
<div class=\"col\"> \
<h3>Misc</h3> \
<ul> \
<li> \
<a href=\"https://github.com/devel0/iot-cathouse-controller\">API</a> \
</li> \
</ul> \
</div> \
</div> \
</div> \
 \
<!-- CONFIG --> \
<div class=\"container j-containers j-config collapse\"> \
<div class=\"row mt-3\"> \
<div class=\"col-auto\"> \
<h1>Config</h1> \
</div> \
</div> \
<div class=\"row\"> \
<div class=\"col-auto\"> \
<table class=\"table table-striped\"> \
<thead> \
<tr> \
<th style='text-align:right'><b>Var</b></th> \
<th><b>Value</b></th> \
<th><b>Unit</b></th> \
</tr> \
</thead> \
<tbody id=\"config-tbl\"> \
<tr> \
<td class='cfg-lbl'>firmware ver</td> \
<td><span id='config-firmwareVersion'></span></td> \
<td></td> \
</tr> \
<tr> \
<td class='cfg-lbl'>WiFi SSID</td> \
<td><span id='config-wifiSSID'></span></td> \
<td></td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Temperature History freeram threshold</td> \
<td><input type='number' step='1' class='form-control' id='config-temperatureHistoryFreeramThreshold'></input></td> \
<td class='align-middle'>bytes</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Temperature History backlog size</td> \
<td><input type='number' step='1' class='form-control' id='config-temperatureHistoryBacklogHours'></input></td> \
<td class='align-middle'>hours</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Consumption update interval</td> \
<td><input type='number' step='1' class='form-control' id='config-updateConsumptionIntervalMs'></input></td> \
<td class='align-middle'>ms</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Freeram update interval</td> \
<td><input type='number' step='1' class='form-control' id='config-updateFreeramIntervalMs'></input></td> \
<td class='align-middle'>ms</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Temperature update interval</td> \
<td><input type='number' step='1' class='form-control' id='config-updateTemperatureIntervalMs'></input></td> \
<td class='align-middle'>ms</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Bottom temperature limit</td> \
<td><input type='number' step='0.1' class='form-control' id='config-tbottomLimit'></input></td> \
<td class='align-middle'>C</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Wood temperature limit</td> \
<td><input type='number' step='0.1' class='form-control' id='config-twoodLimit'></input></td> \
<td class='align-middle'>C</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Ambient temperature limit</td> \
<td><input type='number' step='0.1' class='form-control' id='config-tambientLimit'></input></td> \
<td class='align-middle'>C</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Cooldown time</td> \
<td><input type='number' step='1' class='form-control' id='config-cooldownTimeMs'></input></td> \
<td class='align-middle'>ms</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Ambient >= Extern sys OFF</td> \
<td><input type='number' step='0.1' class='form-control' id='config-tambientVsExternGTESysOff'></input></td> \
<td class='align-middle'>C</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Ambient &lt;= Extern sys ON</td> \
<td><input type='number' step='0.1' class='form-control' id='config-tambientVsExternLTESysOn'></input></td> \
<td class='align-middle'>C</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Bottom >= T fan ON</td> \
<td><input type='number' step='0.1' class='form-control' id='config-tbottomGTEFanOn'></input></td> \
<td class='align-middle'>C</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Bottom &lt;= T fan ON</td> \
<td><input type='number' step='0.1' class='form-control' id='config-tbottomLTEFanOff'></input></td> \
<td class='align-middle'>C</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>(Wood - Bottom) >= T sys ON</td> \
<td><input type='number' step='0.1' class='form-control' id='config-autoactivateWoodBottomDeltaGTESysOn'></input></td> \
<td class='align-middle'>C</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Wood excursion &lt;= T sys OFF</td> \
<td><input type='number' step='0.1' class='form-control' id='config-autodeactivateWoodDeltaLT'></input></td> \
<td class='align-middle'>C</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Inhibit activate after deactivation min time</td> \
<td><input type='number' step='1' class='form-control' id='config-autodeactivateInhibitAutoactivateMinMs'></input></td> \
<td class='align-middle'>ms</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Wood excursion sample slots count</td> \
<td><input type='number' step='1' class='form-control' id='config-autodeactivateExcursionSampleCount'></input></td> \
<td></td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Wood excursion analysis total time interval</td> \
<td><input type='number' step='1' class='form-control' id='config-autodeactivateExcursionSampleTotalMs'></input></td> \
<td class='align-middle'>ms</td> \
</tr> \
<tr> \
<td class='cfg-lbl align-middle'>Extern >= T sys OFF</td> \
<td><input type='number' step='0.1' class='form-control' id='config-texternGTESysOff'></input></td> \
<td class='align-middle'>C</td> \
</tr> \
</tbody> \
</table> \
</div> \
</div> \
<div class=\"row mt-3\"> \
<div class=\"col-auto\"> \
<button class=\"btn btn-primary\" onclick='saveConfig()'>Save config</button> \
</div> \
</div> \
</div> \
 \
<script src=\"https://cdnjs.cloudflare.com/ajax/libs/jquery/3.3.1/jquery.min.js\" integrity=\"sha256-FgpCb/KJQlLNfOu91ta32o/NMZxltwRo8QtmkMRdAu8=\" \
crossorigin=\"anonymous\"></script> \
<script src=\"https://cdnjs.cloudflare.com/ajax/libs/twitter-bootstrap/4.1.3/js/bootstrap.min.js\" integrity=\"sha256-VsEqElsCHSGmnmHXGQzvoWjWwoznFSZc6hs7ARLRacQ=\" \
crossorigin=\"anonymous\"></script> \
<script src=\"https://cdnjs.cloudflare.com/ajax/libs/moment.js/2.22.2/moment.min.js\" integrity=\"sha256-CutOzxCRucUsn6C6TcEYsauvvYilEniTXldPa6/wu0k=\" \
crossorigin=\"anonymous\"></script> \
<script src=\"https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.7.3/Chart.min.js\" integrity=\"sha256-oSgtFCCmHWRPQ/JmR4OoZ3Xke1Pw4v50uh6pLcu+fIc=\" \
crossorigin=\"anonymous\"></script> \
<script src=\"https://cdnjs.cloudflare.com/ajax/libs/require.js/2.3.6/require.min.js\" integrity=\"sha256-1fEPhSsRKlFKGfK3eO710tEweHh1fwokU5wFGDHO+vg=\" \
crossorigin=\"anonymous\"></script> \
<script src=\"app.js\"></script> \
</body> \
 \
</html>")
