#include <WiFiManager.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <SensirionI2CSen5x.h>
#include <SensirionI2CScd4x.h>
#include <Wire.h>
#include <time.h>

// Instantiate the WiFiManager object
WiFiManager wm;

// Buffer to store serial output
String serialBuffer = "";
// Instantiate sensor objects
SensirionI2CSen5x sen5x;
SensirionI2CScd4x scd4x;

// Soft AP (Access Point) credentials
const char *soft_ap_ssid = "IndoorModule2";
const char *soft_ap_password = "testpassword";

// Instantiate the AsyncWebServer on port 80
AsyncWebServer server(80);

// Variables to store sensor readings
float massConcentrationPm1p0;
float massConcentrationPm2p5;
float massConcentrationPm4p0;
float massConcentrationPm10p0;
float ambientHumidity;
float ambientTemperature;
float vocIndex;
float noxIndex;
uint16_t co2;

// Function to print to the serial and buffer the output
void printAndBuffer(String message, bool newline=true) {
  if (newline) {
    Serial.println(message);
    serialBuffer += message + "\n";
  } else {
    Serial.print(message);
    serialBuffer += message;
  }

  // Ensure the buffer doesn't exceed a certain size
  while (serialBuffer.length() > 2000) {
    int nextLineBreak = serialBuffer.indexOf('\n') + 1;
    if (nextLineBreak > 0) {
      serialBuffer = serialBuffer.substring(nextLineBreak);
    } else {
      serialBuffer = serialBuffer.substring(100);  // Default trimming
    }
  }
}

// Function to append character to buffer
void appendToBuffer(char c) {
  serialBuffer += c;
  if (serialBuffer.length() > 2000) {
    serialBuffer = "";  // Clear buffer when size limit is reached
  }
}

// Function to read CO2 from the SCD4x sensor
String readScdCo2() {
  uint16_t c;
  float t;
  float h;

  scd4x.readMeasurement(c, t, h);

  if (c != 0) {
    co2 = c;
  }

  return String(co2);
}

// Functions to read various PM values from the SEN5x sensor
String readPm1p0() {
  sen5x.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
    noxIndex);

  return String(massConcentrationPm1p0);
}

String readPm2p5() {
  sen5x.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
    noxIndex);

  return String(massConcentrationPm2p5);
}

String readPm4p0() {
  sen5x.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
    noxIndex);

  return String(massConcentrationPm4p0);
}

String readPm10p0() {
  sen5x.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
    noxIndex);

  return String(massConcentrationPm10p0);
}

String readAmbientHumidity() {
  sen5x.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
    noxIndex);

  return String(ambientHumidity);
}

String readAmbientTemperature() {
  sen5x.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
    noxIndex);

  return String(ambientTemperature);
}

String readVocIndex() {
  sen5x.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
    noxIndex);

  return String(vocIndex);
}

String readNoxIndex() {
  sen5x.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
    noxIndex);

  return String(noxIndex);
}

// HTML template for the web server
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://kit.fontawesome.com/d91e44f906.js" crossorigin="anonymous"></script>
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 Air Quality Server</h2>
  <p>
    <i class="fas fa-smog" style="color:#000000;"></i> 
    <span class="dht-labels">PM 1.0</span>
    <span id="pm1p0">%PM1P0%</span>
    <sup class="units">ug/m3</sup>
  </p>
  <p>
    <i class="fas fa-smog" style="color:#a0a0a0;"></i>
    <span class="dht-labels">PM 2.5</span>
    <span id="pm2p5">%PM2P5%</span>
    <sup class="units">ug/m3</sup>
  </p>
  <p>
    <i class="fas fa-smog" style="color:#afafaf;"></i>
    <span class="dht-labels">PM 4.0</span>
    <span id="pm4p0">%PM4P0%</span>
    <sup class="units">ug/m3</sup>
  </p>
  <p>
    <i class="fas fa-smog" style="color:#f0f0f0;"></i> 
    <span class="dht-labels">PM 10.0</span>
    <span id="pm10p0">%PM10P0%</span>
    <sup class="units">ug/m3</sup>
  </p>
  <p>
    <i class="fas fa-project-diagram" style="color:#631fc4;"></i> 
    <span class="dht-labels">Carbon Dioxide</span>
    <span id="co2">%CO2%</span>
    <sup class="units">ppm</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Ambient Humidity</span>
    <span id="ambienthumidity">%AMBIENTHUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#f1c40f;"></i>
    <span class="dht-labels">Ambient Temperature</span>
    <span id="ambienttemperature">%AMBIENTTEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-leaf" style="color:#00e024;"></i>
    <span class="dht-labels">VOC Index</span>
    <span id="vocindex">%VOCINDEX%</span>
    <sup class="units"></sup>
  </p>
  <p>
    <i class="fas fa-leaf" style="color:#b3e6b3;"></i>
    <span class="dht-labels">NOx Index</span>
    <span id="noxindex">%NOXINDEX%</span>
    <sup class="units"></sup>
  </p>
  <p>
    <span class="dht-labels">WiFi Manager</span>
    <button onclick="triggerWiFiManager()">Configure WiFi</button>
  </p>
  <script>
    function triggerWiFiManager() {
      window.location.href = "/wifi";
    }
  </script>
</body>
</html>
)rawliteral";

// Function to process the HTML template
String processor(const String& var) {
  if (var == "PM1P0") {
    return readPm1p0();
  } else if (var == "PM2P5") {
    return readPm2p5();
  } else if (var == "PM4P0") {
    return readPm4p0();
  } else if (var == "PM10P0") {
    return readPm10p0();
  } else if (var == "CO2") {
    return readScdCo2();
  } else if (var == "AMBIENTHUMIDITY") {
    return readAmbientHumidity();
  } else if (var == "AMBIENTTEMPERATURE") {
    return readAmbientTemperature();
  } else if (var == "VOCINDEX") {
    return readVocIndex();
  } else if (var == "NOXINDEX") {
    return readNoxIndex();
  }
  return String();
}

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Initialize I2C communication
  Wire.begin();

  // Initialize WiFiManager
  wm.startConfigPortal(soft_ap_ssid, soft_ap_password);  // Change from autoConnect to startConfigPortal

  // Start the sensors
  sen5x.begin(Wire);
  scd4x.begin(Wire);

  // Start the web server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Handle the WiFi configuration route
  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    wm.startConfigPortal(soft_ap_ssid, soft_ap_password);  // Starts the config portal when the /wifi route is accessed
    request->send(200, "text/plain", "WiFi Manager started");
  });

  server.begin();

  // Start the SCD4x sensor
  scd4x.stopPeriodicMeasurement();
  scd4x.startPeriodicMeasurement();
}

void loop() {
  // Put your main code here, to run repeatedly
}
