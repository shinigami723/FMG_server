#ifndef ADC_H
#define ADC_H

#include <Adafruit_ADS1X15.h>

extern Adafruit_ADS1115 ads1;
extern Adafruit_ADS1115 ads2;

void setupADC()
{
    Serial.println("Initializing ADS1115 instances...");
    if (!ads1.begin(0x48))
    {
        Serial.println("Failed to initialize ADS1.");
        while (1);
    }
    if (!ads2.begin(0x49))
    {
        Serial.println("Failed to initialize ADS2.");
        while (1);
    }
    ads1.setGain(GAIN_ONE); // Gain of 1, +/- 4.096V
    ads2.setGain(GAIN_ONE); // Gain of 1, +/- 4.096V
}

float readADC(Adafruit_ADS1115 &ads, int channel)
{
    int16_t adcValue = ads.readADC_SingleEnded(channel);
    float volts = ads.computeVolts(adcValue);
    Serial.print("ADC Channel "); Serial.print(channel); Serial.print(": "); Serial.print(adcValue); Serial.print(" "); Serial.print(volts, 3); Serial.println(" V");
    return volts;
}

#endif