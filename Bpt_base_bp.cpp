#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "max32664.h"
#include "Protocentral_MAX30205.h"

// ===== Wi-Fi Credentials =====
const char* ssid = "Roman";
const char* password = "best4321";

// ===== MAX32664 Setup =====
#define RESET_PIN 12   // GPIO12 (D6 on NodeMCU)
#define MFIO_PIN  16   // GPIO16 (D0 on NodeMCU)
#define SDA_PIN    4   // GPIO4  (D2 on NodeMCU)
#define SCL_PIN    5   // GPIO5  (D1 on NodeMCU)
#define RAWDATA_BUFFLEN 250
max32664 MAX32664(RESET_PIN, MFIO_PIN, RAWDATA_BUFFLEN);

// ===== MAX30205 Setup =====
MAX30205 tempSensor;

// ===== TFT Setup =====
#define TFT_CS   15   // GPIO15 (D8 on NodeMCU)
#define TFT_RST   2   // GPIO2  (D4 on NodeMCU)
#define TFT_DC    0   // GPIO0  (D3 on NodeMCU)
#define TFT_SCLK 14   // GPIO14 (D5 on NodeMCU)
#define TFT_MOSI 13   // GPIO13 (D7 on NodeMCU)
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// ===== Web Server =====
ESP8266WebServer server(80);

// ===== Vitals =====
int sys = 0, dia = 0, hr = 0, spo2 = 0;
float tempC = 0.0;
int animState = 0;

void loadAlgomodeParameters() {
  algomodeInitialiser algoParameters;

  // Calibration values (example only – tune as needed)
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

void drawVitals() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(0, 0);
  tft.printf("SYS: %d\nDIA: %d\nHR: %d\nSpO2: %d%%\n", sys, dia, hr, spo2);
  tft.printf("TEMP: %.2f C\n", tempC);

  // Pulse animation
  int centerX = 64, centerY = 120;
  int radius = animState == 0 ? 5 : 10;
  tft.fillCircle(centerX, centerY, radius, ST77XX_RED);
  animState = !animState;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  // ===== TFT Init =====
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(0, 0);
  tft.println("Initializing...");

  // ===== Wi-Fi =====
  WiFi.begin(ssid, password);
  tft.println("Connecting WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  tft.println("WiFi OK");
  tft.print("IP:");
  tft.println(WiFi.localIP());
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ===== MAX32664 Init =====
  loadAlgomodeParameters();
  if (MAX32664.hubBegin() != CMD_SUCCESS ||
      !MAX32664.startBPTcalibration() ||
      !MAX32664.configAlgoInEstimationMode()) {
    tft.println("MAX32664 fail");
    Serial.println("MAX32664 init failed!");
    while (1);
  }
  tft.println("MAX32664 OK");

  // ===== MAX30205 Init =====
  while (!tempSensor.scanAvailableSensors()) {
    tft.println("Temp sensor not found.");
    Serial.println("MAX30205 not found. Retrying...");
    delay(3000);
  }
  tempSensor.begin();
  tft.println("Temp sensor OK");

  // ===== Web Server =====
  server.on("/", []() {
    String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:sans-serif;background:#000;color:#0f0;text-align:center;}h1{color:#0ff;}";
    html += ".reading{font-size:22px;margin:10px;}#pulse{width:20px;height:20px;background:red;border-radius:50%;animation:pulse 1s infinite;}";
    html += "@keyframes pulse{0%{transform:scale(1);}50%{transform:scale(1.5);}100%{transform:scale(1);}}</style></head><body>";
    html += "<h1>Vitals Monitor</h1>";
    html += "<div class='reading'>SYS: " + String(sys) + " mmHg</div>";
    html += "<div class='reading'>DIA: " + String(dia) + " mmHg</div>";
    html += "<div class='reading'>HR: " + String(hr) + " bpm</div>";
    html += "<div class='reading'>SpO₂: " + String(spo2) + " %</div>";
    html += "<div class='reading'>Temp: " + String(tempC, 2) + " °C</div>";
    html += "<div id='pulse'></div><br><button onclick='location.reload()'>Refresh</button></body></html>";
    server.send(200, "text/html", html);
  });

  server.begin();
}

void loop() {
  server.handleClient();

  uint8_t num_samples = MAX32664.readSamples();
  if (num_samples) {
    sys   = MAX32664.max32664Output.sys;
    dia   = MAX32664.max32664Output.dia;
    hr    = MAX32664.max32664Output.hr;
    spo2  = MAX32664.max32664Output.spo2;
    tempC = tempSensor.getTemperature();

    Serial.print("SYS: "); Serial.print(sys);
    Serial.print(" | DIA: "); Serial.print(dia);
    Serial.print(" | HR: "); Serial.print(hr);
    Serial.print(" | SpO₂: "); Serial.print(spo2);
    Serial.print(" | Temp: "); Serial.print(tempC, 2);
    Serial.println(" °C");

    drawVitals();
  }

  delay(1000);
}
