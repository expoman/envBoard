#include "webserver.h"

String temperature;
String humidity;
String pressure;

AsyncWebServer server(80);

// Replace with your network credentials
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

// Replaces placeholder with sensor values
String processor(const String& var){
  if(var == "DEVICENAME"){
    return deviceName;
  }
  else if(var == "TEMPERATURE"){
    return temperature;
  }
  else if(var == "HUMIDITY"){
    return humidity;
  }
  else if(var == "PRESSURE"){
    return pressure;
  }
  return String();
}

void configWebserver(){
  Serial.println("start webserver");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", temperature.c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", humidity.c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", pressure.c_str());
  });
  // Start server
  server.begin();
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="icon" href="data:,">
  <title>EnvBoardWifi</title>
  <style>
    body {
      margin: 0;
      font-family: Arial, Helvetica, sans-serif;
      text-align: center;
    }
    header {
      margin: 0;
      padding-top: 5vh;
      padding-bottom: 5vh;
      overflow: hidden;
      color: white;
    }
    h2 {
      font-size: 2.0rem;
    }
    p { font-size: 1.2rem; }
    .units { font-size: 1.2rem; }
    .readings { font-size: 2.0rem; }
  </style>
</head>
<body>
  <header>
    <h2>Schluettler EnvBoard v1.0</h2>
    <h3>Device: %DEVICENAME%</h3>
  </header>
<main>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> Temperature: <span id="temperature" class="readings">%TEMPERATURE%</span>
    <sup>&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> Humidity: <span id="humidity" class="readings">%HUMIDITY%</span>
    <sup>&#37;</sup>
  </p>
  <p>
    <i class="fas fa-angle-double-down" style="color:#e8c14d;"></i> Pressure: <span id="pressure" class="readings">%PRESSURE%</span>
    <sup>hpa</sup>
  </p>
</main>
<script>
setInterval(updateValues, 10000, "temperature");
setInterval(updateValues, 10000, "humidity");
setInterval(updateValues, 10000, "pressure");

function updateValues(value) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById(value).innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/" + value, true);
  xhttp.send();
}
</script>
</body>
</html>
)rawliteral";
