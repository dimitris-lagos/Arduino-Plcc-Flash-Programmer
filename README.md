# Arduino-Plcc-Flash-Programmer
##### Arduino Plcc Flash Programmer
-WORK IN PROGRESS-
-Needs only 6 wires/connections to Arduino. Makes use of the LPC protocol.
# Update version 0.1b
##### - Added: Program flashrom from bin file in  sd card.
#####  - Added: Save flashrom to file in sd card . 

At the moment tested and works with:
- SST49LF020A
- Pm49FL002
- SST49LF040

Can read data of specific address, read whole flash, clear block, clear all flash, program flash rom from file, save flashrom to file and display Manufacturer and Device id.
It's reaaaaaally slow (10minutes to read 256k flash), because it's using 1/2Mhz clock instead of 33Mhz and then 115200baud uart to send the data to the host. When it's done reading the flash, copy and paste the serial output to HxD and you have successfully made a backup of your flash.

#TODO
~Header files.
~Structs for different flash devices / ~Maybe use the database of flashrom.
~Wrapper functions for structured "database".
~Maybe give a try @ VUSB instead of using serial.
~Implement buffers to load firmware to program the flash.
