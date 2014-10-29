#!/usr/bin/env python

# https://pbs.twimg.com/media/B1ERDW9CcAELi4h.png:large


# -*- coding: utf-8 -*-
	# Podpora UTF-8 znaku v souboru.
	# Musi byt na 1., nebo 2. radku!

# http://minimalmodbus.sourceforge.net/internalminimalmodbus.html

# http://effbot.org/pyfaq/how-do-i-create-a-pyc-file.htm
# http://code.activestate.com/recipes/576704-python-code-minifier/
# python -m py_compile fileA.py fileB.py fileC.py

import minimalmodbus
import time
from time import sleep
import serial

MODBUS_3  = 3  # Read holding registers
MODBUS_16 = 16 # Write multiple registers
STUPEN    = u'\u00b0' # znak pro stupen

def relativniTlak(absolutniTlak, teplota, nadmorskaVyska):
	# http://forum.amaterskameteorologie.cz/viewtopic.php?f=9&t=65#p1968
	g = 9.80665
	return ((absolutniTlak * g * nadmorskaVyska) / (287 * (273 + teplota) + (nadmorskaVyska / 400))) + absolutniTlak


print "---------------------------------------"
print "Aktualni cas: ",  time.strftime("%Y-%m-%d %H:%M:%S")

"""
DEFAULT VALUES:
instrument.serial.port     = '/dev/ttyUSB1'     # this is the serial port name
instrument.serial.baudrate = 19200   # Baud
instrument.serial.bytesize = 8
instrument.serial.parity   = serial.PARITY_NONE
instrument.serial.stopbits = 1
instrument.serial.timeout  = 0.05   # seconds
instrument.address         = 2      # this is the slave address number
instrument.mode = minimalmodbus.MODE_RTU   # rtu or ascii mode
"""

"""
instrument.debug = True
"""
# SERIAL_8N2: 1 start bit, 8 data bits, 2 stop bits
dev1 = minimalmodbus.Instrument('/dev/ttyUSB1', 2) # port name, slave address (in decimal)
dev1.serial.baudrate = 19200
dev1.serial.bytesize = 8
dev1.serial.stopbits = 2
dev1.serial.parity   = serial.PARITY_NONE

"""
instrumentA = minimalmodbus.Instrument('/dev/ttyUSB1', 1)
instrumentB = minimalmodbus.Instrument('/dev/ttyUSB1', 2)
"""

"""
# registeraddress (int), numberOfDecimals (int), functioncode (int)
data  =  dev1.read_register(0, 1, MODBUS_3)

#registeraddress (int), numberOfRegisters (int), functioncode (int)
data = dev1.read_registers(0, 6, MODBUS_3)


#registeraddress (int), value (int or float), numberOfDecimals (int), functioncode (int)
dev1.write_register(5, ledStatus, 1, MODBUS_16)


#registeraddress (int), values (list of int)
dev1.write_registers(5, ledStatus)
"""

# Posleme prikaz k namereni dat (Arduino si je nameri a ulozi do RAM):
zmerData = 1
dev1.write_register(6, zmerData, 0, MODBUS_16)
sleep(1) # 500ms  - mereni trva asi 250ms

# Precteme namerena data:
data = dev1.read_registers(0, 6, MODBUS_3)
print "RAW data: ", data
"""
  DHT22_TEMP_VAL,  
  DHT22_HUM_VAL,   
  DHT22_ERROR,
  BMP085_TEMP_VAL,        
  BMP085_PRES_VAL,
  LED_STATE,
  MEASURE,
"""
DHT22_TEMP_VAL  = data[0] / 10.0
DHT22_HUM_VAL   = data[1] / 10.0
DHT22_ERROR     = data[2]
BMP085_TEMP_VAL = data[3] / 10.0
BMP085_PRES_VAL = data[4]
LED_STATE       = data[5]

relTlak = relativniTlak(BMP085_PRES_VAL, BMP085_TEMP_VAL, 287) # Jicin ma podle Googlu 287mnm

print "DHT22: ", DHT22_TEMP_VAL,  STUPEN, "C ",  DHT22_HUM_VAL, "%"
print "BMP085 ", BMP085_TEMP_VAL, STUPEN, "C ", BMP085_PRES_VAL, "hPa (absolutni) ",  "{:4.2f}".format(relTlak) , "hPa (relativni)"

# https://docs.python.org/3.3/faq/programming.html#is-there-an-equivalent-of-c-s-ternary-operator   [on_true] if [expression] else [on_false]
print "Stav LED - ", LED_STATE, " - ", ("svitila - VYPINAM" if LED_STATE else "nesvitila - ZAPINAM")

if LED_STATE == 1:
	LED_STATE=0
else:
	LED_STATE=1

#registeraddress (int), value (int or float), numberOfDecimals (int), functioncode (int)
#sleep(1) # 1s
dev1.write_register(5, LED_STATE, 0, MODBUS_16)



print "---------------------------------------"

