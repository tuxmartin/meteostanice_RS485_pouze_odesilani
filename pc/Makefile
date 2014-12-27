# build test executable when user executes "make"

# $ gcc -std=c99 -O2 -Wall -I/usr/include/modbus test.c -o test -L/usr/lib/modbus -L/usr/include/curl -lmodbus -lm -lcurl

CFLAGS=-std=c99 -O2 -Wall -I/usr/include/modbus 
LDFLAGS=-L/usr/lib/modbus -L/usr/include/curl -lmodbus -lm -lcurl

test: test.o
	$(CC) test.o $(LDFLAGS) -o test
test.o: test.c
	$(CC) $(CFLAGS) -c test.c

# remove object files and executable when user executes "make clean"
clean:
	rm *.o test
	

# !!! http://gargoyle-router.com/old-openwrt-coding.html

