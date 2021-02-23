#include <SPI.h>                  // Einbindung der SPI Bibliothek (Serial Peripheral Interface)
#include <RF24.h>                 // Einbindung der RF24 Bibliothek (WLAN Kommunikation)

RF24 ASE_Funk (9, 10);            // Instanziierung des ASE_Funk Objekts
byte Addresses[][6] = {"0"};      // Deklarierung und Initialisierung der Pipe für die Funkübertragung

int Daten_Senden[2]={1111,2222};   // Deklarierung und Initialisierung des Sendedatenspeichers
int Daten_Empfangen[2];            // Deklarierung des Empfangsdatenspeichers

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void setup() {                                // Void setup
  Serial.begin(9600);                         // Kommunikation des seriellen Monitors mit eingestellter Baudrate
  ASE_Funk.begin();                           // Aktivierung des NRF24L01 Funkmoduls
  ASE_Funk.setChannel(10);                    // Einstellung der Sende-/Empfangsfrequenz
  ASE_Funk.setPALevel(RF24_PA_MAX);           // Einstelung der Sendeleistung
  ASE_Funk.setDataRate( RF24_2MBPS );         // Einstellung der Datenübertragungsrate
  ASE_Funk.setAutoAck(1);                     // Aktivierung der Acknowledge-Funktion
  ASE_Funk.enableAckPayload();                // Aktivierung der Acknowledge-Funktion der Paketgröße
  ASE_Funk.enableDynamicPayloads();           // Aktivierung der dynamischen Paketgröße
  ASE_Funk.openReadingPipe(1, Addresses[0]);  // Öffnen einer Pipe zum Empfangen
  ASE_Funk.startListening();                  // Startet den Empfang
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop() {                                                     // Void loop
  if ( ASE_Funk.available()) {                                    // Prüft, ob Daten empfangen werden
    while (ASE_Funk.available()){                                 // Solange Daten empfangen werden
      ASE_Funk.read( &Daten_Empfangen, sizeof(Daten_Empfangen) ); // Datengröße wird ermittelt und im vorgesehenem Speicher abgelegt
    }   
    Serial.println("----------------------------------------");   // Serielle Ausgabe des Textes
    Serial.print("ASE empfängt1: ");                              // Serielle Ausgabe des Textes
    Serial.println(Daten_Empfangen[0]);                           // Serielle Ausgabe des Empfangspeichers
    Serial.print("ASE empfängt2: ");                              // Serielle Ausgabe des Textes
    Serial.println(Daten_Empfangen[1]);                           // Serielle Ausgabe des Empfangspeichers
    Serial.println("----------------------------------------");   // Serielle Ausgabe des Textes  
  }

  delay(100);                                           // Wartezeit für den multitasking Betrieb der Funkübertragung

  ASE_Funk.stopListening();                             // Stoppt den Empfang
  ASE_Funk.openWritingPipe(Addresses[0]);               // Öffnen einer Pipe zum Senden
  ASE_Funk.write(&Daten_Senden, sizeof(Daten_Senden));  // Datengröße wird ermittelt und aus vorgesehenem Speicher ausgelesen und gesendet

  Serial.print("ASE sendet1: ");                        // Serielle Ausgabe des Textes
  Serial.println(Daten_Senden[0]);                      // Serielle Ausgabe des Sendespeichers
  Serial.print("ASE sendet2: ");                        // Serielle Ausgabe des Textes
  Serial.println(Daten_Senden[1]);                      // Serielle Ausgabe des Sendespeichers
  
  ASE_Funk.openReadingPipe(1, Addresses[0]);            // Öffnen einer Pipe zum Empfangen
  ASE_Funk.startListening();                            // Startet den Empfang
}
