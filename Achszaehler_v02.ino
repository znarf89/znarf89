// LGB Achszaehler mit zwei Reedkontakten
// Version 0.2
// Andreas Franz, 03.06.2021

int LED = 13;
unsigned long alteZeit = 0, warteZeit = 100, letzterKontakt = 0, resetZeit = 5000;
volatile bool KontaktA = false, KontaktB = false;
int state = 0, old_state = 0;
signed int count = 0, old_count = 0;

void setup() {
  Serial.begin(19200);
  pinMode(LED, OUTPUT);    // LED Pin
  pinMode(2, INPUT_PULLUP);       // Pin D2 ist INT0
  pinMode(3, INPUT_PULLUP);       // Pin D3 ist INT1
  attachInterrupt(0, Kontakt_A, LOW);
  // Pin 2 (INT 0) geht auf 0V (LOW) dann interruptRoutine aufrufen
  attachInterrupt(1, Kontakt_B, LOW);
  // Pin 3 (INT 1) geht auf 0V (LOW) dann interruptRoutine aufrufen
  Serial.println("Arduino LGB Achszähler V0.2");
}

void loop() {
  switch (state) {
    case 0:                   //Ausgangszustand
      if (KontaktA == true) { //Wenn KontaktA befahren wird, dann
        state = 1;            //in Status 1 wechseln
        KontaktA = false;     //Variable zurücksetzen
      }
      if (KontaktB == true) { //Wenn KontaktB befahren wird, dann
        state = 3;           //in Status 3 wechseln
        KontaktB = false;     //Variable zurücksetzen
      }
      break;
    case 1:                                   //KontaktA wurde befahren
      if (KontaktB == true) {                 //Wenn KontaktB befahren wird, dann
        count++;                              //Zähler um 1 erhöhen
        state = 2;                           //in Status 2 wechseln
        KontaktB = false;                     //Variable zurücksetzen
        attachInterrupt(0, Kontakt_A, LOW);   //Interrupt KontaktA wieder aktivieren
        alteZeit = millis();                  //Zeitpunkt merken
      }
      break;
    case 2:                                       //Warten nach zweitem Kontakt Richtung A->B
      if (KontaktA == true) {                     //Wenn KontaktA befahren wird, dann ist keine Wartezeit nötig und
        state = 1;                                //in Status 1 wechseln
        KontaktA = false;                         //Variable zurücksetzen
        attachInterrupt(1, Kontakt_B, LOW);       //Interrupt KontaktB aktivieren
      }
      if ((millis() - alteZeit) > warteZeit) {    //Wenn warteZeit abgelaufen ist, dann
        state = 0;                                //in Status 0 wechseln
        digitalWrite(LED, LOW);
        attachInterrupt(1, Kontakt_B, LOW);       //Interrupt KontaktB aktivieren
      }
      break;
    case 3:                                   //KontaktB wurde befahren
      if (KontaktA == true) {                 //Wenn KontaktA befahren wird, dann
        count--;                              //Zähler um 1 erniedirgen
        state = 4;                           //in Status 4 wechseln
        KontaktA = false;                     //Variable zurücksetzen
        attachInterrupt(1, Kontakt_B, LOW);   //Interrupt KontaktB wieder aktivieren
        alteZeit = millis();                  //Zeitpunkt merken
      }
      break;
    case 4:                                       //Warten nach zweitem Kontakt Richtung B->A
      if (KontaktB == true) {                     //Wenn KontaktB befahren wird, dann ist keine Wartezeit nötig und
        state = 3;                                //in Status 3 wechseln
        KontaktB = false;                         //Variable zurücksetzen
        attachInterrupt(0, Kontakt_A, LOW);        //Interrupt KontaktA aktivieren
      }
      if ((millis() - alteZeit) > warteZeit) {    //Wenn warteZeit abgelaufen ist, dann
        state = 0;                                //in Status 0 wechseln
        digitalWrite(LED, LOW);
        attachInterrupt(0, Kontakt_A, LOW);       //Interrupt KontaktA aktivieren
      }
      break;
  }
  
  if (state != old_state) {     //Wenn sich der Status ändert, dann
    Serial.print("Status: ");   //Status auf Seriellen Monitor ausgeben
    Serial.println(state);
    old_state = state;          //Vergleichswert anpassen
    letzterKontakt = millis();  //Zeit der letzten Statusänderung speichern
  }

  if (state != 0) {
    digitalWrite(LED, HIGH);                    //Wenn State nicht 0, dann LED einschalten
    if ((millis() - letzterKontakt) > 5000) {   //Wenn Status länger als 5s nicht 0, dann
      state = 0;                                //Status auf 0 zurücksetzen
      digitalWrite(LED, LOW);                   //LED ausschalten
      attachInterrupt(0, Kontakt_A, LOW);       //Interrupt KontaktA aktivieren
      attachInterrupt(1, Kontakt_B, LOW);       //Interrupt KontaktB aktivieren
    }
  }

  if (count != old_count) {           //Wenn sich der Zählerstand geändert hat, dann
    Serial.print("Zaehlerstand: ");   //Zählerstand auf Seriellem Monitor ausgeben
    Serial.println(count);
    old_count = count;
  }

}

void Kontakt_A() {      //ISR für KontaktA am Pin D2 - Interrupt 0
  detachInterrupt(digitalPinToInterrupt(2));   //Interrupt deaktivieren
  KontaktA = true;      //Variabale auf TRUE setzen
}

void Kontakt_B() {      //ISR für KontaktB am Pin D3 - Interrupt 1
  detachInterrupt(digitalPinToInterrupt(3));   //Interrupt deaktivieren
  KontaktB = true;      //Variabale auf TRUE setzen
}
