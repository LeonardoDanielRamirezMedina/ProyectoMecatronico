#ifndef CONTROL_TEMPERATURA_H
#define CONTROL_TEMPERATURA_H

void iniciarControlTemperatura();
void controlarCama();
float obtenerTemperaturaAgua();
float obtenerTemperaturaCama();
float getTemperaturaDeseada();
void setTemperaturaDeseada(float nuevaTemp);

// Acceso desde el main
extern float temperaturaDeseada;

#endif
