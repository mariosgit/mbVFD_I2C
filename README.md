# mbVFD_I2C
VFDs controled with ATTiny refresh controllers, accessable via I2C bus.

Firmware for the ATTiny84 is in the examples/FW_ATTiny84_MAX692x

![pinout](/images/AT84pinout.jpg)
![register](/images/AT84register.jpg)

### Usage example
 in Display5x7Segments - this is copied from my DCF77 clock might not work as is. You just fill the buffer with 5 digits to show and call the displayBufferI2C function which will copy it to the Tiny.

## How it works
The ATTiny needs at least the size information in the register 0x03 and the data in the following.

### registers:
0x00 buttons
0x01 enc_count
0x02 brightnes
0x03 datasize = (bytes per gate) << 5 | (#gates)   // 32 gates mit bis zu 8 byte -> 256
0x04.. data...

The tiny will cycle over all the gates and sends out the specified amount of bytes. So you can store bytes per gate in the data area 0x04++, max 256 bytes of data. Do not write more then 16bytes at once! This is a limitation of the i2c implementation.

 The information about which gate of your display is on or off still needs to be coded in your bytes, because the ATTiny will not know the bit to pin connections of your VFD. It simply dumps out some bytes over SPI, pulls the latch pin of the MAX692x and over again.