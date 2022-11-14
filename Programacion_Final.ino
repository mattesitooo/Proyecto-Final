#define A0 (14)
#define A1 (15)
#define A2 (16)
#define A3 (17)
#define A  (2)
#define B  (3)
#define Led (4)
#include "LedControl.h"
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 20, 4);
LedControl lc = LedControl(7, 9, 8, 1);
File myFile;
volatile int PULSOS = 0;
int ANTERIOR = 0;
int METROS;
float Distancia;
int Distancia2;

const byte botonDOWN = 14;
const byte botonUP = 15;
const byte botonBACK = 16;
const byte botonSelect = 17;
const byte botonRESET = 6;
const byte botonSave = 5;

bool SELECT = LOW;
bool SAVE = LOW;
bool BACK = LOW;
bool RESET;
bool DOWN = LOW;
bool UP = LOW;

unsigned long UltimoAntiRebote = 0;
unsigned long delayAntiRebote = 10;
int Estado_Previo = LOW;    //lastButtonState
String Registro;
static int Pos1;
int id = 0;
////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  if (!SD.begin(10)) {
    lcd.setCursor(2, 1);
    lcd.print ("Error de targeta");
    lcd.setCursor(10, 2);
    lcd.print ("SD");
    while (true);
  }
  if (! rtc.begin()) {
    lcd.print("Error Modulo RTC");
    while (1) delay(10);
  }
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  Menu();
  lc.shutdown(0, false);    // inicia el MAX7219
  lc.setIntensity(0, 5);    // establece nivel de brillo
  lc.clearDisplay(0);       // borra displays
  
  pinMode (A, INPUT);       //DT   (encoder)
  pinMode (B, INPUT);       //CLK  (encoder)
  pinMode (Led, OUTPUT);
  pinMode (botonSelect, INPUT);
  pinMode (botonSave, INPUT);
  pinMode (botonBACK, INPUT);
  pinMode (botonRESET, INPUT);
  pinMode (botonDOWN, INPUT);
  pinMode (botonUP, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(A), encoder, LOW);
  digitalWrite (Led, LOW);
  Registro = String();
  
  File myFile = SD.open("Contador.txt", FILE_WRITE);
  id =(myFile.size()/22);
  //SD.remove("Contador.txt");
}
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
void loop() {
  SELECT = digitalRead (botonSelect);
  SAVE = digitalRead (botonSave);
  BACK = digitalRead (botonBACK);
  DOWN = digitalRead (botonDOWN);
  UP = digitalRead (botonUP);
  DateTime now = rtc.now();
  
  Contador();
  Reset();
  
  if (SELECT == HIGH) {
    Seleccionar();
  }
  if (SAVE == HIGH) {
    Guardado();
  }
  if (BACK == HIGH) {
    Menu();
  }
  if (DOWN == HIGH) {
    HistorialDOWN();
  }
  if (UP == HIGH) {
    Pos1=(myFile.position());
    Pos1-= 132;
    HistorialUP();
  }
}
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
void Contador() {
  Distancia = (METROS) / 10.0;
  Distancia2 = (METROS / 10);
  if (PULSOS != ANTERIOR) {
    ANTERIOR = PULSOS;
    }
  METROS = PULSOS / 4.412;
  lc.setDigit(0, 0, (METROS / 1) % 10, false);
  lc.setDigit(0, 1, (METROS / 10) % 10, true);
  if (METROS > 99) {
    lc.setDigit(0, 2, (METROS / 100) % 10, false);
  }
  if (METROS > 999) {
    lc.setDigit(0, 3, (METROS / 1000) % 10, false);
  }
}
////////////////////////////////////////////////////////////////
void Guardado(){
    int coord;
    DateTime now = rtc.now();
    char Fecha[] = "DD/MM ";
    char Hora[] = "hh:mm";
    myFile = SD.open("Contador.txt");
    id =(myFile.size()/22);
    coord = (myFile.size());
    coord = coord - 22;
    if (coord < 0){
      id = 0;
    }else{
    myFile.seek(coord);
    myFile.read();
    Serial.println (id);
    }
    myFile.close();
    if (METROS < 1) {
      lcd.clear();
      lcd.setCursor(2, 1);
      lcd.print ("NO se registraron");
      lcd.setCursor(5, 2);
      lcd.print ("Mediciones");
      delay(1700);
      lcd.clear();
      Menu();
    }else{
      File myFile = SD.open("Contador.txt", FILE_WRITE);
      if (id > 99){
        id-= 100;
        myFile.print (id);
      }else{
        if (id > 9){
          myFile.print (id);
          myFile.print (" ");
        }else{
          myFile.print (id);
          myFile.print ("  ");
        }
      }
      myFile.print(now.toString(Fecha));
      myFile.print(now.toString(Hora));
      if (METROS < 10) {
        myFile.print("  ");
        myFile.println(Distancia);
    }else{
      if (METROS < 100) {
        myFile.print("  ");
        myFile.println(Distancia);
    }else{
      if (METROS < 1000) {
        myFile.print(" ");
        myFile.println(Distancia);
    }else{
        myFile.print(" ");
        myFile.print(Distancia2);
        myFile.println("  ");   
        }
      }
    }
  myFile.close();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fecha: ");
  lcd.print(now.toString(Fecha));
  lcd.setCursor(0, 1);
  lcd.print("Hora: ");
  lcd.print(now.toString(Hora));
  lcd.setCursor(0, 2);
  lcd.print("Distancia: ");
  lcd.print(Distancia);
  lcd.setCursor(15, 2);
  lcd.print(" Mts.");
  lcd.setCursor(6, 3);
  lcd.print ("GUARDADO");
  delay(2500);
  lcd.clear();
  Menu();
  }
  id++;
}         
////////////////////////////////////////////////////////////////
void Seleccionar() {
  myFile = SD.open("Contador.txt");
  Pos1=0;
  if (myFile.available()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("---{ HISTORIAL: }---");
    myFile.seek (Pos1);
    for (int u = 1; u < 4 ; u++) {
      for (int i = 0; i < 22 ; i++) {
        char letras = myFile.read();
        Registro = letras;
        Registro.trim();
        lcd.setCursor(i, u);
        lcd.print(Registro);
        lcd.setCursor(19, u);
        lcd.print("m");
      }
      delay (10);
      if (myFile.available()) {}
      else{
        myFile.close();
        break;
        }
    }
    Pos1=(myFile.position());
  }else {
    lcd.clear();
    lcd.setCursor(4, 2);
    lcd.print("Sin Registros");
    myFile.close();
    delay (1000);
    lcd.clear();
    Menu();
  }
}
////////////////////////////////////////////////////////////////
void HistorialDOWN() {
  if (myFile.available()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("---{ HISTORIAL: }---");
    myFile.seek (Pos1);
    for (int u = 1; u < 4 ; u++) {
      for (int i = 0; i < 22 ; i++) {
        char letras = myFile.read();
        Registro = letras;
        Registro.trim();
        lcd.setCursor(i, u);
        lcd.print(Registro);
        lcd.setCursor(19, u);
        lcd.print("m");
      }
      delay (10);
      if (myFile.available()) {}
      else{break;}
    }
    Pos1=(myFile.position());
  } else {
    Pos1=(myFile.size());
    Pos1-= 66;
    myFile.close();
  }
}
////////////////////////////////////////////////////////////////
void HistorialUP() {
  lcd.clear();
    if (Pos1 < 66){
       if (Pos1 < 0){
      Pos1=0;
    }
  }
    if (myFile.available()) {
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("---{ HISTORIAL: }---");
    myFile.seek(Pos1);
    for (int u = 1; u < 4 ; u++) {
      for (int i = 0; i < 22 ; i++) {
        char letras = myFile.read();
        Registro = letras;
        Registro.trim();
        lcd.setCursor(i, u);
        lcd.print(Registro);
        lcd.setCursor(19, u);
        lcd.print("m");
      }
      delay (20);
    }
    Pos1-= 66;
  }else{
    lcd.clear(); 
    myFile.close();
    myFile = SD.open("Contador.txt");
    Pos1=(myFile.size());
    Pos1-= 88;
    HistorialUP();
  }
  Pos1=(myFile.position());
}
////////////////////////////////////////////////////////////////
void Reset(){
  bool reading = digitalRead(botonRESET); 
  if (reading != Estado_Previo) {
      UltimoAntiRebote = millis();
    }
    if ((millis() - UltimoAntiRebote) > delayAntiRebote) {
      if (reading != RESET) {
        RESET = reading;
        if (RESET == HIGH) {
          lc.clearDisplay(0);
          PULSOS = 0;
          BACK = digitalRead (botonBACK);
          DOWN = digitalRead (botonDOWN);
          if (BACK == HIGH && DOWN == HIGH) {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("-Vaciando Historial-");
            lcd.setCursor(0, 2);
            lcd.print("Mantenga...");
            for (byte i = 3; i > 0 ; i--) {
              digitalWrite (Led, HIGH);
              lcd.setCursor(13, 2);
              lcd.print(i);
              delay(500);
              digitalWrite (Led, LOW);
              delay(500);
            }
            BACK = digitalRead (botonBACK);
            DOWN = digitalRead (botonDOWN);
          }
          if (BACK == HIGH && DOWN == HIGH) {
            lcd.clear();
            lcd.setCursor(2, 1);
            lcd.print ("Registro Borrado");
            File myFile = SD.open("Contador.txt", FILE_WRITE);
            SD.remove("Contador.txt");
            myFile.close();
            delay(1000);
            lcd.clear();
            Menu();
          }
          Menu();
        }
      }
    }
    
    Estado_Previo = reading;
}
////////////////////////////////////////////////////////////////
void Menu(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("------{ MENU }------");
  lcd.setCursor(0, 2);
  lcd.print(">> HISTORIAL");
}
////////////////////////////////////////////////////////////////
void encoder()  {
  static unsigned long ultimaInterrupcion = 0;  // tiempo a ultima interrupcion
  unsigned long tiempoInterrupcion = millis();  // variable almacena valor de func. millis
  if (tiempoInterrupcion - ultimaInterrupcion > 2) {  // rutina antirebote desestima pulsos menores a 1 mseg.
    if (digitalRead(B) == HIGH) {     // si B es HIGH, sentido horario
      PULSOS++ ;                      // incrementa POSICION en 1
    } else {                          // si B es LOW, sentido anti horario
      lc.clearDisplay(0);
      PULSOS-- ;                      // decrementa POSICION en 1
    }
    PULSOS = min(999999999, max(0, PULSOS));  // establece limite para POSICION
    ultimaInterrupcion = tiempoInterrupcion;  // guarda valor actualizado del tiempo de la interrupcion en variable static
  }
}
