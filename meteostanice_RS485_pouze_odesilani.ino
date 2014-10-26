#include <DHT.h>

#include <Wire.h>
#include <Adafruit_BMP085.h>

#define DHTPIN 7
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE); // DHT22 - vlhkost
Adafruit_BMP085 bmp; // BMP085 - tlak

long interval = 60;  // interval mereni [s]
uint32_t timer;

unsigned long count = 0;

void setup () {
  Serial.begin(9600); 

  /* ####### BMP085 ####### */
  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
	while (1) {}
  }
  /* ####### BMP085 ####### */
}

void loop () {  
  if (millis() > timer) {
    timer = millis() + (interval * 1000);
    
    merit();
    
  }
}

void merit() { 
  
  /* Data posilana do PC maji nasledujici format a zakoncena jsou <CR><LF>:
   *   3;DHT22;23.40C;49.80%;BMP085;22.60C;99581Pa;
   */
  
  //Serial.print(F("Mereni c. "));
  Serial.print(count);
  Serial.print(F(";"));

  
  /* ####### DHT22 ####### */   
  dht.begin();   
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();  
    
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h)) {    
    Serial.println(F("DHT22_read_fail"));
  } else { 
    Serial.print(F("DHT22;"));
    Serial.print(t);
    Serial.print(F("C;"));
    Serial.print(h);
    Serial.print(F("%"));
  }
  /* ####### DHT22 ####### */
  
  Serial.print(F(";"));
    
  /* ####### BMP085 ####### */
  Serial.print(F("BMP085;"));
  Serial.print(bmp.readTemperature());
  Serial.print(F("C;"));
  Serial.print(bmp.readPressure());
  Serial.print(F("Pa;"));
  /* ####### BMP085 ####### */
  
  Serial.println(F(""));

  count++;
}

