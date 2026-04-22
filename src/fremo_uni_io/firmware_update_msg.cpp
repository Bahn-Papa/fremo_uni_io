//##########################################################################
//#
//#		FirmwareUpdateMessage.cpp
//#
//#	Description
//#
//#-------------------------------------------------------------------------
//#
//#	File Version:	1		from: 18.03.2026
//#
//#	Implementation:
//#		-	first working version
//#
//##########################################################################


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

#include "FirmwareUpdateMessage.h"


//==========================================================================
//
//		C L A S S   F U N C T I O N S
//
//==========================================================================


////////////////////////////////////////////////////////////////////////
//	CLASS: IO_ControlClass
//

//******************************************************************
//	computeBytesFromPxct
//------------------------------------------------------------------
//
void FirmwareUpdateMessage::computeBytesFromPxct( byte data[] )
{
	uint8_t pxct1 = data[ 5 ];

	for( uint8_t index = 0 ; index < 4 ; index++ )
	{
		if( bitRead( pxct1, index ) == 1 )
		{
			bitSet( data[ index + 6 ], 7 );
		}
	}


	uint8_t pxct2 = data[ 10 ];

	for( uint8_t index = 0 ; index < 4 ; index++ )
	{
		if( bitRead( pxct2, index ) == 1 )
		{
			bitSet( data[ index + 11 ], 7 );
		}
	}
}


//******************************************************************
//	enterBootloader
//------------------------------------------------------------------
//
void FirmwareUpdateMessage::enterBootloader()
{
	cli();
	MCUSR = 0;

	JUMP_TO_ABSOLUTE( BOOTSTRAP_START_BYTE / 2 );
}


//******************************************************************
//	processMessage
//------------------------------------------------------------------
//
uint8_t FirmwareUpdateMessage::processMessage( lnMsg * lnPacket )
{
	if(		lnPacket->px.command	== OPC_PEER_XFER
		&&	lnPacket->px.mesg_size	== 16
		&&	lnPacket->px.src		== SRC_FIRMWARE
		&&	lnPacket->px.dst_h		== SRC_FIRMWARE
		&&	lnPacket->px.dst_l		== SRC_FIRMWARE		)
	{
		computeBytesFromPxct(lnPacket->data);

		if( 	(lnPacket->px.pxct1 & 0xF0)	== PXCT1_DOWNLOAD
			&&	(lnPacket->px.pxct2 & 0xF0)	== PXCT2_SETUP
			&&	lnPacket->px.d1				== MANUFACTURER_ID
			&&	lnPacket->px.d2				== (byte) (PRODUCT_ID & 0xFF)
			&&	lnPacket->px.d3				== HARDWARE_VERSION
			&&	lnPacket->px.d7				== DEVELOPER_ID
			&&	lnPacket->px.d8				== ((PRODUCT_ID >> 8) & 0xFF)	)
		{
			if(notifyFirmwareUpdate )
			{
				notifyFirmwareUpdate();
			}
		}

		return( 1 );
	}
	else
	{
		return( 0 );
	}
}
