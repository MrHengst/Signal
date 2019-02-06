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
#define HPGRUEN 1
#define HPROT 2
#define HPGELB 3
#define ZS1 4
#define VRGELBO 9
#define VRGRUENO 13
#define VRGELBU 11
#define VRGRUENU 10
#define LSROT 12
#define LSWEISS 8
#define ZS3 0
//#define ZS3V 0
//Pins für Encoder
#define PINA 8
#define PINB 7
#define STEPSPERNOTCH 4 // Klicks bis zum nächsten Schritt
#define TASTER 4     // TASTER am Encoder
#define INTERRUPTPIN 2
/*
   Definition der Variablen
*/
bool manu = false;                    //Umschaltung Auto <> Manuell
byte randomNumber;                     // Zufallszahl
byte lastRandomNumber;                 // Letze Zufallszahl
byte lastRandomNumber2;                // Vorletzte Zufallszahl
byte randomNumberVr;                     // Zufallszahl für Vorsignal
byte lastRandomNumberVr;                 // Letze Zufallszahl für Vorsignal
byte lastRandomNumber2Vr;                // Vorletzte Zufallszahl für Vorsignal
const byte interruptPin1 = 2;          //Pin für Interrupt
bool TASTERPressed = false;
unsigned long time_now = 0;
byte state = LOW;
byte laststate;
byte laststateVr;
unsigned int last;
int lastVr;
int wert;
byte drehen = 0;            //Drehgeber Werte

Adafruit_MCP23017 mcp;                //Definition MCP
ClickEncoder *encoder;

void timerIsr() {
  encoder->service();
}

/*
   Funktionsprototypen
*/
void setHP(bool, int);
void setVr(bool, int);
void setHalt();
int randomgeneratorHP();
int randomgeneratorVr();
void wait(int);
void interrupt();

void setup() {
  /*
     Definition der Pins
  */
  mcp.begin(0);                       //Adresse MCP(Relais)
  mcp.pinMode (HPGRUEN, OUTPUT);
  mcp.pinMode (HPROT, OUTPUT);
  mcp.pinMode (HPGELB, OUTPUT);
  mcp.pinMode (ZS1, OUTPUT);
  mcp.pinMode (ZS3, OUTPUT);
  mcp.pinMode (VRGELBO, OUTPUT);
  mcp.pinMode (VRGRUENO, OUTPUT);
  mcp.pinMode (VRGELBU, OUTPUT);
  mcp.pinMode (VRGRUENU, OUTPUT);
  mcp.pinMode (LSROT, OUTPUT);
  mcp.pinMode (LSWEISS, OUTPUT);
  pinMode(TASTER, INPUT_PULLUP);
  pinMode(13, OUTPUT);
  pinMode(INTERRUPTPIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(INTERRUPTPIN), interrupt, LOW);

  Serial.begin(9600);
  randomSeed(analogRead(0));          // Seed fuer Zufallszahlen


  encoder = new ClickEncoder(PINB, PINA, TASTER, STEPSPERNOTCH);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  last = 0;

  setHalt();
}

void loop() {
  if (manu == false) {             //Automatikmodus
    digitalWrite (13, LOW);
    setHalt();
    wait(2000);
    last = randomgeneratorHP();
    setHp(false, last);
  }
  if (manu == true) {         //Manueller Modus
    digitalWrite (13, HIGH);
    drehen += encoder->getValue();
    if (drehen != last) {
      Serial.println(last);
      if (drehen > 6)
      {
        drehen = 6;
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
     Signal wird auf Halt gestellt(Hauptsignal und Ls), Vorsignal und ZS3(v) wird ausgeschaltet
  */
  mcp.digitalWrite(HPROT, HIGH);
  mcp.digitalWrite(HPGRUEN, LOW);
  mcp.digitalWrite(HPGELB, LOW);
  mcp.digitalWrite(ZS1, LOW);
  mcp.digitalWrite(ZS3, LOW);
  mcp.digitalWrite(VRGELBO, LOW);
  mcp.digitalWrite(VRGRUENO, LOW);
  mcp.digitalWrite(VRGELBU, LOW);
  mcp.digitalWrite(VRGRUENU, LOW);
  mcp.digitalWrite(LSWEISS, LOW);
  mcp.digitalWrite(LSROT, HIGH);
  // Serial.println("Hp0");
}

void setHp(bool manual, int state) {
  if ((manual == true) && (laststate != state)) {
    mcp.digitalWrite(HPROT, LOW);
    mcp.digitalWrite(HPGRUEN, LOW);
    mcp.digitalWrite(HPGELB, LOW);
    mcp.digitalWrite(ZS1, LOW);
    mcp.digitalWrite(ZS3, LOW);
    //    mcp.digitalWrite(VRGELBO, LOW);
    //    mcp.digitalWrite(VRGRUENO, LOW);
    //    mcp.digitalWrite(VRGELBU, LOW);
    //    mcp.digitalWrite(VRGRUENU, LOW);
    mcp.digitalWrite(LSWEISS, LOW);
    mcp.digitalWrite(LSROT, LOW);
  }
  switch (state) {
    case 0:
      setHalt();
      break;
    case 1:
      Serial.println("Hp1");
      mcp.digitalWrite(LSROT, LOW);
      mcp.digitalWrite(LSWEISS, HIGH);
      wait(2000);
      mcp.digitalWrite(HPROT, LOW);
      mcp.digitalWrite(HPGRUEN, HIGH);
      if (manual == false) {
        lastVr = randomgeneratorVr();
        wait(500);
        setVr(manual, lastVr);
      }
      break;
    case 2:
      Serial.println("Hp2");
      mcp.digitalWrite(LSROT, LOW);
      mcp.digitalWrite(LSWEISS, HIGH);
      wait(1000);
      mcp.digitalWrite(HPROT, LOW);
      mcp.digitalWrite(HPGELB, HIGH);
      wait(200);
      mcp.digitalWrite(HPGRUEN, HIGH);
      if (manual == false) {
        lastVr = randomgeneratorVr();
        wait(500);
        setVr(manual, lastVr);
      }
      break;
    case 3:
      Serial.println("Hp2 + ZS3");
      mcp.digitalWrite(LSROT, LOW);
      mcp.digitalWrite(LSWEISS, HIGH);
      wait(1000);
      mcp.digitalWrite(HPROT, LOW);
      mcp.digitalWrite(HPGELB, HIGH);
      mcp.digitalWrite(ZS3, HIGH);
      wait(500);
      mcp.digitalWrite(HPGRUEN, HIGH);
      if (manual == false) {
        lastVr = randomgeneratorVr();
        wait(500);
        setVr(manual, lastVr);
      }
      break;
    case 4:
      Serial.println("Hp0 + ZS1");
      mcp.digitalWrite(HPROT, HIGH);
      mcp.digitalWrite(LSROT , LOW);
      mcp.digitalWrite(LSWEISS , HIGH);
      mcp.digitalWrite(ZS1, HIGH);
      break;
    case 5:
      Serial.println("Hp0 + Sh1");
      mcp.digitalWrite(HPROT, HIGH);
      mcp.digitalWrite(LSROT, LOW);
      mcp.digitalWrite(LSWEISS, HIGH);
      break;
    case 6:
      mcp.digitalWrite(HPROT, HIGH);
      mcp.digitalWrite(HPGRUEN, HIGH);
      break;
  }
  if (manual == false) {        //Im Automatik Modus: warten bis Signal wieder Auf Hp0 geschaltet wird
    wait(2000);
  }
  if (manual == true) {         //Im manuellen Modus: letzter zustand wird beigehalten bis das Signal umgestellt wird
    laststate = state;
  }
}
void setVr(bool manual, int state) {
  if ((manual == true) && (laststateVr != state)) {
    mcp.digitalWrite(VRGELBO, LOW);
    mcp.digitalWrite(VRGRUENO, LOW);
    mcp.digitalWrite(VRGELBU, LOW);
    mcp.digitalWrite(VRGRUENU, LOW);
    //mcp.digitalWrite(ZS3V, LOW);
  }

  switch (state) {
    case 1:
      mcp.digitalWrite(VRGELBO, HIGH);
      mcp.digitalWrite(VRGRUENO, LOW);
      mcp.digitalWrite(VRGELBU, HIGH);
      mcp.digitalWrite(VRGRUENU, LOW);
      //mcp.digitalWrite(ZS3V, LOW);
      Serial.println("Vr0");
      break;
    case 2:
      mcp.digitalWrite(VRGELBO, LOW);
      mcp.digitalWrite(VRGRUENO, HIGH);
      mcp.digitalWrite(VRGELBU, LOW);
      mcp.digitalWrite(VRGRUENU, HIGH);
      //mcp.digitalWrite(ZS3V, LOW);
      Serial.println("Vr1");
      break;
    case 3:
      mcp.digitalWrite(VRGELBO, LOW);
      mcp.digitalWrite(VRGRUENO, HIGH);
      mcp.digitalWrite(VRGELBU, HIGH);
      mcp.digitalWrite(VRGRUENU, LOW);
      //mcp.digitalWrite(ZS3V, LOW);
      Serial.println("Vr2");
      break;
    case 4:
      mcp.digitalWrite(VRGELBO, LOW);
      mcp.digitalWrite(VRGRUENO, HIGH);
      mcp.digitalWrite(VRGELBU, HIGH);
      mcp.digitalWrite(VRGRUENU, LOW);
      //mcp.digitalWrite(ZS3V, HIGH);
      Serial.println("Vr2 + Zs3v");
      break;
    case 5:
      mcp.digitalWrite(VRGELBO, HIGH);
      mcp.digitalWrite(VRGRUENO, LOW);
      mcp.digitalWrite(VRGELBU, HIGH);
      mcp.digitalWrite(VRGRUENU, LOW);
      //mcp.digitalWrite(ZS3V, LOW);
      Serial.println("Vr0 -> Vr2");
      wait(2000);
      mcp.digitalWrite(VRGELBO, LOW);
      mcp.digitalWrite(VRGRUENO, HIGH);
      mcp.digitalWrite(VRGELBU, HIGH);
      mcp.digitalWrite(VRGRUENU, LOW);
      //mcp.digitalWrite(ZS3V, LOW);
      break;
  }
}

int randomgeneratorHP()       //Zufallsgenerator Hauptsignal mit auschluss der letzten zwei erzeugten Zahlen
{
  randomNumber = random(1, 5);
  if (randomNumber == lastRandomNumber || randomNumber == lastRandomNumber2) {
    randomgeneratorHP();
  }
  lastRandomNumber2 = lastRandomNumber;
  lastRandomNumber = randomNumber;
  Serial.println(randomNumber);
  return randomNumber;
}

int randomgeneratorVr()       //Zufallsgenerator Hauptsignal mit auschluss der letzten zwei erzeugten Zahlen
{
  randomNumberVr = random(1, 6);
  if (randomNumberVr == lastRandomNumberVr || randomNumberVr == lastRandomNumber2Vr) {
    randomgeneratorVr();
  }
  lastRandomNumber2Vr = lastRandomNumberVr;
  lastRandomNumberVr = randomNumberVr;
  Serial.println(randomNumberVr);
  return randomNumberVr;
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
