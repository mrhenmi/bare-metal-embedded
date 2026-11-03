#include "arduino_rev3.h"

//NOTE(Mark):
#define F PIN0
#define G PIN1
#define E PIN2
#define D PIN3
#define C PIN4
#define B PIN5
#define A PIN6

 void displayNum(unsigned n)
{
    switch (n)
    {
        case(0):
        {
            PORTD = (A | B | C | D | E | F);
        } break;
        case(1):
        {
            PORTD = (B | C);
        } break;
        case(2):
        {
            PORTD = (A | B | G | E | D);
        } break;
        case(3):
        {
            PORTD = (A | B | G | C | D);
        } break;
        case(4):
        {
            PORTD = (F | G | B | C);
        } break;
        case(5):
        {
            PORTD = (A | F | G | C | D);
        } break;
        case(6):
        {
            PORTD = (A | F | E | D | C | G);
        } break;
        case(7):
        {
            PORTD = (A | B | C);
        } break;
        case(8):
        {
            PORTD = (A | B | C | D | E | G | F);
        } break;
        case(9):
        {
            PORTD = (A | B | C | D | G | F);
        } break;
        default:
        {
            // display a dash for error
            PORTD = G;
        }
    }

}

 void sleep(unsigned long x)
{
    for(long i = 0; i < x; i++)
    {
        PORTB = PIN13;
    }
}

int main()
{
    unsigned int count = 0;
    //setup
    DDRD = (A | B | C | D | E | G | F);
    DDRB = PIN13;
    /* Insert nop for synchronization*/
    //__no_operation();
    //run
    for(;;)
    {
        displayNum(count);
        sleep(1000000);
        count++;
        if (count > 9) count = 0;
    }
}