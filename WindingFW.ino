/**
 *
 * Winding FrameWork 1.0
 *
 * Tekercselő alapszintű szoftvere
 * 
 * @author Tibor Csik <csulok0000@gmail.com>
 * @copyright 2021
 *
 */
#include <AccelStepper.h>
#include "configuration.h"
#include "string.h"

/**
 * 
 * Beállítások
 * 
 */
int coilLength    = 36;                       // Orsó hossza mm-ben
int coilDiameter  = 12;                       // Orsó átmérő mm-ben
int maxTurn       = 1500;
float wireWidth   = 0.3;                      // Huzal vastagság

/**
 * 
 * Működéshez kapcsolódó változók
 * 
 */
int stepperDirection  = 1;
int stepperPosition   = 0;

volatile int turns    = 0;
volatile int endStop  = 0;

int maxPos      = 0;
int tpl         = 0;

int lastTurn    = 0;
int working     = 0;

// Inicializáljuk a léptető motor vezérlését
AccelStepper stepper(AccelStepper::HALF4WIRE, STEPPER_PIN1, STEPPER_PIN2, STEPPER_PIN3, STEPPER_PIN4);

void setup() {
  pinMode(MOTOR_PIN, OUTPUT);
  
  serialInit();

  if (STEPPER_AUTO_INIT) {
    stepperInit();
  }
  
  interruptInit();
  // Orsó forgató motor bekapcsolása

  settings();
  info();
  Serial.println();
  Serial.println("További információk: \"help\" vagy \"?\".");
  
}

void loop() {

  // Nincs munka
  if (working == 0)  {
    digitalWrite(MOTOR_PIN, 0);
    return;
  }

  digitalWrite(MOTOR_PIN, 1);
  
  // Stepper irányának beállítása
  if (/*endStop > 0 || */stepperPosition >= maxPos) {
    stepperDirection = -1;
    endStop = 0;
  }

  else if (stepperPosition <= 0) {
    stepperDirection = 1;
  }

  // Fej pozicionálása a fordulatok alapján
  if (turns > lastTurn) {
    stepperPosition += SPMM * wireWidth * (turns - lastTurn) * stepperDirection;
    lastTurn = turns;
    stepper.moveTo(stepperPosition);
    stepper.runToPosition();
  }

  Serial.print("Menet: ");
  Serial.print(turns);
  Serial.print(" | Irány: ");
  Serial.print(stepperDirection);
  Serial.print(" | Pozicio: ");
  Serial.println(stepperPosition);
}

void serialEvent() {
  if (Serial.available() > 0) {
      // Megvárjuk az adatokat
      delay(300);
      String d = Serial.readString();
      
      d.trim();
      Serial.println("Utasítás: " + d);

      // Tekercselés indítása
      if (d == "start") {
        turns = 0;
        working = 1;
      }

      // Tekercselés leállítása
      else if (d == "stop") {
        working = 0;
      }

      // Stepper beállítása
      else if (d == "init") {
        stepperInit();  
      }

      // Súgó
      else if (d == "help" || d == "?") {
        help();
      }

      // Beállítások listázása
      else if (d == "info") {
        info();
      }

      // Orsó hossz beállítása
      else if (d.startsWith("set length:")) {
        coilLength    = getValue(d.substring(15), '|', 0).toInt();
        settings();
        info();
      }

      // Orsó átmérő beállítása
      else if (d.startsWith("set diameter:")) {
        coilDiameter  = getValue(d.substring(13), '|', 0).toInt();
        settings();
        info();
      }

      // Menetszám beállítása
      else if (d.startsWith("set turns:")) {
        maxTurn    = getValue(d.substring(10), '|', 0).toInt();
        settings();
        info();
      }

      // Huzal beállítása
      else if (d.startsWith("set wire:")) {
        wireWidth    = getValue(d.substring(9), '|', 0).toFloat();
        settings();
        info();
      }

      // Beállítások egyszerre
      else if (d.startsWith("set ")) {
        d = d.substring(4);

        coilLength    = getValue(d, '|', 0).toInt();
        coilDiameter  = getValue(d, '|', 1).toInt();
        maxTurn       = getValue(d, '|', 2).toInt();
        wireWidth     = getValue(d, '|', 3).toFloat();

        settings();
        info();
      }

      // Nem támogatott utasítás
      else {
        Serial.println("HIBA: Ismeretlen parancs!");
        return;
      }
      
  }
}

void serialInit() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("---------------------------------------------------------------------------");
  Serial.print(FW_NAME); Serial.print(" "); Serial.print(FW_VERSION); Serial.print("("); Serial.print(FW_DATE); Serial.println(")");
  Serial.println();
  Serial.print("Fejlesztő: "); Serial.println(FW_AUTHOR);
  Serial.println("---------------------------------------------------------------------------");
  Serial.println();
  Serial.println("Lábkiosztás: ");
  Serial.println("  D2: Fej végállás kapcsoló");
  Serial.println("  D3: Menet számláló");
  Serial.println("  D4: Stepper MA1");
  Serial.println("  D5: Stepper MA2");
  Serial.println("  D6: Stepper MB1");
  Serial.println("  D7: Stepper MB2");
  Serial.println("  D8: Orsó hajtó motor");

  if (STEPPER_AUTO_INIT == false) {
    Serial.println();
    Serial.println("!!! Stepper auto init kikapcsolva, futtasd manuálisan !!!");  
  }
}

void help() {
  Serial.println();
  Serial.println("Elérhető utasítások:");
  Serial.println("  help, ?                 Elérhatő parancsok listázása");
  Serial.println("  info                    Aktuális beállítások listázása");
  Serial.println("  start                   Tekercselés indítása");
  Serial.println("  stop                    Tekercselés leállítása");
  Serial.println("  init                    Stepper beállítása");
  Serial.println("  set length: <value>     Orsó hossz beállítása");
  Serial.println("  set diameter: <value>   Orsó átmérő beállítása");
  Serial.println("  set turns: <value>      Tekercs menetszám beállítása");
  Serial.println("  set wire: <value>       Huzal átmérő beállítása");
  Serial.println("  set <l>|<d>|<t>|<w>     Beállítások megadása egyszerre");
  
}

void info() {
  Serial.println();
  Serial.println("Aktuális beállítások:");  
  Serial.println("");
  Serial.print("Orsó hossza: "); Serial.print(coilLength); Serial.println("mm");
  Serial.print("Orsó átmérő: "); Serial.print(coilDiameter); Serial.println("mm");
  Serial.print("Menetek száma: "); Serial.println(maxTurn);
  Serial.print("Huzal átmérő: "); Serial.print(wireWidth); Serial.println("mm");
  Serial.print("Stepper max pozíció: "); Serial.print(maxPos);Serial.println(" lépés");
  Serial.print("Menetek száma rétegenként: "); Serial.println(tpl);
  
}

void stepperInit() {
  // Léptető motor sebessége
  stepper.setMaxSpeed(500);
  stepper.setAcceleration(500);
  
  Serial.println("Stepper beállítása...");
  int pos = 0;
  while (digitalRead(HEAD_ENDSTOP) != HIGH) {
      Serial.print("|"); Serial.print(pos);
      //stepper.setCurrentPosition(0);
      pos += 32;
      stepper.moveTo(pos);
      stepper.runToPosition();
  }
  Serial.println();
  Serial.println("Stepper min pozició érzékelve!");
  stepper.setCurrentPosition(0);

  Serial.print("Stepper alap pozicióba állítása: "); Serial.print(maxPos); Serial.println(" => 0");
  stepper.moveTo(-maxPos);
  stepper.runToPosition();
  stepper.setCurrentPosition(0);

  Serial.println("Stepper beállítva");
}

void settings() {
  maxPos  = coilLength * SPMM;        // Orsó mérete lépésekben
  tpl     = (int) coilLength / wireWidth;   // Menet szám rétegenként  
}

void interruptInit() {
  pinMode(HEAD_ENDSTOP, INPUT_PULLUP);
  pinMode(TURN_SENSOR, INPUT_PULLUP);
  
  digitalWrite(HEAD_ENDSTOP, LOW);
  digitalWrite(TURN_SENSOR, LOW);
  
  attachInterrupt(digitalPinToInterrupt(HEAD_ENDSTOP), headEndStop, RISING);
  attachInterrupt(digitalPinToInterrupt(TURN_SENSOR), turnSignal, RISING);

  interrupts();
}

/**
 * 
 * Menet számláló
 */
volatile int tpv      = LOW;
volatile int tpv2     = LOW;
void turnSignal() {
  if (digitalRead(TURN_SENSOR) == HIGH && tpv == LOW && tpv2 == LOW) {
    turns++;
    tpv = HIGH;
    tpv2 = HIGH;
  } else {
    // A hibás értékeket szűrjük
    tpv2 = tpv;
    tpv = LOW;
  }
}

/**
 * 
 * Végálláskapcsoló
 */
void headEndStop() {
  if (digitalRead(HEAD_ENDSTOP) == HIGH) {
    endStop = 1;
  }
}
