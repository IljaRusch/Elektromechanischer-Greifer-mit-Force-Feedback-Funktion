#include <SPI.h>                // Einbindung der SPI Bibliothek (Serial Peripheral Interface)
#include <RF24.h>               // Einbindung der RF24 Bibliothek (WLAN Kommunikation)
#include <Dynamixel2Arduino.h>  // Einbindung der Dnamixel2Arduino Bibliothek (Kommunikation Arduino und Dynamixel Servos)    

//======================== Global Servos ===============================

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560)   // Selektion ob Arduino Uno oder Arduino Mega definiert wurde
  #define ASE_SERIAL   Serial                                   // Definition der Kommunikationsart Serial
  const uint8_t ASE_DIR_PIN = 2;                                // Deklarierung und Initialisierung der Dynamixel Shield Direction Pin Variable
#else                                                           // Bei nicht zutreffender Selektion
  #define ASE_SERIAL   Serial1                                  // Definition der Kommunikationsart Serial1
  const uint8_t ASE_DIR_PIN = 2;                                // Deklarierung und Initialisierung der Dynamixel Shield Direction Pin Variable
#endif                                                          // Ende der Selektion

const uint8_t ASE_ID = 3;                                       // Deklarierung und Initialisierung der ASE ID Variable
const float ASE_PROTOCOL_VERSION = 1.0;                         // Deklarierung und Initialisierung der Protokollversions Variable
Dynamixel2Arduino ASE(ASE_SERIAL, ASE_DIR_PIN);                 // Instanziierung des ASE Objekts

//======================== Global WLAN Kommunikation ===================

RF24 ASE_Funk(9, 10);                                                       // Instanziierung des ASE-Funk Objekts
byte Address[][6] = {"1Node", "2Node", "3Node", "4Node", "5Node", "6Node"}; // Deklarierung und Initialisierung der Pipe für die Funkübertragung                                                              

//======================================================================

void setup()                                          // Void setup
{

//======================== Setup Servos ===============================
  
  ASE.begin(115200);                                  // Beginn der Kommunikation zwischen ASE und Arduino Uno mit eingestellter Baudrate
  ASE.setPortProtocolVersion(ASE_PROTOCOL_VERSION);   // Setzen der Protokoll Version
  ASE.torqueOff(ASE_ID);                              // Deaktivierung des Servos um ihn bewegen zu können
  ASE.setOperatingMode(ASE_ID, OP_POSITION);          // Setzen des Operationsmodus (Fahrbetrieb über Position)

//======================== Setup WLAN Kommunikation ===================

  ASE_Funk.begin();                     // Aktivierung des NRF24L01 Funkmoduls
  ASE_Funk.setAutoAck(1);               // Aktivierung der Acknowledge_Funktion
  ASE_Funk.setRetries(0, 15);           // Die Zeit zwischen den Versuchen den Empfänger zu erreichen, Anzahl der Versuche
  ASE_Funk.enableAckPayload();          // Erlaubt das Senden der Daten auf die Anfrage des Empfängers
  ASE_Funk.setPayloadSize(32);          // Packetgröße in Byte
  ASE_Funk.openWritingPipe(Address[0]); // Öffnen einer Pipe zum Senden
  ASE_Funk.setChannel(45);              // Einstellung der Sende-/Empfangsfrequenz
  ASE_Funk.setPALevel (RF24_PA_MAX);    // Einstelung der Sendeleistung
  ASE_Funk.setDataRate (RF24_2MBPS);    // Einstellung der Datenübertragungsrate
  ASE_Funk.powerUp();                   // Arbeitsbeginn
  ASE_Funk.stopListening();             // Stoppt den Empfang
}

//======================================================================

void loop() {                                       // Void loop

  int Daten[2]={ASE_ID,ASE.getPresentPosition(3)};  // Speichern der ASE ID und der aktuellen ASE Position im Sendedatenspeicher
  
  ASE_Funk.write(&Daten, sizeof(Daten));            // Datengröße wird ermittelt und aus vorgesehenem Speicher ausgelesen und gesendet
  
}
