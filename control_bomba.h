#ifndef CONTROL_BOMBA_H
#define CONTROL_BOMBA_H

#include <Servo.h>

// Declaraciones externas (variables definidas en otro archivo)
extern Servo servo1;
extern Servo servo2;
extern Servo servo3;
extern Servo servo4;
extern const int bombaPin;

// Pines del sensor ultrasónico (si se definen aquí)
extern const int triggerPin;
extern const int echoPin;

// Declaración de la función (solo el encabezado)
void modo1();
void modo2();
void modo3();
void controlarModoActual();
float medirNivelAguaCM(); 


#endif
