#include <modbus.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <math.h> 
// don't forget #include <math.h> and link with -lm.

#define NADMORSKA_VYSKA 287 // Jicin ma podle Googlu 287mnm

/*
  DEVICE_VER,  	0
  MEASURE,	1
  BMP085_ERROR,	2
  DHT22_ERROR,	3
  DHT22_TEMP,  	4
  DHT22_HUM,   	5
  BMP085_TEMP,	6   	     
  BMP085_PRES,	7
  LED_STATE,	8
*/

void sleepms(int n);

float relativniTlak(float absolutniTlak, float teplota, float nadmorskaVyska);

float rosnyBod(float teplota, float vlhkost);

int main(void) {
	modbus_t *ctx = NULL;
	modbus_mapping_t *mb_mapping = NULL;

	uint16_t data[64]; // pouziva se jen 9
	uint16_t* poslat[1];
	int rc = 0;

	ctx = modbus_new_rtu("/dev/ttyUSB0", 19200, 'N', 8, 2);
	if (ctx == NULL) {
		fprintf(stderr, "Unable to create the libmodbus context\n");
		return -1;
	}	

	modbus_set_slave(ctx, 2);
	modbus_connect(ctx);

	mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0, MODBUS_MAX_READ_REGISTERS, 0);
	if (mb_mapping == NULL) {
		fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

	//int modbus_write_registers(modbus_t *ctx, int addr, int nb, const uint16_t *src);

	poslat[0] = 1;
	rc = modbus_write_registers(ctx, 1, 1, poslat);
	if (rc == -1) {
		fprintf(stderr, "Write error: %s\n", modbus_strerror(errno));
		return -1;
	}
	
	//int usleep(useconds_t usec);
	sleepms(1000); // mereni trva v Arduinu asi 250ms, proto musime pred ctenim pockat

	rc = modbus_read_registers(ctx, 0, 9, data); // read a register value from modbus object into value 'rc'
	if (rc == -1) {
		fprintf(stderr, "%s\n", modbus_strerror(errno));
		return -1;
	}

	for (int i = 0; i < rc; i++) {
		//printf("data[%d]=%d (0x%X)\n", i, data[i], data[i]);
		printf("data[%d]=%d\n", i, data[i]);
	}

	if (data[8] == 0) {
		poslat[0] = 1;
		printf("nesviti\n");
	}
	if (data[8] == 1) {
		poslat[0] = 0;
		printf("sviti\n");
	}

	sleepms(50); // pokud se neceka, nefunguje to!!! TODO: overit potrebny cas
	rc = modbus_write_registers(ctx, 8, 1, poslat); // (uint16_t *)0
        if (rc == -1) {
                fprintf(stderr, "%s\n", modbus_strerror(errno));
                return -1;
        }

/*
	DEVICE_VER	    = data[0]
	BMP085_ERROR  	= data[2]
	DHT22_ERROR	    = data[3]
	DHT22_TEMP_VAL  = data[4] / 10.0
	DHT22_HUM_VAL   = data[5] / 10.0
	BMP085_TEMP_VAL = data[6] / 10.0
	BMP085_PRES_VAL = data[7] // Pa - pro hPa *10 !!!
	LED_STATE       = data[8]
	relTlak = relativniTlak(BMP085_PRES_VAL, BMP085_TEMP_VAL, 287) # Jicin ma podle Googlu 287mnm
	rBod = rosnyBod(DHT22_TEMP_VAL, DHT22_HUM_VAL)
*/

	float rosBod = rosnyBod( (float) data[4]/10, (float)data[5]/10); // float rosnyBod(float teplota, float vlhkost)
	float relTlak = relativniTlak( (float)data[7], (float)data[6]/10, NADMORSKA_VYSKA); // float relativniTlak(float absolutniTlak, float teplota, float nadmorskaVyska)
	printf("DHT22:   %f °C, %f %, rosny bod  %f °C\n", (float)data[4]/10, (float)data[5]/10, rosBod);
	printf("BMP085:  %f °C, %f hPa (absolutni), %f hPa (relativni)\n", (float)data[6]/10, (float)data[7], relTlak);
	printf("rosnyBod = %f\n", rosBod);
	printf("relativniTlak = %f\n", relTlak);
	if (data[8]) {
		printf("Stav LED -  1  -  svitila - VYPINAM\n");
	} else {
		printf("Stav LED -  0  -  NEsvitila - ZAPINAM\n");
	}

/*
---------------------------------------
Aktualni cas:  2014-11-27 01:31:46
RAW data:  [100, 0, 0, 0, 224, 490, 218, 983, 1]

DHT22:  22.4 ° C,  49.0 %, rosny bod  11.2 ° C
BMP085  21.8 ° C,  983 hPa (absolutni),  1015.7 hPa (relativni)

Nepovedlo se poslat data na server!
Stav LED -  1  -  svitila - VYPINAM
---------------------------------------
*/


/* Poslani dat na webserver pomoci libcurl:
	http://stackoverflow.com/q/7850716/1974494
	http://stackoverflow.com/a/10651038/1974494
*/

	modbus_flush(ctx);
	modbus_close(ctx);
	modbus_free(ctx);

	return 0;
}

void sleepms(int n) {
        usleep(n*1000);
}

float relativniTlak(float absolutniTlak, float teplota, float nadmorskaVyska) { // tlak v Pa
	// http://forum.amaterskameteorologie.cz/viewtopic.php?f=9&t=65#p1968
	float g = 9.80665;
	// return round( (((absolutniTlak * g * nadmorskaVyska) / (287 * (273 + teplota) + (nadmorskaVyska / 400))) + absolutniTlak), 1 );
	// http://stackoverflow.com/a/1344261
	float cislo = ( (((absolutniTlak * g * nadmorskaVyska) / (287 * (273 + teplota) + (nadmorskaVyska / 400))) + absolutniTlak) );
	return cislo;
}


float rosnyBod(float teplota, float vlhkost) {
	float cislo;
	if (teplota > 0) { // https://github.com/MultiTricker/TMEP/blob/a14a69ee1a17eb087808e8fa0456069828cfa509/app/scripts/fce.php#L282
		cislo = (243.12*((log(vlhkost/100)+((17.62*teplota)/(243.12+teplota)))/(17.62-log(vlhkost/100)-((17.62*teplota)/(243.12+teplota)))));
	} else {
		cislo = (272.62*((log(vlhkost/100)+((22.46*teplota)/(272.62+teplota)))/(22.46-log(vlhkost/100)-((22.46*teplota)/(272.62+teplota)))));
	}
	return cislo;
}


// # apt-get install libmodbus-dev libmodbus5
// $ gcc -std=c99 -O2 -Wall -I /usr/include/modbus test.c -o test -L/usr/lib/modbus -lmodbus -lm


