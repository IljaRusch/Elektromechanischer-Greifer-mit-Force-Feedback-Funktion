#include <SPI.h>                     // Einbindung der SPI Bibliothek (Serial Peripheral Interface)
#include <RF24.h>                    // Einbindung der RF24 Bibliothek (WLAN Kommunikation)
#include <Dynamixel2Arduino.h>       // Einbindung der Dnamixel2Arduino Bibliothek (Kommunikation Arduino und Dynamixel Servos)

//======================== Global Servos ===============================

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560)     // Selektion ob Arduino Uno oder Arduino Mega definiert wurde
  #define ASE_SERIAL   Serial                                     // Definition der Kommunikationsart Serial
  const uint8_t ASE_DIR_PIN = 2;                                  // Deklarierung und Initialisierung der Dynamixel Shield Direction Pin Variable
#else                                                             // Bei nicht zutreffender Selektion
  #define ASE_SERIAL   Serial1                                    // Definition der Kommunikationsart Serial1
  const uint8_t ASE_DIR_PIN = 2;                                  // Deklarierung und Initialisierung der Dynamixel Shield Direction Pin Variable
#endif                                                            // Ende der Selektion

const uint8_t ASE_ID = 3;                                         // Deklarierung und Initialisierung der ASE ID Variable 
const uint32_t Timeout=100;                                       // Deklarierung und Initialisierung der timeout Variable für die Kommunikation zwischen Dynamixel Shield und Dynamixel Servo in ms
const float ASE_PROTOCOL_VERSION = 1.0;                           // Deklarierung und Initialisierung der Protokollversions Variable
Dynamixel2Arduino ASE(ASE_SERIAL, ASE_DIR_PIN);                   // Instanziierung des ASE Objekts

//======================== Global WLAN Kommunikation ===================

RF24 ASE_Funk (9, 10);            // Instanziierung des ASE-Funk Objekts
byte Addresses[][6] = {"0"};      // Deklarierung und Initialisierung der Pipe für die Funkübertragung
int Daten_Empfangen[2];            // Deklarierung des Empfangsdatenspeichers
int Daten_Senden[2];               // Deklarierung des Sendedatenspeichers

//======================== Global Datenverarbeitung ===================

int ASE_Last_Smooth       = 0;    // Deklarierung und Initialisierung der Variable für den gleitenden Mittelwert der ASE Last
int ASE_Last_Aktuell      = 0;    // Deklarierung und Initialisierung der Variable für die aktuelle ASE Last
int ASE_Position_Aktuell;         // Deklarierung der Variable für die aktuelle ASE Position
int ASE_Position_Smooth;          // Deklarierung der Variable für den gleitenden Mittelwert der ASE Position
int ASE_Position_Senden;          // Deklarierung der Variable für die zu versendende ASE Position
int Greifer_Last;                 // Deklarierung der Variable für die Greifer Last
int Greifer_Position;             // Deklarierung der Variable für die Greifer Position
byte Zustand = 0;

void setup() {                    // Void setup
  
//======================== Setup Servos ===============================

  ASE.begin(115200);                                        // Beginn der Kommunikation zwischen ASE und Arduino Uno mit eingestellter Baudrate
  ASE.setPortProtocolVersion(ASE_PROTOCOL_VERSION);         // Setzen der Protokoll Version 
  ASE.setOperatingMode(ASE_ID, OP_POSITION);                // Setzen des Operationsmodus (Fahrbetrieb über Position)
  ASE.torqueOn(ASE_ID);                                     // Aktivierung des Servos

  if(ASE.ping(ASE_ID) == false)                             // Prüft, ob die Kommunikation zwischen dem Servomotor und dem Arduino Shield unterbrochen ist
  {
    software_Reset();                                       // Aktivierung der Reset Funktion
  }

  else                                                      // Ausführung bei bestehender Kommunikation zwischen dem Servomotor und dem Arduino Shield
  {
  
  ASE.writeControlTableItem(PUNCH,3,32,Timeout);            // Herabsetzen der Punch_Funktion für einen weicheren Betrieb
  ASE.writeControlTableItem(TORQUE_LIMIT,3,204,Timeout);    // Herabsetzen des ASE Drehmoments zur Erhöhung der Sicherheit bei der Startroutine (20%)
  ASE.writeControlTableItem(MOVING_SPEED,3,204,Timeout);    // Herabsetzen der ASE Geschwindigkeit zur Erhöhung der Sicherheit bei der Startroutine (20%)
  delay(2000);                                              // Wartezeit zur Verarbeitung der oben aufgeführten Einstellungen 
  ASE.setGoalPosition(3,820);                               // Fahren der ASE auf 90°
  delay(2000);                                              // Wartezeit für den Fahrbetrieb auf 90°
  ASE.setGoalPosition(3,500);                               // Fahren der ASE auf 0°
  delay(2000);                                              // Wartezeit für den Fahrbetrieb auf 0°
  ASE.setGoalPosition(3,660);                               // Fahren der ASE auf 45°
  delay(2000);                                              // Wartezeit für den Fahrbetrieb auf 45°
  ASE.writeControlTableItem(TORQUE_LIMIT,3,1023,Timeout);   // Heraufsetzen des ASE Drehmoments für den Nennbetrieb
  ASE.writeControlTableItem(MOVING_SPEED,3,1023,Timeout);   // Heraufsetzen der ASE Geschwindigkeit für den Nennbetrieb
  ASE_Position_Smooth = ASE.getPresentPosition(3);          // Speichern der aktuellen ASE Position in die Variable für den gleitenden Mittelwert

//======================== Setup WLAN Kommunikation ===================
  
  ASE_Funk.begin();                           // Aktivierung des NRF24L01 Funkmoduls
  ASE_Funk.setChannel(45);                    // Einstellung der Sende-/Empfangsfrequenz
  ASE_Funk.setPALevel(RF24_PA_MAX);           // Einstelung der Sendeleistung
  ASE_Funk.setDataRate( RF24_2MBPS );         // Einstellung der Datenübertragungsrate
  ASE_Funk.openReadingPipe(1, Addresses[0]);  // Öffnen einer Pipe zum Empfangen
  ASE_Funk.startListening();                  // Startet den Empfang
  ASE_Funk.setAutoAck(1);                     // Aktivierung der Acknowledge_Funktion
  ASE_Funk.enableDynamicPayloads();           // Aktivierung der dynamischen Paketgröße
  ASE_Funk.setCRCLength(RF24_CRC_16);         // Setzen der CRC-Prüflänge auf 16 Bit

  }
  
}

void loop(){                                                    // Void loop



  if(!ASE_Funk.available() && Zustand < 1)
  {
    delay(1000);
    Zustand++;
    ASE_Position_Senden = 660;
  }
 
  if (ASE_Funk.available())                                     // Prüft, ob Daten empfangen werden
  {                                                             
    while (ASE_Funk.available())                                // Solange Daten empfangen werden
    {                                                           
      ASE_Funk.read( &Daten_Empfangen, sizeof(Daten_Empfangen) ); // Datengröße wird ermittelt und im vorgesehenem Speicher abgelegt
    }

    ASE_Last_Aktuell = ASE.readControlTableItem(PRESENT_LOAD,3,Timeout);  //Auslesen und speichern der aktuellen ASE Last
    ASE_Last_Smooth = 0.8 * ASE_Last_Smooth + 0.2 * ASE_Last_Aktuell;     //Bilden des gleitender Mittelwerts der gewichteten Lastwerte der ASE
    Greifer_Last = Daten_Empfangen[1];                                     // Speichern der empfangenen Greifer Last
    Greifer_Position = Daten_Empfangen[0];                                 // Speichern der empfangenen Greifer Position

//======================== Verriegelung der ASE Position im FFB ===================    

    if (Greifer_Position > 820)       // Prüft, ob der maximale Grenzwert der Greifer Position überschritten wurde
    {             
      Greifer_Position = 820;         // Setzt die Greifer Position auf den maximalen Grenzwert
    }
    
    if (Greifer_Position < 500)       // Prüft, ob der minimale Grenzwert der Greifer Position unterschritten wurde
    {
      Greifer_Position = 500;         // Setzt die Greifer Position auf den minimalen Grenzwert
    }

//======================== Ende der Verriegelung ===================     
    
    ASE.setGoalPosition(3,Greifer_Position);                                        // ASE fährt auf die empfangene Greifer Position
    ASE_Position_Aktuell = ASE.getPresentPosition(3);                               // Speichern der aktuellen ASE Position
    ASE_Position_Smooth = 0.5 * ASE_Position_Smooth + 0.5 * ASE_Position_Aktuell;   //Bilden des gleitender Mittelwerts der gewichteten ASE Position

//======================== Leerlauf ASE Auf ===================

    if( ASE_Last_Smooth <= 1023 && Greifer_Last <= 88 )             // Prüft, ob die ASE im richtigen Lastbereich ist und ob der Greifer im Leerlauf ist                    
    {
      ASE.writeControlTableItem(CW_COMPLIANCE_MARGIN,3,8,Timeout);  // Verringerung der Positionsgenauigkeit der ASE (Zu)
      ASE.writeControlTableItem(CCW_COMPLIANCE_MARGIN,3,8,Timeout); // Verringerung der Positionsgenauigkeit der ASE (Auf)
      ASE_Position_Smooth = ASE_Position_Smooth + 2;                // Berechnung der neuen ASE Position

//======================== Verriegelung der ASE Position im Leerlauf ASE Auf =================== 

      if (ASE_Position_Smooth > 820)                                // Prüft, ob der maximale Grenzwert der ASE Position überschritten wurde
      {
        ASE_Position_Smooth = 820;                                  // Setzt die ASE Position auf den maximalen Grenzwert
      }

//======================== Ende der Verriegelung =================== 
      
      ASE.setGoalPosition(3, ASE_Position_Smooth);                  // Fahren auf die neu berechnete ASE Position
      ASE_Position_Senden = ASE_Position_Smooth;                    // Speichern der aktuellen ASE Position
    }

//======================== Leerlauf ASE Zu ===================

     if( ASE_Last_Smooth >= 1024 && Greifer_Last <= 1248)                   // Prüft, ob die ASE im richtigen Lastbereich ist und ob der Greifer im Leerlauf ist  
     {                 
          ASE.writeControlTableItem(CW_COMPLIANCE_MARGIN,3,8,Timeout);      // Verringerung der Positionsgenauigkeit der ASE (Zu)
          ASE.writeControlTableItem(CCW_COMPLIANCE_MARGIN,3,8,Timeout);     // Verringerung der Positionsgenauigkeit der ASE (Auf)
          ASE_Position_Smooth = ASE_Position_Smooth - 1;                    // Berechnung der neuen ASE Position

//======================== Verriegelung der ASE Position im Leerlauf ASE Zu =================== 

        if (ASE_Position_Smooth < 500)                    // Prüft, ob der maximale Grenzwert der ASE Position unterschritten wurde
         {
            ASE_Position_Smooth = 500;                    // Setzt die ASE Position auf den minimalen Grenzwert
         }

//======================== Ende der Verriegelung =================== 
      
          ASE.setGoalPosition(3,ASE_Position_Smooth);   // Fahren auf die neu berechnete ASE Position
          ASE_Position_Senden = ASE_Position_Smooth;    // Speichern der aktuellen ASE Position
     }

//======================== Zu Hand ===================

    if (ASE_Last_Smooth > Greifer_Last && ASE_Last_Smooth >= 1024 && Greifer_Last >= 1249)    // Prüft, ob die ASE Last größer ist als die Greifer Last und ob die ASE Last im richtigen Lastbereich ist und ob die Greifer Last im Force-Feedback Bereich ist
    {
      ASE.writeControlTableItem(CW_COMPLIANCE_MARGIN,3,1,Timeout);                            // Erhöhung der Positionsgenauigkeit der ASE (Zu)
      ASE.writeControlTableItem(CCW_COMPLIANCE_MARGIN,3,1,Timeout);                           // Erhöhung der Positionsgenauigkeit der ASE (Auf)
      ASE_Position_Aktuell = ASE_Position_Aktuell - 1;                                        // Berechnung der neuen ASE Position
      ASE_Position_Senden = ASE_Position_Aktuell;                                             // Speichern der aktuellen ASE Position
    }
    
//======================== Auf Greifer =================== 

    if (ASE_Last_Smooth < Greifer_Last && ASE_Last_Smooth <= 1023)    // Prüft, ob die ASE Last kleiner ist als die Greifer Last und ob die ASE im richtigen Lastbereich ist
    {
      ASE.writeControlTableItem(CW_COMPLIANCE_MARGIN,3,1,Timeout);    // Erhöhung der Positionsgenauigkeit der ASE (Zu)
      ASE.writeControlTableItem(CCW_COMPLIANCE_MARGIN,3,2,Timeout);   // Erhöhung der Positionsgenauigkeit der ASE (Auf)
      ASE_Position_Aktuell = ASE_Position_Aktuell + 1;                // Berechnung der neuen ASE Position
      ASE_Position_Senden = ASE_Position_Aktuell;                     // Speichern der aktuellen ASE Position
    }

//======================== Auf Hand ===================

    if (ASE_Last_Smooth > Greifer_Last && ASE_Last_Smooth <= 1023 && Greifer_Last >= 88)    //89 Prüft, ob die ASE Last größer ist als die Greifer Last und ob die ASE Last im richtigen Lastbereich ist und ob die Greifer Last im Force-Feedback Bereich ist
    {
      ASE.writeControlTableItem(CW_COMPLIANCE_MARGIN,3,1,Timeout);                          // Erhöhung der Positionsgenauigkeit der ASE (Zu)
      ASE.writeControlTableItem(CCW_COMPLIANCE_MARGIN,3,1,Timeout);                         // Erhöhung der Positionsgenauigkeit der ASE (Auf)
      ASE_Position_Aktuell = ASE_Position_Aktuell + 1;                                      // Berechnung der neuen ASE Position
      ASE_Position_Senden = ASE_Position_Aktuell;                                           // Speichern der aktuellen ASE Position  
    } 
    
  }
  
  delay(10);                                            // Wartezeit für den multitasking Betrieb der Funkübertragung

  Daten_Senden[0] = ASE_Position_Senden;                 // ASE Position in den Sendespeicher legen
  Daten_Senden[1] = ASE_Last_Smooth;                     // ASE Last in den Sendespeicher legen

  ASE_Funk.stopListening();                             // Stoppt den Empfang
  ASE_Funk.openWritingPipe(Addresses[0]);               // Öffnen einer Pipe zum Senden
  ASE_Funk.write(&Daten_Senden, sizeof(Daten_Senden));    // Datengröße wird ermittelt und aus vorgesehenem Speicher ausgelesen und gesendet
  ASE_Funk.openReadingPipe(1, Addresses[0]);            // Öffnen einer Pipe zum Empfangen
  ASE_Funk.startListening();                            // Startet den Empfang

}

void software_Reset()                                   // Reset Methode
{
asm volatile (" jmp 0");                                // Funktion der Reset Methode
}
