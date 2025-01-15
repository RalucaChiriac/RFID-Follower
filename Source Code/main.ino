#include <SPI.h> 
#include <MFRC522.h>
#include <NewPing.h>

#define TRIG A5
#define ECHO A4
#define MAX_DISTANCE 100
NewPing sonar(TRIG, ECHO, MAX_DISTANCE);

#define LINESENSOR1 A2
#define LINESENSOR2 A3 // pini senzori
#define LINESENSOR3 A1
#define BUZZER A0   // pini buzzer

#define MOTOR1A 2      // Pini driver motor
#define MOTOR1B 7
#define MOTOR2A 3
#define MOTOR2B 4

#define spdA 6    // pini viteza motor
#define spdB 5

#define RFID_SDA 10
#define RFID_SCK 13
#define RFID_MOSI 11    // pini RFID
#define RFID_MISO 12
#define RFID_RST 9

int viteza = 120;

MFRC522 mfrc522(RFID_SDA, RFID_RST);
long int last_card_read;

#define COMMANDS_LENGTH 3
char* Type[3] = {"Stop", "Beep&stop", "Stop"}; // functii card
int Value[3] = {3000, 1000, 5000};
char* CardID[3] = {"03 BE 20 0D", "B3 0A 9D F7", "A3 7A CF 0C"}; // coduri card

unsigned long actionStartTime = 0;
bool actionInProgress = false;
int currentActionValue = 0;
String currentActionType = "";

void setup() {
  Serial.begin(9600);
  SPI.begin(); // pornim RFID
  mfrc522.PCD_Init();

  pinMode(LINESENSOR1, INPUT);
  pinMode(LINESENSOR2, INPUT);
  pinMode(LINESENSOR3, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(MOTOR1A, OUTPUT);
  pinMode(MOTOR1B, OUTPUT);
  pinMode(MOTOR2A, OUTPUT);
  pinMode(MOTOR2B, OUTPUT);
  pinMode(spdA, OUTPUT);
  pinMode(spdB, OUTPUT);
}

String lastProcessedCard = "";

void loop() {
  unsigned long currentMillis = millis();

  int distance = sonar.ping_cm();
  if (distance == 0) {
    distance = 30;
  }

  if (actionInProgress) {
    if (currentMillis - actionStartTime < currentActionValue) {
      if (distance <= 10) {
        Stop1();
        digitalWrite(BUZZER, HIGH);
      } else {
        if (currentActionType == "Beep&stop") {
          digitalWrite(BUZZER, HIGH);
        } else {
          digitalWrite(BUZZER, LOW);
        }
        Stop1();
      }
    } else {
      actionInProgress = false;
      digitalWrite(BUZZER, LOW);
    }
    return;
  }

  if (distance <= 10) {
    Stop1();
    digitalWrite(BUZZER, HIGH);
  } else {
    digitalWrite(BUZZER, LOW);

    
    if (digitalRead(LINESENSOR1) == HIGH && digitalRead(LINESENSOR2) == LOW && digitalRead(LINESENSOR3) == HIGH) {
      Forward();
    } else if (digitalRead(LINESENSOR1) == LOW && digitalRead(LINESENSOR2) == HIGH && digitalRead(LINESENSOR3) == HIGH) {
      Right();
    } else if (digitalRead(LINESENSOR1) == HIGH && digitalRead(LINESENSOR2) == HIGH && digitalRead(LINESENSOR3) == LOW) {
      Left();
    } else if (digitalRead(LINESENSOR1) == HIGH && digitalRead(LINESENSOR2) == HIGH && digitalRead(LINESENSOR3) == HIGH) {
      Forward();
    }
  }

  // Procesare card RFID
  if (!actionInProgress) {
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
      return;
    }

    String content = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    content.toUpperCase();
    content = content.substring(1);
    
    if (content == lastProcessedCard) {
      return;
    }

    for (int a = 0; a < COMMANDS_LENGTH; a++) {
      if (content == CardID[a]) {
        currentActionType = Type[a];
        currentActionValue = Value[a];
        actionStartTime = millis();
        last_card_read = millis();
        actionInProgress = true;

        if (currentActionType == "Stop") {
          Stop1();
        } else if (currentActionType == "Beep&stop") {
          digitalWrite(BUZZER, HIGH);
          Stop1();
        }

        lastProcessedCard = content; // Actualizeaza ID-ul cardului procesat
        break;
      }
    }
    
  }
}


void Forward() {
  analogWrite(spdA, viteza);
  analogWrite(spdB, viteza);
  digitalWrite(MOTOR1A, HIGH);
  digitalWrite(MOTOR1B, LOW);
  digitalWrite(MOTOR2A, LOW);
  digitalWrite(MOTOR2B, HIGH);
}

void Right() {
  analogWrite(spdA, viteza);
  analogWrite(spdB, viteza);
  digitalWrite(MOTOR1A, LOW);
  digitalWrite(MOTOR1B, HIGH);
  digitalWrite(MOTOR2A, LOW);
  digitalWrite(MOTOR2B, HIGH);
}

void Left() {
  analogWrite(spdA, viteza);
  analogWrite(spdB, viteza);
  digitalWrite(MOTOR1A, HIGH);
  digitalWrite(MOTOR1B, LOW);
  digitalWrite(MOTOR2A, HIGH);
  digitalWrite(MOTOR2B, LOW);
}

void Stop1() {
  digitalWrite(MOTOR1A, LOW);
  digitalWrite(MOTOR1B, LOW);
  digitalWrite(MOTOR2A, LOW);
  digitalWrite(MOTOR2B, LOW);
}
