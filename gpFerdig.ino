#include <Servo.h>

//Pins:
#define SENSOR1_PIN 10
#define SENSOR2_PIN 11
#define KLOKKE_KNAPP_PIN 12
#define KLOKKE_VIBRASJON_PIN 2
#define ROM1_VIBRASJON_PIN 3
#define ROM2_VIBRASJON_PIN 4

//Konstanter:
#define SERVO_BEVEGELSE_INTERVALL 50 // Styrer tid mellom mikrobevegelse
#define KLOKKE_VIBRASJON_TID 5000 // Styrer hvor lenge klokke-pushvarsler varer
#define ROM_VIBRASJON_TID 30000 // Styrer hvor lenge et rom vibrerer
#define ROM_AKTIVITET_TID 60000 // Styrer hvor lenge et rom holdes aktivt uten å være trigget
#define KLOKKE_COOLDOWN 60000 // Styrer intervallet mellom klokke-pushvarsler
#define SENSOR_TRESHOLD 20000 // Styrer hvor lenge bevegelsessensoren må gi signal for å gjøre et rom aktivt

//Variabler for klokke:
bool klokkeVibrerer = false;
unsigned long klokkeStartTid = 0;
unsigned long klokkeTimer = 0;
int klokkeKnappStatus;
int forrigeKlokkeKnappStatus;

//Variabler for rom 1:
int sensor1Status;
int forrigeSensor1Status;

unsigned long sensor1timer = millis();
unsigned long sensor1Start = 0;

unsigned long rom1StartTid = 0;
unsigned long rom1Timer = 0;

bool rom1Vibrerer = false;
bool rom1ErAktivt = false;
unsigned long rom1CooldownTimer;
unsigned long rom1CooldownStart;

//Variabler for rom 2:
int sensor2Status;
int forrigeSensor2Status;

unsigned long sensor2timer = millis();
unsigned long sensor2Start = 0;

unsigned long rom2StartTid = 0;
unsigned long rom2Timer = 0;

bool rom2Vibrerer = false;
bool rom2ErAktivt = false;
unsigned long rom2CooldownTimer;
unsigned long rom2CooldownStart;

//Variabler for flere rom kan enkelt lages ved å kopiere og endre navn:
//int sensor3Status;
//int forrigeSensor3Status;

//unsigned long sensor3timer = millis();
//unsigned long sensor3Start = 0;

//unsigned long rom3StartTid = 0;
//unsigned long rom3Timer = 0;

//bool rom3Vibrerer = false;
//bool rom3ErAktivt = false;
//unsigned long rom3CooldownTimer;
//unsigned long rom3CooldownStart;
//(NB! HUSK AT ROMMET MÅ LEGGES TIL I KODEN I SETUP OG LOOP OGSÅ)

void setup() {
  Serial.begin(9600);
  pinMode(SENSOR1_PIN, INPUT);
  pinMode(SENSOR2_PIN, INPUT);
  pinMode(KLOKKE_KNAPP_PIN, INPUT_PULLUP);
  pinMode(KLOKKE_VIBRASJON_PIN, OUTPUT);
  pinMode(ROM1_VIBRASJON_PIN, OUTPUT);
  pinMode(ROM2_VIBRASJON_PIN, OUTPUT);
}

void loop() {

  //Sensorstatuser må lagres for å måle romaktivitet i programmet
  sensor1Status = digitalRead(SENSOR1_PIN);
  sensor2Status = digitalRead(SENSOR2_PIN);

  //Status til klokkeknapp sjekkes ved hver loop
  klokkeKnappStatus = digitalRead(KLOKKE_KNAPP_PIN);

  //Timer for sensorer startes når sensorstatus er høy
  if(sensor1Status == HIGH){
    sensor1timer = millis() - sensor1Start;
  }else{
    sensor1timer = 0;
  }
  if(sensor2Status == HIGH){
    sensor2timer = millis() - sensor2Start;
  }else{
    sensor2timer = 0;
  }

  //Tiden for sensorene startes når forrige sensor status var lav og gjeldene er høy
  if(forrigeSensor1Status == LOW && sensor1Status == HIGH){
    sensor1Start = millis();
  }
  if(forrigeSensor2Status == LOW && sensor2Status == HIGH){
    sensor2Start = millis();
  }

  //Ny forrige sensorstatus lagres (etter den forrige blir brukt)
  forrigeSensor1Status = sensor1Status;
  forrigeSensor2Status = sensor2Status;

  //Det sjekkes om sensorene har sendt høyt signal lenge nok (rommene blir aktive henholdsvis)
  if(sensor1timer > SENSOR_TRESHOLD){
    rom1ErAktivt = true;
  
    rom1CooldownStart = millis();
  }
  if(sensor2timer > SENSOR_TRESHOLD){
    rom2ErAktivt = true;
  
    rom2CooldownStart = millis();
  }

  //En cooldowntimer startes for å sjekke hvor lenge rommene har vært aktive
  if(rom1CooldownStart != NULL && rom1ErAktivt){
    rom1CooldownTimer = millis() - rom1CooldownStart;
  }
  if(rom2CooldownStart != NULL && rom2ErAktivt){
    rom2CooldownTimer = millis() - rom2CooldownStart;
  }

  //Rommene blir inaktive hvis cooldowntimer når levetiden til rommene (ROM_AKTIVITET_TID)
  if(rom1CooldownTimer > ROM_AKTIVITET_TID){
    rom1ErAktivt = false;
  }
  if(rom2CooldownTimer > ROM_AKTIVITET_TID){
    rom2ErAktivt = false;
  }

  //Hvis et rom er aktivt og klokken ikke allerede vibrerer starter den å vibrere
  if(rom1ErAktivt || rom2ErAktivt){
    if (!klokkeVibrerer){
      klokkeStartTid = millis();
    }
    if (klokkeTimer < KLOKKE_VIBRASJON_TID){
      klokkeVibrerer = true;
      digitalWrite(KLOKKE_VIBRASJON_PIN, HIGH);
    }else{
      digitalWrite(KLOKKE_VIBRASJON_PIN, LOW);
    }

    klokkeTimer = millis() - klokkeStartTid;
  }

  if (millis() - klokkeStartTid > KLOKKE_COOLDOWN){
    klokkeStartTid = millis();
    klokkeTimer = 0;
  }


  //Hvis rommene er aktive og knappen på klokke trykkes vil de aktive rommene begynne å vibrere
  if (rom1ErAktivt || rom2ErAktivt) {
    if(forrigeKlokkeKnappStatus == HIGH && klokkeKnappStatus == LOW){
      if(!rom1Vibrerer && rom1ErAktivt){
        rom1StartTid = millis();
        rom1Vibrerer = true;
      }
      if(!rom2Vibrerer && rom2ErAktivt){
        rom2StartTid = millis();
        rom2Vibrerer = true;
      }
    }
    if (rom1Timer < ROM_VIBRASJON_TID && rom1Vibrerer){
        digitalWrite(ROM1_VIBRASJON_PIN, HIGH);
    }
    else{
      rom1Vibrerer = false;
      digitalWrite(ROM1_VIBRASJON_PIN, LOW);
    }
    if (rom2Timer < ROM_VIBRASJON_TID && rom2Vibrerer){
        digitalWrite(ROM2_VIBRASJON_PIN, HIGH);
    }
    else{
      rom2Vibrerer = false;
      digitalWrite(ROM2_VIBRASJON_PIN, LOW);
    }
    if(rom1ErAktivt){
      rom1Timer = millis() - rom1StartTid;
    }
    if(rom2ErAktivt){
      rom2Timer = millis() - rom2StartTid;
    }
  }

  //Status for klokkeknapp lagres (etter forrige blir brukt) til bruk i neste loop
  forrigeKlokkeKnappStatus = klokkeKnappStatus;

  //Printer til serial monitor for sjekking av diverse variabler i koden:

  //Serial.println(sensor2timer);
  //Serial.println(klokkeKnappStatus);
  Serial.println(rom1ErAktivt);
  Serial.println(rom2ErAktivt);
}
