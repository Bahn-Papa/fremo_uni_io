//##########################################################################
//#
//#		MyLoconetClass
//#
//#	Diese Klasse behandelt alle Loconet-Nachrichten und was damit
//#	zusammen hängt.
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	3		vom: 06.11.2022
//#
//#	Implementation:
//#		-	add one address for multiple I/Os
//#			up to now there was only one address for one I/O possible
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	2		vom: 15.02.2022
//#
//#	Implementation:
//#		-	remove the check for 'send only input messages' in function
//#			'SendMessage()', because this check is performed elsewhere
//#
//#	Bugfix:
//#		-	handled messages for inputs instead for outputs in function
//#			'LoconetReceived()'
//#			this bug is fixed now
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	1		vom: 14.02.2022
//#
//#	Implementation:
//#		-	first version
//#
//##########################################################################


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

#include "compile_options.h"

#include <Arduino.h>
#include <LocoNet.h>


#ifdef DEBUGGING_PRINTOUT
#include "debugging.h"
#endif

#include "lncv_storage.h"
#include "my_loconet.h"


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define LOCONET_TX_PIN			7


//==========================================================================
//
//		G L O B A L   V A R I A B L E S
//
//==========================================================================

MyLoconetClass	 g_clMyLoconet		= MyLoconetClass();

LocoNetCVClass	 g_clLNCV;
lnMsg			*g_pLnPacket;


//==========================================================================
//
//		C L A S S   F U N C T I O N S
//
//==========================================================================


////////////////////////////////////////////////////////////////////////
//	CLASS: MyLoconetClass
//

//******************************************************************
//	Constructor
//------------------------------------------------------------------
//
MyLoconetClass::MyLoconetClass()
{
	m_uiInputStatus	= 0x0000;
	m_bIsProgMode	= false;
}


//******************************************************************
//	Init
//------------------------------------------------------------------
//
void MyLoconetClass::Init( void )
{
	LocoNet.init( LOCONET_TX_PIN );
}


//******************************************************************
//	CheckForAndHandleMessage
//------------------------------------------------------------------
//
void MyLoconetClass::CheckForMessage( void )
{
	g_pLnPacket = LocoNet.receive();

	if( g_pLnPacket )
	{
		if( !LocoNet.processSwitchSensorMessage( g_pLnPacket ) )
		{
			g_clLNCV.processLNCVMessage( g_pLnPacket );
		}
	}
}


//******************************************************************
//	LoconetReceived
//------------------------------------------------------------------
//	This function checks if the received message is for 'us'.
//	This is done by checking whether the address of the message
//	matches one of the stored addresses.
//	If so, the corresponding bit of the 'InputState' will be set
//	according to the info in the message.
//
void MyLoconetClass::LoconetReceived(	bool isSensor,
										uint16_t adr,
										uint8_t dir,
										uint8_t			)
{
	uint16_t	asOutputs	= g_clLncvStorage.GetAsOutputs();
	uint16_t	asSensor	= g_clLncvStorage.GetAsSensor();
	uint16_t	isInverse	= g_clLncvStorage.GetIsInverse();
	uint16_t	ioAddress	= 0;
	uint16_t	mask		= 0x0001;

	for( uint8_t idx = 0 ; idx < IO_NUMBERS ; idx++ )
	{
		//----------------------------------------------------------
		//	first check if the pin 'idx' is an output
		//	if so, process the message
		//
		if( asOutputs & mask )
		{
			//------------------------------------------------------
			//	second check if we are searching for an address in
			//	a sensor message or in a switch message.
			//
			//	isSensor == false	we are looking for switch messages
			//	isSensor == true	we are looking for sensor messages
			//
			//	'asSensor' will hold the info if the message at the
			//	actual bit position (mask) is expected to be
			//	a sensor message or a switch message.
			//	(bit set => sensor message)
			//
			if( isSensor == (0 != (asSensor & mask)) )
			{
				ioAddress = g_clLncvStorage.GetIOAddress( idx );

				if(	(0 < ioAddress) && (adr == ioAddress) )
				{
					//----------------------------------------------
					//	This is one of our addresses, ergo go on
					//	with the processing
					//

#ifdef DEBUGGING_PRINTOUT
					g_clDebugging.PrintNotifyMsg( adr, dir );
#endif

					//----------------------------------------------
					//	Check if 'dir' should be inverted
					//
					if( isInverse & mask )
					{
						if( 0 == dir )
						{
							dir = 1;
						}
						else
						{
							dir = 0;
						}
					}

					//----------------------------------------------
					//	store direction 'dir' in the input status
					//
					if(	dir )
					{
						m_uiInputStatus |= mask;
					}
					else
					{
						m_uiInputStatus &= ~mask;
					}
				}

			}	//	if( isSensor == (0 != (asSensor & mask)) )

		}	//	if( mask & asInputs )

		mask <<= 1;
	}
}


//**********************************************************************
//	SendMessage
//----------------------------------------------------------------------
//
void MyLoconetClass::SendMessage( uint16_t adr, uint16_t mask, uint8_t dir )
{
	//--------------------------------------------------------------
	//	send the message only if there is an address for it
	//
	if( 0 < adr )
	{
		//----------------------------------------------------------
		//	Check if 'dir' should be inverted
		//
		if( g_clLncvStorage.GetIsInverse() & mask )
		{
			if( 0 < dir )
			{
				dir = 0;
			}
			else
			{
				dir = 1;
			}
		}

		//----------------------------------------------------------
		//	Check if this should be a sensor or
		//	a switch message
		//
		if( g_clLncvStorage.GetAsSensor() & mask )
		{
			//----	sensor message  --------------------------------
			//
			LocoNet.reportSensor( adr, dir );

#ifdef DEBUGGING_PRINTOUT
//			g_clDebugging.PrintReportSensorMsg( adr, dir );
#endif
		}
		else
		{
			//----	switch message  --------------------------------
			//
			LocoNet.requestSwitch( adr, 1, dir );

#ifdef DEBUGGING_PRINTOUT
//			g_clDebugging.PrintReportSwitchMsg( adr, dir );
#endif

			//----	wait befor sending the next message  -----------
			//
			delay( g_clLncvStorage.GetSendDelayTime() );

			LocoNet.requestSwitch( adr, 0, dir );
		}

		//----	wait befor sending the next message  ---------------
		//
		delay( g_clLncvStorage.GetSendDelayTime() );
	}
}


//==========================================================================
//
//		L O C O N E T   C A L L B A C K   F U N C T I O N S
//
//==========================================================================


//**********************************************************************
//
void notifySensor( uint16_t Address, uint8_t State )
{
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintNotifyType( NT_Sensor );
#endif

	g_clMyLoconet.LoconetReceived( true, Address, State, 0 );
}


//**********************************************************************
//
void notifySwitchRequest( uint16_t Address, uint8_t Output, uint8_t Direction )
{
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintNotifyType( NT_Request );
#endif

	g_clMyLoconet.LoconetReceived( false, Address, Direction, Output );
}


//**********************************************************************
//
void notifySwitchReport( uint16_t Address, uint8_t Output, uint8_t Direction )
{
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintNotifyType( NT_Report );
#endif

	g_clMyLoconet.LoconetReceived( false, Address, Direction, Output );
}


//**********************************************************************
//
void notifySwitchState( uint16_t Address, uint8_t Output, uint8_t Direction )
{
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintNotifyType( NT_State );
#endif

	g_clMyLoconet.LoconetReceived( false, Address, Direction, Output );
}


//**********************************************************************
//	notifyLNCVdiscover
//----------------------------------------------------------------------
//	we received a broadcast message, so give back article number
//	and module address.
//
int8_t notifyLNCVdiscover( uint16_t &ArtNr, uint16_t &ModuleAddress )
{
	ArtNr			= g_clLncvStorage.GetArticleNumber();
	ModuleAddress	= g_clLncvStorage.GetModuleAddress();

	g_clMyLoconet.SetProgMode( true );

#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintLncvDiscoverStart( false, ArtNr, ModuleAddress  );
#endif

	return( LNCV_LACK_OK );
}


//**********************************************************************
//	notifyLNCVprogrammingStart
//----------------------------------------------------------------------
//
int8_t notifyLNCVprogrammingStart( uint16_t &ArtNr, uint16_t &ModuleAddress )
{
	int8_t retval = -1;		//	default: ignore request
	
	if( g_clLncvStorage.GetArticleNumber() == ArtNr )
	{
		if( 0xFFFF == ModuleAddress )
		{
			//----	broadcast, so give Module Address back  --------
			g_clMyLoconet.SetProgMode( true );

			ModuleAddress	= g_clLncvStorage.GetModuleAddress();
			retval			= LNCV_LACK_OK;
		}
		else if( g_clLncvStorage.GetModuleAddress() == ModuleAddress )
		{
			//----  that's for me, so process it  ------------------
			g_clMyLoconet.SetProgMode( true );

			retval	= LNCV_LACK_OK;
		}
	}

#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintLncvDiscoverStart( true, ArtNr, ModuleAddress  );
#endif

	return( retval );
}


//**********************************************************************
//	notifyLNCVprogrammingStop
//----------------------------------------------------------------------
//
void notifyLNCVprogrammingStop( uint16_t ArtNr, uint16_t ModuleAddress )
{
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintLncvStop();
#endif

	if( g_clMyLoconet.IsProgMode() )
	{
		if( 	(g_clLncvStorage.GetArticleNumber() == ArtNr)
			&&	(g_clLncvStorage.GetModuleAddress() == ModuleAddress) )
		{
			//----	for me, so switch prog mode off  ---------------
			g_clMyLoconet.SetProgMode( false );
		}
	}
}


//**********************************************************************
//	notifyLNCVread
//----------------------------------------------------------------------
//
int8_t notifyLNCVread( uint16_t ArtNr, uint16_t Address, uint16_t, uint16_t &Value )
{
	int8_t retval = -1;		//	default: ignore request

	if( g_clMyLoconet.IsProgMode() && (g_clLncvStorage.GetArticleNumber() == ArtNr) )
	{
		if( g_clLncvStorage.IsValidLNCVAddress( Address ) )
		{
			Value	= g_clLncvStorage.ReadLNCV( Address );
			retval	= LNCV_LACK_OK;
		}
		else
		{
			retval = LNCV_LACK_ERROR_UNSUPPORTED;
		}
	}

#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintLncvReadWrite( true, Address, Value );
#endif

	return( retval );
}


//**********************************************************************
//	notifyLNCVwrite
//----------------------------------------------------------------------
//
int8_t notifyLNCVwrite( uint16_t ArtNr, uint16_t Address, uint16_t Value )
{
	int8_t retval = -1;		//	default: ignore request

	if( g_clMyLoconet.IsProgMode() && (g_clLncvStorage.GetArticleNumber() == ArtNr) )
	{
		if( g_clLncvStorage.IsValidLNCVAddress( Address ) )
		{
			if(		(LNCV_ADR_VERSION_NUMBER != Address)
				&&	(LNCV_ADR_ARTIKEL_NUMMER != Address) )
			{
				g_clLncvStorage.WriteLNCV( Address, Value );
			}

			retval = LNCV_LACK_OK;
		}
		else
		{
			retval = LNCV_LACK_ERROR_UNSUPPORTED;
		}
	}

#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintLncvReadWrite( false, Address, Value );
#endif

	return( retval );
}
