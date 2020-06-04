#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.
const char *ssid = "ssid";
const char *password = "pwd";

const char *mqtt_server = "broker.hivemq.com";

const char *publishTopic = "buzzerIoTClass";
const char *subscribeTopic = "sensorIoTClass";

WiFiClient espClient;
PubSubClient client(espClient);

// Buzzer
int buzzerState = 0;
#define BUZZER D6 // pin D6 is capable of PWM output to drive tones
int del = 500;

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  const size_t capacity = 2 * JSON_OBJECT_SIZE(2) + 50;
  DynamicJsonDocument doc(capacity);

  const char *json = (char *)payload;

  deserializeJson(doc, json);

  float data_temperature = doc["data"]["temperature"];

  // if temperature over 40, toggle buzzer state = 1;
  if (data_temperature > 40)
  {
    buzzerState = 1;
  }
  else
    buzzerState = 0;
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str()))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(publishTopic, "hello world");
      // ... and resubscribe
      client.subscribe(subscribeTopic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{

  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  // sound buzzer
  if (buzzerState == 1)
  {
    analogWrite(BUZZER, 512); // 50 percent duty cycle tone to the piezo
    delay(del);
    digitalWrite(BUZZER, LOW); // turn the piezo off
    delay(del);
  }
}
