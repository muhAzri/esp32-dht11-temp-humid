#include <WiFi.h>
#include <Arduino_JSON.h>
#include <DHT.h>
#include <WebSocketClient.h>

// WiFi credentials
const char* ssid = "fitriawulansari";
const char* password = "Soulmate321@";

// WebSocket server details
const char* host = "192.168.1.3";
int port = 8080;
const char* path = "/ws";

// DHT sensor configuration
#define DHTPIN 4
#define DHTTYPE DHT11

// WiFiManager class handles WiFi connection
class WiFiManager {
public:
  WiFiManager(const char* ssid, const char* password) : ssid(ssid), password(password) {}

  /**
   * Connects to WiFi network using provided credentials.
   */
  void connect() {
    Serial.println(F("Connecting to WiFi..."));
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(F("."));
    }
    
    Serial.println(F("\nConnected to WiFi"));
    Serial.print(F("IP address: "));
    Serial.println(WiFi.localIP());
  }

private:
  const char* ssid;
  const char* password;
};

// WebSocketManager class handles WebSocket connection and data transmission
class WebSocketManager {
public:
  WebSocketManager(const char* host, int port, const char* path) : host(host), port(port), path(path) {}

  /**
   * Connects to the WebSocket server and performs handshake.
   * 
   * @return true if the connection and handshake are successful, false otherwise.
   */
  bool connect() {
    Serial.print(F("Connecting to WebSocket server: "));
    Serial.print(host);
    Serial.print(F(":"));
    Serial.println(port);
    
    if (!client.connect(host, port)) {
      Serial.println(F("Connection to WebSocket server failed!"));
      return false;
    }

    // Set WebSocketClient path and host
    webSocketClient.path = const_cast<char*>(path); 
    webSocketClient.host = const_cast<char*>(host); 

    if (!webSocketClient.handshake(client)) {
      Serial.println(F("WebSocket handshake failed!"));
      return false;
    }

    Serial.println(F("WebSocket handshake successful"));
    return true;
  }

  /**
   * Sends data to the WebSocket server.
   * 
   * @param data String data to send.
   */
  void sendData(String data) {
    if (client.connected()) {
      webSocketClient.sendData(data);
      Serial.println(F("Data sent to WebSocket"));
    } else {
      Serial.println(F("WebSocket client disconnected!"));
    }
  }

private:
  WiFiClient client;
  WebSocketClient webSocketClient;
  const char* host;
  int port;
  const char* path;
};

// SensorManager class handles DHT sensor operations
class SensorManager {
public:
  SensorManager(int pin, int type) : dht(pin, type) {}

  /**
   * Initializes the DHT sensor.
   * 
   * @return true if initialization succeeds, false otherwise.
   */
  bool begin() {
    dht.begin();
    return true;
  }

  /**
   * Reads temperature from the DHT sensor.
   * 
   * @return float temperature in Celsius.
   */
  float readTemperature() {
    return dht.readTemperature();
  }

  /**
   * Reads humidity from the DHT sensor.
   * 
   * @return float humidity in percentage.
   */
  float readHumidity() {
    return dht.readHumidity();
  }

private:
  DHT dht;
};

// App class manages the setup and loop execution of the application
class App {
public:
  /**
   * Constructs the application with necessary parameters.
   * 
   * @param ssid WiFi network SSID.
   * @param password WiFi network password.
   * @param host WebSocket server hostname or IP address.
   * @param port WebSocket server port.
   * @param path WebSocket server path.
   */
  App(const char* ssid, const char* password, const char* host, int port, const char* path)
    : wifiManager(ssid, password), webSocketManager(host, port, path), sensorManager(DHTPIN, DHTTYPE) {}

  /**
   * Initializes the application and components.
   */
  void setup() {
    Serial.begin(115200);
    sensorManager.begin();
    wifiManager.connect();
    webSocketManager.connect();
  }

  /**
   * Main execution loop of the application.
   * Reads sensor data, sends it to the WebSocket server,
   * and logs the data to Serial output.
   */
  void loop() {
    float temperature = sensorManager.readTemperature();
    float humidity = sensorManager.readHumidity();

    if (isnan(temperature) || isnan(humidity)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      delay(10000); // Wait for 10 seconds before retrying
      return;
    }

    Serial.print(F("Temperature: "));
    Serial.print(temperature);
    Serial.print(F(" °C, Humidity: "));
    Serial.print(humidity);
    Serial.println(F(" %"));

    JSONVar data;
    data["temperature"] = temperature;
    data["humidity"] = humidity;

    String jsonStr = JSON.stringify(data);
    webSocketManager.sendData(jsonStr);

    delay(10000); // Wait for 10 seconds before next reading
  }

private:
  WiFiManager wifiManager;
  WebSocketManager webSocketManager;
  SensorManager sensorManager;
};

// Create an instance of the App class
App app(ssid, password, host, port, path);

/**
 * Arduino setup function.
 * Calls the setup method of the App instance.
 */
void setup() {
  app.setup();
}

/**
 * Arduino main loop function.
 * Calls the loop method of the App instance.
 */
void loop() {
  app.loop();
}
