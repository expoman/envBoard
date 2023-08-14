#include "webserver.h"

String temperature;
String humidity;
String pressure;

const char* PARAM_INPUT_1 = "deviceName";
const char* PARAM_INPUT_2 = "logMethod";
const char* PARAM_INPUT_3 = "logLoraInterval";
const char* PARAM_INPUT_4 = "resetESP32";

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
  else if(var == "LOGLORAINTERVAL"){
    return String(logLoraInterval);
  }
  else if(var == "LOGMETHOD"){
    if(logMethod == 0)
      return String("None");
    else if(logMethod == 1)
      return String("LoRa");
    else if(logMethod == 2)
      return String("TTN");
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
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    if (request->hasParam(PARAM_INPUT_1)) {
      Serial.println("found deviceName parameter in request");
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
      deviceName = inputMessage;
      prefs.begin("envSensor", false);
      prefs.putString("devName", deviceName);
      prefs.end();
    }
    else if (request->hasParam(PARAM_INPUT_2)) {
      Serial.println("found logging parameter in request");
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      uint8_t inputNumber;
      if(inputMessage == "LoRa")
        inputNumber = 1;
      else if(inputMessage == "TTN")
        inputNumber = 2;
      else
        inputNumber = 0;
      inputParam = PARAM_INPUT_2;
      logMethod = inputNumber;
      prefs.begin("envSensor", false);
      prefs.putUInt("logMethod", inputNumber);
      prefs.end();
    }
    else if (request->hasParam(PARAM_INPUT_3)) {
      Serial.println("found lora logging interval parameter in request");
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
      logLoraInterval = inputMessage.toInt();
      prefs.begin("envSensor", false);
      prefs.putUInt("logLoraInterval", logLoraInterval);
      prefs.end();
    }
    else if (request->hasParam(PARAM_INPUT_4)){
      Serial.println("found esp32 reset request");
      inputMessage = request->getParam(PARAM_INPUT_4)->value();
      inputParam = PARAM_INPUT_4;
      if(inputParam = "true")
         ESP.restart();
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.print("received message: ");
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field ("
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
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
  <p>
    <i class="fas fa-angle-double-down" style="color:#e8c14d;"></i> LogMethod: <span id="logMethod" class="readings">%LOGMETHOD%</span>
  </p>
<hr />
<h3>Configuration</h3>
<br>
<p>
  <form action="/get">
    device name: <input type="text" name="deviceName" value="%DEVICENAME%">
    <input type="submit" value="Submit">
  </form>
</p>
<p>
<form action="/get">
  <fieldset>
    <legend>Please select your logging method:</legend>
    <div>
      <input type="radio" id="logMethodNone" name="logMethod" value="None" />
      <label for="logMethodNone">None</label>

      <input type="radio" id="logMethodLora" name="logMethod" value="LoRa" />
      <label for="logMethodLora">LoRa</label>

      <input type="radio" id="logMethodTTN" name="logMethod" value="TTN" />
      <label for="logMethodTTN">TTN</label>
    </div>
    <div>
      <button type="submit">Submit</button>
    </div>
  </fieldset>
</form>
</p>
<p>
  <form action="/get">
    lora log interval [s]: <input type="text" name="logLoraInterval" value="%LOGLORAINTERVAL%">
    <input type="submit" value="Submit">
  </form>
</p>
<p>
  <form action="/get">
    <input type="hidden" name="resetESP32" value="true">
    <input type="submit" value="Reset ESP32">
  </form>
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
