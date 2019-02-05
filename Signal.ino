/*
    Steuerungssoftware fuer das Signal in der Simulatorhalle
                  Version: 1.0
*/

//Bibliotheken und Definitionen
#include <Adafruit_MCP23017.h>      //Bibliothek MCP
#include <Wire.h>                   //I2C- Bibliothek
#include <TimerOne.h>               //Timer-Bibliothek für Encoder
#include <ClickEncoder.h>           //Bibliothek Encoder

/*
   Definition der einzelnen Signallampen (Relais)
   Bezeichung der Signallampe <-> Port auf dem MCP
*/
#define hpGruen 1
#define hpRot 2
#define hpGelb 3
#define zs1 4
#define vrGelbO 9
#define vrGruenO 13
#define vrGelbU 11
#define vrGruenU 10
#define lsRot 12
#define lsWeiss 8
#define zs3 0
//#define zs3v 0
//Pins für Encoder
#define pinA 9
#define pinB 8
#define stepsPerNotch 4 // Klicks bis zum nächsten Schritt
#define taster 4     // Taster am Encoder

/*
   Definition der Variablen
*/
bool manu = false;                    //Umschaltung Auto <> Manuell
int randomNumber;                     // Zufallszahl
int lastRandomNumber;                 // Letze Zufallszahl
int lastRandomNumber2;                // Vorletzte Zufallszahl
const byte interruptPin1 = 2;          //Pin für Interrupt
bool tasterPressed = false;
unsigned long time_now = 0;
byte state = LOW;
int laststate;
int last;
int wert;
byte drehen = 0;            //Drehgeber Werte



Adafruit_MCP23017 mcp;                //Definition MCP
ClickEncoder *encoder;

void timerIsr() {
  encoder->service();
}

void setup() {
  /*
     Definition der Pins
  */
  mcp.begin(0);                       //Adresse MCP(Relais)
  mcp.pinMode (hpGruen, OUTPUT);
  mcp.pinMode (hpRot, OUTPUT);
  mcp.pinMode (hpGelb, OUTPUT);
  mcp.pinMode (zs1, OUTPUT);
  mcp.pinMode (zs3, OUTPUT);
  mcp.pinMode (vrGelbO, OUTPUT);
  mcp.pinMode (vrGruenO, OUTPUT);
  mcp.pinMode (vrGelbU, OUTPUT);
  mcp.pinMode (vrGruenU, OUTPUT);
  mcp.pinMode (lsRot, OUTPUT);
  mcp.pinMode (lsWeiss, OUTPUT);

  pinMode(taster, INPUT_PULLUP);

  Serial.begin(9600);
  randomSeed(analogRead(0));          // Seed fuer Zufallszahlen

  attachInterrupt(digitalPinToInterrupt(2), interrupt, LOW);

  encoder = new ClickEncoder(pinB, pinA, taster, stepsPerNotch);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  last = -1;
}

void loop() {
  if (manu == false) {             //Automatikmodus
    setHalt();
    wait(2000);
    last = randomgenerator();
    setHp(false, last);
  }
  if (manu == true) {         //Manueller Modus
    drehen += encoder->getValue();
    if (drehen != last) {
      Serial.println(last);
      if (drehen > 6)
      {
        drehen = 6;
      }
      if (drehen < 0)
      {
        drehen = 0;
      }
      last = drehen;
      setHp(true, last);
    }
  }
}



/*
   Methoden
*/

void setHalt () {
  /*
     Signal wird auf Halt gestellt(Hauptsignal und Ls), Vorsignal und Zs3(v) wird ausgeschaltet
  */
  mcp.digitalWrite(hpRot, HIGH);
  mcp.digitalWrite(hpGruen, LOW);
  mcp.digitalWrite(hpGelb, LOW);
  mcp.digitalWrite(zs1, LOW);
  mcp.digitalWrite(zs3, LOW);
  mcp.digitalWrite(vrGelbO, LOW);
  mcp.digitalWrite(vrGruenO, LOW);
  mcp.digitalWrite(vrGelbU, LOW);
  mcp.digitalWrite(vrGruenU, LOW);
  mcp.digitalWrite(lsWeiss, LOW);
  mcp.digitalWrite(lsRot, HIGH);
  // Serial.println("Hp0");
}

void setHp(bool manual, int state) {
  if ((manual == true) && (laststate != state)) {
    mcp.digitalWrite(hpRot, LOW);
    mcp.digitalWrite(hpGruen, LOW);
    mcp.digitalWrite(hpGelb, LOW);
    mcp.digitalWrite(zs1, LOW);
    mcp.digitalWrite(zs3, LOW);
    mcp.digitalWrite(vrGelbO, LOW);
    mcp.digitalWrite(vrGruenO, LOW);
    mcp.digitalWrite(vrGelbU, LOW);
    mcp.digitalWrite(vrGruenU, LOW);
    mcp.digitalWrite(lsWeiss, LOW);
    mcp.digitalWrite(lsRot, LOW);
  }
  switch (state) {
    case 0:
      setHalt();
      break;
    case 1:
      Serial.println("Hp1");
      mcp.digitalWrite(lsRot, LOW);
      mcp.digitalWrite(lsWeiss, HIGH);
      wait(2000);
      mcp.digitalWrite(hpRot, LOW);
      mcp.digitalWrite(hpGruen, HIGH);
      //setVr();
      break;
    case 2:
      Serial.println("Hp2");
      mcp.digitalWrite(lsRot, LOW);
      mcp.digitalWrite(lsWeiss, HIGH);
      wait(1000);
      mcp.digitalWrite(hpRot, LOW);
      mcp.digitalWrite(hpGelb, HIGH);
      wait(200);
      mcp.digitalWrite(hpGruen, HIGH);
      //setVr();
      break;
    case 3:
      Serial.println("Hp2 + Zs3");
      mcp.digitalWrite(lsRot, LOW);
      mcp.digitalWrite(lsWeiss, HIGH);
      wait(1000);
      mcp.digitalWrite(hpRot, LOW);
      mcp.digitalWrite(hpGelb, HIGH);
      mcp.digitalWrite(zs3, HIGH);
      wait(500);
      mcp.digitalWrite(hpGruen, HIGH);
      setVr();
      break;
    case 4:
      Serial.println("Hp0 + Zs1");
      mcp.digitalWrite(hpRot, HIGH);
      mcp.digitalWrite(lsRot , LOW);
      mcp.digitalWrite(lsWeiss , HIGH);
      mcp.digitalWrite(zs1, HIGH);
      break;
    case 5:
      Serial.println("Hp0 + Sh1");
      mcp.digitalWrite(hpRot, HIGH);
      mcp.digitalWrite(lsRot, LOW);
      mcp.digitalWrite(lsWeiss, HIGH);
      break;
    case 6:
      mcp.digitalWrite(hpRot, HIGH);
      mcp.digitalWrite(hpGruen, HIGH);
      break;
  }
  if (manual == false) {        //Im Automatik Modus: warten bis Signal wieder Auf Hp0 geschaltet wird
    wait(2000);
  }
  if (manual == true) {         //Im manuellen Modus: letzter zustand wird beigehalten bis das Signal umgestellt wird
    laststate = state;
  }
}

void setVr() {
//Platz für das Vorsignal
}

int randomgenerator()       //Zufallsgenerator mit auschluss der letzten zwei erzeugten Zahlen
{
anfang:
  randomNumber = random(1, 5);
  if (randomNumber == lastRandomNumber || randomNumber == lastRandomNumber2) {
    goto anfang;
  }
  lastRandomNumber2 = lastRandomNumber;
  lastRandomNumber = randomNumber;
  Serial.println(randomNumber);
  return randomNumber;
}

void wait(int period)
{
  time_now = millis();
  while (millis() < time_now + period) {
    //wait approx. [period] ms
  }
}

void interrupt () {
  manu = !manu;
}
