  Esplora Trainz Controler
  
  The sketch has been written for an ARDUINO ESPLORA with LCD shield.
  
  This sketch turns the Esplora into a game pad for Trainz (DCC control only).

![Alt text](Controller.jpg?raw=true "Arduino Esplora")
  
  The buttons of the Esplora handle the following actions:

    Button:              Action:
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
  from Appstore) and a Mac with TANE using a Swiss German keyboard.

  The following two actions might not work on other keyboards without
  adding the following

    alternative key controls:
    move backwards in consist         shift-c
    move forwards in consist          ctrl-c

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