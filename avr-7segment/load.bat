@REM -U memtype:op:filename[:format]
@REM memtype: ifuse, hfuse, efuse, lock, eeprom, flash, etc
@REM Op: r, w, v (read, write, verify)
@REM Path to file or hardcoded number
@REM Foramt: i, m, r, e, s, etc.. i= intel hex format m = immediate (hard coded)
avrdude -v -V -patmega328p -c arduino -P COM5 -Uflash:w:display.hex:i