#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <DHT.h>

#define WIFI_SSID "Komputer"
#define WIFI_PASSWORD "sabrodin123"

#define FIREBASE_HOST "xxx"
#define FIREBASE_AUTH "xxx"

const int ldrPin = A0;
#define DHTPIN D4  
#define DHTTYPE DHT22   

const int pumpRelayPin = D1;  
const int uvRelayPin = D0;    

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
   
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi!");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  dht.begin();

  pinMode(pumpRelayPin, OUTPUT);
  pinMode(uvRelayPin, OUTPUT);
}

void loop() {
  Serial.println("--------------------------------------");
  Serial.println("");
  int ldrValue = analogRead(ldrPin);

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  
  Serial.print("LDR Value: ");
  Serial.println(ldrValue);
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.println("%");
  Serial.println("");

  bool uvAutoStatus = Firebase.getBool("/sinarauto");
  Serial.print("UV Light Automation Status: ");
  Serial.println(uvAutoStatus ? "Active" : "Inactive");

  if (uvAutoStatus) {
    if (ldrValue > 800) {
      digitalWrite(uvRelayPin, HIGH); 
      Serial.println("UV Light Activated (LDR > 800)");
    } else {
      digitalWrite(uvRelayPin, LOW); 
      Serial.println("UV Light Deactivated (LDR < 800)");
    }
  } else {   
    Serial.println("UV Light (Automation Inactive)");
    bool uvLightStatus = Firebase.getBool("/sinar");
    Serial.print("UV Light Activation Status: ");
    Serial.println(uvLightStatus ? "Active" : "Inactive");

    digitalWrite(uvRelayPin, uvLightStatus ? HIGH : LOW); 
    Serial.println(uvLightStatus ? "UV Light Activated" : "UV Light Deactivated");
  }

  Serial.println("");

  bool pumpAutoStatus = Firebase.getBool("/siramauto");
  Serial.print("Pump Automation Status: ");
  Serial.println(pumpAutoStatus ? "Active" : "Inactive");

  if (pumpAutoStatus) {
    if (humidity < 65 && temperature > 30) {
      digitalWrite(pumpRelayPin, HIGH); 
      Serial.println("Pump Activated (Humidity < 65 and Temperature > 30)");
      delay(10000); 
      digitalWrite(pumpRelayPin, LOW); 
      Serial.println("Pump Deactivated after 10 seconds");
    } else {
      Serial.println("Pump Deactivated (Condition not met)");
    }
  } else {
    Serial.println("Pump (Automation Inactive)");
    bool pumpStatus = Firebase.getBool("/siram");
    Serial.print("Pump Activation Status: ");
    Serial.println(pumpStatus ? "Active" : "Inactive");

    digitalWrite(pumpRelayPin, pumpStatus ? HIGH : LOW); 
    Serial.println(pumpStatus ? "Pump Activated" : "Pump Deactivated");

    if (pumpStatus) {
      delay(10000); 
      digitalWrite(pumpRelayPin, LOW); 
      Serial.println("Pump Deactivated after 10 seconds");

      Firebase.setBool("/siram", false);
      Serial.println("Firebase database updated: Pump Deactivated");
    }
  }
  Serial.println("");

  String ldrPath = "/ldrValue";
  Firebase.setInt(ldrPath, ldrValue);

  String tempPath = "/temperature";
  String humPath = "/humidity";

  Firebase.setFloat(tempPath, temperature);
  Firebase.setFloat(humPath, humidity);

  delay(10000); 
  Serial.println("--------------------------------------");
}
