#include <Arduino.h>
#include <avr/interrupt.h> 

const int STARTSTOPBUTTONPIN = 2;
const int DIFFICULTYBUTTONPIN = 3;
const unsigned long ROUND_DURATION_MS = 30000;
const unsigned long COUNTDOWN_DURATION_MS = 3000; 
const unsigned long ERROR_DISPLAY_DURATION_MS = 500; 
const unsigned long BLINK_LED_TIME = 500;

enum Difficulty {EASY = 0, MEDIUM, HARD};

bool gameStarted = false;
bool countdownActive = false;
bool errorDisplayed = false;
unsigned long roundStartTime;
unsigned long countdownStartTime;
unsigned long errorDisplayStartTime;
Difficulty chosenDifficulty = EASY;
int score = 0;
String currentWord;
String userInput = "";
volatile int timerOverflowCounter = 0;

unsigned long lastBlinkTime = 0;
bool ledOnDuringCountdown = false;

int countDown = 3;
const int easyDifficulty = 150;
const int mediumDifficulty = 100;
const int hardDifficulty = 80;
const int baudRate = 9600;

const char* wordDictionary[] = {
  "apple", "banana", "cherry", "date", "elderberry",
  "fig", "grape", "honeydew", "kiwi", "lemon",
  "mango", "nectarine", "orange", "papaya", "quince",
  "raspberry", "strawberry", "tangerine", "watermelon", "zucchini"
};
const int wordCount = sizeof(wordDictionary) / sizeof(wordDictionary[0]);

class RGBLED {
  const int redPin, greenPin, bluePin;

public:
  RGBLED(int rPin, int gPin, int bPin)
      : redPin(rPin), greenPin(gPin), bluePin(bPin) {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
  }

  void turnWhite() {
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, HIGH);
  }

  void turnGreen() {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
  }

  void turnRed() {
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
  }

  void turnOff() {
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
  }
};

class Button {
protected:
  const int pin;
  const uint8_t triggerType;
  const unsigned long debounceDelayMs;
  unsigned long lastInterruptTime = 0;

public:
  volatile bool handleButtonPressNextTick = false;

  Button(int p, uint8_t t, unsigned long debounce) : pin(p), triggerType(t), debounceDelayMs(debounce) {
    pinMode(pin, INPUT_PULLUP);
  }

  bool isOnCooldown() const { return millis() - lastInterruptTime < debounceDelayMs; }

  void resetCooldown() {
    handleButtonPressNextTick = true;
    lastInterruptTime = millis();
  }

  virtual void onPress() {
    handleButtonPressNextTick = false;
  }
};


class StartStopButton : public Button {
public:
  using Button::Button;
  void onPress() override;
};

class DifficultyButton : public Button {
public:
  using Button::Button;
  void onPress() override;
};

void startStopButtonISR();
void difficultyButtonISR();
void startCountdown();
void startRound();
void updateDifficulty();
void generateRandomWord();
void checkCharacterInput(char ch);
void handleBackspace();
void resetErrorDisplay();

RGBLED rgbLed(9, 10, 11); 
StartStopButton startStopButton(STARTSTOPBUTTONPIN, FALLING, 1000);
DifficultyButton difficultyButton(DIFFICULTYBUTTONPIN, FALLING, 1000);

// generare cuvinte cu software counter
ISR(TIMER1_COMPA_vect) {
  static int counter = 0;

  if (gameStarted) {
    counter++;
    
    if (counter >= timerOverflowCounter) {
      generateRandomWord();
      counter = 0; // reset counter 
    }
  }
}

// rutine de intrerupere
void startStopButtonISR() {
  if (!startStopButton.isOnCooldown()) {
    startStopButton.resetCooldown();
  }
}

void difficultyButtonISR() {
  if (!difficultyButton.isOnCooldown()) {
    difficultyButton.resetCooldown();
  }
}

void StartStopButton::onPress() {
  Button::onPress();
  if (!gameStarted && !countdownActive) {
    startCountdown(); // 3 sec countdown
  } else if (gameStarted) {
    gameStarted = false;
    rgbLed.turnWhite();
    Serial.println("Game stopped.");
    Serial.print("Score: ");
    Serial.println(score);
    countDown = 3;
    TCCR1B &= ~(1 << CS11); // stop timer1
  }
}

void DifficultyButton::onPress() {
  Button::onPress();
  chosenDifficulty = static_cast<Difficulty>((chosenDifficulty + 1) % 3);
  updateDifficulty();
}

void updateDifficulty() {
  const char* difficultyNames[] = {"Easy mode on!", "Medium mode on!", "Hard mode on!"};
  Serial.println(difficultyNames[chosenDifficulty]);

  timerOverflowCounter = 0;
  uint16_t timerInterval = 15625; 
  
  switch (chosenDifficulty) {
    case EASY:
      timerOverflowCounter = easyDifficulty;
      break;
    case MEDIUM:
      timerOverflowCounter = mediumDifficulty;   
      break;
    case HARD:
      timerOverflowCounter = hardDifficulty;   
      break;
  }

  cli(); // dezactivare intreruperi globale
  TCCR1A = 0; 
  TCCR1B = 0;
  TCNT1 = 0;  
  OCR1A = timerInterval;
  TCCR1B |= (1 << WGM12); // CTC mode
  TCCR1B |= (1 << CS11) | (1 << CS10);  
  TIMSK1 |= (1 << OCIE1A);
  sei(); // activare intreruperi globale
}

void startCountdown() {
  countdownActive = true;
  countdownStartTime = millis();
  lastBlinkTime = millis(); 
  ledOnDuringCountdown = false;
  rgbLed.turnOff();
  Serial.println("Countdown started...");
}

void startRound() {
  countdownActive = false;
  gameStarted = true;
  score = 0;
  rgbLed.turnGreen();
  roundStartTime = millis();
  Serial.println("Round started!");
  updateDifficulty();
  
  generateRandomWord(); 
}

void generateRandomWord() {
  if(!gameStarted)
    return;
  int randomIndex = random(0, wordCount);
  currentWord = wordDictionary[randomIndex];
  Serial.print("\nType this word: ");
  Serial.println(currentWord);
  userInput = ""; // clear user input
}

void checkCharacterInput(char ch) {
  if (userInput.length() < currentWord.length()) {
    if (ch == currentWord[userInput.length()]) {
      userInput += ch; 
      if (userInput.equals(currentWord)) { 
        score++;
        rgbLed.turnGreen();
        generateRandomWord();
      }
    } else {
      rgbLed.turnRed();
      errorDisplayStartTime = millis();
      errorDisplayed = true;
    }
  }
}

// backspace
void handleBackspace() {
  if (userInput.length() > 0) {
    userInput.remove(userInput.length() - 1);
  }
}

// error display
void resetErrorDisplay() {
  if (errorDisplayed && millis() - errorDisplayStartTime >= ERROR_DISPLAY_DURATION_MS) {
    rgbLed.turnGreen();
    errorDisplayed = false;
  }
}

void setup() {
  Serial.begin(baudRate);
  rgbLed.turnWhite(); 
  updateDifficulty();
  attachInterrupt(digitalPinToInterrupt(STARTSTOPBUTTONPIN), startStopButtonISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(DIFFICULTYBUTTONPIN), difficultyButtonISR, FALLING);
}

void loop() {
  if (startStopButton.handleButtonPressNextTick) startStopButton.onPress();
  if (difficultyButton.handleButtonPressNextTick) difficultyButton.onPress();

  if (countdownActive) {
    if (millis() - lastBlinkTime >= BLINK_LED_TIME) { 
      lastBlinkTime = millis();
      ledOnDuringCountdown = !ledOnDuringCountdown;
      if (ledOnDuringCountdown) {
        Serial.println(countDown);
        countDown--; 
        rgbLed.turnWhite(); 
      } else {
        rgbLed.turnOff();   
      }
    }

    if (millis() - countdownStartTime >= COUNTDOWN_DURATION_MS) {
      startRound(); 
    }
  }

  if (gameStarted && (millis() - roundStartTime >= ROUND_DURATION_MS)) {
    gameStarted = false;
    rgbLed.turnWhite();
    countDown = 3;
    Serial.print("Round ended! Score: ");
    Serial.println(score);
    TCCR1B &= ~(1 << CS11);
  }

  resetErrorDisplay(); 

  while (Serial.available() > 0) {
    char ch = Serial.read();
    Serial.print(ch);
    if (ch == '\b') { 
      handleBackspace();
    } else {
      checkCharacterInput(ch); 
    }
  }
}