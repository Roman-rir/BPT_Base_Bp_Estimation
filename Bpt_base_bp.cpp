// MAX32664 BPT base with web server
#include "max32664.h"
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <Wire.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif

// ===== Wi-Fi Credentials =====
#ifndef WIFI_SSID
#define WIFI_SSID "YOUR_WIFI_SSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;

// ===== MAX32664 Setup =====
#define RESET_PIN D6
#define MFIO_PIN D0
#define RAWDATA_BUFFLEN 250
max32664 MAX32664(RESET_PIN, MFIO_PIN, RAWDATA_BUFFLEN);

// ===== Web Server =====
ESP8266WebServer server(80);

// ===== Vitals =====
int sys = 0, dia = 0, hr = 0, spo2 = 0;

void loadAlgomodeParameters() {
  algomodeInitialiser algoParameters;

  // Calibration values (example values)
  algoParameters.calibValSys[0] = 120;
  algoParameters.calibValSys[1] = 122;
  algoParameters.calibValSys[2] = 125;
  algoParameters.calibValDia[0] = 80;
  algoParameters.calibValDia[1] = 81;
  algoParameters.calibValDia[2] = 82;

  // SpO2 calibration coefficients
  algoParameters.spo2CalibCoefA = 1.5958422;
  algoParameters.spo2CalibCoefB = -34.659664;
  algoParameters.spo2CalibCoefC = 112.68987;

  MAX32664.loadAlgorithmParameters(&algoParameters);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(D2, D1); // SDA = D2, SCL = D1

  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  loadAlgomodeParameters();

  if (MAX32664.hubBegin() != CMD_SUCCESS || !MAX32664.startBPTcalibration() ||
      !MAX32664.configAlgoInEstimationMode()) {
    Serial.println("Sensor initialization failed!");
    while (1)
      ;
  }
  Serial.println("MAX32664 initialized successfully.");

  // Web server route
  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head><meta name='viewport' "
                  "content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:sans-serif;background:#000;color:#0f0;"
            "text-align:center;}h1{color:#0ff;}";
    html +=
        ".reading{font-size:24px;margin:10px;}#pulse{width:20px;height:20px;"
        "background:red;border-radius:50%;animation:pulse 1s infinite;}";
    html += "@keyframes "
            "pulse{0%{transform:scale(1);}50%{transform:scale(1.5);}100%{"
            "transform:scale(1);}}</style></head><body>";
    html += "<h1>Vitals Monitor</h1>";
    html += "<div class='reading'>SYS: " + String(sys) + " mmHg</div>";
    html += "<div class='reading'>DIA: " + String(dia) + " mmHg</div>";
    html += "<div class='reading'>HR: " + String(hr) + " bpm</div>";
    html += "<div class='reading'>SpO2: " + String(spo2) + " %</div>";
    html += "<div id='pulse'></div><br><button "
            "onclick='location.reload()'>Refresh</button></body></html>";
    server.send(200, "text/html", html);
  });

  server.begin();
}

void loop() {
  server.handleClient();

  uint8_t num_samples = MAX32664.readSamples();

  if (num_samples) {
    sys = MAX32664.max32664Output.sys;
    dia = MAX32664.max32664Output.dia;
    hr = MAX32664.max32664Output.hr;
    spo2 = MAX32664.max32664Output.spo2;

    Serial.printf("SYS: %d mmHg | DIA: %d mmHg | HR: %d bpm | SpO2: %d%%\n",
                  sys, dia, hr, spo2);
  }

  delay(1000);
}
