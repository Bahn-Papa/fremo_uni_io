# FREMO Universal Board

This program will control a FREMO universal board.

The board is connected to the Loconet and can handle up to 16 I/O pins.<br>
Each pin can be configured:<br>
* to be an input or an output
* to act as sensor or switch on Loconet side
* to get an individual Loconet address
* that logical ON is low (0 V) or high (5 V)
* ...

## Configuration
To configure the board, use the configuration tool: <b>Uni-I-O-Configurator.html</b><br>
(can be used with any browser)<br>
You can read an existing configuration file, modify it and save it again.

## Building the program with Arduino IDE
First select _**FREMO Boards**_ from the menu **_Tools_ -> _board_ -> _Arduino AVR boards_**<br>
Then select the application **Universal I/O (1512, ATmega32U4 16 MHz)** from the menu **_Tools_ -> _Application (Article No)_**<br>
Now you can compile the application: select **_Sketch_ -> _Export compiled Binary_**<br>
This will compile the application and create two **.hex** files. One with and one without bootloader. The file without bootloader can be used to update the software over loconet.<br>
But you can still upload the application using a programmer.

**NOTE:** If you don't find the menu entries see below how to create them.

### Setup FREMO specific menu entries in Arduino IDE
To simplify the build process add FREMO specific menu entries to the _**Tools**_ menu.<br>
There are two scenarios:

- No **FREMO Boards** menu entry in **_Tools_ -> _board_ -> _Arduino AVR boards_**<br>
Just copy the file **boards.local.txt** from this repositories **ide** folder to the correct folder.<br>
Which folder is the correct one depends on the IDE installation.<br>
If you have a portable setup of the IDE you need to copy the file here:<br>
**...\portable\packages\arduino\hardware\avr\\\<avr version><br>**
Otherwise you need to copy it here:<br>
**...\hardware\arduino\avr**<br>
To verify if you found the correct folder: there should be the file **boards.txt** in there.

- No **Universal I/O (1512, ATmega32U4 16 MHz)** menu entry in **_Tools_ -> _Application (Article No)_**<br>
In this case you have to add the content of the file **boards-local-add-on.txt** from this repositories **ide** folder to the file **boards.local.txt**.<br>
Where to find the file **boards.local.txt** see above.

### Upload the application over Loconet
If you want to upload the application over Loconet you need the upload tool and the bootloader found in this repositories **ide** folder. Copy the bootloader into the **fremo** folder inside the **bootloaders** folder. Then use the Arduino IDE to flash the bootloader.
