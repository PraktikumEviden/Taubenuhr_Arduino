//libraries for LCD display and RTC module
#include <LiquidCrystal.h>
#include <RTClib.h>

//initialisation of pins 
int clock_A = 12;
int clock_B = 13;
int mode_btn = 2;
int enc_A = 3;
int enc_B = 4;
int enc_btn = 5;

//LCD pin setup
const int RS = 6, E = 7, D4 = 8, D5 = 9, D6 = 10, D7 = 11;
LiquidCrystal lcd(RS, E, D4, D5, D6, D7);
RTC_DS3231 rtc;

bool tick = true; //variable to track clock ticks
int aState; // current state of encoder pin A
volatile int aLastState; // last state of encoder pin A
volatile int encValue = 0; // encoder value

//Modes for the program
enum Mode {
  SHOW_TIME, // show the time and the date
  MENU, //show the main menu
  SET_DATE,
  SET_TIME,
  SET_ALARM,
  ALARM_RINGING
};

Mode current_mode = SHOW_TIME; // initiakize to showing time
int selected_menu_items = 0; // track selected menu items
int selected_alarm = 0; // track selected alarm

// Array to store up to 3 alarms (hour : minute)
int alarms[3][2] = {{7, 0}, {0, 0}, {0, 0}};
int alarm_count = 1;
bool alarm_ringing = false;
unsigned long snooze_time = 0;

const unsigned long ALARM_SNOOZE_DURATION = 420000; // snooze duration = 7 minutes

void setup() {
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Initializing...");
  delay(1000);

  pinMode(clock_A, OUTPUT);
  pinMode(clock_B, OUTPUT);
  pinMode(mode_btn, INPUT_PULLUP);

  pinMode(enc_A, INPUT);
  pinMode(enc_B, INPUT);
  pinMode(enc_btn, INPUT_PULLUP);

  aLastState = digitalRead(enc_A); 

  // interrupts for mode button and encoder button
  attachInterrupt(digitalPinToInterrupt(enc_btn), handleEncButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(enc_A), handleEncoder, CHANGE);

  // initialize RTC and check if it's working
  if (!rtc.begin()) {
    lcd.print("RTC not found");
    while (1);
  }

  // adjust RTC time if it lost power
  if (rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  lcd.clear();
}

void loop() {
  DateTime now = rtc.now(); // get the currernt time

  // handle mode button press
  if (digitalRead(mode_btn) == LOW) {
    delay(200);
    if (digitalRead(mode_btn) == LOW) {
      handleModeButton();
      while (digitalRead(mode_btn) == LOW);
    }
  }

  // handle modes for program
  if (current_mode == SHOW_TIME) {
    showTime(now);
    checkAlarm(now);
  } else if (current_mode == MENU) {
    showMenu();
  } else if (current_mode == SET_DATE) {
    setDate();
  } else if (current_mode == SET_TIME) {
    setTime();
  } else if (current_mode == SET_ALARM) {
    setAlarm();
  } else if (current_mode == ALARM_RINGING) {
    showAlarmRinging();
  }
}

void showTime(DateTime now) {
  // show the actual time and date -- standard mode
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  if (now.hour() < 10) lcd.print("0");
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");
  lcd.print(now.minute());
  lcd.print(":");
  if (now.second() < 10) lcd.print("0");
  lcd.print(now.second());

  lcd.setCursor(0, 1);
  lcd.print("Date: ");
  if (now.day() < 10) lcd.print("0");
  lcd.print(now.day());
  lcd.print("/");
  if (now.month() < 10) lcd.print("0");
  lcd.print(now.month());
  lcd.print("/");
  lcd.print(now.year());

  forSec(now.second());
  delay(1000);
}

void forSec(byte currentSecond) {
  //move the clock hand
  static byte lastSecond = 0;
  if (currentSecond != lastSecond) {
    if (tick) {
      digitalWrite(clock_A, HIGH);
      delay(30);
      digitalWrite(clock_A, LOW);
      tick = false;
    } else {
      digitalWrite(clock_B, HIGH);
      delay(30);
      digitalWrite(clock_B, LOW);
      tick = true;
    }
    lastSecond = currentSecond; // Upload th last second
  }
}

void showMenu() {
  /*
  open the main menu with two options: set time/date and set alarm
  arrow ">" shows which item was selected
  with pressing encoder button chooses the item
  */
  lcd.clear(); 

  int arrowPosition = (encValue % 2 ==0 ) ? 0 : 1;

  lcd.setCursor(1, 0);
  lcd.print("Set date/time");
  lcd.setCursor(1, 1);
  lcd.print("Set alarm");

  lcd.setCursor(0, arrowPosition);
  lcd.write(">");

  if (encButtonPress()) {
    if (arrowPosition == 0) {
      showDateTimeMenu();
    } else {
      setAlarm();
    }
    delay(200);
  }
}

void showDateTimeMenu() {
  /*
  open the date/time menu with two items: set th date and set the time
  arrow ">" shows which item was selected
  with pressing encoder button chooses the item
  */
  lcd.clear();

  int arrowPosition = (encValue % 2 ==0 ) ? 0 : 1;

  lcd.setCursor(1, 0);
  lcd.print("Set the date");
  lcd.setCursor(1, 1);
  lcd.print("Set the time");

  lcd.setCursor(0, arrowPosition);
  lcd.write(">");

  if (encButtonPress()) {
    if (arrowPosition == 0) {
      setDate();
    } else {
      setTime();
    }
    delay(200);
  }
}

void setDate() {
  /*
  set the date starting from the day
  rotating encoder change the numbers of day/month/year
  pressing encoder button move it forward and double-click encoder button saves the date
  */

  static int step = 0;
  DateTime now = rtc.now();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set date");

  lcd.setCursor(0, 1);
  if (step == 0) lcd.print("Day: ");
  else if (step == 1) lcd.print("Month: ");
  else if (step == 2) lcd.print("Year: ");

  if (step == 0) lcd.print(now.day() + encValue);
  else if (step == 1) lcd.print(now.month() + encValue);
  else if (step == 2) lcd.print(now.year() + encValue);

  //if (now.day() + encValue >)

  delay(200);

  if (encButtonPress()) {
    if (step == 0) {
      rtc.adjust(DateTime(now.year(), now.month(), now.day() + encValue, now.hour(), now.minute(), now.second()));
      step = 1;
      encValue = 0;
    } else if (step == 1) {
      rtc.adjust(DateTime(now.year(), now.month() + encValue, now.day(), now.hour(), now.minute(), now.second()));
      step = 2;
      encValue = 0;
    } else if (step == 2) {
      rtc.adjust(DateTime(now.year() + encValue, now.month(), now.day(), now.hour(), now.minute(), now.second()));
      step = 0;
      current_mode = SHOW_TIME;
    }
  }
}

void setTime() {
  /*
  set the time (only hour and minute)
  rotating encoder change the numbers of hours/minutes
  pressing encoder button move it forward and double-click encoder button saves the time
  */

  static int step = 0;
  DateTime now = rtc.now();
  lcd.clear();
  lcd.setCursor(0, 4);
  lcd.print("Set Time");

  lcd.setCursor(1, 1);
  if (step == 0) lcd.print("Hour: ");
  else if (step == 1) lcd.print("Minute: ");

  if (step == 0) lcd.print(now.hour() + encValue);
  else if (step == 1) lcd.print(now.minute() + encValue);

  delay(200);

  if (encButtonPress()) {
    if (step == 0) {
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour() + encValue, now.minute(), now.second()));
      step = 1;
      encValue = 0;
    } else if (step == 1) {
      rtc.adjust(DateTime(now.year(), now.month(), now.day(), now.hour(), now.minute() + encValue, now.second()));
      step = 0;
      current_mode = SHOW_TIME;
    }
  }
}

void setAlarm() {
  /*
  open the alarm menu, firstly with two items: Alarm 1: 7:00 and New alarm
  using encoder button selects the item
  rotating encoder change the numbers for alarm (look adjustAlarm(int alarm_index)) und double-click saves the time
  */

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Alarm ");
  lcd.print(selected_alarm + 1);
  lcd.print(": ");
  lcd.print(alarms[selected_alarm][0] < 10 ? "0" : "");
  lcd.print(alarms[selected_alarm][0]);
  lcd.print(":");
  lcd.print(alarms[selected_alarm][1] < 10 ? "0" : "");
  lcd.print(alarms[selected_alarm][1]);

  lcd.setCursor(0, 1);
  if (selected_alarm < alarm_count) {
    lcd.print("Set time");
  } else {
    lcd.print("New alarm +");
  }

  delay(200);

  if (encButtonPress() && selected_alarm < alarm_count) {
    adjustAlarm(selected_alarm);
  } else if (encButtonPress() && selected_alarm == alarm_count) {
    alarm_count++;
    adjustAlarm(selected_alarm);
  }
}

void adjustAlarm(int alarm_index) {
  // rotating encoder change and adjust the time for chosen alarm
  // using encoder button moves it forward hour -> minutes

  static int step = 0;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set Alamr Time");

  lcd.setCursor(1, 0);
  if (step == 0) lcd.print("Hour: ");
  else if (step == 1) lcd.print("Minute: ");

  if (step == 0) lcd.print(alarms[alarm_index][0] + encValue);
  else if (step == 1) lcd.print(alarms[alarm_index][1] + encValue);

  delay(200);

  if (encButtonPress()) {
    if (step == 0) {
      alarms[alarm_index][0] = (alarms[alarm_index][0] + encValue + 24) % 24;
      step = 1;
      encValue = 0;
    } else if (step == 1) {
      alarms[alarm_index][1] = (alarms[alarm_index][1] + encValue + 60) % 60;
      step = 0;
      current_mode = SET_ALARM;
    }
  }
}

void checkAlarm(DateTime now) {
  // check if the alarm time is the same like real time

  for (int i = 0; i < alarm_count; i++) {
    if (now.hour() == alarms[i][0] && now.minute() == alarms[i][1] && !alarm_ringing) {
      ringingAlarm();
    }

    if (alarm_ringing && millis() - snooze_time >= ALARM_SNOOZE_DURATION) {
      ringingAlarm();
    }
  }
}

void ringingAlarm() {
  // ring the alarm

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Alarm! Wake up!");
  //digitalWrite(led_pin, HIGH);
  alarm_ringing = true;
  snooze_time = millis();
  current_mode = ALARM_RINGING;
}

void handleModeButton() {
  // pressing(holding) mode button changes the mode from showing time and date to main menu and back

  if (current_mode == SHOW_TIME) {
    current_mode = MENU;
  } else if (current_mode == MENU) {
    if (selected_menu_items == 0) {
      current_mode = SET_DATE;
    } else if (selected_menu_items == 1) {
      current_mode = SET_ALARM;
    }
  } else if (current_mode == ALARM_RINGING) {
    if (millis() - snooze_time < 500) {
      alarm_ringing = false;
      //digitalWrite(led_pin, LOW);
      current_mode = SHOW_TIME;
    } else {
      snooze_time = millis();
      alarm_ringing = false;
      //digitalWrite(led_pin, LOW);
    }
  }
}

void handleEncButton() {
  if (current_mode == MENU) {
    selected_menu_items = (selected_menu_items + 1) % 2;
  } else if (current_mode == SET_ALARM) {
    selected_alarm = (selected_alarm + 1) % (alarm_count + 1);
  }
}

void handleEncoder() {
  // reading the signal from encoder and change the encoder value to check on which rotate the encoder

  aState = digitalRead(enc_A);
  if (aState != aLastState) {
    if (digitalRead(enc_B) != aState) {
      encValue++;
    } else {
      encValue--;
    }
    aLastState = aState;
  }
}

bool encButtonPress() {
  // press encoder button to choose item from menus

  static unsigned long last_press_time = 0;
  unsigned long currentTime = millis();

  if (digitalRead(enc_btn) == LOW) {
    if (millis() - last_press_time > 200) {
      last_press_time = currentTime;
      return true;
    }
  }
  return false;
}

bool doubleEncPress() {
  // double-click encoder button to save the time/date/alarm

  static unsigned long last_press_time = 0;
  static int press_count = 0;

  if (encButtonPress()) {
    if (millis() - last_press_time < 500) {
      press_count++;
    } else {
      press_count = 1;
    }

    last_press_time = millis();

    if (press_count == 2) {
      press_count = 0;
      return true;
    }
  }
  return false;
}
