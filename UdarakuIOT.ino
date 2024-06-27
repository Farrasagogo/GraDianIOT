#include <MQUnifiedsensor.h>
#include <DHT.h>

// Definitions for MQ135
#define placa "ESP32"
#define Voltage_Resolution 3.3
#define pin 32 // Analog input pin for MQ135 on ESP32
#define type "MQ-135"
#define ADC_Bit_Resolution 12 // For ESP32
#define RatioMQ135CleanAir 3.6 // RS / R0 = 3.6 ppm

// Definitions for DHT11
#define DHTPIN 33 // Digital pin for DHT11
#define DHTTYPE DHT11

// Declare sensor objects
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Init the serial port communication - to debug the library
  Serial.begin(115200);
  dht.begin();

  // Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1); // _PPM =  a*ratio^b

  /*****************************  MQ Init ********************************************/
  // Remarks: Configure the pin of ESP32 as input.
  MQ135.init();
  
  // Calibration
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for (int i = 1; i <= 10; i++) {
    MQ135.update(); // Update data, the ESP32 will read the voltage from the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0 / 10);
  Serial.println("  done!");

  if (isinf(calcR0)) {
    Serial.println("Warning: Connection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
    while (1);
  }
  if (calcR0 == 0) {
    Serial.println("Warning: Connection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
    while (1);
  }

  Serial.println("** Values from MQ-135 **");
  Serial.println("|    CO   |  Alcohol |   CO2  |  Toluene  |  NH4  |  Acetone  |");
}

void loop() {
  // Update MQ135 sensor data
  MQ135.update();

  // Calculate gas concentrations
  MQ135.setA(605.18); MQ135.setB(-3.937); // CO
  float CO = MQ135.readSensor();

  MQ135.setA(77.255); MQ135.setB(-3.18); // Alcohol
  float Alcohol = MQ135.readSensor();

  MQ135.setA(110.47); MQ135.setB(-2.862); // CO2
  float CO2 = MQ135.readSensor();

  MQ135.setA(44.947); MQ135.setB(-3.445); // Toluene
  float Toluene = MQ135.readSensor();

  MQ135.setA(102.2); MQ135.setB(-2.473); // NH4
  float NH4 = MQ135.readSensor();

  MQ135.setA(34.668); MQ135.setB(-3.369); // Acetone
  float Acetone = MQ135.readSensor();

  // Read DHT11 sensor values
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if any reads failed and exit early (to try again)
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Print values to serial monitor
  Serial.print("|   "); Serial.print(CO);
  Serial.print("   |   "); Serial.print(Alcohol);
  Serial.print("   |   "); Serial.print(CO2 + 400); // Adding 400 PPM for CO2 as baseline
  Serial.print("   |   "); Serial.print(Toluene);
  Serial.print("   |   "); Serial.print(NH4);
  Serial.print("   |   "); Serial.print(Acetone);
  Serial.println("   |");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" *C");

  delay(2000); // Sampling frequency
}
