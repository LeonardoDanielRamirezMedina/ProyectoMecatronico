#include <Arduino.h>
#include "control_temperatura.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <math.h>

// Pines
#define PIN_DS18B20 27
#define PIN_NTC A1
#define PIN_CAMA_CALIENTE 26

// Termistor
#define CONFIG_THERMISTOR_RESISTOR 10000  // resistencia en serie

// Variables globales
OneWire oneWire(PIN_DS18B20);
DallasTemperature sensoresDS18B20(&oneWire);
float temperaturaDeseada = 26.0;
bool estadoCama = false;

unsigned long ultimaLectura = 0;
const unsigned long intervaloLectura = 2000;

int32_t thermistor_get_resistance(uint16_t adcval) {
  if (adcval == 0) return -1;
  return CONFIG_THERMISTOR_RESISTOR * (1023.0 / adcval - 1.0);
}

float thermistor_get_temperature(int32_t resistance) {
  const float beta = 3950.0;
  const float T0 = 298.15;  // 25°C en Kelvin
  const float R0 = 100000.0;

  float tempK = 1.0 / (1.0 / T0 + (1.0 / beta) * log(resistance / R0));
  return tempK - 273.15;
}

void iniciarControlTemperatura() {
  pinMode(PIN_CAMA_CALIENTE, OUTPUT);
  digitalWrite(PIN_CAMA_CALIENTE, LOW);
  sensoresDS18B20.begin();
  Serial.println("Sistema de control de temperatura inicializado.");
}

float obtenerTemperaturaAgua() {
  sensoresDS18B20.requestTemperatures();
  return sensoresDS18B20.getTempCByIndex(0);
}

float obtenerTemperaturaCama() {
  uint16_t adcValue = analogRead(PIN_NTC);
  int32_t resistencia = thermistor_get_resistance(adcValue);
  return thermistor_get_temperature(resistencia);
}

void controlarCama() {
  if (millis() - ultimaLectura >= intervaloLectura) {
    ultimaLectura = millis();

    float tempAgua = obtenerTemperaturaAgua();
    float tempCama = obtenerTemperaturaCama();

    // Lógica de control igual al standalone
    if (tempAgua >= 40.0 || tempCama >= 70.0) {
      digitalWrite(PIN_CAMA_CALIENTE, HIGH);  // Apagar cama
      estadoCama = false;
    } else if (tempAgua < temperaturaDeseada && tempCama < 60.0) {
      digitalWrite(PIN_CAMA_CALIENTE, LOW);  // Encender cama
      estadoCama = true;
    } else {
      digitalWrite(PIN_CAMA_CALIENTE, HIGH);  // Apagar cama por seguridad
      estadoCama = false;
    }

    // Monitor Serial
    Serial.print("Temp Agua (DS18B20): ");
    Serial.print(tempAgua, 1);
    Serial.print(" °C  |  Temp Cama (NTC): ");
    Serial.print(tempCama, 1);
    Serial.print(" °C  |  Cama: ");
    Serial.println(estadoCama ? "ON" : "OFF");
  }
}
float getTemperaturaDeseada() {
  return temperaturaDeseada;
}

void setTemperaturaDeseada(float nuevaTemp) {
  temperaturaDeseada = nuevaTemp;
}
