
#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>

#include <mbVFD_i2c.h>

byte hour=0,minute=0,second=0;

void setup() {
    Serial.begin(115200);
    //while (!Serial)
    //  delay(10);     // will pause Zero, Leonardo, etc until serial console opens

    Serial.println("aaaaaaaaaaaaaaa");
    Wire.begin();
    Serial.println("bbbbbbbbbbbbb");
}


void clockUp()
{
    second++;
    if(second >= 60)
    {
        second = 0;
        minute++;
        if(minute >= 60)
        {
            minute = 0;
            hour++;
            if(hour >= 24)
            hour = 0;
        }
    }

    mbVFD_i2c::buffer[0] = hour/10;
    mbVFD_i2c::buffer[1] = hour%10;
    mbVFD_i2c::buffer[3] = minute/10;
    mbVFD_i2c::buffer[4] = minute%10;
    //buffer[3] = second/10;
    //buffer[4] = second%10;
    mbVFD_i2c::buffer[2] = second%2;  // this is the dot
}

// the loop routine runs over and over again forever:
void loop()
{
    clockUp();
    if (Serial.available() >= 6)
    {
        Serial.print("o:");
        // read the incoming byte:
        int hh = Serial.read();
        int hl = Serial.read();
        int mh = Serial.read();
        int ml = Serial.read();
        int sh = Serial.read();
        int sl = Serial.read();

        hh -= '0';
        hl -= '0';
        mh -= '0';
        ml -= '0';
        sh -= '0';
        sl -= '0';
        if( hh >= 0 && hh <= 2  &&  hl >= 0 && hl <= 9  &&
            mh >= 0 && mh <= 6  &&  ml >= 0 && ml <= 9  &&
            sh >= 0 && sh <= 6  &&  sl >= 0 && sl <= 9)
        {
            hour   = hh*10 + hl;
            minute = mh*10 + ml;
            second = sh*10 + sl;
            Serial.print(hour); Serial.print(",");
            Serial.print(minute); Serial.print(",");
            Serial.print(second);
        }
        else
        {
            Serial.println("Wrong time format, enter hhmmss");
        }
    }
    else
    {
        Serial.print(hour); Serial.print(",");
        Serial.print(minute); Serial.print(",");
        Serial.print(second);
        Serial.println(".");
    }
    
    mbVFD_i2c::displayBufferI2C();
    delay(1000);
}
