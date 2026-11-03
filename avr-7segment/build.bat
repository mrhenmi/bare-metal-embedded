@echo off
@REM 
avr-gcc -mmcu=atmega328p display.c -o display.out
avr-objcopy -O ihex display.out display.hex