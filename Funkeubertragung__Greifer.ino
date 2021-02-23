#include <SPI.h>                  // Einbindung der SPI Bibliothek (Serial Peripheral Interface)
#include <RF24.h>                 // Einbindung der RF24 Bibliothek (WLAN Kommunikation)

RF24 Greifer_Funk (9, 10);        // Instanziierung des Greifer_Funk Objekts
byte Addresses[][6] = {"0"};      // Deklarierung und Initialisierung der Pipe für die Funkübertragung

int Daten_Senden[2]={3333,4444};   // Deklarierung und Initialisierung des Sendedatenspeichers
int Daten_Empfangen[2];            // Deklarierung des Empfangsdatenspeichers

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void setup() {                                    // Void setup                            
  Serial.begin(9600);                             // Kommunikation des seriellen Monitors mit eingestellter Baudrate
  Greifer_Funk.begin();                           // Aktivierung des NRF24L01 Funkmoduls
  Greifer_Funk.setChannel(10);                    // Einstellung der Sende-/Empfangsfrequenz
  Greifer_Funk.setPALevel(RF24_PA_MAX);           // Einstelung der Sendeleistung
  Greifer_Funk.setDataRate( RF24_2MBPS );         // Einstellung der Datenübertragungsrate
  Greifer_Funk.setAutoAck(1);                     // Aktivierung der Acknowledge-Funktion
  Greifer_Funk.enableAckPayload();                // Aktivierung der Acknowledge-Funktion der Paketgröße
  Greifer_Funk.enableDynamicPayloads();           // Aktivierung der dynamischen Paketgröße
  Greifer_Funk.openReadingPipe(1, Addresses[0]);  // Öffnen einer Pipe zum Empfangen
  Greifer_Funk.startListening();                  // Startet den Empfang
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop() {                                                         // Void loop
  if ( Greifer_Funk.available()) {                                    // Prüft, ob Daten empfangen werden
    while (Greifer_Funk.available()){                                 // Solange Daten empfangen werden
      Greifer_Funk.read( &Daten_Empfangen, sizeof(Daten_Empfangen) ); // Datengröße wird ermittelt und im vorgesehenem Speicher abgelegt
    }    
    Serial.println("--------------------------------------------");   // Serielle Ausgabe des Textes
    Serial.print("Greifer empfängt1: ");                              // Serielle Ausgabe des Textes
    Serial.println(Daten_Empfangen[0]);                               // Serielle Ausgabe des Empfangspeichers
    Serial.print("Greifer empfängt2: ");                              // Serielle Ausgabe des Textes
    Serial.println(Daten_Empfangen[1]);                               // Serielle Ausgabe des Empfangspeichers
    Serial.println("--------------------------------------------");   // Serielle Ausgabe des Textes
  }

  delay(100);                                               // Wartezeit für den multitasking Betrieb der Funkübertragung

  Greifer_Funk.stopListening();                             // Stoppt den Empfang
  Greifer_Funk.openWritingPipe(Addresses[0]);               // Öffnen einer Pipe zum Senden
  Greifer_Funk.write(&Daten_Senden, sizeof(Daten_Senden));  // Datengröße wird ermittelt und aus vorgesehenem Speicher ausgelesen und gesendet

  Serial.print("Greifer sendet1: ");                        // Serielle Ausgabe des Textes
  Serial.println(Daten_Senden[0]);                          // Serielle Ausgabe des Sendespeichers 
  Serial.print("Greifer sendet2: ");                        // Serielle Ausgabe des Textes
  Serial.println(Daten_Senden[1]);                          // Serielle Ausgabe des Sendespeichers
  
  Greifer_Funk.openReadingPipe(1, Addresses[0]);          // Öffnen einer Pipe zum Empfangen
  Greifer_Funk.startListening();                          // Startet den Empfang
}
