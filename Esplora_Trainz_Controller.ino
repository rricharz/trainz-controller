/*
  Esplora Trainz Controler
  
  Version 06 2017/2/1
  
  The sketch has been written of a ARDUINO ESPLORA with LCD shield.
  
  This sketch turns the Esplora into a game pad for Trainz (DCC control only).
  
  The buttons of the Esplora handle the following actions:
    top button           toggle map mode
    left button          Refill supply (handled internally in Esplora)
    right button         toggle forward switch (turnout)
    Reverse button       toggle driving direction (handled internally in Esplora)
    
  The slider sets the DCC throttle
  
  The joystick can be toggled two different states by pushing it down
  
    First state:
    left                 view direction towards left
    right                view direction towards right
    top                  Zoom in
    down                 Zoom out
    
    Second state:
    left                 move backwards in consist
    right                move forward in consist
    top                  view direction upwards
    bottom               view direction downwards
    
  The lcd display also shows the consumpion of a supply (for example water for a steam engine, or diesel for a diesel engine). The supply lasts for
  approximately 45 minutes at full throttle. To reset the supply level to maximum, the "Refill" button on the esplora needs to be pressed. This
  supply level is not connected to any internal supply level in Trainz, because the Esplora has no knowledge of any Trainz internals (no two way
  communication). But it makes driving in DCC mode much more interesting. Reset the supply level only at the corresponding refill station in Trainz.
  If the supply has been used up, the engine cannot be moved any further. The LED is green, if the supply is above a 10% level, below it is red.
    
  IMPORTANT: The sketch uses "keyboard.press" and "keyboard.release". These press and release actual buttons on the keyboard, and not the codes,
  which are normally assigned to the states of the keys on the keyboard. Therefore, the "shift", "alt" and "ctrl" keys need to be activaded
  at the same time, if for expample the keyboard code "ctrl-j" needs to be executed. Search for "KEY_LEFT_CTRL" and "KEY_LEFT_SHIFT" in the sketch.
  These might be replaced with "NULL" if keys without "ctrl" or "shift" are assigned to these actions. The sketch has been tested on a Mac (Trainz
  from Appstore) and a Mac with TANE using a Swiss German keyboard. The following two actions might not work on other keyboards without
  adding the following alternative key controls:
    move backwards in consist         <shift> c
    move forwards in consist          <ctrl>  c

  Coyright 2017 rricharz

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
  MA 02110-1301, USA.
  
  Based on the original game pad sketch by Enrico Gueli <enrico.gueli@gmail.com>

  
*/

#include <Keyboard.h>
#include <Esplora.h>
#include <TFT.h>  // Arduino LCD library
#include <SPI.h>  // Arduino SPI library, used by LCD library

boolean buttonStates[8];          // last sensed state of each of the buttons
int joystickButton = LOW;         // the joystick button must be handled differently
boolean joystickState1 = false;   // second state of the joystick

const byte buttons[] =       // names of the buttons being read
{
  JOYSTICK_DOWN,
  JOYSTICK_LEFT,
  JOYSTICK_UP,
  JOYSTICK_RIGHT,
  SWITCH_RIGHT,
  SWITCH_LEFT,
  SWITCH_UP,
  SWITCH_DOWN,
};

/*
  This array tells what keystroke to send to the PC when a
  button is pressed.
  If you look at this array and the above one, you can see that
  the "cursor down" keystroke is sent when the joystick is moved
  down, the "cursor up" keystroke when the joystick is moved up
  and so on. Note that the first 4 of these keystokes are modified
  for the second state of the joystick.
*/

int keystrokes[] = 
{
  KEY_PAGE_DOWN,   // Joystick Down
  KEY_LEFT_ARROW,  // Joystick Left
  KEY_PAGE_UP,     // Joystick Up
  KEY_RIGHT_ARROW, // Joystick Right
  'j',             // Switch forward
  0,               // Refill
  'm',             // Map
  0                // Reverse (handled internally)
};

boolean forwards = true;         // Train moves currently forwards
int  currentSupply = 0;          // Current supply level

// The following handles the 4 buttons of the Esplora

const byte buttonIndex[] = {2, 1, 0, 3}; 

const char *buttonLabels[]   = {"Map", "Refill", "Switch", "Reverser"};

const int buttonHEIGHT = 25;
const int buttonX = 32;
const int boxOFFSET = 26;
const int textLOWER = 7;

#define WHITE        EsploraTFT.stroke(255, 255, 255)
#define LIGHT_GRAY   EsploraTFT.stroke(180, 180, 180)
#define GRAY         EsploraTFT.stroke(127, 127, 127)
#define BLACK        EsploraTFT.stroke(0, 0, 0)
#define RED          EsploraTFT.stroke(255, 0, 0)
#define DARK_RED     EsploraTFT.stroke(210, 0, 0)
#define GREEN        EsploraTFT.stroke(0, 255, 0)
#define DARK_GREEN   EsploraTFT.stroke(0, 160, 0)
#define BLUE         EsploraTFT.stroke(0, 0, 200)
#define BRONZE       EsploraTFT.stroke(255, 200, 0)
#define DARK_BRONZE  EsploraTFT.stroke(215, 160, 0)

void displayButton(int buttonNumber, boolean isDown)     // Display a button
{
  int textY, left, width, top, height;
  top = boxOFFSET;
  height = buttonHEIGHT - 4;
  left = buttonX,
  width = ((EsploraTFT.width() - left) / 2) - 5;
  switch(buttonNumber)
  {
    case 0:
      {
        left += width / 2 + 2;
        if (isDown)
          WHITE;
        else
          LIGHT_GRAY;
        break;
      }
    case 1:
      {
        top += buttonHEIGHT;
        if (isDown)
          WHITE;
        else
          LIGHT_GRAY;
        break;
      }
    case 2:
      {
        left += width + 4;
        top += buttonHEIGHT;
        if (isDown)
          WHITE;
        else
          LIGHT_GRAY;           // set the color to light gray
        break;
      }
    case 3:
      {
        left += width / 2 + 2;
        top += 2 * buttonHEIGHT;
        if (forwards)
        {
          if (isDown)
            RED;
          else
            DARK_GREEN;
        }
        else
        {
          if (isDown)
            GREEN; 
          else
            DARK_RED;
        }
        break;
      }
    }

  EsploraTFT.text(buttonLabels[buttonNumber], left + 2 + ((9 - strlen(buttonLabels[buttonNumber])) * 6) / 2, top + textLOWER);

  if (isDown)
    WHITE;
  else
    LIGHT_GRAY;           
  EsploraTFT.rect(left, top, width, height);
  if (isDown)
    LIGHT_GRAY;
  else
    GRAY; 
  EsploraTFT.line(left + 1, top - 1, left + width - 1, top - 1);
  EsploraTFT.line(left + 1, top + height, left + width - 1, top + height);
  EsploraTFT.line(left - 1, top + 1, left - 1, top + height - 1);
  EsploraTFT.line(left + width, top + 1, left + width, top + height - 1);
}

void setupButtons()
{
  for (int i = 0; i < 4; i++)
    displayButton(i, false);
}


// The following code handles the throttle 

const int maxTHROTTLE = 20;      // Maximum throttle value in trainz (DCC mode)
const int throttleHEIGHT = 20;   // The height of the throttle display
int currentThrottle = 0;         // The current throttle value in trainz

void setupThrottle()
{
  // Display the label
  BRONZE;
  EsploraTFT.rect(1, EsploraTFT.height() - throttleHEIGHT - 1, EsploraTFT.width() - 2, throttleHEIGHT);
  DARK_BRONZE;
  EsploraTFT.line(2, EsploraTFT.height() - 1, EsploraTFT.width() - 2, EsploraTFT.height() - 1);
  EsploraTFT.line(2, EsploraTFT.height() - throttleHEIGHT - 2, EsploraTFT.width() - 2, EsploraTFT.height() - throttleHEIGHT - 2);
  EsploraTFT.line(0, EsploraTFT.height() - 2, 0, EsploraTFT.height() - throttleHEIGHT);
  EsploraTFT.line(EsploraTFT.width() - 1, EsploraTFT.height() - 2, EsploraTFT.width() - 1, EsploraTFT.height() - throttleHEIGHT);

  GRAY;               // set the color to gray
  for (int i = 2; i < EsploraTFT.width() - 2; i++)
    EsploraTFT.line(i, EsploraTFT.height() - 2, i, EsploraTFT.height() - throttleHEIGHT);
  BLACK;
  for (int tick =0; tick <= 20; tick++)
  {
     int x = ((76 * tick) / 10) + 4;
     if ((tick % 5) == 0)
     {
       EsploraTFT.line(x, EsploraTFT.height() - 2, x, EsploraTFT.height() - 12);
     }
     else
     {
       EsploraTFT.line(x, EsploraTFT.height() - 2, x, EsploraTFT.height() - 8);
     }
  }
  setThrottle(0,'i');               // i stands for initialize
}

void setThrottle(int newValue, char sendKey)
{
  int x;
  
  x = ((76 * currentThrottle) / 10) + 3;      // x position of the pointer
  GRAY;                                       // erase the last pointer
  EsploraTFT.line(x, EsploraTFT.height() - 12, x, EsploraTFT.height() - throttleHEIGHT + 1);
  EsploraTFT.line(x - 1, EsploraTFT.height() - 2, x - 1, EsploraTFT.height() - throttleHEIGHT + 1);
  EsploraTFT.line(x + 1, EsploraTFT.height() - 2, x + 1, EsploraTFT.height() - throttleHEIGHT + 1);
  BLACK;
  if ((currentThrottle % 5) == 0)
  {
     EsploraTFT.line(x, EsploraTFT.height() - 2, x, EsploraTFT.height() - 12);
  }
  else
  {
    EsploraTFT.line(x, EsploraTFT.height() - 2, x, EsploraTFT.height() - 8);
    GRAY;
    EsploraTFT.line(x, EsploraTFT.height() - 8, x, EsploraTFT.height() - 12); 
  }

  currentThrottle = newValue;
  x = ((76 * currentThrottle) / 10) + 3;   // x position of the pointer
  RED;
  EsploraTFT.line(x, EsploraTFT.height() - 2, x, EsploraTFT.height() - throttleHEIGHT + 1);
  BLACK;              // black borders
  EsploraTFT.line(x - 1, EsploraTFT.height() - 8, x - 1, EsploraTFT.height() - throttleHEIGHT + 1);
  EsploraTFT.line(x + 1, EsploraTFT.height() - 8, x + 1, EsploraTFT.height() - throttleHEIGHT + 1);
  if (sendKey!='i')                       // i stands for initialize
  {
    Keyboard.press(sendKey);              // set the throttle
    buzzer();
    delay(40);
    Keyboard.release(sendKey);
    delay(50);
  }
}

void checkThrottle(boolean reverserChange)
{
  char up, down;
  if (reverserChange)
  {
    setThrottle(0,'s');
    forwards = !forwards;
  }
  if (forwards)
  {
    up = 'w';
    down = 'x';
  }
  else
  {
    up = 'x';
    down = 'w';
  }
  int slider = 20 - (Esplora.readSlider() * maxTHROTTLE / 1000);
  if (currentSupply < 1)      // supplies are used up
  {
    slider = 0;
  }
  if (slider != currentThrottle)
  {
    if (slider == 0)     // stop the engine
    {
       setThrottle(0,'s');
    }
    else
    {
      while (currentThrottle > slider)
      {
        setThrottle(currentThrottle-1,down);
      }
      while (currentThrottle < slider)
      {
        setThrottle(currentThrottle+1,up);
      }
    }
  }    
}

// The following code handles the supply

const int supplyWIDTH = 20;             // width of the supply display
const int supplyHEIGHT = 85;           // height of the supply display

const int maxSUPPLY = supplyHEIGHT - 8;
long lastTime = 0;

void setupSupply()
{
  GRAY;     // set the color to gray
  EsploraTFT.rect(4, 10, supplyWIDTH, supplyHEIGHT);
  DARK_BRONZE;               // set the color to dark bronze
  
  EsploraTFT.line(3, 9, supplyWIDTH + 5, 9);
  EsploraTFT.line(3, 8, supplyWIDTH + 5, 8);  
  EsploraTFT.line(3, 7, supplyWIDTH + 5, 7);
  EsploraTFT.line((supplyWIDTH / 2) + 3,  7, (supplyWIDTH / 2) + 3, 3);
  EsploraTFT.line((supplyWIDTH / 2) + 4,  7, (supplyWIDTH / 2) + 4, 3);
  
  EsploraTFT.line(3, supplyHEIGHT + 10, supplyWIDTH + 5, supplyHEIGHT + 10);
  EsploraTFT.line(3, supplyHEIGHT + 11, supplyWIDTH + 5, supplyHEIGHT + 11);  
  EsploraTFT.line(3, supplyHEIGHT + 12, supplyWIDTH + 5, supplyHEIGHT + 12);
  EsploraTFT.line((supplyWIDTH / 2) + 3,  supplyHEIGHT + 13, (supplyWIDTH / 2) + 3, supplyHEIGHT + 17);
  EsploraTFT.line((supplyWIDTH / 2) + 4,  supplyHEIGHT + 13, (supplyWIDTH / 2) + 4, supplyHEIGHT + 17);

  setSupply(maxSUPPLY);
}

void setSupply(int newSupply)
{
  if (newSupply > currentSupply)
  {
     BLUE;
     for (int i = currentSupply; i <newSupply; i++)
       EsploraTFT.line(5,  supplyHEIGHT + 8 - i, supplyWIDTH + 3,  supplyHEIGHT + 8 - i);
  }
  else if (newSupply < currentSupply)
  {
     BLACK;
     for (int i = newSupply + 1; i <= currentSupply; i++)
       EsploraTFT.line(5,  supplyHEIGHT + 8 - i, supplyWIDTH + 3,  supplyHEIGHT + 8 - i);
  }
  currentSupply = newSupply;
}

void reduceSupply()
{
  long currentTime = millis();
  long timeLasting = currentTime - lastTime;                    // time in msec since last change
  long period = (100000 * (long)maxTHROTTLE) / ((long)currentThrottle + 2);  // 100 seconds at max throttle
  if ((timeLasting > (long)period) && (currentSupply > 0))
  {
    setSupply(currentSupply - 1);
    lastTime = currentTime;
  }
  if (!joystickState1)
    Esplora.writeRGB(0, 0, 31);     // blue
  else
  {
    if (currentSupply > (maxSUPPLY / 10))
      Esplora.writeRGB(0, 3, 0);      // dark green
    else
      Esplora.writeRGB(255, 0, 0);     // red
  }
}

// The buzzer is used to indicate that a key has been pressed, or the slider setting has been changed

void buzzer()
{
  Esplora.tone(500);
  delay(10);
  Esplora.noTone();
}

// The following code is run only at startup, to initialize the virtual USB keyboard.

void setup()
{
  Keyboard.begin();
  EsploraTFT.begin();                 // initialize the display
  EsploraTFT.background(0, 0, 0);     // clear the background to dark gray
  LIGHT_GRAY;
  EsploraTFT.text("Trainz Controller", buttonX + 10, 10);
  setupThrottle();
  setupSupply();
  setupButtons();
}

/*
  After setup() is finished, this code is called continuously.
  Here we continuously check if something happened with the buttons.
*/

void loop()
{  
  // Check all buttons
  for (byte thisButton=0; thisButton<8; thisButton++) {
    boolean lastState = buttonStates[thisButton];
    boolean newState = Esplora.readButton(buttons[thisButton]);
    if (lastState != newState)
    {
      if (newState == PRESSED)    // a putton has been pressed
      {
        buzzer();
        switch (buttons[thisButton])
        {
          case SWITCH_UP:
            displayButton(buttonIndex[thisButton - 4], true);
            break;
          case SWITCH_DOWN:
            displayButton(buttonIndex[thisButton - 4], true);
            checkThrottle(true);                 // Reverser has changed
            break;
          case SWITCH_LEFT:
            displayButton(buttonIndex[thisButton - 4], true);
            setSupply(maxSUPPLY);
            break;
          case SWITCH_RIGHT:
            displayButton(buttonIndex[thisButton - 4], true);
            if (!forwards)
            {
               Keyboard.press(KEY_LEFT_CTRL);       // Ctlr key must be turned on
               delay(5);
            }
            break;
        }
        if (keystrokes[thisButton] != 0) {
          if (keystrokes[thisButton]== 'c')
          {
            Keyboard.press(KEY_LEFT_CTRL);
            delay(5);
          }
          Keyboard.press(keystrokes[thisButton]);
        }
      }
      else if (newState == RELEASED)      // a button has been released
      {
        if (keystrokes[thisButton] != 0) {
          Keyboard.release(keystrokes[thisButton]);
          if (keystrokes[thisButton]== 'c')
          {
            delay(5);
            Keyboard.release(KEY_LEFT_CTRL);
          }
        }
        switch (buttons[thisButton])
        {
         case SWITCH_UP:
            displayButton(buttonIndex[thisButton - 4], false);
            break;
          case SWITCH_DOWN:
            displayButton(buttonIndex[thisButton - 4], false);
            break;
          case SWITCH_LEFT:
            displayButton(buttonIndex[thisButton - 4], false);
            break;
          case SWITCH_RIGHT:
            displayButton(buttonIndex[thisButton - 4], false);
            if (!forwards)
            {
              delay(5);
              Keyboard.release(KEY_LEFT_CTRL);       // Ctlr key must be turned off
            }
            break;
        }

      }
    }
    // Store the new button state, so you can sense a difference later:
    buttonStates[thisButton] = newState;

    // The joystick button must be handled differently

    int lastJoystickbutton = joystickButton;
    int newJoystickbutton = Esplora.readJoystickButton();
    if (lastJoystickbutton != newJoystickbutton)
    {
      if (newJoystickbutton == HIGH)
      {
        if (joystickState1) // toggle action of joystick
        {
          keystrokes[0] = KEY_DOWN_ARROW;
          keystrokes[1] = 'C';
          keystrokes[2] = KEY_UP_ARROW;
          keystrokes[3] = 'c';
        }
        else
        {
          keystrokes[0] = KEY_PAGE_DOWN;
          keystrokes[1] = KEY_LEFT_ARROW;
          keystrokes[2] = KEY_PAGE_UP;
          keystrokes[3] = KEY_RIGHT_ARROW;
        }
        joystickState1 = !joystickState1;
      }
      else // handle joystick actions which were activated accidentally
      {
        Keyboard.release(KEY_PAGE_DOWN);
        Keyboard.release('C');
        Keyboard.release(KEY_PAGE_UP);
        Keyboard.release(KEY_LEFT_SHIFT);
        Keyboard.release(KEY_LEFT_CTRL);
        Keyboard.release(KEY_DOWN_ARROW);
        Keyboard.release(KEY_UP_ARROW);
        Keyboard.release(KEY_LEFT_ARROW);
        Keyboard.release(KEY_RIGHT_ARROW);
      }     
    }    
    joystickButton = newJoystickbutton;        
  }
  
  // check the Slider
  
  checkThrottle(false);
  
  // check and adjust the supply level
  
  reduceSupply();
  
  /*
    Wait a little bit (50ms) between a check and another. When a mechanical switch is pressed
    or released, the contacts may bounce very rapidly. If the check is done too fast, these bounces
    may be confused as multiple presses and may lead to unexpected behaviour.
   */

  delay(50);
}

