const int led[4] = {10, 9, 8, 7};  // definirea pinilor pentru cele 4 led-uri
int ledState[4] = {LOW, LOW, LOW, LOW};  // starea initiala a led-urilor 
const int nrLeduri = 4;

const int rgbR = 6;  
const int rgbG = 5;  
const int rgbB = 4;  

int rgbRState = LOW;  // starea initiala a led-ului rgb rosu
int rgbGState = HIGH;  // starea initiala a led-ului rgb verde
int rgbBState = LOW;  // starea initiala a led-ului rgb albastru

int lockButton1 = 1;  // variabila pentru blocarea butonului 1
int lockButton2 = 0;  // variabila pentru blocarea butonului 2

const int button1 = 3;  // pinul pentru butonul start
const int button2 = 2;  // pinul pentru butonul stop

unsigned long ledPreviousMillis[4] = {0, 0, 0, 0};  // timpi anteriori pentru led-uri
const long ledBlinkInterval = 500;  // intervalul de timp pentru blink
int blinkTimes[4] = {0, 0, 0, 0};  
int blinkBool[4] = {0, 0, 0, 0};  // flag-uri pentru a verifica daca led-urile clipeasc

bool allLedsBlinking = false;  // flag pentru a verifica daca toate led-urile clipeasc
bool hasStarted = false;  // flag pentru a verifica daca procesul a inceput

int button1State;  
int button2State;  
int lastButton1State = HIGH;  
int lastButton2State = HIGH;  
unsigned long button1LastDebTime = 0;  
unsigned long button2LastDebTime = 0;  
unsigned long button1DebDelay = 50;  
unsigned long button2DebDelay = 50;  

unsigned long button2PressTime = 0;  

// Initializarea led
void blink3Ini(int ledNumber) {
  blinkTimes[ledNumber] = 6;  
  ledPreviousMillis[ledNumber] = millis();  
  ledState[ledNumber] = HIGH;  
  digitalWrite(led[ledNumber], ledState[ledNumber]);  
}

// Functia blink led 
void blink3(int ledNumber) {
  
  if (!blinkBool[ledNumber]) {  
    blink3Ini(ledNumber);  // initializare blink
    blinkBool[ledNumber] = 1;  
  }

  if (blinkTimes[ledNumber] > 0) {  // daca led-ul inca clipeste
    unsigned long currentMillis = millis();  
    
    if (currentMillis - ledPreviousMillis[ledNumber] >= ledBlinkInterval) {  
      ledPreviousMillis[ledNumber] = currentMillis;  // actualizarea timpului anterior

      ledState[ledNumber] = !ledState[ledNumber];  
      digitalWrite(led[ledNumber], ledState[ledNumber]);  

      blinkTimes[ledNumber]--;  
    }
  } 
  else if (blinkTimes[ledNumber] == 0 && ledState[ledNumber] == LOW) {  
    ledState[ledNumber] = HIGH;  // led-ul ramane aprins la final
    digitalWrite(led[ledNumber], ledState[ledNumber]);  
  }
}

// Functie start proces
void start() {
  for (int i = 0; i < nrLeduri; i++) { 
    blinkBool[i] = 0;  
    while (blinkTimes[i] > 0 || ledState[i] == LOW) {  
      blink3(i);  
      buttonStop();  // verificare daca butonul de stop este apasat
    }
  }

  allLedsBlinking = true;  // setarea flag-ului pentru a indica ca toate led-urile clipeasc
  
  for (int i = 0; i < nrLeduri; i++) {  
    blinkBool[i] = 0;  // resetarea flag-ului de blink
    blink3Ini(i);  
  }
  
  bool blinkingComplete = false;  
  while (!blinkingComplete) {  
    blinkingComplete = true;  
    
    for (int i = 0; i < nrLeduri; i++) {  
      blink3(i);  
      buttonStop();  // verificare daca butonul de stop este apasat
      
      if (blinkTimes[i] > 0) {  
        blinkingComplete = false;  
      }
    }
  }

  for (int i = 0; i < nrLeduri; i++) {  
    digitalWrite(led[i], LOW);  // stingerea led-ului
    ledState[i] = LOW;  
    blinkBool[i] = 0;  
  }

  allLedsBlinking = false;  
  hasStarted = true;  // setarea flag-ului pentru start
}

// Functia stop proces + resetare program
void buttonStop() {
    int reading = digitalRead(button2);  

    if (reading != lastButton2State) { 
      button2LastDebTime = millis();  // actualizarea timpului de debouncing
    }

    if (reading == LOW) {  
      if (lastButton2State == HIGH) {  
          button2PressTime = millis();  
        } 
        else if (millis() - button2PressTime >= 1000) {  // daca butonul a fost apasat mai mult de 1 secunda
          allLedsBlinking = true;  // setarea flag-ului pentru a indica ca toate led-urile clipeasc
  
          for (int i = 0; i < nrLeduri; i++) {  
            blinkBool[i] = 0; 
            blink3Ini(i);  
          }
              
          bool blinkingComplete = false;  
          while (!blinkingComplete) {  
            blinkingComplete = true;  

          for (int i = 0; i < nrLeduri; i++) {  
            blink3(i);  
            if (blinkTimes[i] > 0) {  
              blinkingComplete = false;  
            }
          }
        }
        asm volatile ("  jmp 0");  // reseteaza programul
      }
    } 
    else {
      button2PressTime = 0;  // resetarea timpului de apasare pentru butonul 2
    }
    
    lastButton2State = reading;  
}

// Functia pentru incepere proces 
void buttonStart() {
  int reading = digitalRead(button1); 
  long currentMillis = millis();  

  if (reading != lastButton1State) { 
    button1LastDebTime = currentMillis;  // actualizarea timpului de debouncing
  }
  
  if (currentMillis - button1LastDebTime >= button1DebDelay) {  
    if (reading != button1State) {  
      button1State = reading;  
    
      if (button1State == LOW && lockButton1) {  // daca butonul este apasat si este deblocat
        lockButton1 = 1;  // blocarea butonului 1
        lockButton2 = 0;  // deblocarea butonului 2
        
        rgbRState = HIGH;  
        digitalWrite(rgbR, rgbRState);  
        rgbGState = LOW;  
        digitalWrite(rgbG, rgbGState); 
        
        if (!hasStarted) {  // daca procesul nu a inceput inca
          start();  
        }
      }
    }
  }
  
  lastButton1State = reading;  
  rgbRState = LOW;  
  digitalWrite(rgbR, rgbRState);  
  rgbGState = HIGH;  
  digitalWrite(rgbG, rgbGState);  
  hasStarted = false;  // resetarea flag-ului pentru a indica ca procesul nu a inceput
}

void setup() {
  for (int i = 0; i < nrLeduri; i++) {  
    pinMode(led[i], OUTPUT);  
    digitalWrite(led[i], ledState[i]); 
  }
  
  pinMode(rgbR, OUTPUT);  
  pinMode(rgbG, OUTPUT);  
  pinMode(rgbB, OUTPUT);  
  
  pinMode(button1, INPUT_PULLUP);  
  pinMode(button2, INPUT_PULLUP);  
  
  digitalWrite(rgbR, rgbRState);  
  digitalWrite(rgbG, rgbGState);  
  digitalWrite(rgbB, rgbBState);  
}


void loop() {
  buttonStart();  
}
