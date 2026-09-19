# FREMO Universal Board

This program will control a FREMO universal board.

The board is connected to the Loconet and can handle up to 16 I/O pins.<br>
Each pin can be configured:<br>
* to be an input or an output
* to act as sensor or switch on Loconet side
* to get an individual Loconet address
* that logical ON is low (0 V) or high (5 V)
* ...

---

### Configuration
To configure the board, use the configuration tool: <b>Uni-I-O-Configurator.html</b><br>
(can be used with any browser)<br>
You can read an existing configuration file, modify it and save it again.

---

### Building the program with Arduino IDE
To simplify the build process add an entry to the menu <i>Tools / board -> Arduino AVR boards</i><br>
The entry will be <b>FREMO Boards</b> and will come with at least one option in<br>
the menu <i>Tools / Processor</i>, like: <b>Universal I/O (1512, ATmega32U4  16 MHz)</b>

To get this you will need to modify/create the file: <b><i>boards.local.txt</i></b><br>
If you have a portable setup of the IDE you will find the file here:<br>
...\portable\packages\arduino\hardware\avr\1.8.6\boards.local.txt<br>
Otherwise you will find it here:<br>
...\hardware\arduino\avr\boards.local.txt

If you don't find the file, create a new one.

Add the content of the file <b><i>boards-local-add-on.txt</i></b> to the file above.<br>
