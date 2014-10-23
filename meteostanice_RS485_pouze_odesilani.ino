#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

#include <Wire.h>
#include <Adafruit_BMP085.h>

#define ONE_WIRE_PIN 4
#define DHTPIN 7
#define DHTTYPE DHT22   // DHT 22  (AM2302)

#define FW 100   // verze firmware

OneWire oneWire(ONE_WIRE_PIN);
DallasTemperature sensors(&oneWire);
DHT dht(DHTPIN, DHTTYPE); // DHT22 - vlhkost
Adafruit_BMP085 bmp; // BMP085 - tlak

long interval = 10;  // interval mereni [s]
uint32_t timer;

void setup () {
  Serial.begin(9600); 
  
  /* ####### DS18B20 ####### */
  sensors.begin();
  sensors.setResolution(11); // presnost mereni DS18B20 9=0.5°C  10=0.25°C  11=0.125°C  12=0.0625°C
  /* ####### DS18B20 ####### */
}

void loop () {  
  if (millis() > timer) {
    timer = millis() + (interval * 1000);
    
    merit();
    
  }
}

void merit() { 
  
  /* ####### DS18B20 ####### */    
  sensors.requestTemperatures();
  byte count = sensors.getDeviceCount();
  DeviceAddress da;

  if (!count == 0) {
    for (byte i=0; i<count; i++) {
      sensors.getAddress(da, i);
      
      Serial.print(F("DS18B20,"));
      Serial.print(i);
      Serial.print(F(",\""));
      Serial.print(da[0], HEX);  Serial.print("-");
      Serial.print(da[1], HEX);  Serial.print("-");
      Serial.print(da[2], HEX);  Serial.print("-");
      Serial.print(da[3], HEX);  Serial.print("-");
      Serial.print(da[4], HEX);  Serial.print("-");
      Serial.print(da[5], HEX);  Serial.print("-");
      Serial.print(da[6], HEX);  Serial.print("-");
      Serial.print(da[7], HEX);  Serial.print("\",");
      Serial.println(sensors.getTempCByIndex(i)); 
    }
  }
  /* ####### DS18B20 ####### */
    
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
    Serial.print(F("DHT22,"));
    Serial.print(t);
    Serial.print(F(","));
    Serial.println(h);
  }
  /* ####### DHT22 ####### */
    
  /* ####### BMP085 ####### */
  Serial.print(F("BMP085,"));
  Serial.print(bmp.readTemperature());
  Serial.print(F(","));
  Serial.println(bmp.readPressure());
  /* ####### BMP085 ####### */
}

