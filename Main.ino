  #include <U8g2lib.h>
#include <Wire.h>
#include <RTClib.h>
#include <Servo.h>
#include "control_bomba.h"
#include "Ph.h"
#include "feeder.h"
#include "control_temperatura.h"
#include <Encoder.h>


// ---------------------- Objetos y Pines ----------------------

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, 13, 11, 10);
RTC_DS3231 rtc;

#define ENC_A 2
#define ENC_B 3
#define ENC_BTN 4

const int servo1Pin = 50;
const int servo2Pin = 51;
const int servo3Pin = 52;
const int servo4Pin = 53;
const int bombaPin = 7;

Servo servo1, servo2, servo3, servo4;

// ---------------------- Variables Globales ----------------------


int menuIndex = 0;
int subMenuIndex = -1;
int subMenuOpcion = 0;
bool inSubMenu = false;

Encoder myEncoder(ENC_A, ENC_B);
bool btnPressed = false;
unsigned long lastBtnPress = 0;

unsigned long lastTimeCambioAgua = 0;
bool cambioAguaActivo = false;

unsigned long ultimaLecturaPH = 0;
const unsigned long intervaloPH = 1000;

// Hora para alimentar
int horaFeeder = 12;
int minutoFeeder = 0;



// ---------------------- Setup ----------------------

void setup() {
    Serial.begin(9600);
    pinMode(ENC_BTN, INPUT_PULLUP); 
    pinMode(bombaPin, OUTPUT);

    Wire.begin();
    rtc.begin();
    u8g2.begin();

    servo1.attach(servo1Pin);
    servo2.attach(servo2Pin);
    servo3.attach(servo3Pin);
    servo4.attach(servo4Pin);

    servo1.write(108);
    servo2.write(108);
    servo3.write(108);
    servo4.write(100);
    digitalWrite(bombaPin, LOW);

    iniciarSensorPH(A3, 0, 0);
    iniciarFeeder();
    iniciarControlTemperatura();
    pinMode(triggerPin, OUTPUT);
    pinMode(echoPin, INPUT);

    modo1();

}

// ---------------------- Loop ----------------------

void loop() {
    DateTime now = rtc.now();

    if (!inSubMenu) {
        manejarEncoder(menuIndex, 6);
        dibujarPantallaInicio(now);
    } else {
        if (subMenuIndex == 5) {
            configurarFeeder();  // Menú para configurar hora/minuto del alimentador
        }else if (subMenuIndex == 0) {
            configurarTemperaturaDeseada();
        }else {
            int maxOpciones = (subMenuIndex == 4) ? 4 : 3;
            manejarEncoder(subMenuOpcion, maxOpciones);
            dibujarSubMenu();
        }
    }


    verificarFeeder(now);  // Revisa si se debe activar el alimentador
    controlarCama();
    controlarModoActual(); 
}

// ---------------------- Pantallas y Menús ----------------------

void dibujarPantallaInicio(DateTime now) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);

    const char* opciones[] = {"Temp:", "pH:", "Hora:", "Cambio:", "Limp:", "Feeder"};

    char bufferPH[6];
    dtostrf(obtenerPH(), 4, 2, bufferPH);

    char bufferHora[10];
    sprintf(bufferHora, "%02d:%02d", now.hour(), now.minute());

    char bufferTemp[6];
    dtostrf(obtenerTemperaturaAgua(), 4, 1, bufferTemp);  // 1 decimal, ejemplo: 26.5


    const char* valores[] = {
        bufferTemp,        
        bufferPH,
        bufferHora,
        "OFF",
        "OK",
        "SET"
    };

    for (int i = 0; i < 6; i++) {
        int x = (i % 2) * 64;
        int y = (i / 2) * 20;

        if (menuIndex == i) {
            u8g2.drawBox(x, y, 60, 18);
            u8g2.setDrawColor(0);
        }

        u8g2.drawStr(x + 5, y + 8, opciones[i]);
        u8g2.drawStr(x + 5, y + 16, valores[i]);

        if (menuIndex == i) u8g2.setDrawColor(1);
    }
    
    Serial.print("RTC -> ");
    Serial.println(bufferHora); // Ver si se construyó correctamente
    u8g2.sendBuffer();
}

void dibujarSubMenu() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);

    switch (subMenuIndex) {
        case 4:
            u8g2.drawStr(5, 9, "Limpieza");
            u8g2.drawStr(10, 30, (subMenuOpcion == 0) ? "> Normal" : "  Normal");
            u8g2.drawStr(10, 40, (subMenuOpcion == 1) ? "> Purgar" : "  Purgar");
            u8g2.drawStr(10, 50, (subMenuOpcion == 2) ? "> Llenar" : "  Llenar");
            u8g2.drawStr(10, 60, (subMenuOpcion == 3) ? "> Regresar" : "  Regresar");
            break;
    }

    u8g2.sendBuffer();
}

// ---------------------- Encoder y Navegación ----------------------
void manejarEncoder(int &index, int maxOpciones) {
    static long oldPos = 0;
    long newPos = myEncoder.read();
    
    // Cada 4 pulsos = 1 detente (ajusta según tu encoder)
    if (abs(newPos - oldPos) >= 2) {
        if (newPos > oldPos) {
            index = (index + 1) % maxOpciones;
        } else {
            index = (index - 1 + maxOpciones) % maxOpciones;
        }
        oldPos = newPos;
    }

    // Manejo del botón mejorado
    bool btnState = (digitalRead(ENC_BTN) == LOW);
    
    if (btnState && !btnPressed && (millis() - lastBtnPress > 300)) {
        btnPressed = true;
        lastBtnPress = millis();
        
        if (!inSubMenu) {
            subMenuIndex = menuIndex;
            subMenuOpcion = 0;
            inSubMenu = true;
        } else {
            ejecutarAccionSubMenu();
        }
    }
    else if (!btnState && btnPressed) {
        btnPressed = false;
    }
}

void regresarAlMenuPrincipal() {
    inSubMenu = false;
    subMenuIndex = -1;
}

// ---------------------- Submenús ----------------------

void ejecutarAccionSubMenu() {
    if (subMenuIndex == 4) {
        if (subMenuOpcion == 0) {
            modo1();
            lastTimeCambioAgua = millis();
            cambioAguaActivo = true;
        } else if (subMenuOpcion == 1) {
            modo2();
            lastTimeCambioAgua = millis();
            cambioAguaActivo = true;
        } else if (subMenuOpcion == 2) {
            modo3();
            lastTimeCambioAgua = millis();
            cambioAguaActivo = true;
        }else {
            regresarAlMenuPrincipal();
        }
    }
}

void configurarTemperaturaDeseada() {
    static int tempDeseadaEntera = (int)getTemperaturaDeseada();  // Usamos la función del control de temperatura
    manejarEncoder(tempDeseadaEntera, 40);  // max 39°C por ejemplo

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setCursor(10, 30);
    u8g2.print("Temp: ");
    u8g2.print(tempDeseadaEntera);
    u8g2.print(" C");
    u8g2.sendBuffer();

    if (digitalRead(ENC_BTN) == LOW) {
        delay(300);
        setTemperaturaDeseada((float)tempDeseadaEntera); // Guardar temperatura deseada
        regresarAlMenuPrincipal();
    }
}



// ---------------------- Configuración del Feeder ----------------------

void configurarFeeder() {
    static int etapa = 0;  // 0: seleccionar hora, 1: seleccionar minuto
    static int valorTemporal = horaFeeder;

    manejarEncoder(valorTemporal, (etapa == 0) ? 24 : 60);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    u8g2.setCursor(10, 30);

    if (etapa == 0) {
        u8g2.print("Hora: ");
        u8g2.print(valorTemporal);
    } else {
        u8g2.print("Minuto: ");
        u8g2.print(valorTemporal);
    }

    u8g2.sendBuffer();

    if (digitalRead(ENC_BTN) == LOW) {
        delay(300);
        if (etapa == 0) {
            horaFeeder = valorTemporal;
            valorTemporal = minutoFeeder;
            etapa = 1;
        } else {
            minutoFeeder = valorTemporal;
            etapa = 0;
            regresarAlMenuPrincipal();
        }
    }
}
