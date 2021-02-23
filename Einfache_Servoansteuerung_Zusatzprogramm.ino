#include <Dynamixel2Arduino.h>                                // Einbindung der Dnamixel2Arduino Bibliothek (Kommunikation Arduino und Dynamixel Servos)

#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560) // Selektion ob Arduino Uno oder Arduino Mega definiert wurde
  #define Greifer_SERIAL   Serial                             // Definition der Kommunikationsart Serial
  const uint8_t Greifer_DIR_PIN = 2;                          // Deklarierung und Initialisierung der Dynamixel Shield Direction Pin Variable
#else                                                         // Bei nicht zutreffender Selektion
  #define Greifer_SERIAL   Serial1                            // Definition der Kommunikationsart Serial1
  const uint8_t Greifer_DIR_PIN = 2;                          // Deklarierung und Initialisierung der Dynamixel Shield Direction Pin Variable
#endif                                                        // Ende der Selektion

const uint8_t Greifer_ID = 1;                                 // Deklarierung und Initialisierung der Greifer ID Variable
const float Greifer_PROTOCOL_VERSION = 1.0;                   // Deklarierung und Initialisierung der Protokollversions Variable

Dynamixel2Arduino Greifer(Greifer_SERIAL, Greifer_DIR_PIN);   // Instanziierung des Greifer Objekts

void setup() {                                                // Void setup

  Greifer.begin(115200);                                      // Beginn der Kommunikation zwischen Greifer und Arduino Uno mit eingestellter Baudrate
  Greifer.setPortProtocolVersion(Greifer_PROTOCOL_VERSION);   // Setzen der Protokoll Version
  Greifer.torqueOff(Greifer_ID);                              // Deaktivierung der Servos
  Greifer.setOperatingMode(Greifer_ID, OP_POSITION);          // Setzen des Operationsmodus (Fahrbetrieb über Position)
  Greifer.torqueOn(Greifer_ID);                               // Aktivierung der Servos
}

void loop() {                                                 // Void loop

  Greifer.setGoalPosition(Greifer_ID, 512 );                  // Greifer fährt auf die Position
  delay(1000);                                                // Wartezeit

  Greifer.setGoalPosition(Greifer_ID, 105, UNIT_DEGREE);      // Greifer fährt auf die Position
  delay(1000);                                                // Wartezeit

}
