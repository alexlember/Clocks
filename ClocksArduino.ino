#include <FastLED.h>
#include <virtuabotixRTC.h>
#include <EasyButton.h>

enum ColorScheme { blueLagoon, redDragon, fadeToGray, greenForrest }; // Набор возможных цветовых гамм.
enum TimeMode { withSeconds, noSeconds, secondsOnDetect }; // Набор возможных режимов отображения.
enum SetupMode { hours, minutes, seconds, global, none }; // Набор возможных режимов настройки.

FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif
#define HOUR_MULTIPLIER 5
#define DATA_PIN 9
#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define NUM_LEDS 60

#define BRIGHTNESS 20
#define FRAMES_PER_SECOND 120
#define LONG_CLICK_MS 2000
#define BACK_BUTTON_PIN 11
#define FRD_BUTTON_PIN 12
#define MODE_BUTTON_PIN 13

int movementDetectorInputPin = 10; // Пин для получения данных с детектора движения.
int movementDetectorState = LOW; // Изначальное состояние детектора движения.

const int withSecondsOnDetectDisplayLedPin = A0; // Пин для светодиода. Отображение секунд только при детекте.
const int withNoSecondsDisplayLedPin = A1; // Пин для светодиода. Секунды не отображаются.
const int withSecondsDisplayLedPin = A2; // Пин для светодиода. Секунды отображаются всегда.
const int hoursSetupLedPin = A5; // Пин для светодиода. Настройка часов.
const int minutesSetupLedPin = A4; // Пин для светодиода. Настройка минут.
const int secondsSetupLedPin = A3; // Пин для светодиода. Настройка секунд.

EasyButton modeButton(MODE_BUTTON_PIN); // Кнопка переключения режимов (отображения или настройки)
EasyButton frdButton(FRD_BUTTON_PIN); // Кнопка "вперед" (для отображения - переключение цветовой гаммы, для настройки - увеличение времени).
EasyButton backButton(BACK_BUTTON_PIN); // Кнопка "назад" (для отображения - переключение цветовой гаммы, для настройки - уменьшение времени).

TimeMode timeMode = withSeconds; // Текущий режим отображения.
ColorScheme color = greenForrest; // Текущая цветовая гамма.
SetupMode setupMode = none; // Текущий режим настройки.

virtuabotixRTC myRTC(6, 7, 8); // Структура для работы с часами. 6 - CLK, 7 - DAT, 8 - RST.
CRGB leds[NUM_LEDS]; // Структура для работы со светодиодным кольцом.

String serialCmd; // Команда, пришедшая через serial interface

int setupSeconds = 0; // Установленные секунды
int setupMinutes = 0; // Установленные минуты
int setupHours = 0; // Установленные часы

void onModePressedForDuration() {
  Serial.println("Mode btn long click");
  if (setupMode == none) {
    setupMode = hours;
    toggleSetupLeds();
  } else {
    setupMode = none;
    toggleTimeLeds();
    clearLeds();
    myRTC.setDS1302Time(setupSeconds, setupMinutes, setupHours, 2, 29, 10, 2019);
  }
}

void onModePressed() {
  Serial.println("Mode btn single click");
  if (setupMode == none) {
    timeMode = nextTimeMode();
    toggleTimeLeds();
  } else {
    setupMode = nextSetupMode();
    toggleSetupLeds();
  }
}

void onFrdPressed() {
  Serial.println("Frd btn single click");
  if (setupMode == none) {
     color = nextColorScheme();
  } else if (setupMode == hours) {
    nextHours();
  } else if (setupMode == minutes) {
    nextMinutes();
  } else if (setupMode == seconds) {
    nextSeconds();
  }
}

void onBackPressed() {
  Serial.println("Back btn single click");
  if (setupMode == none) {
     color = previousColorScheme();
  } else if (setupMode == hours) {
    previousHours();
  } else if (setupMode == minutes) {
    previousMinutes();
  } else if (setupMode == seconds) {
    previousSeconds();
  }
}

void setup() {
  Serial.begin(9600);

  pinMode(withSecondsOnDetectDisplayLedPin, OUTPUT);
  pinMode(withNoSecondsDisplayLedPin, OUTPUT);
  pinMode(withSecondsDisplayLedPin, OUTPUT);
  pinMode(hoursSetupLedPin, OUTPUT);
  pinMode(minutesSetupLedPin, OUTPUT);
  pinMode(secondsSetupLedPin, OUTPUT);

  modeButton.begin();
  frdButton.begin();
  backButton.begin();

  modeButton.onPressedFor(LONG_CLICK_MS, onModePressedForDuration);
  modeButton.onPressed(onModePressed);
  frdButton.onPressed(onFrdPressed);
  backButton.onPressed(onBackPressed);

  delay(1000); // 1 second delay

  toggleTimeLeds();

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  //myRTC.setDS1302Time(00, 00, 00, 2, 29, 10, 2019);
}

void loop()
{
  modeButton.read();
  frdButton.read();
  backButton.read();

  if (setupMode != none) {
    delay(20);
    if (setupMode == global) {
      clearLeds();
    } else if (setupMode == hours) {
       leds[getModifiedHour(setupHours)] = CRGB::Black;
    } else if (setupMode == minutes) {
       leds[setupMinutes] = CRGB::Black;
    } else if (setupMode == seconds) {
       leds[setupSeconds] = CRGB::Black;
    }
    FastLED.show();
    delay(20);
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
  if (Serial.available() > 0) {
      serialCmd = Serial.readStringUntil('$');
      int commandIndex = serialCmd.indexOf(";");
      String cmdName = serialCmd.substring(0, commandIndex);
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
          String time = serialCmd.substring(commandIndex + 1);
/////////////////
          int hourIndex = time.indexOf(":");
          Serial.print(" hourIndex ");
          Serial.print(hourIndex);
          String hour_str = time.substring(0, hourIndex);
          Serial.print(" hour_str ");
          Serial.print(hour_str);
          //previousSetupHours = setupHours;
          setupHours = hour_str.toInt();
/////////////////
          int minuteIndex = time.indexOf(":", hourIndex + 1);
          Serial.print(" minuteIndex ");
          Serial.print(minuteIndex);
          String minute_str = time.substring(hourIndex + 1, minuteIndex);
          Serial.print(" minute_str ");
          Serial.print(minute_str);
          //previousSetupMinutes = setupMinutes;
          setupMinutes = minute_str.toInt();
/////////////////
          String second_str = time.substring(minuteIndex + 1);
          Serial.print(" second_str ");
          Serial.print(second_str);
          //previousSetupSeconds = setupSeconds;
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
          myRTC.setDS1302Time(setupSeconds, setupMinutes, setupHours, 2, 29, 10, 2019);
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
        String colorSchemeName = serialCmd.substring(commandIndex + 1);
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
        String modeName = serialCmd.substring(commandIndex + 1);
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
      int detectorState = digitalRead(movementDetectorInputPin);
      if (detectorState == HIGH) {
        leds[setupSeconds] = getSecondColor();
        if (movementDetectorState == LOW) {
          Serial.println("---------------Motion detected!-------------");
          movementDetectorState = HIGH;
        }
      }
      else {
        if (movementDetectorState == HIGH) {
          leds[setupSeconds] = getSecondColor();
          Serial.println("-----------------Motion ended!---------------");
          movementDetectorState = LOW;
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

void nextSeconds() {
  Serial.print(" next seconds: ");
  if (setupSeconds == 59) {
    setupSeconds = 0;
  } else {
    setupSeconds ++;
  }
  Serial.println(setupSeconds);
}

void previousSeconds() {
  Serial.print(" previous seconds: ");
  if (setupSeconds == 0) {
    setupSeconds = 59;
  } else {
    setupSeconds--;
  }
  Serial.println(setupSeconds);
}

void nextMinutes() {
  Serial.print(" next minutes: ");
  if (setupMinutes == 59) {
    setupMinutes= 0;
  } else {
    setupMinutes++;
  }
  Serial.println(setupMinutes);
}

void previousMinutes() {
  Serial.print(" previous minutes: ");
  if (setupMinutes == 0) {
    setupMinutes= 59;
  } else {
    setupMinutes--;
  }
  Serial.println(setupMinutes);
}

void nextHours() {
  Serial.print(" next hours: ");
  if (setupHours == 23) {
    setupHours = 0;
  } else {
    setupHours++;
  }
  Serial.println(setupHours);
}

void previousHours() {
  Serial.print(" previous hours: ");
  if (setupHours == 0) {
    setupHours = 23;
  } else {
    setupHours--;
  }
  Serial.println(setupHours);
}

CRGB getHourColor() {
  if (color == blueLagoon) {
    return CRGB::CadetBlue;
  } else if (color == redDragon) {
    return CRGB::Red;
  } else if (color == fadeToGray) {
    return CRGB::Purple;
  } else {
    return CRGB::Green;
  }
}

CRGB getMinuteColor() {
  if (color == blueLagoon) {
    return CRGB::Blue;
  } else if (color == redDragon) {
    return CRGB::Fuchsia;
  } else if (color == fadeToGray) {
    return CRGB::Peru;
  } else {
    return CRGB::SeaGreen;
  }
}

CRGB getSecondColor() {
  if (color == blueLagoon) {
    return CRGB::Coral;
  } else if (color == redDragon) {
    return CRGB::Pink;
  } else if (color == fadeToGray) {
    return CRGB::Gray;
  } else {
    return CRGB::GreenYellow;
  }
}