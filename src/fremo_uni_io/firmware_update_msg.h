//##########################################################################
//#
//#		FirmwareUpdateMessage.h
//#
//#	Description
//#
//#-------------------------------------------------------------------------
//#
//#	File Version:	2		from: 01.04.2026
//#
//#	Implementation:
//#		-	add header file 'version_info.h' to get the the ARCTICLE_NUMBER
//#			to set the PRODUCT_ID
//#
//#-------------------------------------------------------------------------
//#
//#	File Version:	1		from: 18.03.2026
//#
//#	Implementation:
//#		-	first working version
//#
//##########################################################################


#ifndef FirmwareUpdateMessage_h
#define FirmwareUpdateMessage_h


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

#include <Arduino.h>
#include <LocoNet.h>

#include "version_info.h"


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define BOOTSTRAP_START_BYTE 0x7C00

#define JUMP_TO_ABSOLUTE(word_addr) __asm__ __volatile__ ("ijmp"::"z"(word_addr))


//==========================================================================
//
//		C L A S S   D E F I N I T I O N S
//
//==========================================================================


////////////////////////////////////////////////////////////////////////////
//	CLASS: FirmwareUpdateMessage
//
class FirmwareUpdateMessage
{
	public:
		static void enterBootloader();
		uint8_t processMessage( lnMsg * );

	private:
		static const uint8_t SRC_FIRMWARE		= 0x7F;
		static const uint8_t PXCT1_DOWNLOAD		= 0x40;
		static const uint8_t PXCT2_SETUP		= 0x00;

		static const uint8_t MANUFACTURER_ID	= 13;
		static const uint8_t DEVELOPER_ID		= 1;
		static const uint16_t PRODUCT_ID		= ARTICLE_NUMBER;
		static const uint8_t HARDWARE_VERSION	= 4;

		void computeBytesFromPxct( byte[] );
};


//==========================================================================
//
//		G L O B A L   F U N C T I O N S
//
//==========================================================================

extern void notifyFirmwareUpdate() __attribute__((weak));


//==========================================================================

#endif
