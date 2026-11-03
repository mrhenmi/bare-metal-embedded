# Bare-Metal Embedded C (AVR + STM32)

Two small self-study projects that program microcontrollers in C with no Arduino core.

*Originally written Sep–Oct 2022. Uploaded to GitHub in Nov 2026. The first commit is the original code; a later commit fixes a bug I found while reviewing it (see below).*

## `avr-7segment/`: ATmega328P (Arduino Uno), no Arduino core
- Counts 0–9 on a 7-segment display wired to port D (PD0–PD6).
- **Register definitions from the datasheet:** `arduino_rev3.h` defines `DDRD`, `PORTD`, `DDRB` and `PORTB` myself, as `volatile` pointers to their I/O addresses (0x2A, 0x2B, 0x24, 0x25). I didn't use `<avr/io.h>`.
- **Display logic:** `display.c` sets the pin directions, then writes a segment pattern for each digit.
- **Build and flash** with the bare toolchain (`build.bat` / `load.bat`):
  ```
  avr-gcc -mmcu=atmega328p display.c -o display.out
  avr-objcopy -O ihex display.out display.hex
  avrdude -patmega328p -carduino -PCOM5 -Uflash:w:display.hex:i
  ```
  The result is a 316-byte firmware image (with `-Os`).
- **Known weakness:** the delay is a busy-wait loop. A hardware timer would be the right fix.

## `stm32-uart-button/`: STM32F030R8 (Nucleo-F030R8, Cortex-M0)
This builds on the chapter 10 serial-I/O example from *Bare Metal C* by Steve Oualline (No Starch Press). The book provides the UART setup and the interrupt-driven, circular-buffer **transmit** path, including the `debugEvent` log.

**What I added (programming problems 10.p / 10.p2):**
- **Interrupt-driven receive:** in `USART2_IRQHandler`, when `RXNE` is set, the byte goes into a separate receive ring buffer. When that buffer fills, the RX interrupt is disabled (`RXNEIE`) so no data is overwritten.
- **Reading a line of input:** code that pulls characters from the receive buffer until Enter (`0x0D`).
- **User-button interrupt:** the Nucleo's B1 button is set up as a rising-edge EXTI interrupt and enabled with `NVIC_EnableIRQ(EXTI4_15_IRQn)`. Each press advances `global_click`, and the main loop sends a different message.

**Bug fix (2026 commit):**
- The EXTI handler tested `EXTI->PR | EXTI_PR_PR13_Msk`, which is always true. It should be `&`.
- It also cleared the flag with `|=`. On a write-1-to-clear register that can clear other pending lines too. It now writes only the line 13 bit.

**Build:**
- It's a System Workbench for STM32 (Ac6) project; open the folder in SW4STM32.
- ST's HAL and CMSIS (`HAL_Driver/`, `CMSIS/`, `Utilities/`) aren't included because they're about 17 MB of vendor code. They come from STM32CubeF0, or from the project wizard when you create a new Nucleo-F030R8 project.
- With them in place it builds with arm-none-eabi-gcc: text 6636 B, data 1240 B, bss 3372 B.
- The startup file, linker script, `system_stm32f0xx.c` and `syscalls.c` are ST/Ac6 template files.

## Concepts practiced
- Memory-mapped I/O and `volatile`
- Reading pin and register maps from datasheets
- Interrupt service routines, NVIC enabling, and EXTI pending flags
- Ring buffers shared between an ISR and the main loop
- The bare toolchain: compile, objcopy, flash
