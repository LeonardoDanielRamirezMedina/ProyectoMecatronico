#include <Arduino.h>
#include "control_bomba.h"

const int servo1Pin = 50;
const int servo2Pin = 51;
const int servo3Pin = 52;
const int servo4Pin = 53;

const int triggerPin = A5;
const int echoPin = A4;
extern const int bombaPin;

// Variables de control para modos 2 y 3
bool modo2_iniciado = false;
bool modo3_iniciado = false;

// Variable para controlar el modo activo
static int modoActivo = 1;  // 1 = modo1 por defecto

float medirNivelAguaCM() {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  long duracion = pulseIn(echoPin, HIGH);
  float distancia = duracion * 0.0343 / 2.0;  // Convertir a cm
  return distancia;
}

void modo1() {           // PECERA - MODO BASE
    modoActivo = 1;
    Serial.println("Modo 1 activado");

    // Movimiento de servos 1 y 2
    servo1.attach(servo1Pin);
    servo2.attach(servo2Pin);
    servo1.write(7);
    servo2.write(108);
    delay(3000);
    servo1.detach();
    servo2.detach();
    delay(1000);

    // Movimiento de servos 3 y 4
    servo3.attach(servo3Pin);
    servo4.attach(servo4Pin);
    servo3.write(108);
    servo4.write(7);
    delay(3000);
    servo3.detach();
    servo4.detach();
    delay(1000);

    // Enciende la bomba después del movimiento de servos
    digitalWrite(bombaPin, HIGH);
    delay(500000);  // Bomba activa por 5 segundos
    digitalWrite(bombaPin, LOW);
}

void modo2() {  // VACIADO
    modoActivo = 2;
    modo2_iniciado = false; // reseteamos flag para iniciar secuencia
    Serial.println("Modo 2 activado");
}

void checkModo2() {
    if (!modo2_iniciado) {
        modo2_iniciado = true;

        // Abrir servo1 y cerrar servo2
        servo1.attach(servo1Pin);
        servo2.attach(servo2Pin);
        servo1.write(7);    // abrir
        servo2.write(108);  // cerrar
        delay(3000);
        servo2.detach();
        delay(1000);

        // Abrir servo3 y cerrar servo4
        servo3.attach(servo3Pin);
        servo4.attach(servo4Pin);
        servo3.write(7);    // abrir salida
        servo4.write(108);  // cerrar otra
        delay(3000);
        servo3.detach();
        servo4.detach();

        // Encender bomba
        digitalWrite(bombaPin, HIGH);
    }

    float nivel = medirNivelAguaCM();
    Serial.print("Modo2 - Nivel de agua: ");
    Serial.print(nivel);
    Serial.println(" cm");

    // Condición para terminar vaciado: nivel <= 10 cm (ajusta según tu pecera)
    if (nivel <= 10.0) {
        digitalWrite(bombaPin, LOW);

        // Cerrar servo1
        servo1.attach(servo1Pin);
        servo1.write(108);
        delay(3000);
        servo1.detach();

        modo2_iniciado = false;
        modoActivo = 1;  // Regresar a modo1
        modo1();
    }
}

void modo3() {  // LLENADO
    modoActivo = 3;
    modo3_iniciado = false; // reset flag para iniciar secuencia
    Serial.println("Modo 3 activado");
}

void checkModo3() {
    if (!modo3_iniciado) {
        modo3_iniciado = true;

        // Cerrar servo1 y abrir servo2
        servo1.attach(servo1Pin);
        servo2.attach(servo2Pin);
        servo1.write(108);  // cerrar
        servo2.write(7);    // abrir
        delay(3000);
        servo1.detach();
        servo2.detach();
        delay(1000);

        // Cerrar servo3 y abrir servo4
        servo3.attach(servo3Pin);
        servo4.attach(servo4Pin);
        servo3.write(108);  // cerrar
        servo4.write(7);    // abrir
        delay(3000);
        servo3.detach();
        servo4.detach();
        delay(1000);

        // Encender bomba
        digitalWrite(bombaPin, HIGH);
    }

    float nivel = medirNivelAguaCM();
    Serial.print("Modo3 - Nivel de agua: ");
    Serial.print(nivel);
    Serial.println(" cm");

    // Condición para terminar llenado: nivel >= 20 cm (ajusta según tu pecera)
    if (nivel >= 20.0) {
        digitalWrite(bombaPin, LOW);

        // Abrir servo1 y cerrar servo2 para dejar válvulas en estado seguro
        servo1.attach(servo1Pin);
        servo2.attach(servo2Pin);
        servo1.write(7);    // abrir
        servo2.write(108);  // cerrar
        delay(3000);
        servo1.detach();
        servo2.detach();

        modo3_iniciado = false;
        modoActivo = 1;  // Regresar a modo1
        modo1();
    }
}

void controlarModoActual() {
    switch(modoActivo) {
        case 1:
            // No hacer nada, modo base
            break;
        case 2:
            checkModo2();
            break;
        case 3:
            checkModo3();
            break;
    }
}
