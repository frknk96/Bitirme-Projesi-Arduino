#include <Keypad.h>

const byte ROWS = 4; //
const byte COLS = 3; //

char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};


byte rowPins[ROWS] = {A1, A0, 6, 5}; // keypadin satir pinoutlari R1 = D8, R2 = D7, R3 = D6, R4 = D5
byte colPins[COLS] = {4, 3, 2};    // keypadin sütun pinoutları C1 = D4, C2 = D3, C3 = D2

/* Yani keypadin pinleri soldan sağa D8'den D2'ye gidiyor */

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

#define buzzerPin 9
int keySayac = 0;
const char sifre[5] = "1881";
char girilen[5];
int eslesen = 0;

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);
const byte rxAddr[6] = "00001";
int gelen[1];

int nemYuzdesi = 0;
int sicaklikDerecesi = 0;
bool gazAlarmi = false;
bool pencereAlarmi = false;
bool alarmKurulu = false;
bool alarmCaliyor = false;

unsigned long oncekiZaman = 0;
unsigned long simdikiZaman = 0;
unsigned long btGondermeInterval = 2000;
#define rolePin A2

void setup(){  
    Serial.begin(9600);

    pinMode(A2, OUTPUT);
    pinMode(A4, OUTPUT);
    pinMode(A5, OUTPUT);
    
    digitalWrite(18, LOW);
    digitalWrite(19, LOW);
    digitalWrite(rolePin, HIGH);

    radio.begin();
    radio.openReadingPipe(0, rxAddr);
    radio.startListening();
}

void loop(){
    char key = keypad.getKey();
    if (key != NO_KEY){
        //Serial.println(key);
        if(alarmCaliyor == false)   
            tone(buzzerPin, 3000, 50);     
        girilen[keySayac] = key;       
        keySayac++;      
        if(keySayac == 4){
            keySayac = 0;
            eslesen = 0;            
            for(int i=0; i<4; i++){                
                if(sifre[i] == girilen[i]){                    
                    eslesen++;                  
                }            
            }   
            if(eslesen == 4){
                eslesen = 0; 
                //Serial.println("DOGRU SIFRE");
                tone(buzzerPin, 1500, 200);
                tone(buzzerPin, 3000, 200);
                if(alarmKurulu == false) {
                    alarmKurulu = true;
                    digitalWrite(18, HIGH);
                    digitalWrite(19, LOW);      
                }
                else {
                    //analogWrite(buzzerPin, 0);
                    digitalWrite(18, LOW);
                    digitalWrite(19, LOW);
                    alarmKurulu = false;
                    alarmCaliyor = false;
                }
            } 
            else{
                //Serial.println("YANLIS SIFRE");
                if(alarmCaliyor == false) {
                    tone(buzzerPin, 5000, 200);
                    tone(buzzerPin, 1000, 200);                   
                }

                if(alarmKurulu == true) {
                    alarmCaliyor = true;
                    analogWrite(buzzerPin, 150);
                    digitalWrite(18, LOW);
                    digitalWrite(19, HIGH);
                }
            }
        }
    }

    if(Serial.available()) {
        int btVeri = Serial.read();
        if(btVeri == 54) {
            digitalWrite(rolePin, LOW);
        }
        else if(btVeri == 55) {
            digitalWrite(rolePin, HIGH);
        }
    }
        
    if (radio.available()){
        radio.read(&gelen, sizeof(gelen));        
        //tone(buzzerPin, 5000, 200);
        if(gelen[0] / 100 == 1) {  // nrf'den nem verisi geldi
            //nemYuzdesi = gelen[0];
            nemYuzdesi = gelen[0]%100;
            //Serial.println("Nem Yuzdesi: " + (String)nemYuzdesi);
        }
        if(gelen[0] / 100 == 2) {  // nrf'den sicaklik verisi geldi
            //sicaklikDerecesi = gelen[0];
            sicaklikDerecesi = gelen[0]%100;
            //Serial.println("Sicaklik: " + (String)sicaklikDerecesi);
        }
        if(gelen[0] / 100 == 3) {  // nrf'den gaz verisi geldi
            if(gelen[0]%100 != 0) {     //tehlike
                alarmCaliyor = true;
                analogWrite(buzzerPin, 150);
                digitalWrite(18, LOW);
                digitalWrite(19, HIGH);
            }
            else {  // gaz degerleri normale dondu
                alarmCaliyor = false;
                analogWrite(buzzerPin, 0);
                digitalWrite(18, LOW);
                digitalWrite(19, LOW);
            }
        }
        if(gelen[0] / 100 == 4) {  // nrfden hall-effect verisi geldi (pencere)
            //Serial.println(gelen[0]);
            if((alarmKurulu == true) && (gelen[0]%100 != 0)) {  // pencere alarm kuruluyken acildi, hirsiz!
                analogWrite(buzzerPin, 150);
                alarmCaliyor = true;
                digitalWrite(18, LOW);
                digitalWrite(19, HIGH);
            }
        }
    }
    else {
        // DEBUG -> Serial.println("radio available degil");
    }

    simdikiZaman = millis();
    if((simdikiZaman - oncekiZaman > btGondermeInterval) && sicaklikDerecesi != 0 && nemYuzdesi != 0)  {
        Serial.print(sicaklikDerecesi);
        Serial.print("a");
        Serial.print(nemYuzdesi);        
        Serial.print("a");
        if(alarmKurulu == true)
            Serial.print("AÇIK");
        else
            Serial.print("KAPALI");
        oncekiZaman = simdikiZaman;      
    }
}
