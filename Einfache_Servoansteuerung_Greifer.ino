#include <SPI.h>                // Einbindung der SPI Bibliothek (Serial Peripheral Interface)
#include <RF24.h>               // Einbindung der RF24 Bibliothek (WLAN Kommunikation)
#include <Dynamixel2Arduino.h>  // Einbindung der Dnamixel2Arduino Bibliothek (Kommunikation Arduino und Dynamixel Servos)

//======================== Global Servos ===============================

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560)   // Selektion ob Arduino Uno oder Arduino Mega definiert wurde
  #define Greifer_SERIAL   Serial                               // Definition der Kommunikationsart Serial
  const uint8_t Greifer_DIR_PIN = 2;                            // Deklarierung und Initialisierung der Dynamixel Shield Direction Pin Variable
#else                                                           // Bei nicht zutreffender Selektion
  #define Greifer_SERIAL   Serial1                              // Definition der Kommunikationsart Serial1
  const uint8_t Greifer_DIR_PIN = 2;                            // Deklarierung und Initialisierung der Dynamixel Shield Direction Pin Variable
#endif                                                          // Ende der Selektion

const uint8_t Gripper_ID1 = 1;                                  // Deklarierung und Initialisierung der Gripper1 ID Variable
const uint8_t Gripper_ID2 = 2;                                  // Deklarierung und Initialisierung der Gripper2 ID Variable
const uint32_t Timeout=100;                                     // Deklarierung und Initialisierung der timeout Variable für die Kommunikation zwischen Dynamixel Shield und Dynamixel Servo in ms
const float Greifer_PROTOCOL_VERSION = 1.0;                     // Deklarierung und Initialisierung der Protokollversions Variable
Dynamixel2Arduino Gripper1(Greifer_SERIAL, Greifer_DIR_PIN);    // Instanziierung des Gripper1 Objekts
Dynamixel2Arduino Gripper2(Greifer_SERIAL, Greifer_DIR_PIN);    // Instanziierung des Gripper2 Objekts

//======================== Global WLAN Kommunikation ===================

RF24 Greifer_Funk(9,10);                                                // Instanziierung des Greifer-Funk Objekts
byte Address[][6] = {"1Node","2Node","3Node","4Node","5Node","6Node"};  //Bezeichnugen der Pipes
int Position;                                                           // Deklarierung der Variable für die umgerechnete Greifer Position

//======================================================================

void setup(){                                             // Void setup

//======================== Setup Servos ===============================

  Gripper1.begin(115200);                                     // Beginn der Kommunikation zwischen Gripper1 und Arduino Uno mit eingestellter Baudrate
  Gripper2.begin(115200);                                     // Beginn der Kommunikation zwischen Gripper2 und Arduino Uno mit eingestellter Baudrate
  Gripper1.setPortProtocolVersion(Greifer_PROTOCOL_VERSION);  // Setzen der Protokoll Version
  Gripper2.setPortProtocolVersion(Greifer_PROTOCOL_VERSION);  // Setzen der Protokoll Version
  Gripper1.torqueOff(Gripper_ID1);                            // Deaktivierung des Servos vom Gripper1
  Gripper2.torqueOff(Gripper_ID2);                            // Deaktivierung des Servos vom Gripper2
  Gripper1.setOperatingMode(Gripper_ID1, OP_POSITION);        // Setzen des Operationsmodus (Fahrbetrieb über Position)
  Gripper2.setOperatingMode(Gripper_ID2, OP_POSITION);        // Setzen des Operationsmodus (Fahrbetrieb über Position)
  Gripper1.torqueOn(Gripper_ID1);                             // Aktivierung des Servos vom Gripper1
  Gripper2.torqueOn(Gripper_ID2);                             // Aktivierung des Servos vom Gripper2

//======================== Setup WLAN Kommunikation ===================
 
  Greifer_Funk.begin();                       // Aktivierung des NRF24L01 Funkmoduls
  Greifer_Funk.setAutoAck(1);                 // Aktivierung der Acknowledge_Funktion
  Greifer_Funk.setRetries(0,15);              // Die Zeit zwischen den Versuchen den Empfänger zu erreichen, Anzahl der Versuche
  Greifer_Funk.enableAckPayload();            // Erlaubt das Senden der Daten auf die Anfrage des Empfängers
  Greifer_Funk.setPayloadSize(32);            // Packetgröße in Byte
  Greifer_Funk.openReadingPipe(1,Address[0]); // Öffnen einer Pipe zum Empfangen
  Greifer_Funk.setChannel(45);                // Einstellung der Sende-/Empfangsfrequenz
  Greifer_Funk.setPALevel (RF24_PA_MAX);      // Einstelung der Sendeleistung
  Greifer_Funk.setDataRate (RF24_2MBPS);      // Einstellung der Datenübertragungsrate
  Greifer_Funk.powerUp();                     // Arbeitsbeginn
  Greifer_Funk.startListening();              // Startet den Empfang
  
}

void loop() {                                       // Void loop
    byte PipeNo;                                    // Deklarierung der Variable für die Pipe Nummer
    int Daten[2];                                   // Deklarierung des Empfangsdatenspeichers
    while( Greifer_Funk.available(&PipeNo)){        // Solange Daten empfangen werden
      Greifer_Funk.read( &Daten, sizeof(Daten) );   // Datengröße wird ermittelt und im vorgesehenem Speicher abgelegt

      int Empfangen1=Daten[0];                      // Speichern der empfangenen ASE ID
      int Empfangen2=Daten[1];                      // Speichern der empfangenen ASE Position

      Position= 512-(Empfangen2-512);               // Rechnet die empfangene ASE Position um für die Greifer Position
  
  Gripper1.setGoalPosition(Gripper_ID1, Position ); // Gripper1 fährt auf die umgerechnete Position
  Gripper2.setGoalPosition(Gripper_ID2, Position ); // Gripper2 fährt auf die umgerechnete Position
   
   }
}
