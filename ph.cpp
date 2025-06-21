#include <Arduino.h>
#include "Ph.h"

static int ph_pin;
static int m_4;
static int m_7;
static float valorPH = 0;

void iniciarSensorPH(int pin, int valor4, int valor7) {
    ph_pin = pin;
    m_4 = valor4;
    m_7 = valor7;
}

void actualizarPH() {
    int buffer_arr[10];
    for (int i = 0; i < 10; i++) {
        buffer_arr[i] = analogRead(ph_pin);
        delay(30);
    }

    // Ordenar el arreglo (simple bubble sort)
    for (int i = 0; i < 9; i++) {
        for (int j = i + 1; j < 10; j++) {
            if (buffer_arr[i] > buffer_arr[j]) {
                int temp = buffer_arr[i];
                buffer_arr[i] = buffer_arr[j];
                buffer_arr[j] = temp;
            }
        }
    }

    // Promediar las 6 lecturas centrales
    unsigned long avgval = 0;
    for (int i = 2; i < 8; i++) {
        avgval += buffer_arr[i];
    }

    float phval = (float)avgval * 5.0 / 1024.0 / 6.0;  // Voltaje promedio

    // Ajusta este valor con la calibración correcta para tu sensor
    float calibration_value = 21.34;

    valorPH = -5.70 * phval + calibration_value;

    // Limita rango de pH
    if (valorPH < 0) valorPH = 0;
    if (valorPH > 14) valorPH = 14;
}


float obtenerPH() {
    return valorPH;
}
