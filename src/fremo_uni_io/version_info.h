
#pragma once

//##########################################################################
//#
//#		version_info.h
//#
//#	This file contains the project/change history and
//#	some definitions used for compile options:
//#
//#		-	ARTICLE_NUMBER
//#			defines for which board version the software will be compiled.
//#			(differences in the I/O assignment)
//#			normally the definition will come from the IDE
//#			(selecting the right board)
//#			article number		processor				board
//#				1				Atmega 32U4, 16 MHz		Leonardo
//#				1512			ATmega 32U4, 16 MHz		Leonardo
//#
//#		-	VERSION_MAIN
//#			the main software version
//#			will change when new development or bigger changes were made
//#
//#		-	VERSION_MINOR
//#			the sub software version
//#			will change by bug fixes and smaller changes in the software
//#
//#		-	VERSION_NUMBER
//#			is calculated from VERSION_MAIN and VERSION_MINOR and
//#			stored in LNCV #3 as a decimal number (to make it readable)
//#
//#		-	DEBUGGING_PRINTOUT
//#			If defined, debug info and/or text is shown on an OLED display
//#
//#		-	OLED_CHIP_TYPE_SH1106
//#			If defined, an OLED display with chip type SH1106 is connected.
//#			Otherwise the chip type is SSD1306
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	3		vom: 22.04.2026
//#
//#	Implementation:
//#		-	change the handling of different hardware (platine) versions
//#			before it was a fixed definition (PLATINE_VERSION)
//#			now it is a definition set by the IDE during compile time
//#			(ARTICLE_NUMBER)
//#		-	change the format of the software version number
//#			removed the hardware part form the version number, so now
//#			the format is VVVmm with
//#				VVV = VERSION_MAIN and
//#				mm  = VERSION_MINOR
//#		-	renamed file compile_options.h to version_info.h
//#		-	moved the project/change history from fremo_uni_io.ino
//#			to version_info.h
//#		-	moved version definitions from fremo_uni_io.ino
//#			to version_info.h
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	2		vom: 31.10.2023
//#
//#	Implementation:
//#		-	new definition to specify the chip type of the connected
//#			OLED display
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	1		vom: 27.01.2023
//#
//#	Implementation:
//#		-	initial version
//#
//##########################################################################


//==========================================================================
//
//		C O D E   S E L E C T I O N   D E F I N I T I O N S
//
//==========================================================================

//----------------------------------------------------------------------
//	my artikle number
//
#ifndef ARTICLE_NUMBER
	#define ARTICLE_NUMBER		1512
#endif

//----------------------------------------------------------------------
//	version number
//
#define VERSION_MAIN			10
#define	VERSION_MINOR			0

#define VERSION_NUMBER			((VERSION_MAIN * 100) + VERSION_MINOR)

//----------------------------------------------------------------------
//	other definitions
//
#define DEBUGGING_PRINTOUT

#define OLED_CHIP_TYPE_SH1106


//##########################################################################
//#
//#		Version History:
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	10.00		from: 22.04.2026
//#
//#	Implementation:
//#		-	change handling of version number and article number
//#			change in files
//#				fremo_uni_io.ino
//#				lncv_storage.cpp
//#				io_control.cpp, io_control.h
//#				debugging.cpp, debugging.h
//#		-	add handling for updates over loconet
//#			new files
//#				firmware_update_msg.cpp, firmware_update_msg.h
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.09.06		from: 05.11.2025
//#
//#	Bug Fix:
//#		-	write the default config if article number is not identical
//#			change in file
//#				lncv_storage.cpp
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.09.05		from: 27.10.2025
//#
//#	Bug Fix:
//#		-	detection of disable message improved
//#			changes in file
//#				my_loconet.cpp
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.09.04		from: 24.10.2025
//#
//#	Bug Fix:
//#		-	add setting of the initial states for the outputs
//#		-	correction of LNCV_ADR_LAST_TOGGLE_ADDRESS
//#		-	correction of toggle output handling
//#			changes in files
//#				my_loconet.h, my_loconet.cpp
//#				lncv_storage.h
//#			changes in function
//#				setup()
//#				CheckToSendIOState()
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.09.03		from: 23.10.2025
//#
//#	Bug Fix:
//#		-	correction of one button function configuration
//#			changes in file
//#				my_loconet.cpp
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.09.02		from: 22.10.2025
//#
//#	Bug Fix:
//#		-	forgot to initialize the index in a for loop
//#			changes in file
//#				my_loconet.cpp
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.09.01		from: 20.10.2025
//#
//#	Implementation:
//#		-	add check of configuration
//#			changes in file
//#				my_loconet.cpp
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.09.00		from: 17.10.2025
//#
//#	Implementation:
//#		-	add a 'one button toggle' functionallity
//#			changes in files
//#				lncv_storage.h, lncv_storage.cpp
//#				my_loconet.h, my_loconet.cpp
//#				io_control.h, io_control.cpp
//#			change in functions
//#				setup()
//#				loop()
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.08.00		from: 07.10.2025
//#
//#	Implementation:
//#		-	change the handling of the signal way from input to
//#			loconet message
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.07.02		from: 07.02.2024
//#
//#	Bug Fix:
//#		-	changed position of paramters for NT_Report messages
//#			change in function
//#				CheckToSendIOState()
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.07.01		from: 07.02.2024
//#
//#	Bug Fix:
//#		-	new evaluation if a loconet message should be send
//#			parameters for messages of type NT_Report were evaluated the
//#			wrong way
//#			change in function
//#				CheckToSendIOState()
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.07.00		from: 18.11.2023
//#
//#	Implementation:
//#		-	avoid missunderstanding, so rename
//#			clMyLoconet.GetInputStatus()	=>	clMyLoconet.GetOutputStatus()
//#		-	add address to send the status of all inputs
//#			change in function
//#				loop()
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.06.02		from: 03.11.2023
//#
//#	Bug Fix:
//#		-	change in handling of ModuleAddress and Article Number
//#			in module my_loconet
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.06.01		from: 03.11.2023
//#
//#	Implementation:
//#		-	changes in handling of msg types
//#
//#-------------------------------------------------------------------------
//#
//#	Version:	x.05.01		from: 04.06.2023
//#
//#	Bug Fix:
//#		-	do not go into prog mode when a discover msg was detected
//#
//#-------------------------------------------------------------------------
//#
//#	Version: x.05.00	vom: 05.02.2023
//#
//#	Implementation:
//#		-	add support for board version 4
//#
//#-------------------------------------------------------------------------
//#
//#	Version: x.04.00	vom: 04.02.2023
//#
//#	Implementation:
//#		-	change debug text and info for switch/sensor messages
//#
//#-------------------------------------------------------------------------
//#
//#	Version: x.03.05	vom: 29.01.2023
//#
//#	Bug Fix:
//#		-	now correct interpreting of the configuration
//#
//#-------------------------------------------------------------------------
//#
//#	Version: x.03.04	vom: 29.01.2023
//#
//#	Bug Fix:
//#		-	change in interpreting the configuration
//#
//#-------------------------------------------------------------------------
//#
//#	Version: x.03.03	vom: 29.01.2023
//#
//#	Implementation:
//#		-	add function to print the configuration
//#
//#-------------------------------------------------------------------------
//#
//#	Version: x.03.02	vom: 27.01.2023
//#
//#	Implementation:
//#		-	add version number to EEPROM
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 1.03.01	vom: 09.11.2022
//#
//#	Bug Fix:
//#		-	in function 'WriteLNCV()' Address was not declared
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 1.03.00	vom: 06.11.2022
//#
//#	Implementation:
//#		-	add one address for multiple I/Os
//#			up to now there was only one address for one I/O possible
//#		-	only write to EEPROM if the new value is different
//#			than the old one
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 1.02.00	vom: 19.07.2022
//#
//#	Implementation:
//#		-	new configuration method:
//#			address, input, output, switch, sensor, green and red
//#         is configured in one word.
//#         The word has the following format:
//#			xxxx m	-	xxxx	address
//#						m		mode
//#								0	-	output switch msg RED   (0) active
//#								1	-	output switch msg GREEN (1) active
//#								2	-	output sensor LOW  (0) active
//#								3	-	output sensor HIGH (1) active
//#								4	-	input  switch msg RED   (0) active
//#								5	-	input  switch msg GREEN (1) active
//#								6	-	input  sensor LOW  (0) active
//#								7	-	input  sensor HIGH (1) active
//#         output means: lissening on Loconet and set IO pins
//#         input  means: check state of IO pins and send loconet msg
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 1.01.00	vom: 18.02.2022
//#
//#	Implementation:
//#		-	add IO pin off delay timer (0 ms up to 65535 ms)
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 1.00.00	vom: 18.02.2022
//#
//#	Implementation:
//#		-	add some comments an explanations
//#		-	all test were good, so set version to 1.0.0
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 0.09.02	vom: 16.02.2022
//#
//#	Bugfix:
//#		-	mismatch of input and output state corrected
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 0.09.01	vom: 15.02.2022
//#
//#	Implementation:
//#		-	remove the check for 'send only input messages' in function
//#			'SendMessage()', because this check is performed elsewhere
//#		-	add function GetAsInputs()
//#			the function will return a bit mask where each '1' bit
//#			stands for an input
//#
//#	Bugfix:
//#		-	correction of initial state in function 'setup()'
//#		-	handled messages for inputs instead for outputs in function
//#			'LoconetReceived()'. This bug is fixed now
//#
//#-------------------------------------------------------------------------
//#
//#	Version: 0.09.00	vom: 14.02.2022
//#
//#	Implementation:
//#		-	first working version
//#
//##########################################################################
