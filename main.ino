#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

const char* WIFI_SSID = "AzriZ";
const char* WIFI_PASS = "WIFI_PASS";
const char* SERVER_ENDPOINT = "http://localhost:8080/api/v1/temp-humidity";

class WifiConnection {
public:
  WifiConnection(const char* ssid, const char* password)
    : ssid(ssid), password(password) {}

  void connect() {
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      Serial.println(F("Connecting to WiFi..."));
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(F("."));
      }
      Serial.println(F("\nConnected to WiFi"));
    }
  }

private:
  const char* ssid;
  const char* password;
};

class DHTSensor {
public:
  DHTSensor(uint8_t pin, uint8_t type)
    : dht(pin, type) {}

  void begin() {
    dht.begin();
  }

  float readTemperature() {
    return dht.readTemperature();
  }

  float readHumidity() {
    return dht.readHumidity();
  }

private:
  DHT dht;
};

class ServerClient {
public:
  ServerClient(const char* url)
    : serverUrl(url) {}

  void sendData(float temp, float humidity) {
    if (WiFi.status() == WL_CONNECTED) {
      http.begin(serverUrl);
      http.addHeader(F("Content-Type"), F("application/json"));

      JSONVar jsonObj;
      jsonObj["temp"] = temp;
      jsonObj["humidity"] = humidity;

      String jsonString = JSON.stringify(jsonObj);

      int httpResponseCode = http.POST(jsonString);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print(F("HTTP Response code: "));
        Serial.println(httpResponseCode);
      } else {
        Serial.print(F("Error on sending POST: "));
        Serial.println(httpResponseCode);
      }

      http.end();
    } else {
      Serial.println(F("Error in WiFi connection"));
    }
  }

private:
  const char* serverUrl;
  HTTPClient http;
};

WifiConnection wifiConnection(WIFI_SSID, WIFI_PASS);
DHTSensor dhtSensor(DHTPIN, DHTTYPE);
ServerClient serverClient(SERVER_ENDPOINT);

void setup() {
  Serial.begin(115200);
  dhtSensor.begin();
  wifiConnection.connect();
}

void loop() {
  wifiConnection.connect();

  // float temp = dhtSensor.readTemperature();
  // float humidity = dhtSensor.readHumidity();

  // if (isnan(temp) || isnan(humidity)) {
  //   Serial.println(F("Failed to read from DHT sensor!"));
  //   return;
  // }

  Serial.print(F("Temperature: "));
  // Serial.print(temp);
  Serial.print(F(" °C, Humidity: "));
  // Serial.print(humidity);
  // Serial.println(F(" %"));

  // serverClient.sendData(temp, humidity);

  delay(6000);
}
