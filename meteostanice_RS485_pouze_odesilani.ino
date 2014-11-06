#include <SimpleModbusSlave.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

#define DHTPIN 7
#define DHTTYPE DHT22   // DHT 22  (AM2302)
DHT dht(DHTPIN, DHTTYPE); // DHT22 - vlhkost

Adafruit_BMP085 bmp; // BMP085 - tlak

#define SLAVE_ID 2

#define DEVICE_VER_CONST    100

#define  ledPin  13 // onboard led

boolean BMP085ok = false;

//////////////// registers of your slave ///////////////////
enum 
{  
  DEVICE_VER,  
  MEASURE,
  BMP085_ERROR,
  DHT22_ERROR,
  DHT22_TEMP,  
  DHT22_HUM,   
  BMP085_TEMP,        
  BMP085_PRES,
  LED_STATE,
  HOLDING_REGS_SIZE // leave this one
};
unsigned int holdingRegs[HOLDING_REGS_SIZE]; // function 3 and 16 register array
//////////////// registers of your slave ///////////////////

void setup () {
  holdingRegs[DEVICE_VER] = DEVICE_VER_CONST;
  // SERIAL_8N2: 1 start bit, 8 data bits, 2 stop bits
  modbus_configure(&Serial, 19200, SERIAL_8N2, SLAVE_ID, 2, HOLDING_REGS_SIZE, holdingRegs);
  
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  /* ####### BMP085 ####### */
  for (byte i=1; i<=5; i++) { // 5x zkusime inicializovat cidlo (celkem se 5s) //FIXME: asi zbytecne cekat, pokud neodpovi porve, jit asi dal - zkusit!
    if (!bmp.begin()) {
        holdingRegs[BMP085_ERROR] = 1;
        BMP085ok = false;
  	//Serial.println("Could not find a valid BMP085 sensor, check wiring!");
  	delay(1000); // [ms]
    } else {
      holdingRegs[BMP085_ERROR] = 0;
      BMP085ok = true;
      break;
    }
  }
  /* ####### BMP085 ####### */
  
  merit();
}

void loop () {  
  modbus_update();

  if (holdingRegs[MEASURE] == 1) { // Mam merit?	  
    merit();
    holdingRegs[MEASURE] = 0;
    serialFlushBuffer(); // vyprazdime buffer - to bude delat bordel, pokud budou aktualne prichazet data...
  }
  // read the LED_STATE register value and set the onboard LED high or low with function 16
  if (holdingRegs[LED_STATE] == 1) {	  
    digitalWrite(ledPin, HIGH);
    holdingRegs[LED_STATE] = 1;
  } else {
    digitalWrite(ledPin, LOW);
    holdingRegs[LED_STATE] = 0;
  }
  
}

void merit() {
  /* ####### DHT22 ####### */   
  dht.begin();   
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();  
    
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h)) {    
    holdingRegs[DHT22_ERROR] = 1; // 1 = DHT22_read_fail
  } else { 
    holdingRegs[DHT22_ERROR] = 0;
    holdingRegs[DHT22_TEMP] = t*10;
    holdingRegs[DHT22_HUM] = h*10;
  }
  /* ####### DHT22 ####### */

  /* ####### BMP085 ####### */
  if (BMP085ok) { // pokud bylo cidlo inicializovano
    holdingRegs[BMP085_ERROR] == 0;
    holdingRegs[BMP085_TEMP] = int( bmp.readTemperature() * 10 );
    holdingRegs[BMP085_PRES] = ( bmp.readPressure() / 100);
  }
  /* ####### BMP085 ####### */  
}

void serialFlushBuffer() {
  while (Serial.read() >= 0) {
    // do nothing
  }
}
