#include <SoftwareSerial.h>
#include <EEPROM.h>

// Medidor de pulso cardiaco 
// Modificado por: www.elprofegarcia.com
// Visualiza en LCD 16x2 el valor de BPM, grafica el ritmo cardiaco por el Serial Plotter y pulso por el pin 13
// Conexion Sensor Pulsos -> Arduino UNO:   S -> A0 , Vcc -> 5V , (-) -> GND
// Conexion i2C -> Arduino UNO:  SCL-> A5   SDA->A4  VCC->5V  GND->GND

//#include <LiquidCrystal_I2C.h>
//LiquidCrystal_I2C lcd(0x27,16,2);   // inicializa la interfaz I2C del LCD 16x2
int pulsePin = 6;                   // Sensor de Pulso conectado al puerto A6
// Estas variables son volatiles porque son usadas durante la rutina de interrupcion en la segunda Pestaña
volatile int BPM;                   // Pulsaciones por minuto
volatile int Signal;                // Entrada de datos del sensor de pulsos
volatile int IBI = 500;             // tiempo entre pulsaciones
volatile boolean Pulse = false;     // Verdadero cuando la onda de pulsos es alta, falso cuando es Baja
volatile boolean QS = false;        // Verdadero cuando el Arduino Busca un pulso del Corazon

int n = 0;
int bPM = 0;
int cont = 0;

//PANTALLA
#include "U8glib.h"

// setup u8g object, please remove comment from one of the following constructor calls
// IMPORTANT NOTE: The following list is incomplete. The complete list of supported 
// devices with all constructor calls is here: http://code.google.com/p/u8glib/wiki/device

//U8GLIB_SSD1306_128X64 u8g(13, 11, 10, 9);  // SW SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9
//U8GLIB_SSD1306_128X64 u8g(10, 9);   // HW SPI Com: CS = 10, A0 = 9 (Hardware Pins are  SCK = 13 and MOSI = 11)
//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI 
//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);  // Display which does not send ACK
//U8GLIB_SSD1306_128X32 u8g(13, 11, 10, 9); // SW SPI Com: SCK = 13, MOSI = 11, CS = 10, A0 = 9
//U8GLIB_SSD1306_128X32 u8g(10, 9);             // HW SPI Com: CS = 10, A0 = 9 (Hardware Pins are  SCK = 13 and MOSI = 11)

U8GLIB_SSD1306_128X32 u8g(U8G_I2C_OPT_NONE);  // I2C / TWI 


void draw(int BPM) {
  // graphic commands to redraw the complete screen should be placed here  
  u8g.setFont(u8g_font_unifont);
  u8g.setPrintPos(0, 20); 
  // call procedure from base class, http://arduino.cc/en/Serial/Print
  u8g.print(" BPM:");
  u8g.print(BPM);
}



void setup(){
   EEPROM.get(0, n);
   cont = 0;
   pinMode(13, OUTPUT);                              
   Serial.begin(9600);                // Puerto serial configurado a 9600 Baudios
   interruptSetup();                  // Configura la interrucion para leer el sensor de pulsos cada 2mS  
}
  
void loop(){
  while(cont <= 1250){//gets some iterations so that the BPM can be averaged
  cont++;
   if(Serial.available() < 0){
    if(n == 0){
      Serial.println("LABEL,BPM");//the first serial message is the header
    }
    else{
     int i;
     i = 0;
     while(i != n){//sends every BPM saved on EEPROM to serial
      EEPROM.get(i, bPM);
      Serial.println(bPM);
      i++;
     }
    }
   }
  
    
   int pulso = analogRead(A6);           //reads the value of pulsemeter on A0
   if (BPM > 100 || BPM < 60) {          // turns on led 13 when the pulse is out of the ordinary
      digitalWrite(2, HIGH);
   }  
   else{
      digitalWrite(2, LOW);
   }
   if(cont == 1250){//only saves the BPM on EEPROM and prints on OLED on the last iteration
    n++;
    //Serial.print("BPM: ");
    //Serial.print(BPM);
    //Serial.print("\n");
    EEPROM.put(0, n);//updates the value of n
    EEPROM.put(n, BPM);//saves the BPM on the n position
    //OLED Print
     u8g.firstPage();
    do {
      draw(BPM);//prints the BPM on the OLED screen
    } while( u8g.nextPage() );
    // rebuild the picture after some delay
    delay(500);
   }   
    if (QS == true){                       // Bandera del Quantified Self es verdadera cuando el Arduino busca un pulso del corazon
      QS = false;                          // Reset a la bandera del Quantified Self 
    }
  }
  delay(2500);//delay to save memory
  cont = 0;
}
