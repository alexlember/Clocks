#include <FastLED.h>
#include <virtuabotixRTC.h>

enum ColorScheme { blueLagoon, redDragon, fadeToGray, greenForrest };
enum TimeMode { withSeconds, noSeconds, secondsOnDetect };
enum SetupMode { hours, minutes, seconds, global, none };

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define HOUR_MULTIPLIER    5

#define DATA_PIN    9

#define LED_TYPE    WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS    60
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          20
#define FRAMES_PER_SECOND  120

virtuabotixRTC myRTC(6, 7, 8); //If you change the wiring change the pins here also

// choose the input pin (for PIR sensor)
int inputPin = 10;
int pirState = LOW;
int val = 0;

const int modeButtonPin = 13;
const int frdButtonPin = 12;
const int backButtonPin = 11;
const int withSecondsOnDetectDisplayLedPin = A0;
const int withNoSecondsDisplayLedPin = A1;
const int withSecondsDisplayLedPin = A2;

const int hoursSetupLedPin = A5;      // Пин для светодиода. Настройка часов.
const int minutesSetupLedPin = A4;      // Пин для светодиода. Настройка минут.
const int secondsSetupLedPin = A3;      // Пин для светодиода. Настройка секунд.

int modeButtonState = HIGH;
int lastModeButtonState = HIGH;

int frdButtonState = HIGH;
int lastFrdButtonState = HIGH;

int backButtonState = HIGH;
int lastBackButtonState = HIGH;

long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

boolean modeButtonActive = false;
boolean longPressActive = false;

long buttonTimer = 0;
long longPressTime = 1000;

TimeMode timeMode = withSeconds;
ColorScheme color = blueLagoon;
SetupMode setupMode = none;

String cmd;

// Currently set time
int setupSeconds = 0;
int setupMinutes = 0;
int setupHours = 0;

int previousSetupSeconds = 0;
int previousSetupMinutes = 0;
int previousSetupHours = 0;

void setup() {
  Serial.begin(9600);

  pinMode(modeButtonPin, INPUT);
  pinMode(withSecondsOnDetectDisplayLedPin, OUTPUT);
  pinMode(withNoSecondsDisplayLedPin, OUTPUT);
  pinMode(withSecondsDisplayLedPin, OUTPUT);

  delay(1000); // 1 second delay

  toggleTimeLeds();

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  //myRTC.setDS1302Time(00, 00, 00, 6, 26, 10, 2019);
}

void loop()
{

  if (setupMode == global) {

    delay(200);
    clearLeds();
    FastLED.show();
    delay(200);

    leds[setupSeconds] = getSecondColor();
    leds[setupMinutes] = getMinuteColor();
    leds[getModifiedHour(setupHours)] = getHourColor();
    FastLED.show();

  }


  // Module RTC time update.
  myRTC.updateTime();
  int currentSeconds = myRTC.seconds;
  int currentMinutes = myRTC.minutes;
  int currentHours = myRTC.hours;

  if (setupMode == none) {
    setupSeconds = currentSeconds;
    setupMinutes = currentMinutes;
    setupHours = currentHours;
  }

// Uncomment to debug
//  Serial.print(currentSeconds);
//  Serial.print(" ");
//  Serial.print(currentMinutes);
//  Serial.print(" ");
//  Serial.print(currentHours);
//  Serial.print(" ");


// --------------------------------------------
// MODE btn click

  int modeBtnReading = digitalRead(modeButtonPin);

  if (modeBtnReading != lastModeButtonState) {
    lastDebounceTime = millis();
    Serial.println("MODE BTN CLICKED");
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (modeBtnReading != modeButtonState) {
      Serial.println("Mode btn state changed condition");
      modeButtonState = modeBtnReading;

      if (modeButtonState == HIGH) {
        if ((millis() - lastDebounceTime) > longPressTime) {

          Serial.println("SETUP RELEASED");
          if (setupMode == none) {
            setupMode = hours;
            toggleSetupLeds();
          } else {
            setupMode = none;
            toggleTimeLeds();
          }

        } else {


            Serial.println("MODE RELEASED");

            timeMode = nextTimeMode();
            toggleTimeLeds();
        }
      }
    }
  }

  lastModeButtonState = modeBtnReading;


// --------------------------------------------
// FRD btn click

  int frdBtnReading = digitalRead(frdButtonPin);
  if (frdBtnReading != lastFrdButtonState) {
    lastDebounceTime = millis();
    Serial.println("FRD BTN CLICKED");
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (frdBtnReading != frdButtonState) {
      Serial.println("Frd state changed condition");
      frdButtonState = frdBtnReading;

      if (frdButtonState == HIGH) {
        Serial.println("FRD RELEASED");

        color = nextColorScheme();
      }
    }
  }

  lastFrdButtonState = frdBtnReading;


// --------------------------------------------
// BACK btn click

  int backBtnReading = digitalRead(backButtonPin);
    if (backBtnReading != lastBackButtonState) {
    lastDebounceTime = millis();
    Serial.println("BACK BTN CLICKED");
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {

    if (backBtnReading != backButtonState) {
      Serial.println("Back btn state changed condition");
      backButtonState = backBtnReading;

      if (backButtonState == HIGH) {
        Serial.println("BACK RELEASED");

        color = previousColorScheme();
      }
    }
  }

  lastBackButtonState = backBtnReading;
// --------------------------------------------


  if (Serial.available() > 0) {
      cmd = Serial.readStringUntil('$');

      int commandIndex = cmd.indexOf(";");
      String cmdName = cmd.substring(0, commandIndex);
      Serial.print("cmdName: " );
      Serial.println(cmdName);

      if (cmdName == "info") {
        Serial.println("");
        Serial.print("time: ");

        String str_hours = String(currentHours);
        if (currentHours < 10) {
          str_hours = "0" + str_hours;
        }

        String str_minutes = String(currentMinutes);
        if (currentMinutes < 10) {
          str_minutes = "0" + str_minutes;
        }

        String str_seconds = String(currentSeconds);
        if (currentSeconds < 10) {
          str_seconds = "0" + str_seconds;
        }

        Serial.print(str_hours);
        Serial.print(":");
        Serial.print(str_minutes);
        Serial.print(":");
        Serial.print(str_seconds);
        Serial.print(";");
        Serial.print(" timeMode: ");
        Serial.print(timeMode);
        Serial.print(";");
        Serial.print(" possible time modes: [withSeconds, noSeconds, secondsOnDetect];");
        Serial.print(" is global setup mode: ");
        Serial.print(setupMode == global);
        Serial.print(";");
        Serial.print(" colorScheme: ");
        Serial.print(color);
        Serial.print(";");
        Serial.print(" possible color schemes: [blueLagoon, redDragon, fadeToGray, greenForrest];");
        Serial.println("");
      }

      if (cmdName == "setup") {
        setupMode = global;
        toggleSetupLeds();
        Serial.print(" Global setup is on " );
      }



      if (cmdName == "time") {
        if (setupMode == global) {
          String time = cmd.substring(commandIndex + 1);

/////////////////
          int hourIndex = time.indexOf(":");
          Serial.print(" hourIndex ");
          Serial.print(hourIndex);

          String hour_str = time.substring(0, hourIndex);
          Serial.print(" hour_str ");
          Serial.print(hour_str);

          previousSetupHours = setupHours;
          setupHours = hour_str.toInt();
/////////////////
          int minuteIndex = time.indexOf(":", hourIndex + 1);
          Serial.print(" minuteIndex ");
          Serial.print(minuteIndex);

          String minute_str = time.substring(hourIndex + 1, minuteIndex);
          Serial.print(" minute_str ");
          Serial.print(minute_str);

          previousSetupMinutes = setupMinutes;
          setupMinutes = minute_str.toInt();
/////////////////
          String second_str = time.substring(minuteIndex + 1);
          Serial.print(" second_str ");
          Serial.print(second_str);

          previousSetupSeconds = setupSeconds;
          setupSeconds = second_str.toInt();
/////////////////
        }
        Serial.print(" Presetting new time.");
      }

      if (cmdName == "ok") {
        if (setupMode == global) {
          setupMode = none;
          toggleTimeLeds();
          clearLeds();
          myRTC.setDS1302Time(setupSeconds, setupMinutes, setupHours, 6, 26, 10, 2019);
        }
        Serial.print(" New time is set. Global setup is off" );
      }

      if (cmdName == "cancel") {
        if (setupMode == global) {
          setupMode = none;
          toggleTimeLeds();
          clearLeds();
        }
        Serial.print(" Returned to previous time. Global setup is off" );
      }

      Serial.println(setupMode);

      if (cmdName == "color") {
        String colorSchemeName = cmd.substring(commandIndex + 1);
        Serial.print("colorSchemeName: " );
        Serial.println(colorSchemeName);

        if (colorSchemeName.startsWith("blueLagoon")) {
          Serial.print(" Color switched to blueLagoon");
          color = blueLagoon;
        }
        else if (colorSchemeName.startsWith("redDragon")) {
          Serial.print(" Color switched to redDragon");
          color = redDragon;
        }
        else if (colorSchemeName.startsWith("fadeToGray")) {
          Serial.print(" Color switched to fadeToGray");
          color = fadeToGray;
        }
        else if (colorSchemeName.startsWith("greenForrest")) {
          Serial.print(" Color switched to greenForrest");
          color = greenForrest;
        }
      }

      if (cmdName == "mode") {
        String modeName = cmd.substring(commandIndex + 1);
        Serial.print("modeName: " );
        Serial.println(modeName);

        if (modeName.startsWith("withSeconds")) {
          Serial.print(" Mode switched to withSeconds");
          timeMode = withSeconds;
          toggleTimeLeds();
        }
        else if (modeName.startsWith("noSeconds")) {
          Serial.print(" Mode switched to noSeconds");
          timeMode = noSeconds;
          toggleTimeLeds();
        }
        else if (modeName.startsWith("secondsOnDetect")) {
          Serial.print(" Mode switched to secondsOnDetect");
          timeMode = secondsOnDetect;
          toggleTimeLeds();
        }
      }
  }

  if (setupMode == none) {

    clearLeds();

    int modifiedHours = getModifiedHour(setupHours);

  // Uncomment for debug
  // Serial.println(" ");
  // Serial.println(" modifiedHours ");
  // Serial.println(modifiedHours);

    if (setupSeconds == 0) {
      leds[NUM_LEDS - 1] = CRGB::Black;
    } else {
      leds[setupSeconds - 1] = CRGB::Black;
    }

    if (setupMinutes == 0) {
      leds[NUM_LEDS - 1] = CRGB::Black;
    } else {
      leds[setupMinutes - 1] = CRGB::Black;
    }

    if (modifiedHours == 0) {
      leds[NUM_LEDS - HOUR_MULTIPLIER] = CRGB::Black;
    } else {
      leds[modifiedHours - HOUR_MULTIPLIER] = CRGB::Black;
    }

    if (timeMode == withSeconds) {
      leds[setupSeconds] = getSecondColor();
    } else if (timeMode == secondsOnDetect) {

      val = digitalRead(inputPin);
      if (val == HIGH) {


        leds[setupSeconds] = getSecondColor();

        if (pirState == LOW) {
          Serial.println("---------------Motion detected!-------------");
          pirState = HIGH;
        }
      }
      else {

        if (pirState == HIGH) {
          leds[setupSeconds] = getSecondColor();

          Serial.println("-----------------Motion ended!---------------");
          pirState = LOW;
        } else {
          leds[setupSeconds] = CRGB::Black;
        }
      }

    }

    leds[setupMinutes] = getMinuteColor();
    leds[modifiedHours] = getHourColor();

  } else {

    // For setup mode.

    clearLeds();

    leds[setupSeconds] = getSecondColor();
    leds[setupMinutes] = getMinuteColor();
    leds[getModifiedHour(setupHours)] = getHourColor();

  }

  FastLED.show();
}

int getModifiedHour(int hour) {
      int modifiedHour = hour;
    if (hour > 11) {
      modifiedHour = hour - 12;
    }

    modifiedHour *= HOUR_MULTIPLIER;

    return modifiedHour;
}

void clearLeds() {
    FastLED.show();
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
    }
}

CRGB getHourColor() {

  if (color == blueLagoon) {
    return CRGB::Yellow;
  } else if (color == redDragon) {
    return CRGB::Red;
  } else if (color == fadeToGray) {
    return CRGB::Grey;
  } else {
    return CRGB::Green;
  }

}

CRGB getMinuteColor() {

  if (color == blueLagoon) {
    return CRGB::Red;
  } else if (color == redDragon) {
    return CRGB::Pink;
  } else if (color == fadeToGray) {
    return CRGB::Gold;
  } else {
    return CRGB::GreenYellow;
  }

}

CRGB getSecondColor() {

  if (color == blueLagoon) {
    return CRGB::Blue;
  } else if (color == redDragon) {
    return CRGB::Red;
  } else if (color == fadeToGray) {
    return CRGB::Gray;
  } else {
    return CRGB::Green;
  }

}

TimeMode nextTimeMode() {
  if (timeMode == withSeconds) {
    return noSeconds;
  }
  else if (timeMode == noSeconds) {
    return secondsOnDetect;
  }
  else if (timeMode == secondsOnDetect) {
    return withSeconds;
  }
}

SetupMode nextSetupMode() {
  if (setupMode == hours) {
    return minutes;
  }
  else if (setupMode == minutes) {
    return seconds;
  }
  else if (setupMode == seconds) {
    return hours;
  }
  else {
    return none;
  }
}

ColorScheme nextColorScheme() {
  if (color == blueLagoon) {
    return redDragon;
  }
  else if (color == redDragon) {
    return fadeToGray;
  }
  else if (color == fadeToGray) {
    return greenForrest;
  }
  else {
    return blueLagoon;
  }
}

ColorScheme previousColorScheme() {
  if (color == blueLagoon) {
    return greenForrest;
  }
  else if (color == redDragon) {
    return blueLagoon;
  }
  else if (color == fadeToGray) {
    return redDragon;
  }
  else {
    return fadeToGray;
  }
}

void toggleTimeLeds() {
  digitalWrite(hoursSetupLedPin, LOW);
  digitalWrite(minutesSetupLedPin, LOW);
  digitalWrite(secondsSetupLedPin, LOW);

  if (timeMode == withSeconds) {
    Serial.println("withSecondsDisplayLedPin");
    digitalWrite(withSecondsDisplayLedPin, HIGH);
    digitalWrite(withNoSecondsDisplayLedPin, LOW);
    digitalWrite(withSecondsOnDetectDisplayLedPin, LOW);
  }
  else if (timeMode == noSeconds) {
    Serial.println("withNoSecondsDisplayLedPin");
    digitalWrite(withSecondsDisplayLedPin, LOW);
    digitalWrite(withNoSecondsDisplayLedPin, HIGH);
    digitalWrite(withSecondsOnDetectDisplayLedPin, LOW);
  }
  else if (timeMode == secondsOnDetect) {
    Serial.println("withSecondsOnDetectDisplayLedPin");
    digitalWrite(withSecondsDisplayLedPin, LOW);
    digitalWrite(withNoSecondsDisplayLedPin, LOW);
    digitalWrite(withSecondsOnDetectDisplayLedPin, HIGH);
  }
}

void toggleSetupLeds() {

  digitalWrite(withSecondsDisplayLedPin, LOW);
  digitalWrite(withNoSecondsDisplayLedPin, LOW);
  digitalWrite(withSecondsOnDetectDisplayLedPin, LOW);

  if (setupMode == hours) {
    Serial.println("setupMode == hours");
    digitalWrite(hoursSetupLedPin, HIGH);
    digitalWrite(minutesSetupLedPin, LOW);
    digitalWrite(secondsSetupLedPin, LOW);
  }
  else if (setupMode == minutes) {
    Serial.println("setupMode == minutes");
    digitalWrite(hoursSetupLedPin, LOW);
    digitalWrite(minutesSetupLedPin, HIGH);
    digitalWrite(secondsSetupLedPin, LOW);
  }
  else if (setupMode == seconds) {
    Serial.println("setupMode == seconds");
    digitalWrite(hoursSetupLedPin, LOW);
    digitalWrite(minutesSetupLedPin, LOW);
    digitalWrite(secondsSetupLedPin, HIGH);
  }
}

