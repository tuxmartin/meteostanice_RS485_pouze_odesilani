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
from time import sleep # cekani pred ctenim z meteostanice (aby DHT22 melo cas provest mereni)
from time import strftime # vypis aktualniho data a casu
import serial
from math import log # pro rosny bod
from math import e # pro rosny bod

import urllib # posilani na server (REST client)
import urllib2 # posilani na server (REST client)

url = 'http://localhost/meteo/'

frekvenceMereni = 10 # [s]

MODBUS_3  = 3  # Read holding registers
MODBUS_16 = 16 # Write multiple registers
STUPEN    = u'\u00b0' # znak pro stupen

def relativniTlak(absolutniTlak, teplota, nadmorskaVyska):
	# http://forum.amaterskameteorologie.cz/viewtopic.php?f=9&t=65#p1968
	g = 9.80665
	return round( (((absolutniTlak * g * nadmorskaVyska) / (287 * (273 + teplota) + (nadmorskaVyska / 400))) + absolutniTlak), 1 )


def rosnyBod(teplota, vlhkost):
	if (teplota > 0): # https://github.com/MultiTricker/TMEP/blob/a14a69ee1a17eb087808e8fa0456069828cfa509/app/scripts/fce.php#L282
		return round(243.12*((log(vlhkost/100)+((17.62*teplota)/(243.12+teplota)))/(17.62-log(vlhkost/100)-((17.62*teplota)/(243.12+teplota)))), 1)
	else:
		return round(272.62*((log(vlhkost/100)+((22.46*teplota)/(272.62+teplota)))/(22.46-log(vlhkost/100)-((22.46*teplota)/(272.62+teplota)))), 1)

	""" - nefunguje
	# http://www.klusik.cz/cs/article/2-mraky-nad-nami-jak-na-vypocet-rosneho-bodu
	Tdp = (243.5 * log10 ( (vlhkost/100) * e **((17.67*teplota)/(243.5*teplota)) )) / ( 17.67 - log10 ( (vlhkost/100) * e**( (17.67*teplota)/(243.5+teplota) ) )  )
	print Tdp
	"""

def main():
	print "---------------------------------------"
	print "Aktualni cas: ",  strftime("%Y-%m-%d %H:%M:%S")

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
	dev1.write_register(1, zmerData, 0, MODBUS_16) # MEASURE - 1
	sleep(1) # 500ms  - mereni DHT22 trva asi 250ms

	# Precteme namerena data:
	data = dev1.read_registers(0, 9, MODBUS_3)
	print "RAW data: ", data
	"""
	  DEVICE_VER,  	0
	  MEASURE,	1
	  BMP085_ERROR,	2
	  DHT22_ERROR,	3
	  DHT22_TEMP,  	4
	  DHT22_HUM,   	5
	  BMP085_TEMP,	6   	     
	  BMP085_PRES,	7
	  LED_STATE,	8
	"""
	DEVICE_VER	= data[0]
	BMP085_ERROR	= data[2]
	DHT22_ERROR	= data[3]
	DHT22_TEMP_VAL  = data[4] / 10.0
	DHT22_HUM_VAL   = data[5] / 10.0
	BMP085_TEMP_VAL = data[6] / 10.0
	BMP085_PRES_VAL = data[7]
	LED_STATE       = data[8]

	relTlak = relativniTlak(BMP085_PRES_VAL, BMP085_TEMP_VAL, 287) # Jicin ma podle Googlu 287mnm
	rBod = rosnyBod(DHT22_TEMP_VAL, DHT22_HUM_VAL)

	print ""
	print "DHT22: ", DHT22_TEMP_VAL,  STUPEN, "C, ",  DHT22_HUM_VAL, "%, rosny bod ", rBod,  STUPEN, "C" 
	print "BMP085 ", BMP085_TEMP_VAL, STUPEN, "C, ", BMP085_PRES_VAL, "hPa (absolutni), ",  relTlak , "hPa (relativni)"
	print ""

	params = urllib.urlencode({ # napleneni dat k odeslani
	  'DEVICE_VER'		: DEVICE_VER,
	  'BMP085_ERROR'	: BMP085_ERROR,
	  'DHT22_ERROR'		: DHT22_ERROR,
	  'DHT22_TEMP_VAL'	: DHT22_TEMP_VAL,
	  'DHT22_HUM_VAL'	: DHT22_HUM_VAL,
	  'DHT22_ROSNY_BOD'	: rBod,
	  'BMP085_TEMP_VAL'	: BMP085_TEMP_VAL,
	  'BMP085_PRES_VAL'	: BMP085_PRES_VAL,
	  'BMP085_REL_TLAK'	: relTlak
	})
	# DHT22_ROSNY_BOD=12.2&BMP085_PRES_VAL=986&DHT22_HUM_VAL=54.0&BMP085_REL_TLAK=1018.81084798&BMP085_TEMP_VAL=21.7&DHT22_TEMP_VAL=21.9
	try:
	    response = urllib2.urlopen(url, params, timeout = 5).read() # poslani na server a precteni odpovedi
	except urllib2.URLError, e:
	    print "Nepovedlo se poslat data na server!"

	# https://docs.python.org/3.3/faq/programming.html#is-there-an-equivalent-of-c-s-ternary-operator   [on_true] if [expression] else [on_false]
	print "Stav LED - ", LED_STATE, " - ", ("svitila - VYPINAM" if LED_STATE else "nesvitila - ZAPINAM")

	if LED_STATE:
		LED_STATE=0
	else:
		LED_STATE=1

	dev1.write_register(8, LED_STATE, 0, MODBUS_16)


	print "---------------------------------------"

while 1:
	main()
	sleep(frekvenceMereni)

