#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <EEPROM.h>

#define LAMP D0
#define CLK D1
#define DATA D2
#define LATCH D3
#define IR_PIN 14

uint8_t LRch, Cch, SWch, sLRch, toneTR, toneBASS, outPort, bypass;

IRrecv irrecv(IR_PIN);
decode_results results;

void setup() {
   pinMode(LATCH, OUTPUT); 
   pinMode(DATA, OUTPUT);
   pinMode(CLK, OUTPUT);
   pinMode(LAMP, OUTPUT);
   
   digitalWrite(LATCH, LOW);
   digitalWrite(DATA, LOW);
   digitalWrite(CLK, LOW);
   digitalWrite(LAMP, LOW);
  
   irrecv.enableIRIn();  // Start the receiver
   Serial.begin(115200); 
   EEPROM.begin(512);    // Initialize EEPROM
   
   LRch = EEPROM.read(1);     if (LRch > 63)     { LRch = 22;    EEPROM.write(1, 22); EEPROM.commit(); }
   Cch = EEPROM.read(2);      if (Cch > 63)      { Cch = 22;     EEPROM.write(2, 22); EEPROM.commit(); }
   SWch = EEPROM.read(3);     if (SWch > 63)     { SWch = 22;    EEPROM.write(3, 22); EEPROM.commit(); }
   sLRch = EEPROM.read(4);    if (sLRch > 63)    { sLRch = 22;   EEPROM.write(4, 22); EEPROM.commit(); }
   toneTR = EEPROM.read(5);   if (toneTR > 15)   { toneTR = 3;   EEPROM.write(5, 3);  EEPROM.commit(); }
   toneBASS = EEPROM.read(6); if (toneBASS > 15) { toneBASS = 3; EEPROM.write(6, 3);  EEPROM.commit(); }
   outPort = bypass = 0; 

   setAll();
   printAll();
}

void loop() {
    delay(80);   
  
    if (irrecv.decode(&results)) {
         
        Serial.print("RC: "); 
        serialPrintUint64(results.value, HEX);
        Serial.println("");

        if (outPort != 0) 
            switch(results.value) {
                case 0xE218EF48B7:   // - "POWER" 
                    outPort = 0;
                    digitalWrite(LAMP, LOW);
                    EEPROM.write(1, LRch);     // LRch
                    EEPROM.write(2, Cch);      // Cch
                    EEPROM.write(3, SWch);     // SWch
                    EEPROM.write(4, sLRch);    // sLRch
                    EEPROM.write(5, toneTR);   // toneTR
                    EEPROM.write(6, toneBASS); // toneBASS
                    EEPROM.commit();           // Store data to EEPROM
                    setAudio(1);
                    Serial.print("POWER ");
                    printAll();
                    delay(1000);
                    break;
                          
                case 0xE218EF28D7:  // LR down - vol-
                    if (LRch < 63)
                        LRch++;
                    Serial.print("LRch ");
                    setAudio(2);
                    break;
                
                case 0xE218EF08F7: // LR up - vol+
                    if (LRch > 0)
                        LRch--;
                    Serial.print("LRch ");
                    setAudio(2);
                    break;
              
                case 0xE218EFE01F:  // toneTR up - 7
                    if (toneTR >=10 && toneTR <= 15)
                        toneTR--;
                    else if (toneTR >=0 && toneTR <= 6)
                        toneTR++;
                    else if (toneTR == 9)
                        toneTR = 0;
                    Serial.print("TONE ");
                    setAudio(1);
                    delay(100);
                    break;
                
                case 0xE218EF50AF:  // toneTR down - -/--
                    if (toneTR >=1 && toneTR <= 7)
                        toneTR--;
                    else if (toneTR >=9 && toneTR <= 14)
                        toneTR++;
                    else if (toneTR == 0)
                        toneTR = 9;
                    Serial.print("TONE ");
                    setAudio(1);
                    delay(100);
                    break;
                
                case 0xE218EF10EF:  // toneBASS up - 8
                    if (toneBASS >=10 && toneBASS <= 15)
                        toneBASS--;
                    else if (toneBASS >=0 && toneBASS <= 6)
                        toneBASS++;
                    else if (toneBASS == 9)
                        toneBASS = 0;
                    Serial.print("TONE ");
                    setAudio(1);
                    delay(100);
                    break;
                  
                case 0xE218EF00FF:  // toneBASS down - 0
                    if (toneBASS >=1 && toneBASS <= 7)
                        toneBASS--;
                    else if (toneBASS >=9 && toneBASS <= 14)
                        toneBASS++;
                    else if (toneBASS == 0)
                        toneBASS = 9;
                    Serial.print("TONE ");
                    setAudio(1);
                    delay(100);
                    break;
                  
                case 0xE218EF20DF: //C down - 4
                    if (Cch < 63)
                        Cch++;
                    Serial.print("C SW ");
                    setAudio(3);
                    break;
                
                case 0xE218EF807F: //C up - 1
                    if (Cch > 0)
                        Cch--;
                    Serial.print("C SW ");
                    setAudio(3);
                    break;    
                  
                case 0xE218EFE817: //SW down - PR-
                    if (SWch < 63)
                        SWch++;
                    Serial.print("C SW ");
                    setAudio(3);
                    break;
                
                case 0xE218EFC837: //SW up - PR+
                    if (SWch > 0)
                        SWch--;
                    Serial.print("C SW ");
                    setAudio(3);
                    break;    
              
                case 0xE218EF609F: // reset - 6
                    LRch = 22; Cch = 22; SWch = 22; sLRch = 22; toneTR = 0; toneBASS = 0;
                    Serial.println("RESET!!! ");
                    setAll();
                    break;    
              
                case 0xE218EFA05F: //SURR_LR down - 5
                    if (sLRch < 63)
                        sLRch++;
                    Serial.print("SURR_LRch ");
                    setAudio(4);
                    break;
                
                case 0xE218EF40BF: //SURR_LR up - 2
                    if (sLRch > 0)
                        sLRch--;
                    Serial.print("SURR_LRch ");
                    setAudio(4);
                    break;
            }
        else                                        // if outPort == 0
            if (results.value == 0xE218EF48B7) {    // - "POWER" 
                outPort = 15;
                digitalWrite(LAMP, HIGH);
                LRch = EEPROM.read(1);
                Cch = EEPROM.read(2);
                SWch = EEPROM.read(3);
                sLRch = EEPROM.read(4);
                toneTR = EEPROM.read(5);
                toneBASS = EEPROM.read(6);
                setAll();
                Serial.print("POWER ");
                printAll();
                delay(1000);
            }
        irrecv.resume(); 
    }
}

void setAudio(uint8_t num) {
    uint16_t s;
    switch (num) {
        case 1: s = 0; s = s + toneTR; s = s << 4; s = s + outPort; s = s << 4; s = s + toneBASS; s = s << 2; s = s + bypass; s = s << 2; s = s + 0b00; break;
        case 2: s = 0; s = s + LRch; s = s << 7; s = s + LRch; s = s << 2; s = s + 0b01; break;
        case 3: s = 0; s = s + Cch; s = s << 7; s = s + SWch; s = s << 2; s = s + 0b10; break;
        case 4: s = 0; s = s + sLRch; s = s << 7; s = s + sLRch; s = s << 2; s = s + 0b11; break;
    }

    delay(1);
    digitalWrite(LATCH, HIGH); delayMicroseconds(20);
    digitalWrite(LATCH, LOW); delayMicroseconds(20);
   
    for (uint8_t i = 1; i <= 16; i++) {
        if (s&(0b1000000000000000)) { 
            delayMicroseconds(4); 
            digitalWrite(DATA, HIGH); 
            Serial.print("1");
        } else { 
            delayMicroseconds(4); 
            digitalWrite(DATA, LOW); 
            Serial.print("0"); 
        }
        s = s << 1;
        delayMicroseconds(20);
        digitalWrite(CLK, HIGH); delayMicroseconds(20);
        digitalWrite(CLK, LOW);  delayMicroseconds(20);
    }
    delayMicroseconds(20);
    digitalWrite(LATCH, HIGH); delayMicroseconds(20);
    digitalWrite(LATCH, LOW); delayMicroseconds(20);
    Serial.println();
}

void printAll() {
    Serial.print(LRch); Serial.print("\t");
    Serial.print(Cch); Serial.print("\t");
    Serial.print(SWch); Serial.print("\t");
    Serial.print(sLRch); Serial.print("\t");
    Serial.print(toneTR); Serial.print("\t");
    Serial.print(toneBASS); Serial.print("\t");
    Serial.print(outPort); Serial.println();
}

void setAll() {
    for (uint8_t i = 0; i < 5; i++) 
        setAudio(i); 
}
