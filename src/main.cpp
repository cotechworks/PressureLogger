#include <Arduino.h>
#include <WiFiManager.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Adafruit_BMP280.h>

// Wi-Fi
const char *SSID = "AutoConnectAP";
const char *PASS = "password";
const char *GOOGLE_URL = "https://script.google.com/macros/s/AKfycbyssorHTLP7UH4iY-f-GVSHPlCbRK-Z7Ne3_Z9Pfg0BXB3fi6PKcUFhlmHHib2SV7pNgA/exec";

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
  bool status = bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID);
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
  // 環境データ測定
  float temp = bmp.readTemperature();
  float press = bmp.readPressure() / 100;
  Serial.println("気温= " + String(temp) + "*C / 気圧= " + String(press) + "hPa");

  // HTTPリクエスト
  WiFiClient client;
  HTTPClient http;
  char url[200];
  char temp_s[10];
  char press_s[10];
  dtostrf(temp, 4, 2, temp_s);
  dtostrf(press, 6, 2, press_s);
  sprintf(url, "%s?temperature=%s&pressure=%s", GOOGLE_URL, temp_s, press_s);
  Serial.print("URL: ");
  Serial.println(url);

  String url_s = "https://script.google.com/macros/s/AKfycbyssorHTLP7UH4iY-f-GVSHPlCbRK-Z7Ne3_Z9Pfg0BXB3fi6PKcUFhlmHHib2SV7pNgA/exec?temperature=28.03&pressure=1011.40";
  if (http.begin(client, url_s))
  {
    // リクエスト送信結果の確認
    int httpCode = http.GET();
    Serial.println("[HTTP] Status Code: " + String(httpCode));
    if (httpCode > 0)
    {
      String payload = http.getString();
      Serial.println("[HTTP] Payload: " + payload);
    }
    http.end();
  }
  else
  {
    Serial.println("[HTTP] Unable to connect");
  }

  // 待機
  Serial.println();
  delay(10000);
}
