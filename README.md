# Taubenuhr_Arduino
Clock with alarm built using Ardiuno Uno and built in old pigeon clock with an Arduino, breadboard, RTC module DS3231, LCD display 16x2, rotary encoder and one button.

Schematic (without clock):
![image](https://github.com/user-attachments/assets/47807684-33c7-4005-836e-efd743dff7e8)

* Potentiometer was use for regulatin the brightness on the disply (not on the scheme); connected to GND, 5V and V0 on the LCD display
** Clock hands connected to pins 12 and 13 

Project description:

In the showTime mode, the current date and time (updated every second) are shown on the screen. This is the default mode, running in parallel with the clock hands. When the button is pressed and held, the main menu appears on the screen, offering options to set the time/date and configure alarms.

The ">" indicator shows the currently selected menu item. Pressing the encoder button confirms the selection from the menu.

In the time and date settings section, a submenu opens to choose between setting the date or time. In the alarm settings section, you can select the first alarm or add another one.

A double press of the encoder button saves the time/date/alarm settings and returns to the time display mode.

Buttons:

1. Normal button(hold) - change from show time mode to main menu and back; if alarm ring - stop alarm.
2. Encoder button(one click) - choose the items in the menus and move forward while the time/date/alarm (numbers) setting; if alarm ring - snooze alarm for 7 minues.
3. Encoder button(double-click) - save the time/date/alarm.
4. Encoder rotator - change the numbers by setting; move the indicator ">" from item to item in menus.
