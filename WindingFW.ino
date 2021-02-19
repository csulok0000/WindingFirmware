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

#define FW_NAME "Csulok Winding Firmaware"
#define FW_VERSION "v1.0"
#define FW_AUTHOR "Tibor Csik <csulok0000@gmail.com>"
#define FW_BUILD "1 (2021.02.18.)"

#define POSR_ENDSTOP  2
#define TURN_SENSOR   3

// Léptető motor vezérlő portjai(ULN2003 IN1-4)
#define STEPPER_PIN1  4 // Stepper
#define STEPPER_PIN2  5 // Stepper
#define STEPPER_PIN3  6 // Stepper
#define STEPPER_PIN4  7 // Stepper

// Főmotor vezérlő portja (ULN2003 IN5)
#define MOTOR_PIN     8 // Sima motor

// Léptető motor teljes fordulat lépései(360°)
#define STEPS         2048 // ~ 23,5mm
#define SPMM          88 // 2048 / 23,5 ~= 87.15

// Inicializáljuk a léptető motor vezérlését
AccelStepper stepper(AccelStepper::HALF4WIRE, STEPPER_PIN1, STEPPER_PIN2, STEPPER_PIN3, STEPPER_PIN4);

int stepperDirection  = 1;
int stepperPosition   = 0;

volatile int turns    = 0;
volatile int endStop  = 0;
volatile int tpv      = LOW;
volatile int tpv2     = LOW;

float wireWidth = 0.3;                      // Huzal vastagság
int coilLength  = 36;                       // Orsó hossza mm-ben
int coilWidth   = 12;                       // Orsó átmérő mm-ben
float tpl       = coilLength / wireWidth;   // Menet szám rétegenként
int maxPos      = coilLength * SPMM;         // Orsó mérete lépésekben
int lastTurn    = 0;
int working     = 0;

void setup() {
  pinMode(MOTOR_PIN, OUTPUT);
  
  serialInit();
  stepperInit();
  interruptInit();
  // Orsó forgató motor bekapcsolása

  Serial.println("Waiting for command!");
  help();
}

void loop() {

  

  // Nincs munka
  if (working == 0)  {
    digitalWrite(MOTOR_PIN, 0);
    return;
  }

  digitalWrite(MOTOR_PIN, 1);
  
  // Stepper irányának beállítása
  if (endStop > 0 || stepperPosition >= maxPos) {
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

  Serial.print("Turns: ");
  Serial.print(turns);
  Serial.print(" | Dir: ");
  Serial.print(stepperDirection);
  Serial.print(" | Pos: ");
  Serial.println(stepperPosition);
}

void serialEvent() {
  if (Serial.available() > 0) {
      // Megvárjuk az adatokat
      delay(300);
      String d = Serial.readString();
      d.trim();
      Serial.println("Capture serial data: " + d);
      if (d == "start") {
        Serial.println("Command: " + d);
        working = 1;
      } else if (d == "stop") {
        Serial.println("Command: " + d);
        working = 0;
      } else if (d == "init") {
        initStepper();  
      } else if (d == "help" || d == "?") {
        help();
      }
  }
}

void help() {
  Serial.println("Available commands:");
  Serial.println("  start: start the winding process");
  Serial.println("  stop: stop the winding process");
  Serial.println("  init: initialize stepper");
}


void serialInit() {
  Serial.begin(9600);
  Serial.println();
  Serial.println(String(FW_NAME) + " " + String(FW_VERSION) + " build: " + String(FW_BUILD));
  Serial.println();
  Serial.println(String("Author: ") + String(FW_AUTHOR));
  Serial.println();
  Serial.println();
  Serial.println("Pinout: ");
  Serial.println("  D2: Positioner endstop");
  Serial.println("  D3: Turn counter");
  Serial.println("  D4: MA1");
  Serial.println("  D5: MA2");
  Serial.println("  D6: MA3");
  Serial.println("  D7: MA4");
  Serial.println("  D8: MB1");
  Serial.println();  
}

void stepperInit() {
  // Léptető motor sebessége
  stepper.setMaxSpeed(500);
  stepper.setAcceleration(500);
  
  Serial.print("Stepper init...");
  
  while (digitalRead(POSR_ENDSTOP) != HIGH) {
      //Serial.print("POS: ");
      //Serial.println(digitalRead(POSR_ENDSTOP));
      stepper.setCurrentPosition(0);
      stepper.moveTo(32);
      stepper.runToPosition();
  }
  Serial.println();
  Serial.println("Stepper min position detected!");
  stepper.setCurrentPosition(0);

  Serial.println("Stepper going to neutral position...");
  stepper.moveTo(-maxPos);
  stepper.runToPosition();
  stepper.setCurrentPosition(0);

  Serial.println("Stepper initialized");
}

void interruptInit() {
  pinMode(POSR_ENDSTOP, INPUT_PULLUP);
  pinMode(TURN_SENSOR, INPUT_PULLUP);
  digitalWrite(POSR_ENDSTOP, LOW);
  digitalWrite(TURN_SENSOR, LOW);
  attachInterrupt(digitalPinToInterrupt(POSR_ENDSTOP), posrEndStop, RISING);
  attachInterrupt(digitalPinToInterrupt(TURN_SENSOR), turnSignal, RISING);

  interrupts();
}

/**
 * 
 * Menet számláló
 */
void turnSignal() {
  if (digitalRead(TURN_SENSOR) == HIGH && tpv == LOW && tpv2 == LOW) {
    turns++;
    tpv = HIGH;
    tpv2 = HIGH;
  } else {
    tpv2 = tpv;
    tpv = LOW;
  }
}

/**
 * 
 * Végálláskapcsoló
 */
void posrEndStop() {
  //endStop = digitalRead(POSR_ENDSTOP);
  if (digitalRead(POSR_ENDSTOP) == HIGH) {
    endStop = 1;
  } else {
    //endStop = 0;
  }
}
