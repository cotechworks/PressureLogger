#include <Arduino.h>
#include <WiFiManager.h>
#include <Adafruit_BMP280.h>
#include <WiFiClientSecure.h>

// Wi-Fi
const char *SSID = "AutoConnectAP";
const char *PASS = "password";
const String GOOGLE_URL = "https://script.google.com/macros/s/AKfycbyssorHTLP7UH4iY-f-GVSHPlCbRK-Z7Ne3_Z9Pfg0BXB3fi6PKcUFhlmHHib2SV7pNgA/exec";

const String host = "script.google.com";   // ホスト
const String url = "/macros/s/AKfycbyssorHTLP7UH4iY-f-GVSHPlCbRK-Z7Ne3_Z9Pfg0BXB3fi6PKcUFhlmHHib2SV7pNgA/exec";  // URL後半部分

// BMP280
Adafruit_BMP280 bmp;

bool Https_Get_access(String host, String url, String argument){
    BearSSL::WiFiClientSecure client;
    client.setTimeout(500);
    client.setInsecure();

    const int httpPort = 443;
    const char* host2 = host.c_str();
    if (!client.connect(host2, httpPort)) {
        Serial.println("connection failed");
        return false;
    }
    client.print(String("GET ") + url + "?" + argument + " HTTP/1.1\r\n" +
                        "Host: " + host + "\r\n" + 
                        "User-Agent: ESP8266/1.0\r\n" + 
                        "Connection: close\r\n\r\n");
    unsigned long timeout = micros();
    while (client.available() == 0) {
        if ( micros() - timeout  > 5000000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return false;
        }
    }
    while(client.available()){
        String line = client.readStringUntil('\r');
        Serial.print(line);
    }
    return true;
}

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
  // 環境データ測定
  float temp = bmp.readTemperature();
  float press = bmp.readPressure() / 100;

  // URL作成
  String params;
  params += "temperature=" + String(temp);
  params += "&pressure=" + String(press);
  Serial.println(params);

  //POST
  Https_Get_access( host, url, params);

  // 待機
  Serial.println();
  delay(60000);
}