// feeder.h
#ifndef FEEDER_H
#define FEEDER_H

#include <RTClib.h>
#include <Stepper.h>

// Configuración del motor 28BYJ-48 con ULN2003
const int stepsPerRevolution = 2048;
Stepper feederStepper(stepsPerRevolution, 48, 46, 44, 42); 

// Variables para la hora de alimentación (estas se deben establecer desde el menú)
extern int horaFeeder;
extern int minutoFeeder;
bool yaAlimento = false;

void iniciarFeeder() {
    feederStepper.setSpeed(10);
}

void verificarFeeder(DateTime ahora) {
    if (ahora.hour() == horaFeeder && ahora.minute() == minutoFeeder && !yaAlimento) {
        feederStepper.step(stepsPerRevolution * 2);  // 2 vueltas
        yaAlimento = true;
    }

    // Resetear bandera al pasar el minuto
    if (ahora.minute() != minutoFeeder) {
        yaAlimento = false;
    }
}

#endif
