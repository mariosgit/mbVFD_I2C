/**
 * Example sketch for writing to and reading from a slave in transactional manner
 *
 * On write the first byte received is considered the register addres to modify/read
 * On each byte sent or read the register address is incremented (and it will loop back to 0)
 *
 * You can try this with the Arduino I2C REPL sketch at https://github.com/rambo/I2C/blob/master/examples/i2crepl/i2crepl.ino 
 * If you have bus-pirate remember that the older revisions do not like the slave streching the clock, this leads to all sorts of weird behaviour
 *
 * To read third value (register number 2 since counting starts at 0) send "[ 8 2 [ 9 r ]", value read should be 0xBE
 * If you then send "[ 9 r r r ]" you should get 0xEF 0xDE 0xAD as response (demonstrating the register counter looping back to zero)
 *
 * You need to have at least 8MHz clock on the ATTiny for this to work (and in fact I have so far tested it only on ATTiny85 @8MHz using internal oscillator)
 * Remember to "Burn bootloader" to make sure your chip is in correct mode 
 */

/* MB VFD: this shifts out the values of internal register, to use with MAX69[23]x chips
 * 
 * ATTiny84 pinout
 * 
 * 
 * 
 */

/* MB VFD usage notes... (BusPirat I2C mode)
 * Change PWM:             [ 8 2 128 ]   // chip-adr, register-adr, value
 * Read Button, Encoder:   [ 8 0 [ 9 r r ]   // chip-adr, register-adr, value
*/

#define I2C_SLAVE_ADDRESS 0x4 // the 7-bit address (remember to change this when adapting this example)
// Get this from https://github.com/rambo/TinyWire
#include <TinyWireS.h>
// The default buffer size, Can't recall the scope of defines right now
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 20 )
#endif

#include <ClickEncoder.h>
ClickEncoder encoder(10, 9, 5);

const byte pinLED = 0;
const byte pinMOSI = 1;
const byte pinSCLK = 2;
const byte pinLOAD = 3;

const byte pinPWM  = 7;
const byte pinPWM_CLK  = 8;

// buttons
// enc_count
// brightnes
// [3] datasize = (bytes per gate) << 3 | (#gates)        // 32 gates mit bis zu 8 byte -> 128
// data...
volatile uint8_t i2c_regs[8*32] =
{
    0xDE, 0xAD, 0xBE, 12, 
    0xCE, 0x9D, 0xAE, 0xDF, 
    0xBE, 0x8D, 0x9E, 0xCF, 
    0xAE, 0x7D, 0x8E, 0xBF, 
};


volatile byte reg_position;
void requestEvent()
{  
    TinyWireS.send(i2c_regs[reg_position]);
    if(reg_position == 0 || reg_position == 1)
      i2c_regs[reg_position] = 0;
    
    // Increment the reg position on each read, and loop back to zero
    reg_position = (reg_position+1) % sizeof(i2c_regs);
}

/**
 * The I2C data received -handler
 *
 * This needs to complete before the next incoming transaction (start, data, restart/stop) does 
 */
void receiveEvent(uint8_t howMany)
{
    if (howMany < 1)
    {
        // Sanity-check
        return;
    }
    if (howMany > TWI_RX_BUFFER_SIZE)
    {
        // Also insane number
        return;
    }

    reg_position = TinyWireS.receive();
    howMany--;
    if (!howMany)
    {
        // This write was only to set the buffer for next read
        return;
    }
    while(howMany--)
    {
        i2c_regs[reg_position%sizeof(i2c_regs)] = TinyWireS.receive();
        reg_position++;
    }
}


void setup()
{
    // TODO: Tri-state this and wait for input voltage to stabilize 
    pinMode(pinLED, OUTPUT); // OC1B-, Arduino pin 3, ADC
    digitalWrite(pinLED, LOW); // Note that this makes the led turn on, it's wire this way to allow for the voltage sensing above.

    //pinMode(1, OUTPUT); // OC1A, also The only HW-PWM -pin supported by the tiny core analogWrite
    pinMode(pinMOSI, OUTPUT);
    pinMode(pinSCLK, OUTPUT);
    pinMode(pinLOAD, OUTPUT);
    
    pinMode(pinPWM, OUTPUT);
    pinMode(pinPWM_CLK, OUTPUT);
    analogWrite(pinPWM_CLK, 127);

    /**
     * Reminder: taking care of pull-ups is the masters job
     */

    TinyWireS.begin(I2C_SLAVE_ADDRESS);
    TinyWireS.onReceive(receiveEvent);
    TinyWireS.onRequest(requestEvent);

    
    // Whatever other setup routines ?
    
    digitalWrite(pinLED, HIGH);
}

// run between 250 and 1000 Hz / ever 4 milliSeconds
byte gateNr = 0;
void displayBuffer()
{
  byte bytesPerGate = i2c_regs[3];
  byte gates = bytesPerGate & 0x1f;
  bytesPerGate = bytesPerGate >> 5;

  gateNr = gateNr % gates;
  
  for(byte i = 0; i < bytesPerGate; i++)
  {
    shiftOut(pinMOSI, pinSCLK, MSBFIRST, i2c_regs[gateNr * bytesPerGate + i + 4]);
  }
  digitalWrite(pinLOAD, HIGH);
  digitalWrite(pinLOAD, LOW);

  gateNr++;
}

void loop()
{
    /**
     * This is the only way we can detect stop condition (http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=984716&sid=82e9dc7299a8243b86cf7969dd41b5b5#984716)
     * it needs to be called in a very tight loop in order not to miss any.
     * It will call the function registered via TinyWireS.onReceive(); if there is data in the buffer on stop.
     */
    TinyWireS_stop_check();
    displayBuffer();
    analogWrite(pinPWM, i2c_regs[2]);

    encoder.service();
    i2c_regs[1] += encoder.getValue();
    i2c_regs[0]  = encoder.getButton();
}

