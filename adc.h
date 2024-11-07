#ifndef ADC_H
#define ADC_H

#include <Adafruit_ADS1X15.h>

void setupADC()
{
    Serial.println("Getting single-ended readings from AIN0");
    Serial.println("ADC Range: +/- 4.096V (1 bit = 0.125mV)");
    if (!ads.begin())
    {
        Serial.println("Failed to initialize ADS.");
        while (1);
    }
    ads.setGain(GAIN_ONE); // Gain of 1, +/- 4.096V
}

float readADC()
{
    int16_t adc0 = ads.readADC_SingleEnded(0);
    float volts0 = ads.computeVolts(adc0);
    Serial.print("AIN0: "); Serial.print(adc0); Serial.print(" "); Serial.print(volts0, 3); Serial.println(" V");
    return volts0;
}