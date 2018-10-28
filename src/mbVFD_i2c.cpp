#include <Wire.h>

#include "mbVFD_i2c.h"

namespace mbVFD_i2c {

    const byte numberTo7seg[] = { B1111110, B0110000, B1101101, B1111001,
                                  B0110011, B1011011, B1011111, B1110000, 
                                  B1111111, B1111011, B1110111, B0011111,
                                  B1001110, B0111101, B1001111, B1000111 };

    byte buffer[5] = {0,1,2,3,4};

    void displayBufferI2C()
    {
        Wire.beginTransmission(4);

        Wire.write(3); // register start adr
        Wire.write( (2<<5)|5 ); // per gate data length (bytes) << 5 | digit/gate cnt
        byte displayDigit = 0;
        int16_t gate = (1 << displayDigit);
        int16_t out = (numberTo7seg[buffer[displayDigit]] << 5) | gate;
        Wire.write(byte(out>>8));
        Wire.write(byte(out&0xff));
        displayDigit++;
        gate = (1 << displayDigit);
        out = (numberTo7seg[buffer[displayDigit]] << 5) | gate;
        Wire.write(byte(out>>8));
        Wire.write(byte(out&0xff));
        displayDigit++;
        gate = (1 << displayDigit);
        out = (numberTo7seg[buffer[displayDigit]] << 5) | gate;
        Wire.write(byte(out>>8));
        Wire.write(byte(out&0xff));
        displayDigit++;
        gate = (1 << displayDigit);
        out = (numberTo7seg[buffer[displayDigit]] << 5) | gate;
        Wire.write(byte(out>>8));
        Wire.write(byte(out&0xff));
        displayDigit++;
        gate = (1 << displayDigit);
        out = (numberTo7seg[buffer[displayDigit]] << 5) | gate;
        Wire.write(byte(out>>8));
        Wire.write(byte(out&0xff));

        Wire.endTransmission();
    }
}
