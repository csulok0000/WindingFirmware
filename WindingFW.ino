/**
 *
 * Tekercselőgép vezérlő 1.0
 *
 * Saját tervezésű tekercselőgép vezérlését megvalósító szoftver.
 * 
 * @author Tibor Csik <csulok0000@gmail.com>
 * @copyright 2021
 *
 */
#include <AccelStepper.h>
#include <avr/pgmspace.h>
#include "configuration.h"
#include "string.h"

/**
 * 
 * Beállítások
 * 
 */
int coilLength    = 36;                       // Orsó hossza mm-ben
int coilDiameter  = 12;                       // Orsó átmérő mm-ben
int maxTurn       = 1100;
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
  // Soros kommunikáció beállítása, alap infók küldése
  serialInit();
  
  // Orsó forgató motor vezérlő portja, HIGH: forog, LOW: nem forog
  pinMode(MOTOR_PIN, OUTPUT);

  // Kikapcsolható, főleg fejlesztéshez, mert "sok" idő
  if (STEPPER_AUTO_INIT) {
    stepperInit();
  }

  // Senzorok figyelése
  interruptInit();

  // Beállítások elvégzése
  settings();

  // Aktuális értékek megjelenítése
  info();
  
  Serial.println();
  Serial.println(F("További információk: \"help\" vagy \"?\"."));
  
}

void loop() {

  // Csak a start parancs kiadását követően indul el
  if (working == 0)  {
    digitalWrite(MOTOR_PIN, 0);
    return;
  }

  //
  // Tekercselés kész, leállítjuk a futást
  //
  if (turns >= maxTurn) {
    digitalWrite(MOTOR_PIN, 0);
    delay(500);
    Serial.println("A tekercs elkészült!");
    
    working = 0;
    turns = 0;
    return;
  }

  // Orsó forgató motor bekapcsolása, elég lenne csak a start parancs kiadásakor beállítáani
  digitalWrite(MOTOR_PIN, 1);

  //
  // Stepper irányának beállítása
  //
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

  Serial.print(F("Menet: "));
  Serial.print(turns);
  Serial.print(F(" | Irány: "));
  Serial.print(stepperDirection);
  Serial.print(F(" | Pozicio: "));
  Serial.println(stepperPosition);
}

//
// Üzenet fogadása soros porton keresztül
//
void serialEvent() {
  if (Serial.available() > 0) {
      // Megvárjuk az adatokat
      delay(300);
      String d = Serial.readString();
      
      d.trim();
      Serial.print(F("Utasítás: ")); Serial.println(d);

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
        Serial.println(F("HIBA: Ismeretlen parancs!"));
        return;
      }
      
  }
}

void serialInit() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println(F("***********************************************"));
  Serial.println(F("*                                             *"));
  Serial.println(F("*           Csülök Winding Firmware           *"));
  Serial.println(F("*                                             *"));
  Serial.println(F("***********************************************"));
  Serial.println();
  Serial.print("Version: "); Serial.print(FW_VERSION); Serial.print("("); Serial.print(FW_DATE); Serial.println(")");
  Serial.println();
  Serial.print("Fejlesztő: "); Serial.println(FW_AUTHOR);
  
  Serial.println();
  Serial.println(F("Lábkiosztás: "));
  Serial.println(F("  D2: Fej végállás kapcsoló"));
  Serial.println(F("  D3: Menet számláló"));
  Serial.println(F("  D4: Stepper MA1"));
  Serial.println(F("  D5: Stepper MA2"));
  Serial.println(F("  D6: Stepper MB1"));
  Serial.println(F("  D7: Stepper MB2"));
  Serial.println(F("  D8: Orsó hajtó motor"));

  if (STEPPER_AUTO_INIT == false) {
    Serial.println();
    Serial.println(F("!!! Stepper auto init kikapcsolva, futtasd manuálisan !!!"));
  }
}

void help() {
  Serial.println();
  Serial.println(F("Elérhető utasítások:"));
  Serial.println(F("  help, ?                 Elérhatő parancsok listázása"));
  Serial.println(F("  info                    Aktuális beállítások listázása"));
  Serial.println(F("  start                   Tekercselés indítása"));
  Serial.println(F("  stop                    Tekercselés leállítása"));
  Serial.println(F("  init                    Stepper beállítása"));
  Serial.println(F("  set length: <value>     Orsó hossz beállítása"));
  Serial.println(F("  set diameter: <value>   Orsó átmérő beállítása"));
  Serial.println(F("  set turns: <value>      Tekercs menetszám beállítása"));
  Serial.println(F("  set wire: <value>       Huzal átmérő beállítása"));
  Serial.println(F("  set <l>|<d>|<t>|<w>     Beállítások megadása egyszerre"));
  
}

void info() {
  Serial.println();
  Serial.println("Aktuális beállítások:");  
  Serial.println("");
  Serial.print(F("Orsó hossza: ")); Serial.print(coilLength); Serial.println("mm");
  Serial.print(F("Orsó átmérő: ")); Serial.print(coilDiameter); Serial.println("mm");
  Serial.print(F("Menetek száma: ")); Serial.println(maxTurn);
  Serial.print(F("Huzal átmérő: ")); Serial.print(wireWidth); Serial.println("mm");
  Serial.print(F("Stepper max pozíció: ")); Serial.print(maxPos);Serial.println(" lépés");
  Serial.print(F("Menetek száma rétegenként: ")); Serial.println(tpl);
  
}

void stepperInit() {
  // Léptető motor sebessége
  stepper.setMaxSpeed(500);
  stepper.setAcceleration(500);
  
  Serial.println(F("Stepper beállítása..."));
  int pos = 0;

  // Addig megy amig nem éri el a végállás kapcsolót
  while (digitalRead(HEAD_ENDSTOP) != HIGH) {
      Serial.print(F("|")); Serial.print(pos);
      //stepper.setCurrentPosition(0);
      pos += 32;
      stepper.moveTo(pos);
      stepper.runToPosition();
  }
  Serial.println();
  Serial.println(F("Stepper min pozició érzékelve!"));
  stepper.setCurrentPosition(0);

  // Elmozdítjuk picit a min pozícióból
  stepper.moveTo(-HEAD_MARGIN);
  stepper.runToPosition();
  stepper.setCurrentPosition(0);

  // Fej átmozgatása az orsó másik végére.
  Serial.print(F("Stepper alap pozicióba állítása: ")); Serial.print(maxPos); Serial.println(" => 0");
  stepper.moveTo(-maxPos);
  stepper.runToPosition();
  stepper.setCurrentPosition(0);

  Serial.println(F("Stepper beállítva"));
}

// A beállítások alapján kiszámoljuk a szükséges értékeket
void settings() {
  maxPos  = coilLength * SPMM - HEAD_MARGIN * 2;        // Orsó mérete lépésekben
  tpl     = coilLength / wireWidth;   // Menet szám rétegenként  
}

// Senszorok figyelésének indítása
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
 * Menet számláló, jelenleg szoftveresen "kezeljük" a hibákat, de hamarosan kap a hardver egy szűrő elektronikát
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
