//Verbesserungswürdig aber okee.

//based on @achnina (https://achnina.info/logbuch/f/die-jodelnde-trinkflasche, https://create.arduino.cc/editor/achninaa/1f74cf62-a077-4d1f-be6b-a2a84c827b16/preview)

#include "DFRobotDFPlayerMini.h"
#include <Arduino.h>
#include "HX711.h"

//Zum Kalibrieren
#define calibration_factor 430 //Vorher festgestellten Wert einsetzen

//Pins vom HX711
#define DOUT  26
#define CLK  25

HX711 scale;

unsigned long timealt;
int wiegen = 0;
int altgewicht;
int trinksoll = 150;   // Wie viel Gramm sollen fehlen?

int busy = 33;


HardwareSerial mySoftwareSerial(1);
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

void setup() {
  Serial.begin(9600);
  mySoftwareSerial.begin(9600, SERIAL_8N1, 19, 18);  // speed, type, RX, TX
  pinMode(busy, INPUT);

  //Standard DF Player Code @achnina
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(mySoftwareSerial, false)) {  //Use softwareSerial to communicate with mp3.

    Serial.println(myDFPlayer.readType(), HEX);
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while (true);
  }
  Serial.println(F("DFPlayer Mini online."));

  myDFPlayer.setTimeOut(500); //Set serial communictaion time out 500ms

  //----Set volume----
  myDFPlayer.volume(30);  //Set volume value (0~30).

  myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  int delayms = 100;
  //Standard DF Copy Pasta Ende


  scale.begin(DOUT, CLK);
  scale.set_scale(calibration_factor); 
  delay(2000);
  scale.tare(); //Reset scale to 0
  timealt = millis();
  myDFPlayer.play(5);
}


void loop() {
  //Wiegen
  Serial.print("Reading: ");
  Serial.print(scale.get_units(), 1); 
  Serial.print(" g"); 
  Serial.println();

  //Wenn etwas neu drauf steht (>50g) oder das Gewicht höher ist als vorher (neue Flasche starten),
  //dann setze das Startgewicht und beginne Timer!  
  if (scale.get_units() > 50 && wiegen == 0 or scale.get_units() > altgewicht + 100) {
    delay(2000); //Doppelte Abfrage mit Pause um falsche Messungen beim Draufstellen zu vermeiden
    if (scale.get_units() > 50 or scale.get_units() > altgewicht + 100) {
      altgewicht = scale.get_units();
      Serial.print("Gewicht wird gesetzt: ");
      Serial.println(altgewicht);
      wiegen ++;
    }
  }

// Wenn Timer abgelaufen und etwas steht auf der Plattform:

  if (timealt + 10000 <= millis() && scale.get_units() > 50) {     // Hier Timerlänge bestimmen, zum Testen auf 10 Sekunden
    Serial.println("Timer abgelaufen");
    //Solange jodeln, bis genug Gewicht fehlt
    while (scale.get_units() > altgewicht - trinksoll or scale.get_units() < 50) {
      if (digitalRead(busy) == HIGH) {
        myDFPlayer.play(random(0, 5));
      }
      //Auch aufhören, wenn neue volle Flasche aufgestellt wird
      if (scale.get_units() > altgewicht + 100) {
        break;
      }
      delay(1000);
    }
    //Durchlauf beendet, alles stoppen und neu setzen
    myDFPlayer.stop();
    wiegen = 0;
    Serial.print("Timer und Gewicht wird neu gesetzt.");
    timealt = millis();
  }


} //Loop Ende ------------------------
