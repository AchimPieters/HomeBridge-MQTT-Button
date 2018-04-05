/* Project name: HomeBridge – MQTT Button
   Project URI: https://www.studiopieters.nl/homebridge-mqtt-button/
   Description: HomeBridge – MQTT Button
   Version: 1.7.4
   License: MIT
*/


#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char *ssid =  "Your_SSID";    // cannot be longer than 32 characters!
const char *pass =  "Your_PASS";    

int inPin = 0;         // the number of the input pin
int ledPin = 2;        // Control Led to see if everything is working.

int state = HIGH;      // the current state of the output pin
int reading;           // the current reading from the input pin
int previous = LOW;    // the previous reading from the input pin

// the follow variables are long's because the time, measured in miliseconds, will quickly become a bigger number than can be stored in an int.
long Time = 0;         // the last time the output pin was toggled
long debounce = 200;   // the debounce time, increase if the output flickers



// Update these with values suitable for your network.
IPAddress server(192, 168, 1, 10);

#define BUFFER_SIZE 100


void callback(const MQTT::Publish& pub) {
  pinMode(ledPin, OUTPUT);

  // check if the message in On or Off
  if (pub.payload_string() == "ON") {
    state = LOW;
    // turn LED on:
    digitalWrite(ledPin, HIGH);
    Serial.println(pub.payload_string());
  } else {
    state = HIGH;
    // turn LED off:
    digitalWrite(ledPin, LOW);
    Serial.println(pub.payload_string());
  }
}

WiFiClient wclient;
PubSubClient client(wclient, server);

void setup() {
  // Setup console
  pinMode(inPin, INPUT);
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.println();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to ");
    Serial.print(ssid);
    Serial.println("...");
    WiFi.begin(ssid, pass);

    if (WiFi.waitForConnectResult() != WL_CONNECTED)
      return;
    Serial.println("WiFi connected");
  }

  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      if (client.connect("arduinoClient")) {
        client.set_callback(callback);
        client.subscribe("button/01"); // change InTopic to your Topic description
      }
    }

    if (client.connected())
      client.loop();

    reading = digitalRead(inPin);

    // if the input just went from LOW and HIGH and we've waited long enough
    // to ignore any noise on the circuit, toggle the output pin and remember
    // the time
    if (reading == HIGH && previous == LOW && millis() - Time > debounce) {
      if (state == HIGH) {
        state = LOW;
        digitalWrite(ledPin, HIGH);
        client.publish("button/01", "ON");
      }
      else {
        state = HIGH;
        digitalWrite(ledPin, LOW);
        client.publish("button/01", "OFF");
      }

      Time = millis();
    }

    previous = reading;

  }

}
