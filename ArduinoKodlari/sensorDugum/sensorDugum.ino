#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "dht.h"

RF24 radio(7, 8);
const byte rxAddr[6] = "00001";
int gonderilen[1];

dht DHT;
#define dht_apin A0

#define gazSensorPin A1
#define gazSinirDeger 500
int gazSensorDeger = 0;
int gazSensorDegerOnceki = 0;
float gazSensorVolt = 0;

unsigned long oncekiZaman = 0;
unsigned long simdikiZaman = 0;
bool siraNemde = true;
bool siraSicaklikta = false;

#define hallEffectPin A2
#define hallEffectSinirDeger 300
int hallEffectDeger = 0;
int hallEffectDegerOnceki = 0;

0000000000000000000000000000000000000000000000000000000000000000000000
void setup() {
     Serial.begin(9600);
    radio.begin();
    radio.openWritingPipe(rxAddr);
    radio.stopListening();
}
void loop() {
    simdikiZaman = millis();
    
    if(simdikiZaman - oncekiZaman > 2000 && siraNemde == true) {
        DHT.read11(dht_apin);      
        gonderilen[0] = (int)DHT.humidity + 100;        // Nem verisi 1 ile başlıyor, ör: 140
        radio.write(&gonderilen, sizeof(gonderilen));    
        siraNemde = false;
        siraSicaklikta = true;
    }
    
    if(simdikiZaman - oncekiZaman > 4000 && siraSicaklikta == true) {
        DHT.read11(dht_apin);      
        gonderilen[0] = (int)DHT.temperature + 200;        // Sıcaklık verisi 2 ile başlıyor, ör: 225
        radio.write(&gonderilen, sizeof(gonderilen));
        siraSicaklikta = false;
        siraNemde = true;
        oncekiZaman = simdikiZaman;
        gazSensorDeger = analogRead(gazSensorPin);        
    } 
    
    //Serial.println(gazSensorDeger);
    //gazSensorVolt = gazSensorDeger/1024*5.0;       // voltaja cevir
    if((gazSensorDeger > gazSinirDeger) && (gazSensorDegerOnceki < gazSinirDeger)) {
        gonderilen[0] = 311;     // Gaz değerleri sınırı aştı!
        //Serial.println("tehlike");               
        radio.write(&gonderilen, sizeof(gonderilen));
    }
    
    if((gazSensorDeger < gazSinirDeger) && (gazSensorDegerOnceki > gazSinirDeger)) {
        gonderilen[0] = 300;     // Gaz değerleri normale döndü
        //Serial.println("normal");               
        radio.write(&gonderilen, sizeof(gonderilen));  
    }
    
    gazSensorDegerOnceki = gazSensorDeger;

    hallEffectDeger = analogRead(hallEffectPin);
    Serial.println(hallEffectDeger);
    if((hallEffectDeger > hallEffectSinirDeger) && (hallEffectDegerOnceki < hallEffectSinirDeger) ) { // Cam açıldı
        gonderilen[0] = 411;
        radio.write(&gonderilen, sizeof(gonderilen));  
    }

    if((hallEffectDeger < hallEffectSinirDeger) && (hallEffectDegerOnceki > hallEffectSinirDeger) ) { // Cam kapandi
        gonderilen[0] = 400;
        radio.write(&gonderilen, sizeof(gonderilen));  
    }

    hallEffectDegerOnceki = hallEffectDeger;
}
