
#pragma once

//##########################################################################
//#
//#		compile_options.h
//#
//#	This file contains compile options:
//#		-	DEBUGGING_PRINTOUT
//#			If defined, debug info and/or text is shown on an OLED display
//#
//#		-	OLED_CHIP_TYPE_SH1106
//#			If defined, an OLED display with chip type SH1106 is connected.
//#			Otherwise the chip type is SSD1306
//#
//#		-	PLATINE_VERSION
//#			defines for which board version the software will be compiled.
//#			(differences in the I/O assignment)
//#
//#-------------------------------------------------------------------------
//#
//#		Platine Version 1:	ATmega 32U4, 16 MHz (z.B.: Leonardo)
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

#define DEBUGGING_PRINTOUT

#define OLED_CHIP_TYPE_SH1106

#define PLATINE_VERSION			4
