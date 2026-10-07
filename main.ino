#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

int irPins[5]  = {8, 9, 10, 11, 12};
int ledPins[5] = {2, 3, 4, 5, 6};
int buzzerPin = 13;

float slotSize[5] = {5.5, 5.5, 7.0, 7.0, 15.0};
int slotFlag[5] = {0, 0, 0, 0, 0};

int slotAssigned = -1;
float len;

void setup() {
  Serial.begin(9600);

  for(int i=0;i<5;i++){
    pinMode(irPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);

  lcd.init();
  lcd.backlight();
  lcd.print("Smart Parking");
  delay(1500);
  lcd.clear();

  Serial.println("SYSTEM_READY");
}

void loop() {

  // -------- EXIT CHECK --------
  for(int i=0;i<5;i++){
    if(slotFlag[i] == 1 && digitalRead(irPins[i]) == HIGH){
      slotFlag[i] = 0;
      Serial.print("SLOT_FREED:");
      Serial.println(i);
    }
  }

  // -------- SLOT ASSIGNED CHECK --------
  if(slotAssigned != -1){

    bool wrongSlot = false;

    for(int i=0;i<5;i++){
      if(i == slotAssigned) continue;

      // Only check empty slots
      if(slotFlag[i] == 0 && digitalRead(irPins[i]) == LOW){
        wrongSlot = true;
      }
    }

    // Buzzer control
    if(wrongSlot){
      tone(buzzerPin, 1000);
    } else {
      noTone(buzzerPin);
    }

    // Correct slot reached
    if(digitalRead(irPins[slotAssigned]) == LOW){

      noTone(buzzerPin);

      slotFlag[slotAssigned] = 1;

      // LED OFF when parked
      digitalWrite(ledPins[slotAssigned], LOW);

      Serial.print("CAR_PARKED:");
      Serial.println(slotAssigned);

      slotAssigned = -1;
    }
  }

  // -------- COUNT FREE --------
  int freeSlots = 0;
  for(int i=0;i<5;i++){
    if(slotFlag[i] == 0) freeSlots++;
  }

  Serial.print("FREE_SLOTS:");
  Serial.println(freeSlots);

  lcd.setCursor(0,0);
  lcd.print("Free Slots: ");
  lcd.print(freeSlots);
  lcd.print("   ");

  lcd.setCursor(0,1);
  lcd.print("Waiting...     ");

  // -------- ENTRY LOGIC --------
  if (Serial.available() > 0 && slotAssigned == -1) {

    String data = Serial.readStringUntil('\n');
    len = data.toFloat();

    if(len <= 0) return;
    if(freeSlots == 0) return;

    for(int i=0;i<5;i++){
      if(len <= slotSize[i] && slotFlag[i] == 0){

        slotAssigned = i;

        // LED ON while guiding
        digitalWrite(ledPins[i], HIGH);

        Serial.print("SLOT_ASSIGNED:");
        Serial.println(i);

        break;
      }
    }
  }

  delay(100);
}
