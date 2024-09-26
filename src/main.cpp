#include <Arduino.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Adafruit_BMP280.h>
#include <WiFiClientSecure.h>

// Wi-Fi
const char *SSID = "AutoConnectAP";
const char *PASS = "password";
const char *GOOGLE_URL = "https://script.google.com/macros/s/AKfycbyssorHTLP7UH4iY-f-GVSHPlCbRK-Z7Ne3_Z9Pfg0BXB3fi6PKcUFhlmHHib2SV7pNgA/exec";

const String host = "script.google.com";   // ホスト
const String url = "/macros/s/AKfycbyssorHTLP7UH4iY-f-GVSHPlCbRK-Z7Ne3_Z9Pfg0BXB3fi6PKcUFhlmHHib2SV7pNgA/exec";  // URL後半部分

// BMP280
Adafruit_BMP280 bmp;

void setup()
{
  delay(2000);

  // シリアル初期化
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println();

  // GPIO初期化
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // BMP280初期化
  bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);

  // Wi-Fi接続
  WiFiManager wm;
  bool res = wm.autoConnect(SSID, PASS);
  if (!res)
  {
    Serial.println("Failed to connect");
    while (1)
    {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(100);
    }
  }
  else
  {
    Serial.println("connected.");
  }
}

void loop()
{
  // POSTメッセージ作成
  String params;
  params = "temperature=" + String(bmp.readTemperature());
  params += "&pressure=" + String(bmp.readPressure() / 100);
  Serial.println(params);
  
  // SSL接続開始、POSTメッセージ送信
  WiFiClientSecure sslclient;
  if (sslclient.connect(host, 443) > 0) {
    sslclient.println("POST " + url + " HTTP/1.1");
    sslclient.println("Host: " + host);
    sslclient.println("User-Agent: ESP8266/1.0");
    sslclient.println("Connection: close");
    sslclient.println("Content-Type: application/x-www-form-urlencoded;");
    sslclient.print("Content-Length: ");
    sslclient.println(params.length());
    sslclient.println();
    sslclient.println(params);
    delay(10);
    String response = sslclient.readString();
    int bodypos =  response.indexOf("\r\n");
  } else {
    // HTTP client errors
    Serial.println("[HTTPS] no connection or no HTTP server.");
  }

  // 待機
  Serial.println();
  delay(10000);
}
