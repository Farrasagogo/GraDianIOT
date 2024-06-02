#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <DHT.h>

#define WIFI_SSID "Gradian"
#define WIFI_PASSWORD "udinalfa"
#define FIREBASE_HOST "gradian-1a459-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "gradian-1a459-firebase-adminsdk-5pj0e-134bf5943b.json"

const int ldrPin = A0;
#define DHTPIN D5
#define DHTTYPE DHT11

const int pumpRelayPin1 = D1; // Pump for watering
const int pumpRelayPin2 = D2; // Pump for fungicide
const int pumpRelayPin3 = D3; // Pump for pesticide
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
  pinMode(pumpRelayPin1, OUTPUT);
  pinMode(pumpRelayPin2, OUTPUT);
  pinMode(pumpRelayPin3, OUTPUT);
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
  Serial.println(uvAutoStatus ? "Inactive" : "Active");

  if (uvAutoStatus) {
    if (ldrValue > 800) {
      digitalWrite(uvRelayPin, LOW);
      Serial.println("UV Light Deactivated (LDR > 800)");
    } else {
      digitalWrite(uvRelayPin, HIGH);
      Serial.println("UV Light Activated (LDR < 800)");
    }
  } else {
    Serial.println("UV Light (Automation Active)");
    bool uvLightStatus = Firebase.getBool("/sinar");
    Serial.print("UV Light Activation Status: ");
    Serial.println(uvLightStatus ? "Inactive" : "Active");
    digitalWrite(uvRelayPin, uvLightStatus ? LOW : HIGH);
    Serial.println(uvLightStatus ? "UV Light Deactivated" : "UV Light Activated");
  }

bool ObatFungisida = Firebase.getBool("/siramfungi");
bool ObatPestisida = Firebase.getBool("/sirampesti");

if (ObatFungisida) {
  digitalWrite(pumpRelayPin2, LOW);
  Serial.println("Fungicide Pump Deactivated");
  delay(10000); // Pump for 10 seconds
  digitalWrite(pumpRelayPin2, HIGH);
  Serial.println("Fungicide Pump Activated");
  Firebase.setBool("/siramfungi", false);
  Serial.println("Firebase database updated: Fungicide Manual Completed");
} else {
  digitalWrite(pumpRelayPin2, HIGH);
}

if (ObatPestisida) {
  digitalWrite(pumpRelayPin3, LOW);
  Serial.println("Pesticide Pump Deactivated");
  delay(10000); // Pump for 10 seconds
  digitalWrite(pumpRelayPin3, HIGH);
  Serial.println("Pesticide Pump Activated");
  Firebase.setBool("/sirampesti", false);
  Serial.println("Firebase database updated: Pesticide Manual Completed");
} else {
  digitalWrite(pumpRelayPin3, HIGH);
}

Serial.println("");

bool obatAutoStatus = Firebase.getBool("/obatauto");
Serial.print("Fertilizer/Pesticide Automation Status: ");
Serial.println(obatAutoStatus ? "Inactive" : "Active");

if (obatAutoStatus) {
  bool jadwalObatFungisida = Firebase.getBool("/jadwalobatfungisida");
  bool jadwalObatPestisida = Firebase.getBool("/jadwalobatpestisida");

  if (jadwalObatFungisida) {
    digitalWrite(pumpRelayPin2, LOW);
    Serial.println("Fungicide Pump Deactivated");
    delay(10000); // Pump for 10 seconds
    digitalWrite(pumpRelayPin2, HIGH);
    Serial.println("Fungicide Pump Activated");
    Firebase.setBool("/jadwalobatfungisida", false);
    Serial.println("Firebase database updated: Fungicide Schedule Completed");
  } else {
    digitalWrite(pumpRelayPin2, HIGH);
  }

  if (jadwalObatPestisida) {
    digitalWrite(pumpRelayPin3, LOW);
    Serial.println("Pesticide Pump Deactivated");
    delay(10000); // Pump for 10 seconds
    digitalWrite(pumpRelayPin3, HIGH);
    Serial.println("Pesticide Pump Activated");
    Firebase.setBool("/jadwalobatpestisida", false);
    Serial.println("Firebase database updated: Pesticide Schedule Completed");
  } else {
    digitalWrite(pumpRelayPin3, HIGH);
  }
}

  Serial.println("");

  bool pumpAutoStatus = Firebase.getBool("/siramauto");
  Serial.print("Pump Automation Status: ");
  Serial.println(pumpAutoStatus ? "Inactive" : "Active");

  if (pumpAutoStatus) {
    if (humidity < 65 && temperature > 30) {
      digitalWrite(pumpRelayPin1, LOW);
      Serial.println("Pump Deactivated (Humidity < 65 and Temperature > 30)");
      delay(10000);
      digitalWrite(pumpRelayPin1, HIGH);
      Serial.println("Pump Activated after 10 seconds");
    } else {
      Serial.println("Pump Activated (Condition not met)");
    }
  } else {
    Serial.println("Pump (Automation Active)");
    bool pumpStatus = Firebase.getBool("/siram");
    Serial.print("Pump Activation Status: ");
    Serial.println(pumpStatus ? "Inactive" : "Active");
    digitalWrite(pumpRelayPin1, pumpStatus ? LOW : HIGH);
    Serial.println(pumpStatus ? "Pump Deactivated" : "Pump Activated");
    if (pumpStatus) {
      delay(10000);
      digitalWrite(pumpRelayPin1, HIGH);
      Serial.println("Pump Activated after 10 seconds");
      Firebase.setBool("/siram", false);
      Serial.println("Firebase database updated: Pump Activated");
    }
  }

  delay(1000);
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
