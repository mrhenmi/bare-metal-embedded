#include <stdint.h>
#define byte uint8_t
// pin registers

/*
The DDxn bit in the DDRx register selects the direction of this pin. If DDxn is written logic one, Pxn is configured as an output
pin. If DDxn is written logic zero, Pxn is configured as an input pin.
If PORTxn is written logic one when the pin is configured as an input pin, the pull-up resistor is activated. To switch the
pull-up resistor off, PORTxn has to be written logic zero or the pin has to be configured as an output pin. The port pins are
tri-stated when reset condition becomes active, even if no clocks are running.
If PORTxn is written logic one when the pin is configured as an output pin, the port pin is driven high (one). If PORTxn is
written logic zero when the pin is configured as an output pin, the port pin is driven low (zero).
*/

#define DDRD *((volatile byte *) 0x2A) // The Port D Data Direction Register // 1 output 0 input
// port d register
// NOTE(Mark): arduino call the in pin0 but the chip calls it pd0
// where are using the arduino naming convention
#define PORTD *((volatile byte *) 0x2B) // port d register location
#define PIN0 (1 << 0) // pin 0 bit in port d register
#define PIN1 (1 << 1) // pin 1 bit in port d register
#define PIN2 (1 << 2) // pin 2 bit in port d register
#define PIN3 (1 << 3) // pin 3 bit in port d register
#define PIN4 (1 << 4) // pin 4
#define PIN5 (1 << 5)
#define PIN6 (1 << 6)
#define PIN7 (1 << 7)

#define DDRB *((volatile byte *) 0x24) // 1 output 0 input
//port b register
#define PORTB *((volatile byte *) 0x25)  // port b register locatin
#define PIN8 (1 << 0)
#define PIN9 (1 << 1)
#define PIN10 (1 << 2)
#define PIN11 (1 << 3)
#define PIN12 (1 << 4)
#define PIN13 (1 << 5)