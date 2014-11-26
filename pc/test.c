#include <modbus.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

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

int main(void) {

	modbus_t *ctx = NULL;
	modbus_mapping_t *mb_mapping = NULL;
	uint16_t tab_reg[64]; // pouziva se jen 9
	uint16_t* zapis[1];
	uint16_t* test[1];
	int rc;
	int i;
	ctx = modbus_new_rtu("/dev/ttyUSB0", 19200, 'N', 8, 2);
	modbus_set_slave(ctx, 2);
	modbus_connect(ctx);

	mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                    MODBUS_MAX_READ_REGISTERS, 0);


	if (mb_mapping == NULL) {
		fprintf(stderr, "Failed to allocate the mapping: %s\n",
				modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

	//int modbus_write_registers(modbus_t *ctx, int addr, int nb, const uint16_t *src);

	zapis[0] = 1;
	rc = modbus_write_registers(ctx, 1, 1, zapis );

	if (rc == -1) {
		fprintf(stderr, "%s\n", modbus_strerror(errno));
		return -1;
	}
	
	//int usleep(useconds_t usec);
	sleep(1);
	//	usleep(500000); // 500ms

	rc = modbus_read_registers(ctx, 0, 9, tab_reg); // read a register value from modbus object into value 'rc'

	if (rc == -1) {
		fprintf(stderr, "%s\n", modbus_strerror(errno));
		return -1;
	}

	for (i=0; i < rc; i++) {
		printf("tab_reg[%d]=%d (0x%X)\n", i, tab_reg[i], tab_reg[i]);
	}

	if (tab_reg[8] == 0) {
		test[0] = 1;
		printf("nesviti\n");
	}
	if (tab_reg[8] == 1) {
		test[0] = 0;
		printf("sviti\n");
	}

	sleep(1); // pokud se neceka, nefunguje to!!! TODO: overit potrebny cas
	//test[0] = 1;
	rc = modbus_write_registers(ctx, 8, 1, test ); // (uint16_t *)0

	modbus_flush(ctx);
	modbus_close(ctx);
	modbus_free(ctx);

	return 0;
}

// # apt-get install libmodbus-dev libmodbus5
// $ gcc -O2 -Wall -I /usr/include/modbus test.c -o test -L/usr/lib/modbus -lmodbus


