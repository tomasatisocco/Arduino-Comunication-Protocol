# Arduino Comunication Protocol

The idea is to send and recive commands between the Arduino micro and the program MyBoxBall in the computer following the protocol as shown in the pdf.

##Control

The objetive is to control the initial variables of the ball in the program the variables for control are:
* Velovity
* Angle
* Position in X axis
* Position in Y axis

This is done with 4 buttons in the Arduino wich make:
* Btn1: Change the actual variable
* Btn2: Add 1 to the actual variables
* Btn3: Subtract 1 to the actual variable
* Btn4: Shot or Stop the ball
----
## Rebounds

The rebounds of the ball in the walls of the box are shown with 4 leds conected to the arduino. <br />

The secuence of rebound of the last shot is saved and you can play it by pressing the Btn1 for 1500 milliseconds or you can save it if you want in the eeprom memory pressing the Btn1 until the four leds turn on.
