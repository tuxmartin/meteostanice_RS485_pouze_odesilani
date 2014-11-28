#include <modbus.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <string.h>

#include <curl/curl.h>

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

int poslatNaServer(char *POSTstring, char *serverURL, int timeoutS, int verboseOutput);

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

int DEVICE_VER				= data[0];
int DHT22_ERROR				= data[3];
int BMP085_ERROR			= data[2];
float DHT22_TEMP_VAL	= data[4] / 10.0;
float DHT22_HUM_VAL		= data[5] / 10.0;
float DHT22_ROSNY_BOD	= rosnyBod( DHT22_TEMP_VAL, DHT22_HUM_VAL); // float rosnyBod(float teplota, float vlhkost)
float BMP085_TEMP_VAL	= data[6] / 10.0;
float BMP085_PRES_VAL	= data[7]; // Pa - pro hPa *10 !!!
float BMP085_REL_TLAK	= relativniTlak( BMP085_PRES_VAL, BMP085_TEMP_VAL, NADMORSKA_VYSKA); // float relativniTlak(float absolutniTlak, float teplota, float nadmorskaVyska)

	printf("DHT22:   %.2f °C, %.2f %, rosny bod  %.2f °C\n", DHT22_TEMP_VAL, DHT22_HUM_VAL, DHT22_ROSNY_BOD);
	printf("BMP085:  %.2f °C, %.2f hPa (absolutni), %.2f hPa (relativni)\n", BMP085_TEMP_VAL, BMP085_PRES_VAL, BMP085_REL_TLAK);
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
	//char dataToSend[]="VGhpcyBpcyBqdXN0IGEgbWVzc2FnZSwgaXQncyBub3QgdGhlIHJlYWwgdGhpbmc=";
	char dataToSend[255];
	char server[]= "http://localhost:8765/skript.php";

	// BMP085_ERROR=0&DEVICE_VER=100&DHT22_ROSNY_BOD=8.5&BMP085_PRES_VAL=983&BMP085_TEMP_VAL=20.6&DHT22_HUM_VAL=45.1&DHT22_ERROR=0&BMP085_REL_TLAK=1015.8&DHT22_TEMP_VAL=20.9
	sprintf(dataToSend, "DEVICE_VER=%d&DHT22_ERROR=%d&BMP085_ERROR=%d&DHT22_TEMP_VAL=%.2f&DHT22_HUM_VAL=%.2f&DHT22_ROSNY_BOD=%.2f&BMP085_TEMP_VAL=%.2f&BMP085_PRES_VAL=%.2f&BMP085_REL_TLAK=%.2f\n\0", \
		DEVICE_VER, DHT22_ERROR, BMP085_ERROR, DHT22_TEMP_VAL, DHT22_HUM_VAL, DHT22_ROSNY_BOD, BMP085_TEMP_VAL, BMP085_PRES_VAL, BMP085_REL_TLAK);

	printf("Data na server: %s", dataToSend);
	int status = poslatNaServer(dataToSend, server, 3, 0);

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

int poslatNaServer(char *POSTstring, char *serverURL, int timeoutS, int verboseOutput) {
	/* Poslani dat na webserver pomoci libcurl:
		http://stackoverflow.com/q/7850716/1974494
		http://stackoverflow.com/a/10651038/1974494
	*/
	CURL *curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_VERBOSE, verboseOutput); // 0 / 1
	//curl_easy_setopt(curl, CURLOPT_URL, "http://www.example.com/hello-world");
	curl_easy_setopt(curl, CURLOPT_URL, serverURL);
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	//curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322; .NET CLR 2.0.5");
	//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "foo=bar&foz=baz");
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, POSTstring);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(POSTstring));
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutS);
	curl_easy_perform(curl);

	curl_easy_cleanup(curl);

	return 0;
}


// # apt-get install libmodbus-dev libmodbus5
// $ gcc -std=c99 -O2 -Wall -I /usr/include/modbus test.c -o test -L/usr/lib/modbus -L/usr/include/curl -lmodbus -lm -lcurl
// $ nc -l localhost 8765

