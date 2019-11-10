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

#define MOVE_DETECT_PIN 10

#define WITH_SEC_ON_DETECT_LED_PIN A0 // Пин для светодиода. Отображение секунд только при детекте.
#define WITH_NO_SEC_LED_PIN A1 // Пин для светодиода. Секунды не отображаются.
#define WITH_SEC_LED_PIN A2 // Пин для светодиода. Секунды отображаются всегда.
#define SEC_SETUP_LED_PIN A3 // Пин для светодиода. Настройка секунд.
#define MIN_SETUP_LED_PIN A4 // Пин для светодиода. Настройка минут.
#define HOUR_SETUP_LED_PIN A5 // Пин для светодиода. Настройка часов.

EasyButton modeButton(MODE_BUTTON_PIN); // Кнопка переключения режимов (отображения или настройки)
EasyButton frdButton(FRD_BUTTON_PIN); // Кнопка "вперед" (для отображения - переключение цветовой гаммы, для настройки - увеличение времени).
EasyButton backButton(BACK_BUTTON_PIN); // Кнопка "назад" (для отображения - переключение цветовой гаммы, для настройки - уменьшение времени).

TimeMode timeMode = withSeconds; // Текущий режим отображения.
ColorScheme color = greenForrest; // Текущая цветовая гамма.
SetupMode setupMode = none; // Текущий режим настройки.

virtuabotixRTC myRTC(6, 7, 8); // Структура для работы с часами. 6 - CLK, 7 - DAT, 8 - RST.
CRGB leds[NUM_LEDS]; // Структура для работы со светодиодным кольцом.

int setupSeconds = 0; // Установленные секунды
int setupMinutes = 0; // Установленные минуты
int setupHours = 0; // Установленные часы

int movementDetectorState = LOW; // Изначальное состояние детектора движения.

void onModePressedForDuration() {
  //Serial.println("Mode btn long click");
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
  //Serial.println("Mode btn single click");
  if (setupMode == none) {
    timeMode = nextTimeMode();
    toggleTimeLeds();

    // Example of sending request via serial interface
    Serial.print("BEG|onModeChanged|req|-1|current mode: ");
    Serial.print(timeMode);
    Serial.println("|END");
    Serial.flush();

  } else {
    setupMode = nextSetupMode();
    toggleSetupLeds();
  }
}

void onFrdPressed() {
  //Serial.println("Frd btn single click");
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
  //Serial.println("Back btn single click");
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

  pinMode(WITH_SEC_ON_DETECT_LED_PIN, OUTPUT);
  pinMode(WITH_NO_SEC_LED_PIN, OUTPUT);
  pinMode(WITH_SEC_LED_PIN, OUTPUT);
  pinMode(HOUR_SETUP_LED_PIN, OUTPUT);
  pinMode(MIN_SETUP_LED_PIN, OUTPUT);
  pinMode(SEC_SETUP_LED_PIN, OUTPUT);

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

  Serial.println("initialized");
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

      //Serial.println("New serial command:");
      //Serial.flush();

      String serialCmd = Serial.readStringUntil('\n');
      //Serial.println(serialCmd);

      int commandIndex = serialCmd.indexOf("|");
      String cmdName = serialCmd.substring(0, commandIndex);
      Serial.print("cmdName: " );
      Serial.print(cmdName);

      int replyIdIndex = serialCmd.indexOf("|", commandIndex + 1);
      String replyId = serialCmd.substring(commandIndex + 1, replyIdIndex);

      Serial.print(" replyId: " );
      Serial.print(replyId);

      int bodyIndex = serialCmd.indexOf("|", replyIdIndex + 1);
      String body = serialCmd.substring(replyIdIndex + 1);

      Serial.print(" body: " );
      Serial.println(body);

      //Serial.println("cmdName: " + cmdName + " replyId: " + replyId + " body: " + body);

      if (cmdName == "info") {

        Serial.print("BEG|info|");
        Serial.print("rsp|");
        Serial.print(replyId);
        Serial.print("|time: ");
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
        Serial.print(" possible color schemes: [blueLagoon, redDragon, fadeToGray, greenForrest]");
        Serial.println("|END");
        Serial.flush();
      } else if (cmdName == "setup") {
        setupMode = global;
        toggleSetupLeds();

        Serial.print("BEG|setup|");
        Serial.print("rsp|");
        Serial.print(replyId);
        Serial.print("|Global setup is on.");
        Serial.println("|END");
        Serial.flush();

      } else if (cmdName == "time") {

        Serial.print("BEG|time|rsp|" + replyId);

        if (setupMode == global) {
          //String time = body.substring(commandIndex + 1);
/////////////////
          int hourIndex = body.indexOf(":");
          //Serial.print(" hourIndex " + hourIndex);
          //Serial.print(hourIndex);
          String hour_str = body.substring(0, hourIndex);
          //Serial.print(" hour_str " + hour_str);
          //Serial.print(hour_str);
          setupHours = hour_str.toInt();
/////////////////
          int minuteIndex = body.indexOf(":", hourIndex + 1);
          //Serial.print(" minuteIndex " + minuteIndex);
          //Serial.print(minuteIndex);
          String minute_str = body.substring(hourIndex + 1, minuteIndex);
          //Serial.print(" minute_str " + minute_str);
          //Serial.print(minute_str);
          setupMinutes = minute_str.toInt();
/////////////////
          String second_str = body.substring(minuteIndex + 1);
          //Serial.print(" second_str " + second_str);
          //Serial.print(second_str);
          setupSeconds = second_str.toInt();
          Serial.print("|Presetting new time.");
/////////////////
        } else {
          Serial.print("|time is ignored because global setup mode if off.");
        }

        Serial.println("|END");
        Serial.flush();

      } else if (cmdName == "ok") {

        Serial.print("BEG|ok|");
        Serial.print("rsp|");
        Serial.print(replyId);

        if (setupMode == global) {
          setupMode = none;
          toggleTimeLeds();
          clearLeds();
          myRTC.setDS1302Time(setupSeconds, setupMinutes, setupHours, 2, 29, 10, 2019);

          Serial.print("|New time is set. Global setup is off.");

        } else {
          Serial.print("|cmd ok is ignored because global setup mode if off.");
        }

        Serial.println("|END");
        Serial.flush();

      } else if (cmdName == "cancel") {

        Serial.print("BEG|cancel|");
        Serial.print("rsp|");
        Serial.print(replyId);

        if (setupMode == global) {
          setupMode = none;
          toggleTimeLeds();
          clearLeds();
          Serial.print("|Returned to previous time. Global setup is off.");
        } else {
          Serial.print("|cancel ok is ignored because global setup mode if off.");
        }

        Serial.println("|END");
        Serial.flush();

      } else if (cmdName == "color") {

        Serial.print("BEG|color|");
        Serial.print("rsp|");
        Serial.print(replyId);
        Serial.print("|");

//        String colorSchemeName = body.substring(commandIndex + 1);
//        Serial.print("colorSchemeName: " );
//        Serial.println(colorSchemeName);

        if (body.startsWith("blueLagoon")) {
          //Serial.print(" Color switched to blueLagoon");
          color = blueLagoon;
        }
        else if (body.startsWith("redDragon")) {
          //Serial.print(" Color switched to redDragon");
          color = redDragon;
        }
        else if (body.startsWith("fadeToGray")) {
          //Serial.print(" Color switched to fadeToGray");
          color = fadeToGray;
        }
        else if (body.startsWith("greenForrest")) {
          //Serial.print(" Color switched to greenForrest");
          color = greenForrest;
        }

        Serial.print("color switched to: ");
        Serial.print(color);
        Serial.println("|END");
        Serial.flush();

      } else if (cmdName == "mode") {

        Serial.print("BEG|mode|");
        Serial.print("rsp|");
        Serial.print(replyId);

        if (body.startsWith("withSeconds")) {
          //Serial.print(" Mode switched to withSeconds");
          timeMode = withSeconds;
          toggleTimeLeds();
        }
        else if (body.startsWith("noSeconds")) {
          //Serial.print(" Mode switched to noSeconds");
          timeMode = noSeconds;
          toggleTimeLeds();
        }
        else if (body.startsWith("secondsOnDetect")) {
          //Serial.print(" Mode switched to secondsOnDetect");
          timeMode = secondsOnDetect;
          toggleTimeLeds();
        }

        Serial.print("|");
        Serial.print("mode switched to: ");
        Serial.print(timeMode);
        Serial.println("|END");
        Serial.flush();
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
      int detectorState = digitalRead(MOVE_DETECT_PIN);
      if (detectorState == HIGH) {
        leds[setupSeconds] = getSecondColor();
        if (movementDetectorState == LOW) {
          //Serial.println("---------------Motion detected!-------------");
          movementDetectorState = HIGH;
        }
      }
      else {
        if (movementDetectorState == HIGH) {
          leds[setupSeconds] = getSecondColor();
          //Serial.println("-----------------Motion ended!---------------");
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
  digitalWrite(HOUR_SETUP_LED_PIN, LOW);
  digitalWrite(MIN_SETUP_LED_PIN, LOW);
  digitalWrite(SEC_SETUP_LED_PIN, LOW);
  if (timeMode == withSeconds) {
    //Serial.println("WITH_SEC_LED_PIN");
    digitalWrite(WITH_SEC_LED_PIN, HIGH);
    digitalWrite(WITH_NO_SEC_LED_PIN, LOW);
    digitalWrite(WITH_SEC_ON_DETECT_LED_PIN, LOW);
  }
  else if (timeMode == noSeconds) {
    //Serial.println("WITH_NO_SEC_LED_PIN");
    digitalWrite(WITH_SEC_LED_PIN, LOW);
    digitalWrite(WITH_NO_SEC_LED_PIN, HIGH);
    digitalWrite(WITH_SEC_ON_DETECT_LED_PIN, LOW);
  }
  else if (timeMode == secondsOnDetect) {
    //Serial.println("WITH_SEC_ON_DETECT_LED_PIN");
    digitalWrite(WITH_SEC_LED_PIN, LOW);
    digitalWrite(WITH_NO_SEC_LED_PIN, LOW);
    digitalWrite(WITH_SEC_ON_DETECT_LED_PIN, HIGH);
  }
}
void toggleSetupLeds() {
  digitalWrite(WITH_SEC_LED_PIN, LOW);
  digitalWrite(WITH_NO_SEC_LED_PIN, LOW);
  digitalWrite(WITH_SEC_ON_DETECT_LED_PIN, LOW);
  if (setupMode == hours) {
    //Serial.println("setupMode == hours");
    digitalWrite(HOUR_SETUP_LED_PIN, HIGH);
    digitalWrite(MIN_SETUP_LED_PIN, LOW);
    digitalWrite(SEC_SETUP_LED_PIN, LOW);
  }
  else if (setupMode == minutes) {
    //Serial.println("setupMode == minutes");
    digitalWrite(HOUR_SETUP_LED_PIN, LOW);
    digitalWrite(MIN_SETUP_LED_PIN, HIGH);
    digitalWrite(SEC_SETUP_LED_PIN, LOW);
  }
  else if (setupMode == seconds) {
    //Serial.println("setupMode == seconds");
    digitalWrite(HOUR_SETUP_LED_PIN, LOW);
    digitalWrite(MIN_SETUP_LED_PIN, LOW);
    digitalWrite(SEC_SETUP_LED_PIN, HIGH);
  }
}

void nextSeconds() {
  //Serial.print(" next seconds: ");
  if (setupSeconds == 59) {
    setupSeconds = 0;
  } else {
    setupSeconds ++;
  }
  //Serial.println(setupSeconds);
}

void previousSeconds() {
  //Serial.print(" previous seconds: ");
  if (setupSeconds == 0) {
    setupSeconds = 59;
  } else {
    setupSeconds--;
  }
  //Serial.println(setupSeconds);
}

void nextMinutes() {
  //Serial.print(" next minutes: ");
  if (setupMinutes == 59) {
    setupMinutes= 0;
  } else {
    setupMinutes++;
  }
  //Serial.println(setupMinutes);
}

void previousMinutes() {
  //Serial.print(" previous minutes: ");
  if (setupMinutes == 0) {
    setupMinutes= 59;
  } else {
    setupMinutes--;
  }
  //Serial.println(setupMinutes);
}

void nextHours() {
  //Serial.print(" next hours: ");
  if (setupHours == 23) {
    setupHours = 0;
  } else {
    setupHours++;
  }
  //Serial.println(setupHours);
}

void previousHours() {
  //Serial.print(" previous hours: ");
  if (setupHours == 0) {
    setupHours = 23;
  } else {
    setupHours--;
  }
  //Serial.println(setupHours);
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