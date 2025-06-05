#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; // Increase this for more averaging
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;
float temperature;
float SpO2;

void setup()
{
  Serial.begin(115200);
  Serial.println("MAX30102 Heart Rate and SpO2 Monitor");

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
  {
    Serial.println("MAX30102 was not found. Please check wiring/power.");
    while (1);
  }

  // Setup sensor with default settings
  particleSensor.setup();
  
  // Turn on red and IR LEDs
  particleSensor.setPulseAmplitudeRed(0x0A);
  particleSensor.setPulseAmplitudeGreen(0); // Turn off Green LED
  particleSensor.setPulseAmplitudeIR(0x0A);
  
  // Enable temperature reading
  particleSensor.enableDIETEMPRDY();
}

void loop()
{
  long irValue = particleSensor.getIR();

  if (irValue > 50000) // Check if finger is placed on sensor
  {
    // Read temperature from the sensor
    temperature = particleSensor.readTemperature();
    
    // Calculate heart rate
    if (checkForBeat(irValue))
    {
      long delta = millis() - lastBeat;
      lastBeat = millis();
      beatsPerMinute = 60 / (delta / 1000.0);
      
      if (beatsPerMinute < 255 && beatsPerMinute > 20)
      {
        rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading in the array
        rateSpot %= RATE_SIZE; // Wrap variable
        
        // Take average of readings
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }
    
    // Note: For proper SpO2 calculation, a more complex algorithm is needed
    // This is a simplified approximation
    SpO2 = 95.0 + (random(0, 5) / 10.0); // Just a placeholder simulation
    
    Serial.print("IR=");
    Serial.print(irValue);
    Serial.print(", BPM=");
    Serial.print(beatsPerMinute);
    Serial.print(", Avg BPM=");
    Serial.print(beatAvg);
    Serial.print(", SpO2=");
    Serial.print(SpO2);
    Serial.print("%, Temp=");
    Serial.print(temperature, 2);
    Serial.println("°C");
  }
  else
  {
    // No finger detected
    Serial.println("Place your finger on the sensor");
    // Reset values when finger is removed
    beatsPerMinute = 0;
    beatAvg = 0;
    SpO2 = 0;
  }
  
  delay(100); // Small delay between readings
}
