# Adafruit_SHT4x_Environment_Logger
Environment Logger for Adafruit SHT4x Trinkey with Dew point current, max and average
Dew point max and average calculated over 3 minutes samples.

- Temperature °C
- Relative Humidity %
- Dew Point Current °C
- Dew Point Average °C
- Dew Point Maximum °C
- Vapour-Pressure Deficit kPa
- Serial number of the Trinkey
- Customizable Name
  
The low complexity makes it ideal for integration in any software via serial connection.
Main use case is sub-ambient cooling systems.

Sketch is available for customization or just use the pre-compiled uf2 archive.

## How to install:
- Double click on the reset button on the Trinkey to enter bootloader mode
- Drop on the Trinkey drive the uf2 file
- Connect via serial

## Commands
Two commands can be sent via serial, followed by return:
- `n` or `N`; set a new name, max 100 chars eg. `bAMBIENT_EXTERNAL\n`
- `r` or `R`; soft reboot the Trinkey eg. `r\n`
