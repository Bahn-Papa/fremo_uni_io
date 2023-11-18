//##########################################################################
//#
//#		MyLoconetClass
//#
//#	Diese Klasse behandelt alle Loconet-Nachrichten und was damit
//#	zusammen hängt.
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	7		vom: 17.11.2023
//#
//#	Implementation:
//#		-	avoid missunderstanding, so rename
//#				GetInputStatus()	=>	GetOutputStatus()
//#				m_uiInputStatus		=>	m_uiOutputStatus
//#		-	add message to send status of all inputs
//#			new variable
//#				m_uiAdrSendStatus
//#				m_bSendStatus
//#			change in function
//#				Init()
//#				LoconetReceived()
//#				CheckForMessage()
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	6		vom: 03.11.2023
//#
//#	Implementation:
//#		-	Article number and module address are global variables now
//#			new variables
//#				g_uiArticleNumber
//#				g_uiModuleAddress
//#			changes in function
//#				Init()
//#				notifyLNCV...
//#		-	change in message handling
//#			change in function
//#				LoconetReceived()
//#				SendMessage()
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	5		from: 04.06.2023
//#
//#	Bug Fix:
//#		-	do not go into prog mode when a discover msg was detected
//#			change in function
//#				notifyLNCVdiscover()
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	4		vom: 04.02.2023
//#
//#	Implementation:
//#		-	change debug text and info for switch/sensor messages
//#			change in function
//#				LoconetReceived()
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

//----------------------------------------------------------------------
//	configuration masks
//
#define SWITCH_RED				0
#define SWITCH_THROWN			SWITCH_RED
#define SWITCH_ABZWEIGEN		SWITCH_RED
#define SWITCH_GREEN			1
#define SWITCH_CLOSED			SWITCH_GREEN
#define SWITCH_GERADE			SWITCH_GREEN


//==========================================================================
//
//		G L O B A L   V A R I A B L E S
//
//==========================================================================

MyLoconetClass	 g_clMyLoconet		= MyLoconetClass();

LocoNetCVClass	 g_clLNCV;
lnMsg			*g_pLnPacket;

uint16_t		 g_uiArticleNumber;
uint16_t		 g_uiModuleAddress;


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
	m_uiOutputStatus	= 0x0000;
	m_uiAdrSendStatus	= 0x0000;
	m_bIsProgMode		= false;
	m_bIsProgMode		= false;
}


//******************************************************************
//	Init
//------------------------------------------------------------------
//
void MyLoconetClass::Init( void )
{
	g_uiArticleNumber	= g_clLncvStorage.ReadLNCV( LNCV_ADR_ARTIKEL_NUMMER );
	g_uiModuleAddress	= g_clLncvStorage.ReadLNCV( LNCV_ADR_MODULE_ADDRESS );

	m_uiAdrSendStatus	= g_clLncvStorage.ReadLNCV( LNCV_ADR_SEND_STATUS );

	LocoNet.init( LOCONET_TX_PIN );
}


//******************************************************************
//	CheckForAndHandleMessage
//------------------------------------------------------------------
//
bool MyLoconetClass::CheckForMessage( void )
{
	m_bSendStatus	= false;

	g_pLnPacket		= LocoNet.receive();

	if( g_pLnPacket )
	{
		if( !LocoNet.processSwitchSensorMessage( g_pLnPacket ) )
		{
			g_clLNCV.processLNCVMessage( g_pLnPacket );
		}
	}

	return( m_bSendStatus );
}


//******************************************************************
//	LoconetReceived
//------------------------------------------------------------------
//	This function checks if the received message is for 'us'.
//	This is done by checking whether the address of the message
//	matches one of the stored addresses.
//	If so, the corresponding bit of the 'OutputState' will be set
//	according to the info in the message.
//
void MyLoconetClass::LoconetReceived(	notify_type_t	type,
										uint16_t		uiAdr,
										uint8_t			usDirClosed,
										uint8_t			usOutputThrown )
{
	uint16_t	asOutputs	= g_clLncvStorage.GetAsOutputs();
	uint16_t	asSensor	= g_clLncvStorage.GetAsSensor();
	uint16_t	isInverse	= g_clLncvStorage.GetIsInverse();
	uint16_t	asReport	= g_clLncvStorage.GetAsReport();
	uint16_t	ioAddress	= 0;
	uint16_t	mask		= 0x0001;
	uint8_t		usInfo		= 0;
	bool		bFound;

	//--------------------------------------------------------------
	//	check if the status of all inputs should be send
	//
	if( uiAdr == m_uiAdrSendStatus )
	{
		m_bSendStatus	= true;

		return;
	}

	//--------------------------------------------------------------
	//	
	for( uint8_t idx = 0 ; idx < IO_NUMBERS ; idx++ )
	{
		bFound = false;

		//----------------------------------------------------------
		//	first check if the pin 'idx' is an output
		//	if so, process the message
		//
		if( asOutputs & mask )
		{
			//------------------------------------------------------
			//	second check if there is an address for this pin
			//	and if so is the address for the pin the one that
			//	we are searching for
			//
			ioAddress = g_clLncvStorage.GetIOAddress( idx );

			if(	(0 < ioAddress) && (uiAdr == ioAddress) )
			{
				//--------------------------------------------------
				//	third handle the different types of messages,
				//	but only if the pin configuration and
				//	the notify type are matching
				//
				if( asSensor & mask )
				{
					//------------------------------------------
					//	pin is configured as sensor
					//
					if( NT_Sensor == type )
					{
						bFound	= true;
						usInfo	= usDirClosed;
					}
				}
				else if( asReport & mask )
				{
					//------------------------------------------
					//	pin is configured as switch report
					//
					if( NT_Report == type )
					{
						bFound	= true;

						if( 0 == ((idx + 1) % 2) )
						{
							usInfo	= usDirClosed;
						}
						else
						{
							usInfo	= usOutputThrown;
						}
					}
				}
				else
				{
					//------------------------------------------
					//	pin is configured as switch request
					//
					if( NT_Request == type )
					{
						bFound	= true;
						usInfo	= usDirClosed;
					}
				}

				if( bFound )
				{
					if( isInverse & mask )
					{
						if( 0 == usInfo )
						{
							usInfo = 1;
						}
						else
						{
							usInfo = 0;
						}
					}

					if(	usInfo )
					{
						m_uiOutputStatus |= mask;
					}
					else
					{
						m_uiOutputStatus &= ~mask;
					}

#ifdef DEBUGGING_PRINTOUT
					g_clDebugging.PrintNotifyMsg( idx, usDirClosed, usOutputThrown );
#endif
				}

			}	//	if( (0 < ioAddress) && (uiAdr == ioAddress) )

		}	//	if( asOutputs & mask )

		mask <<= 1;
	}
}


//**********************************************************************
//	SendMessage
//----------------------------------------------------------------------
//
void MyLoconetClass::SendMessage( notify_type_t type, uint16_t uiAdr, uint8_t usDirClosed, uint8_t usOutputThrown )
{
	//--------------------------------------------------------------
	//	send the message only if there is an address for it
	//
	if( 0 < uiAdr )
	{
		if( NT_Sensor == type )
		{
			//----	sensor message  --------------------------------
			//
			LocoNet.reportSensor( uiAdr, usDirClosed );

#ifdef DEBUGGING_PRINTOUT
//			g_clDebugging.PrintReportSensorMsg( uiAdr, usDirClosed );
#endif
		}
		else if( NT_Request == type )
		{
			//----	switch request message  ------------------------
			//
			LocoNet.requestSwitch( uiAdr, 1, usDirClosed );

#ifdef DEBUGGING_PRINTOUT
//			g_clDebugging.PrintReportSwitchMsg( uiAdr, usDirClosed );
#endif

			//----	wait befor sending the next message  -----------
			//
			delay( g_clLncvStorage.GetSendDelayTime() );

			LocoNet.requestSwitch( uiAdr, 0, usDirClosed );
		}
		else if( NT_Report == type )
		{
			uint8_t AddrH = (--uiAdr >> 7) & 0x0F;
			uint8_t AddrL = uiAdr & 0x7F;

			if( usOutputThrown )
			{
				AddrH |= OPC_SW_REP_THROWN;
			}

			if( usDirClosed )
			{
				AddrH |= OPC_SW_REP_CLOSED;
			}

			LocoNet.send( OPC_SW_REP, AddrL, AddrH);
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

	g_clMyLoconet.LoconetReceived( NT_Sensor, Address, State, 0 );
}


//**********************************************************************
//
void notifySwitchRequest( uint16_t Address, uint8_t usClosed, uint8_t usThrown )
{
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintNotifyType( NT_Request );
#endif

	g_clMyLoconet.LoconetReceived( NT_Request, Address, usThrown, usClosed );
}


//**********************************************************************
//
void notifySwitchOutputsReport( uint16_t Address, uint8_t Output, uint8_t Direction )
{
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintNotifyType( NT_Report );
#endif

	g_clMyLoconet.LoconetReceived( NT_Report, Address, Direction, Output );
}


//**********************************************************************
//
/*
void notifySwitchState( uint16_t Address, uint8_t Output, uint8_t Direction )
{
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintNotifyType( NT_State );
#endif

	g_clMyLoconet.LoconetReceived( false, Address, Direction, Output );
}
*/


//**********************************************************************
//	notifyLNCVdiscover
//----------------------------------------------------------------------
//	we received a broadcast message, so give back article number
//	and module address.
//
int8_t notifyLNCVdiscover( uint16_t &ArtNr, uint16_t &ModuleAddress )
{
	ArtNr			 = g_uiArticleNumber;
	ModuleAddress	 = g_uiModuleAddress;

//	g_clMyLoconet.SetProgMode( true );

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
	
	if( g_uiArticleNumber == ArtNr )
	{
		if( 0xFFFF == ModuleAddress )
		{
			//-----------------------------------------------------
			//	valid article number, but broadcast address,
			//	so only give back Module Address
			//
//			g_clMyLoconet.SetProgMode( true );

			ModuleAddress	= g_uiModuleAddress;
			retval			= LNCV_LACK_OK;

#ifdef DEBUGGING_PRINTOUT
			g_clDebugging.PrintText( "Broadcast" );
#endif

		}
		else if( g_uiModuleAddress == ModuleAddress )
		{
			//----  that's for me, so process it  ------------------
			g_clMyLoconet.SetProgMode( true );

			retval	= LNCV_LACK_OK;

#ifdef DEBUGGING_PRINTOUT
			g_clDebugging.PrintText( "in Prog mode" );
#endif

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
		if( 	(g_uiArticleNumber == ArtNr)
			&&	(g_uiModuleAddress == ModuleAddress) )
		{
			//------------------------------------------------------
			//	valid article number and valid module address,
			//	so switch off programming mode
			//
			g_clMyLoconet.SetProgMode( false );
		}
	}
}


//**********************************************************************
//	notifyLNCVread
//----------------------------------------------------------------------
//
int8_t notifyLNCVread( uint16_t ArtNr, uint16_t Address, uint16_t &Value )
{
	int8_t retval = -1;		//	default: ignore request

	if( g_clMyLoconet.IsProgMode() && (g_uiArticleNumber == ArtNr) )
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
	g_clDebugging.PrintText( "Notify READ" );
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

	if( g_clMyLoconet.IsProgMode() && (g_uiArticleNumber == ArtNr) )
	{
		if( g_clLncvStorage.IsValidLNCVAddress( Address ) )
		{
			if(		(LNCV_ADR_VERSION_NUMBER != Address)
				&&	(LNCV_ADR_ARTIKEL_NUMMER != Address) )
			{
				g_clLncvStorage.WriteLNCV( Address, Value );

				if( LNCV_ADR_MODULE_ADDRESS == Address )
				{
					g_uiModuleAddress = Value;
				}
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
	g_clDebugging.PrintText( "Notify WRITE" );
#endif

	return( retval );
}
