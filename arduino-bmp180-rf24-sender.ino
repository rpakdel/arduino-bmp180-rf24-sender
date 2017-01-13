// reads data from BMP180 (I2C) and sends it to a base station via NRF24L01
// base station must have the same pta.h file compiled in

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

#include <RF24_config.h>
#include <nRF24L01.h>
#include <SPI.h>
#include <RF24.h>

#include "pta.h"

RF24 radio(9, 10);
static const int senderAdd[2] = { 0xF0F0F0F0AA, 0xF0F0F0F066 };
static const int sender = 0;

Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

void setup() 
{
    Serial.begin(9600);
    Serial.print(F("SENDER PIPE "));
    Serial.println(sender + 1);

    radio.begin();
    radio.setPALevel(RF24_PA_LOW);
    radio.setRetries(15, 15);
    radio.openWritingPipe(senderAdd[sender]);
    radio.stopListening();

    if (!bmp.begin())
    {
        Serial.print("Ooops, no BMP085 detected ... Check your wiring or I2C ADDR!");
    }
}

void displayValue(byte value)
{    
    Serial.print(F("Pipe "));
    Serial.print(sender + 1);
    Serial.print(F(", Value: "));
    Serial.print(value);
    Serial.print(" ... ");
}

byte value = 0;
void loop()
{
    sensors_event_t event;
    bmp.getEvent(&event);

    if (event.pressure)
    {
        PTA pta;

        pta.pressure = event.pressure;
        float temperature;
        bmp.getTemperature(&temperature);
        pta.temperature = temperature;
        // mean sea level pressure in hpa from 
        // http://vancouver.weatherstats.ca/charts/pressure_sea-2weeks.html
        float seaLevelPressure = 1025.2;
        pta.altitude = bmp.pressureToAltitude(
            seaLevelPressure,
            pta.pressure,
            pta.temperature);

        PrintPTA(pta, Serial);

        if (!radio.write(&pta, sizeof(PTA)))
        {
            Serial.println("No Ack");
        }
    }

    delay(10000);
}
