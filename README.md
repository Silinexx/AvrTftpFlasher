# Atmega328p TFTP flasher
Atmega328p firmware updater over TFTP protocol
---------
#### 3rd party dependencies:
Please install the following tools before build.
  - avr-gcc
  - avrdude

#### Build:
Build this boot-loader and upload it to Atmega328p using commands:
  - make clean
  - make all
  - make upload 
You can skip last step and upload boot-loader to MCU by using your own favorite programmer. In this case output file name is 'TftpFlasher.hex' and it is located under bin directory. In this version 'make upload use USB-ASP programmer to flash the MCU.

#### Usage:
  - Connect enc28j60 chip over SPI.
  - Connect Ethernet to enc28j60.
  - Device should response to pinging 192.168.1.10 IP address.
  - Upload firmware to MCU over TFTP (eg. tftp 192.168.1.10 -m binary -c put example_fw.bin)*

*Please note that only BINARY format is accepted and convert firmware to this format before start TFTP tool. You can do this on-line on http://matrixstorm.com/avr/hextobin/ihexconverter.html or locally by usage Hex2Bin converter. Source code of one of this king of tools can be found here http://hex2bin.sourceforge.net/.

#### Required hardware:
  - Board with Atmega328p (Arduino or similar)
  - Adapter with enc28j60
  - AVR programmer (upload boot-loader once).