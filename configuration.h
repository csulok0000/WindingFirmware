/**
 * 
 * @Author Tibor Csik <csulok0000@gmail.com>
 */

#pragma once

/**
 * A Firmaware neve
 */
#define FW_NAME "Csulok Winding Firmaware"

/**
 * Firmware verzió
 */
#define FW_VERSION "v1.0.0"

/**
 * Firmware módosításának dátuma
 */
#define FW_DATE "2021.02.20."

/**
 * Fejlesztő
 */
#define FW_AUTHOR "Tibor Csik <csulok0000@gmail.com>"

/**
 * Pozicionáló fej végállás érzékelő
 */
#define HEAD_ENDSTOP  2

/**
 * Menet számláló érzékelő
 */
#define TURN_SENSOR   3

/**
 * Léptető motor vezérlő portjai(ULN2003 IN1-4)
 * 
 * Hardveresen van megoldva az átkötés, ezért vannak "sorban"
 * 
 * MA1, MA2, MB1, MB2
 */
#define STEPPER_PIN1  4 // Stepper
#define STEPPER_PIN2  5 // Stepper
#define STEPPER_PIN3  6 // Stepper
#define STEPPER_PIN4  7 // Stepper

/**
 * Főmotor vezérlő portja (ULN2003 IN5)
 * 
 */
#define MOTOR_PIN     8 // Sima motor

/**
 * Léptető motor teljes fordulat lépései(360°)
 */
#define STEPPER_FULL_TURN 2048 // ~ 24mm

/**
 * Teljes fordulat alatt megtett távolság mm-ben
 */
#define STEPPER_FULL_TURN_DISTANCE 24

/**
 * 1mm megtételéhez szükséges lépések (lefelé kerekítjük)
 */
#define SPMM          floor(STEPPER_FULL_TURN / STEPPER_FULL_TURN_DISTANCE) * 0.96 // Magic :D 

/**
 * Induláskor a stepper automatikus beállítása
 */
#define STEPPER_AUTO_INIT false

 
