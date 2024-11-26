#include <WiFi.h>
#include <HTTPClient.h>
#include "adc.h"
#include "mpu.h"

Adafruit_ADS1115 ads1;
Adafruit_ADS1115 ads2;
Adafruit_MPU6050 mpu;

const char* ssid = "NESUG-4_2G";
const char* password = "9911773437";
const char* serverName = "http://192.168.1.37:5000/sensor";

int counter = 0;

void setup(void) {
  Serial.begin(512000);
    
  // WiFi
  WiFi.begin(ssid, password);

  // ADC
  setupADC();

  // MPU6050
  setupMPU();
}

void loop(void) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    float adcValues[8];
    for (int i = 0; i < 4; i++) {
      adcValues[i] = readADC(ads1, i); 
      adcValues[i + 4] = readADC(ads2, i);
    }

    sensors_event_t a, g, temp;
    readMPU(a, g, temp);

    String jsonData = "{ \"counter\": " + String(counter) + // Add counter to JSON data
                      ", \"AIN0\": " + String(adcValues[0], 3) + 
                      ", \"AIN1\": " + String(adcValues[1], 3) + 
                      ", \"AIN2\": " + String(adcValues[2], 3) + 
                      ", \"AIN3\": " + String(adcValues[3], 3) + 
                      ", \"AIN4\": " + String(adcValues[4], 3) + 
                      ", \"AIN5\": " + String(adcValues[5], 3) + 
                      ", \"AIN6\": " + String(adcValues[6], 3) +
                      ", \"AIN7\": " + String(adcValues[7], 3) + 
                      ", \"Acceleration_x\": " + String(a.acceleration.x) + 
                      ", \"Acceleration_y\": " + String(a.acceleration.y) + 
                      ", \"Acceleration_z\": " + String(a.acceleration.z) + 
                      ", \"Rotation_x\": " + String(g.gyro.x) + 
                      ", \"Rotation_y\": " + String(g.gyro.y) + 
                      ", \"Rotation_z\": " + String(g.gyro.z) + "}";

    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
      counter++;
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(1);
}
