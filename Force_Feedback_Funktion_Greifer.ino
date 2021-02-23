#include <SPI.h>                      // Einbindung der SPI Bibliothek (Serial Peripheral Interface)
#include <RF24.h>                     // Einbindung der RF24 Bibliothek (WLAN Kommunikation)
#include <Dynamixel2Arduino.h>        // Einbindung der Dnamixel2Arduino Bibliothek (Kommunikation Arduino und Dynamixel Servos)

//======================== Global Servos ===============================

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560)     // Selektion ob Arduino Uno oder Arduino Mega definiert wurde
  #define Greifer_SERIAL   Serial                                 // Definition der Kommunikationsart Serial
  const uint8_t Greifer_DIR_PIN = 2;                              // Deklarierung und Initialisierung der Dynamixel Shield Direction Pin Variable
#else                                                             // Bei nicht zutreffender Selektion
  #define Greifer_SERIAL   Serial1                                // Definition der Kommunikationsart Serial1
  const uint8_t Greifer_DIR_PIN = 2;                              // Deklarierung und Initialisierung der Dynamixel Shield Direction Pin Variable
#endif                                                            // Ende der Selektion

const uint8_t Gripper_ID1 = 1;                                    // Deklarierung und Initialisierung der Gripper1 ID Variable
const uint8_t Gripper_ID2 = 2;                                    // Deklarierung und Initialisierung der Gripper2 ID Variable
const uint32_t Timeout=100;                                       // Deklarierung und Initialisierung der timeout Variable für die Kommunikation zwischen Dynamixel Shield und Dynamixel Servo in ms
const float Greifer_PROTOCOL_VERSION = 1.0;                       // Deklarierung und Initialisierung der Protokollversions Variable
Dynamixel2Arduino Gripper1(Greifer_SERIAL, Greifer_DIR_PIN);      // Instanzierung des Gripper1 Objekts
Dynamixel2Arduino Gripper2(Greifer_SERIAL, Greifer_DIR_PIN);      // Instanzierung des Gripper2 Objekts

//======================== Global WLAN Kommunikation ===================

RF24 Greifer_Funk (9, 10);       // Instanzierung des Greifer-Funk Objekts
byte Addresses[][6] = {"0"};     // Deklarierung und Initialisierung der Pipe für die Funkübertragung
int Daten_Empfangen[2];          // Deklarierung des Empfangsdatenspeichers

//======================== Global Datenverarbeitung ====================

int Position;               // Deklarierung der Variable für die Greifer Position
int Greifer_Position;       // Deklarierung der Variable für die Greifer Position für die ASE
int ASE_Last;               // Deklarierung der Variable für die ASE Last
int Greifer_Last;           // Deklarierung der Variable für die Greifer Last
int Greifer_Last_Alt;       // Deklarierung der Variable für die alte Greifer Last  
int Greifer_Last_Smooth;    // Deklarierung der Variable für den gleitenden Mittelwert der Greifer Last

void setup() {              // Void setup

//======================== Setup Servos ===============================

  Gripper1.begin(115200);                                         // Beginn der Kommunikation zwischen Gripper1 und Arduino Uno mit eingestellter Baudrate
  Gripper2.begin(115200);                                         // Beginn der Kommunikation zwischen Gripper2 und Arduino Uno mit eingestellter Baudrate
  Gripper1.setPortProtocolVersion(Greifer_PROTOCOL_VERSION);      // Setzen der Protokoll Version für Gripper1
  Gripper2.setPortProtocolVersion(Greifer_PROTOCOL_VERSION);      // Setzen der Protokoll Version für Gripper2
  Gripper1.setOperatingMode(Gripper_ID1, OP_POSITION);            // Setzen des Operationsmodus für Gripper1 (Fahrbetrieb über Position)
  Gripper2.setOperatingMode(Gripper_ID2, OP_POSITION);            // Setzen des Operationsmodus für Gripper2 (Fahrbetrieb über Position)
  Gripper1.torqueOn(Gripper_ID1);                                 // Aktivierung des Gripper1
  Gripper2.torqueOn(Gripper_ID2);                                 // Aktivierung des Gripper2
  if(Gripper1.ping(Gripper_ID1) == false)
  {
    software_Reset();                                             // Methode des Software-Reset
  }

  else
  {
  Gripper1.writeControlTableItem(PUNCH,1,32,Timeout);             // Herabsetzen der Punch_Funktion für einen weicheren Betrieb vom Gripper1
  Gripper2.writeControlTableItem(PUNCH,2,32,Timeout);             // Herabsetzen der Punch_Funktion für einen weicheren Betrieb vom Gripper2
  Gripper1.writeControlTableItem(TORQUE_LIMIT,1,204,Timeout);     // Herabsetzen des Gripper1 Drehmoments zur Erhöhung der Sicherheit bei der Startroutine (20%)
  Gripper1.writeControlTableItem(MOVING_SPEED,1,204,Timeout);     // Herabsetzen der Gripper1 Geschwindigkeit zur Erhöhung der Sicherheit bei der Startroutine (20%)
  Gripper2.writeControlTableItem(TORQUE_LIMIT,2,204,Timeout);     // Herabsetzen des Gripper2 Drehmoments zur Erhöhung der Sicherheit bei der Startroutine (20%)
  Gripper2.writeControlTableItem(MOVING_SPEED,2,204,Timeout);     // Herabsetzen der Gripper2 Geschwindigkeit zur Erhöhung der Sicherheit bei der Startroutine (20%)
  delay(2000);                                                    // Wartezeit zur Verarbeitung der oben aufgeführten Einstellungen 
  Gripper1.setGoalPosition(1,356);                                // Fahren des Gripper1 auf 90°
  Gripper2.setGoalPosition(2,356);                                // Fahren des Gripper2 auf 90°
  delay(2000);                                                    // Wartezeit für den Fahrbetrieb auf 90°
  Gripper1.setGoalPosition(1,522);                                // Fahren des Gripper1 auf 0°
  Gripper2.setGoalPosition(2,522);                                // Fahren des Gripper2 auf 0°
  delay(2000);                                                    // Wartezeit für den Fahrbetrieb auf 0°
  Gripper1.setGoalPosition(1,439);                                // Fahren des Gripper1 auf 45°
  Gripper2.setGoalPosition(2,439);                                // Fahren des Gripper2 auf 45°
  delay(2000);                                                    // Wartezeit für den Fahrbetrieb auf 45°
  Gripper1.writeControlTableItem(TORQUE_LIMIT,1,1023,Timeout);    // Heraufsetzen des Gripper1 Drehmoments für den Nennbetrieb
  Gripper1.writeControlTableItem(MOVING_SPEED,1,1023,Timeout);    // Heraufsetzen der Gripper1 Geschwindigkeit für den Nennbetrieb
  Gripper2.writeControlTableItem(TORQUE_LIMIT,2,1023,Timeout);    // Heraufsetzen des Gripper2 Drehmoments für den Nennbetrieb
  Gripper2.writeControlTableItem(MOVING_SPEED,2,1023,Timeout);    // Heraufsetzen der Gripper2 Geschwindigkeit für den Nennbetrieb 

//======================== Setup WLAN Kommunikation ===================
  
  Greifer_Funk.begin();                               // Aktivierung des NRF24L01 Funkmoduls
  Greifer_Funk.setChannel(45);                        // Einstellung der Sende-/Empfangsfrequenz
  Greifer_Funk.setPALevel(RF24_PA_MAX);               // Einstelung der Sendeleistung
  Greifer_Funk.setDataRate( RF24_2MBPS );             // Einstellung der Datenübertragungsrate
  Greifer_Funk.openReadingPipe(1, Addresses[0]);      // Öffnen einer Pipe zum Empfangen
  Greifer_Funk.startListening();                      // Startet den Empfang
  Greifer_Funk.setAutoAck(1);                         // Aktivierung der Acknowledge_Funktion
  Greifer_Funk.enableDynamicPayloads();               // Aktivierung der dynamischen Paketgröße
  Greifer_Funk.setCRCLength(RF24_CRC_16);             // Setzen der CRC-Prüflänge auf 16 Bit

  }
  
}

void loop(){                                                         // Void loop

  if (Greifer_Funk.available())                                      // Prüft, ob Daten empfangen werden
  {
    while (Greifer_Funk.available())                                 // Solange Daten empfangen werden
    {
      Greifer_Funk.read(&Daten_Empfangen, sizeof(Daten_Empfangen) ); // Datengröße wird ermittelt und im vorgesehenem Speicher abgelegt
    }

    int Position_Empfangen = Daten_Empfangen[0];                     // Deklarierung und Initialisierung der Variable

    Position = map(Daten_Empfangen[0],500,820,522,356);              // Rechnet die empfangene ASE Position um für die Greifer Position
                           

//======================== Verriegelung der Greifer Position ===================

    if (Position > 522)   // Prüft, ob der maximale Grenzwert der Greifer Position überschritten wurde
    {
      Position = 522;     // Setzt die Greifer Position auf den maximalen Grenzwert
    }
    
    if (Position < 356)   // Prüft, ob der minimale Grenzwert der Greifer Position unterschritten wurde
    {
      Position = 356;     // Setzt die Greifer Position auf den minimalen Grenzwert
    }

//======================== Ende der Verriegelung ===================
    
    Gripper1.setGoalPosition(1,Position);                                   // Gripper1 fährt auf die umgerechnete Position
    Gripper2.setGoalPosition(2,Position);                                   // Gripper2 fährt auf die umgerechnete Position    
   
  }

  delay(10);                                                                // Wartezeit für den multitasking Betrieb der Funkübertragung

  Greifer_Funk.stopListening();                                             // Stoppt den Empfang
  int Greifer_Position_test = Gripper1.getPresentPosition(1);                 
  Greifer_Position=map(Greifer_Position_test,522,356,500,820);              // Rechnet die aktuelle Position des Greifers um für die ASE
  Greifer_Last = Gripper2.readControlTableItem (PRESENT_LOAD,2,Timeout);    //Auslesen und speichern der aktuellen Greifer Last
  ASE_Last = Daten_Empfangen[1];                                             // Speichern der empfangenen ASE Last

//======================== Lastfilter ===================

  if(ASE_Last >= 1024 && Greifer_Last >= 1024)            // Prüft, ob die ASE im Lastbereich ZU ist und ob der Greifer im Lastbereich ZU ist
  {
    Greifer_Last_Alt=Greifer_Last;                        // Setzt die alte Greifer Last auf den Wert der aktuellen Greifer Last
  }
  
  if(ASE_Last >= 1024 && Greifer_Last < 1024)             // Prüft, ob die ASE im Lastbereich ZU ist und ob der Greifer im Lastbereich AUF ist
  {      
    Greifer_Last = Greifer_Last_Alt;                      // Setzt die aktuelle Greifer Last auf den Wert der alten Greifer Last
  }
 
  if(ASE_Last <= 1023 && Greifer_Last <= 1023)            // Prüft, ob die ASE im Lastbereich AUF ist und ob der Greifer im Lastbereich AUF ist  
  {
    Greifer_Last_Alt = Greifer_Last;                      // Setzt die alte Greifer Last auf den Wert der aktuellen Greifer Last
  }

  if(ASE_Last <= 1023 && Greifer_Last >1023)              // Prüft, ob die ASE im Lastbereich AUF ist und ob der Greifer im Lastbereich ZU ist 
  {     
    Greifer_Last = Greifer_Last_Alt;                      // Setzt die aktuelle Greifer Last auf den Wert der alten Greifer Last                 
  }

//======================== Lastglättung ===================  

  Greifer_Last_Smooth = 0.80 * Greifer_Last_Smooth + 0.20 * Greifer_Last;   // Bildet den gleitenden Mittelwert der gewichteten Lastwerte des Greifers 
  
  int Daten_Senden[2]= {Greifer_Position,Greifer_Last_Smooth};              // Deklarierung und Befüllung des Sendedatenspeichers mit der gefilterten Greifer Position und der geglätteten Greifer Last  
   
  Greifer_Funk.openWritingPipe(Addresses[0]);                               // Öffnen einer Pipe zum Senden
  Greifer_Funk.write(&Daten_Senden, sizeof(Daten_Senden));                  // Datengröße wird ermittelt und aus vorgesehenem Speicher ausgelesen und gesendet
  Greifer_Funk.openReadingPipe(1, Addresses[0]);                            // Öffnen einer Pipe zum Empfangen
  Greifer_Funk.startListening();                                            // Startet den Empfang
  
}

void software_Reset()
{
asm volatile (" jmp 0");
}
