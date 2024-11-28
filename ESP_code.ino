#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_ADS1X15.h>
#include <Adafruit_Sensor.h>

Adafruit_ADS1115 ads1;
Adafruit_ADS1115 ads2;
Adafruit_MPU6050 mpu;

#define MPU6050_ADDRESS 0x68
#define UDP_PORT 5000

const char* ssid = "NESUG-4_2G";
const char* password = "9911773437";
IPAddress serverIP(192, 168, 1, 37); // Correctly define the server IP

WiFiUDP udp;
int counter = 0;

void setup() {
  Serial.begin(512000);

  // Initialize I2C with Fast Mode
  Wire.begin();
  Wire.setClock(400000);

  // Connect to WiFi
  WiFi.begin(ssid, password);

  // Initialize ADS1115
  if (!ads1.begin(0x48)) {
    Serial.println("Failed to initialize ADS1.");
    while (1);
  }
  if (!ads2.begin(0x49)) {
    Serial.println("Failed to initialize ADS2.");
    while (1);
  }
  ads1.setGain(GAIN_ONE);
  ads2.setGain(GAIN_ONE);
  ads1.setDataRate(RATE_ADS1115_860SPS);
  ads2.setDataRate(RATE_ADS1115_860SPS);

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1);
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_260_HZ);

  // Configure MPU6050 sample rate to 1kHz
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x19);  // Sample Rate Divider register
  Wire.write(0x00);  // Set to 1kHz
  Wire.endTransmission();
}

void readMPU(uint8_t* buffer) {
  Wire.beginTransmission(MPU6050_ADDRESS);
  Wire.write(0x3B);  // Start reading at ACCEL_XOUT_H
  Wire.endTransmission(false);  // Restart for burst read
  Wire.requestFrom(MPU6050_ADDRESS, 14);  // Read 14 bytes (accel + gyro + temp)

  for (int i = 0; i < 14; i++) {
    buffer[i] = Wire.read();
  }
}

float readADC(Adafruit_ADS1115& ads, int channel) {
  int16_t adcValue = ads.readADC_SingleEnded(channel);
  return ads.computeVolts(adcValue);
}

void loop() {
  // Read ADC values
  float adcValues[8];
  for (int i = 0; i < 4; i++) {
    adcValues[i] = readADC(ads1, i);
    adcValues[i + 4] = readADC(ads2, i);
  }

  // Read MPU6050 data
  uint8_t mpuBuffer[14];
  readMPU(mpuBuffer);

  float ax = (int16_t)((mpuBuffer[0] << 8) | mpuBuffer[1]) / 16384.0;
  float ay = (int16_t)((mpuBuffer[2] << 8) | mpuBuffer[3]) / 16384.0;
  float az = (int16_t)((mpuBuffer[4] << 8) | mpuBuffer[5]) / 16384.0;
  float gx = (int16_t)((mpuBuffer[8] << 8) | mpuBuffer[9]) / 131.0;
  float gy = (int16_t)((mpuBuffer[10] << 8) | mpuBuffer[11]) / 131.0;
  float gz = (int16_t)((mpuBuffer[12] << 8) | mpuBuffer[13]) / 131.0;

  // Prepare data for transmission
  String data = String(counter) + "," +
                String(adcValues[0], 3) + "," + String(adcValues[1], 3) + "," +
                String(adcValues[2], 3) + "," + String(adcValues[3], 3) + "," +
                String(adcValues[4], 3) + "," + String(adcValues[5], 3) + "," +
                String(adcValues[6], 3) + "," + String(adcValues[7], 3) + "," +
                String(ax) + "," + String(ay) + "," + String(az) + "," +
                String(gx) + "," + String(gy) + "," + String(gz);

  Serial.println(data);

  // Transmit data via UDP
  udp.beginPacket(serverIP, UDP_PORT);  // Use IPAddress for server IP
  udp.write((const uint8_t*)data.c_str(), data.length());
  udp.endPacket();

  counter++;
}
