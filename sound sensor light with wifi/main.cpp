#include <Arduino.h>
#include <WiFi.h>

const char* ssid = "seamk-titelab";
const char* password = "Sula289tite";

const int soundSensorPin = 34;
const int ledPin = 2;

bool sensorActive = true; // track if the sensor is active or not
bool ledOn = false; // track if LED is on
unsigned long lastSoundTime = 0; // Variable to store the time when the sound was last detected
bool shouldStayOn = false; // track if LED should stay on after sound level drops below 2300

WiFiServer server(80);

void setup() {
  pinMode(soundSensorPin, INPUT);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600); 
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    Serial.println(WiFi.localIP());
  }
  
  // Start the server
  server.begin();
}

void loop() {
  // Handle incoming client requests
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client connected");
    while (client.connected()) {
      if (client.available()) {
        String request = client.readStringUntil('\r');
        if (request.indexOf("GET /sensoron") != -1) {
          sensorActive = true;
          client.println("HTTP/1.1 302 Found");
          client.println("Location: /");
        } else if (request.indexOf("GET /sensoroff") != -1) {
          sensorActive = false;
          client.println("HTTP/1.1 302 Found");
          client.println("Location: /");
        }

        // Generate the main control panel HTML
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println();
        client.println("<!DOCTYPE html>");
        client.println("<html>");
        client.println("<head>");
        client.println("<title>Control Panel</title>");
        client.println("<style>");
        client.println("body { font-family: Arial, sans-serif; text-align: center; font-size: 60px }");
        client.println("h1 { margin-bottom: 600px; }");
        client.println(".button { display: inline-block; background-color: #4CAF50; border: none; color: white; padding: 70px 100px; text-align: center; text-decoration: none; display: inline-block; font-size: 60px; margin: 8px 4px; cursor: pointer; border-radius: 12px; }");
        client.println(".button-on { background-color: #4CAF50; }");
        client.println(".button-off { background-color: #f44336; }");
        client.println("</style>");
        client.println("</head>");
        client.println("<body>");
        client.println("<h1>Control Panel</h1>");
        client.print("<p>Sound sensor is ");
        client.print(sensorActive ? "activated" : "deactivated");
        client.println("</p>");
        client.println("<form action=\"/sensoron\" method=\"GET\">");
        client.println("<button class=\"button button-on\">Turn Sensor On</button>");
        client.println("</form>");
        client.println("<form action=\"/sensoroff\" method=\"GET\">");
        client.println("<button class=\"button button-off\">Turn Sensor Off</button>");
        client.println("</form>");
        client.println("</body>");
        client.println("</html>");
        break;
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }

  // if the sensor is active
  if (sensorActive) {
    int sensorValue = analogRead(soundSensorPin);

    Serial.print("Sound level: ");
    Serial.println(sensorValue); 

      if (sensorValue > 2300) { 
    if (!ledOn) { 
      digitalWrite(ledPin, HIGH); 
      ledOn = true; // Update flag
      lastSoundTime = millis(); // Record the time when the sound was last detected
    } else if (millis() - lastSoundTime > 2000) { // If LED is already on and 2 seconds have passed since last sound
      digitalWrite(ledPin, LOW);
      ledOn = false; // Update flag
    }
    shouldStayOn = true; // Set the flag to true as the LED should stay on
  } else {
    if (shouldStayOn) { // If LED was previously on due to sound level > 2300
      lastSoundTime = millis(); 
      shouldStayOn = false; 
    }
    if (millis() - lastSoundTime < 2000) { // If LED should stay on after sound level drops below 2300
      digitalWrite(ledPin, HIGH); 
      ledOn = true; 
    } else {
      digitalWrite(ledPin, LOW); 
      ledOn = false; 
    }
  }
  } else {
    // If the sensor is deactivated, turn off the LED
    digitalWrite(ledPin, LOW);
  }

  delay(100); 
}
