#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <sys/time.h>
#include "adc.h"
#include "mpu.h"

Adafruit_ADS1115 ads1;
Adafruit_ADS1115 ads2;
Adafruit_MPU6050 mpu;

const char* ssid = "FMG_SERVER";
const char* password = "AIIMS_FMG_SERVER";
const char* serverName = "http://192.168.1.37:5000/sensor";

void setup(void) {
  Serial.begin(9600);
  
  // WiFi
  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime <10000) {
    delay(500);
    Serial.println("Connecting...");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect to WiFi");
  }

  configTime(0, 0, "pool.ntp.org");
  Serial.println("Waiting for time sync...");
  while(!time(nullptr)) {
    delay(500);
    Serial.println(".");
  }
  Serial.println("\nTime synced");

  // ADC
  setupADC();

  // MPU6050
  setupMPU();
}

String timeWithMillisec() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  struct tm* p_tm = gmtime(&tv.tv_sec);
  char timestamp[30];
  snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
           p_tm->tm_year + 1900, p_tm->tm_mon + 1, p_tm->tm_mday,
           p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec, (int)(tv.tv_usec / 1000));
  return String(timestamp);
}

void loop(void) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    // Sensor read time
    String read_timestamp = timeWithMillisec();
    // Data send time
    String send_timestamp = timeWithMillisec();

    float adcValues[8];
    for (int i = 0; i < 4; i++)
    {
      adcValues[i] = readADC(ads1, i); 
      adcValues[i + 4] = readADC(ads2, i); 
    }

    sensors_event_t a, g, temp;
    readMPU(a, g, temp);

    String jsonData = "{\"sensor_read_time\": \"" + read_timestamp + 
                      "\", \"data_send_time\": \""+ send_timestamp + 
                      "\", \"AIN0\": " + String(adcValues[0], 3) + 
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
    
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
  delay(100);
}
