#include <Adafruit_ADS1X15.h>
#include <Adafruit_MPU6050.h>
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <time.h>
#include <sys/time.h>

Adafruit_ADS1115 ads;
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
  Serial.println("Getting single-ended readings from AIN0");
  Serial.println("ADC Range: +/- 4.096V (1 bit = 0.125mV)");
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
  ads.setGain(GAIN_ONE); // Gain of 1, +/- 4.096V

  // MPU6050
  Serial.println("MPU6050 Test");
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  Serial.print("Accelerometer range set to: ");
  switch (mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_2_G: Serial.println("+-2G"); break;
    case MPU6050_RANGE_4_G: Serial.println("+-4G"); break;
    case MPU6050_RANGE_8_G: Serial.println("+-8G"); break;
    case MPU6050_RANGE_16_G: Serial.println("+-16G"); break;
  }
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  Serial.print("Gyro range set to: ");
  switch (mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG: Serial.println("+- 250 deg/s"); break;
    case MPU6050_RANGE_500_DEG: Serial.println("+- 500 deg/s"); break;
    case MPU6050_RANGE_1000_DEG: Serial.println("+- 1000 deg/s"); break;
    case MPU6050_RANGE_2000_DEG: Serial.println("+- 2000 deg/s"); break;
  }
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);
  Serial.print("Filter bandwidth set to: ");
  switch (mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ: Serial.println("260 Hz"); break;
    case MPU6050_BAND_184_HZ: Serial.println("184 Hz"); break;
    case MPU6050_BAND_94_HZ: Serial.println("94 Hz"); break;
    case MPU6050_BAND_44_HZ: Serial.println("44 Hz"); break;
    case MPU6050_BAND_21_HZ: Serial.println("21 Hz"); break;
    case MPU6050_BAND_10_HZ: Serial.println("10 Hz"); break;
    case MPU6050_BAND_5_HZ: Serial.println("5 Hz"); break;
  }
  Serial.println("");
  delay(100);
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

    int16_t adc0 = ads.readADC_SingleEnded(0);
    float volts0 = ads.computeVolts(adc0);

    Serial.println("-----------------------------------------------------------");
    Serial.print("AIN0: "); Serial.print(adc0); Serial.print("  "); Serial.print(volts0, 3); Serial.println(" V");

    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    Serial.print("Acceleration X: "); Serial.print(a.acceleration.x); Serial.print(", Y: "); Serial.print(a.acceleration.y); Serial.print(", Z: "); Serial.println(a.acceleration.z); Serial.print(" m/s^2");
    Serial.print("Rotation X: "); Serial.print(g.gyro.x); Serial.print(", Y: "); Serial.print(g.gyro.y); Serial.print(", Z: "); Serial.println(g.gyro.z); Serial.println(" rad/s");
    Serial.println("");

    String jsonData = "{\"sensor_read_time\": \"" + read_timestamp + 
                      "\", \"data_send_time\": \""+ send_timestamp + 
                      "\", \"AIN0\": " + String(volts0, 3) + 
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
