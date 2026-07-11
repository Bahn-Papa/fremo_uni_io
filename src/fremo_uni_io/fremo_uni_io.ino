//##########################################################################
//#
//#	fremo_uni_io		copyright: Michael Pfeil
//#
//#	This program controls the hardware for universal boards
//#
//#-------------------------------------------------------------------------
//#
//# board:		Leonardo
//#	processor:	ATmega32U4, 16 MHz
//#
//#-------------------------------------------------------------------------
//#
//#	Needed resources:
//#		LocoNet Library		V 1.1.13
//#		SimpleOled Library	V 1.2.1
//#
//##########################################################################


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

#include "version_info.h"
#include "io_control.h"
#include "lncv_storage.h"
#include "my_loconet.h"

#ifdef DEBUGGING_PRINTOUT
#include "debugging.h"
#endif


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define READ_INPUTS_TIME		20
#define PRINT_STATUS_TIME		250


//==========================================================================
//
//		G L O B A L   V A R I A B L E S
//
//==========================================================================

uint32_t	g_ulReadInputTimer					= 0L;
uint32_t	g_ulPrintStatusTimer				= 0L;
uint32_t	g_arulOffDelayTimer[ IO_NUMBERS ];
uint16_t	g_uiLnStateReceived;
uint16_t	g_uiLnStateSend;
uint16_t	g_uiIOState;
bool		g_bIsProgMode;


//==========================================================================
//
//		G L O B A L   F U N C T I O N S
//
//==========================================================================

//**********************************************************************
//	resetFunc
//----------------------------------------------------------------------
//	This function will reset the hole processor because by calling the
//	function the program will jump to the hardware reset vector.
//
void (*resetFunc)( void ) = 0;


//**************************************************************************
//	CheckLnStateAndSetOutputs
//--------------------------------------------------------------------------
//	The function will check the changes in the Loconet state and 
//	will switch the output(s) accordingly
//
void CheckLnStateAndSetOutputs( uint16_t uiNewLnState )
{
	//------------------------------------------------------------------
	//	get difference between old and actual state ...
	//
	uint16_t	uiDiff		= g_uiLnStateReceived ^ uiNewLnState;
	uint16_t	uiMask		= 0x0001;
	uint8_t		usDir		= 0;
	uint8_t		idx			= 0;

	//------------------------------------------------------------------
	//	... but handle outputs only
	//
	uiDiff &= g_clLncvStorage.GetAsOutputs();

	//------------------------------------------------------------------
	//	now for each change set/clear the appropriate IO pin
	//
	while( 0 < uiDiff )
	{
		if( uiDiff & uiMask )
		{
			if( uiNewLnState & uiMask )
			{
				usDir = 1;
			}
			else
			{
				usDir = 0;
			}

			g_clControl.SetOutput( idx, usDir );

			//----------------------------------------------------------
			//	this change was handled,
			//	so clear the corresponding bit
			//
			uiDiff &= ~uiMask;
		}

		idx++;
		uiMask <<= 1;
	}

	g_uiLnStateReceived = uiNewLnState;
}


//**************************************************************************
//	GetIOState
//--------------------------------------------------------------------------
//	The function collects the actual IO state and returns it.
//
uint16_t GetIOState( void )
{
	uint16_t	uiInputs	= g_clLncvStorage.GetAsInputs();
	uint16_t	uiIOState	= 0x0000;
	uint16_t	uiMask		= 0x0001;

	//------------------------------------------------------------------
	//	get IO states
	//
	for( uint8_t idx = 0 ; idx < IO_NUMBERS ; idx++ )
	{
		if( uiInputs & uiMask )
		{
			if( g_clControl.IsInputSet( idx ) )
			{
				uiIOState |= uiMask;
			}
		}

		uiMask <<= 1;
	}
	
	return( uiIOState );
}


//**************************************************************************
//	CheckIOState
//--------------------------------------------------------------------------
//	The function will check the changes in the IO state and 
//	will set up the info which Loconet messages to send
//
uint16_t CheckIOState( uint16_t uiNewIOState )
{
	uint16_t	uiNewLnStateSend	= g_uiLnStateSend;
	uint32_t	ulOffTimer			= 0L;
	uint16_t	uiOffDelay			= 0;

	//------------------------------------------------------------------
	//	get difference between old and actual state ...
	//
	uint16_t	uiDiff	= g_uiIOState ^ uiNewIOState;
	uint16_t	uiMask	= 0x0001;
	uint8_t		idx		= 0;

	//------------------------------------------------------------------
	//	... but handle inputs only
	//
	uiDiff &= g_clLncvStorage.GetAsInputs();

	//------------------------------------------------------------------
	//	now for each change send the appropriate Loconet message
	//
	while( 0 < uiDiff )
	{
		if( uiDiff & uiMask )
		{
			if( uiNewIOState & uiMask )
			{
				//----------------------------------------------
				//	the new state of the pin is ON
				//
				if( g_arulOffDelayTimer[ idx ] )
				{
					//------------------------------------------
					//	if the off delay timer is active
					//	then stop timer and stay in 'ON' state
					//	there is no need to send a msg
					//
					g_arulOffDelayTimer[ idx ] = 0L;
				}
				else
				{
					//------------------------------------------
					//	else update the new lN send state
					//
					uiNewLnStateSend |= uiMask;
				}
			}
			else
			{
				//--------------------------------------------------
				//	the IO pin has changed to OFF, so if there is a
				//	delay time configured then start the delay timer
				//
				uiOffDelay = g_clLncvStorage.GetIOOffDelay( idx );
				
				if( uiOffDelay )
				{
					g_arulOffDelayTimer[ idx ] = millis() + uiOffDelay;
				}
				else
				{
					//------------------------------------------
					//	else update the new lN send state
					//
					uiNewLnStateSend &= ~uiMask;
				}
			}

			//------------------------------------------------------
			//	this change was handled,
			//	so clear the corresponding bit
			//
			uiDiff &= ~uiMask;
		}

		idx++;
		uiMask <<= 1;
	}

	g_uiIOState = uiNewIOState;
	
	//------------------------------------------------------------------
	//	now check if any delay timer is lapsed and if so stop the timer
	//	and update the new LN send state
	//
	uiMask = 0x0001;

	for( idx = 0 ; idx < IO_NUMBERS ; idx++ )
	{
		ulOffTimer = g_arulOffDelayTimer[ idx ];

		if( ulOffTimer && (millis() > ulOffTimer) )
		{
			g_arulOffDelayTimer[ idx ] = 0L;

			uiNewLnStateSend &= ~uiMask;
		}

		uiMask <<= 1;
	}

	return( uiNewLnStateSend );
}


//**************************************************************************
//	CheckToSendIOState
//--------------------------------------------------------------------------
//	The function will check the changes in the IO state and 
//	will send the appropriate Loconet messages accordingly
//
void CheckToSendIOState( uint16_t uiNewIOState )
{
	notify_type_t	type;
	uint16_t		asInputs		= g_clLncvStorage.GetAsInputs();
	uint16_t		asSensor		= g_clLncvStorage.GetAsSensor();
	uint16_t		asReport		= g_clLncvStorage.GetAsReport();
	uint16_t		isLowActive		= g_clLncvStorage.GetIsLowActive();
	uint16_t		uiSingleMessage	= g_clLncvStorage.GetSingleMessage();
	uint16_t		uiSendRed		= g_clLncvStorage.GetSendRed();
	uint16_t		uiAddress		= 0;
	uint8_t			usInfo;
	uint8_t			usOutputThrown	= 0;
	bool			bDoSendLnMsg;

	//------------------------------------------------------------------
	//	get difference between old and actual state ...
	//
	uint16_t		uiDiff			= g_uiLnStateSend ^ uiNewIOState;
	uint16_t		uiMask			= 0x0001;
	uint8_t			idx				= 0;

	//------------------------------------------------------------------
	//	... but handle inputs only
	//
	uiDiff &= asInputs;

	//------------------------------------------------------------------
	//	now for each change send the appropriate Loconet message
	//
	while( 0 < uiDiff )
	{
		bDoSendLnMsg = false;	//	by default don't send a loconet message

		//----------------------------------------------------------
		//	first check if the pin 'idx' is an input
		//	if so, process the pin
		//
		if( asInputs & uiMask )
		{
			//------------------------------------------------------
			//	second check if there is an address for this pin
			//	and if so go on with processing
			//
			uiAddress = g_clLncvStorage.GetIOAddress( idx );
			
			if( 0 < uiAddress )
			{
				//--------------------------------------------------
				//	prepare the bit info
				//
				if( asReport & uiMask )
				{
					if( uiNewIOState & uiMask )
					{
						usInfo = 1;
					}
					else
					{
						usInfo = 0;
					}
				}
				else
				{
					if( uiNewIOState & uiMask )
					{
						if( uiSendRed & uiMask )
						{
							usInfo = 0;
						}
						else
						{
							usInfo = 1;
						}
					}
					else if( uiSendRed & uiMask )
					{
						usInfo = 1;
					}
					else
					{
						usInfo = 0;
					}
				}

				//--------------------------------------------------
				//	find out and remember if a loconet message
				//	must be send
				//
				if( uiDiff & uiMask )
				{
					bDoSendLnMsg = true;

					if( 0 != usInfo )
					{
						g_clMyLoconet.CheckAndHandleToggleFunc( idx );
					}
				}

				//--------------------------------------------------
				//	now dected which kind of message will be send
				//
				if( asSensor & uiMask )
				{
					//------------------------------------------
					//	pin is configured as sensor
					//
					type = NT_Sensor;
				}
				else if( asReport & uiMask )
				{
					//------------------------------------------
					//	pin is configured as switch report
					//	we need the info of a second pin
					//	so remember the actual pin info ...
					//
					type			= NT_Report;
					usOutputThrown	= usInfo;

					//------------------------------------------
					//	... then remove the actual pin from
					//	the diff list, ...
					//
					uiDiff &= ~uiMask;

					//------------------------------------------
					//	... go to the next pin, ...
					//
					idx++;
					uiMask <<= 1;
					
					//------------------------------------------
					//	... get the bit info, ...
					//
					if( uiNewIOState & uiMask )
					{
						usInfo = 1;
					}
					else
					{
						usInfo = 0;
					}

					//------------------------------------------
					//	... and find out if a loconet message
					//	must be send
					//
					if( uiDiff & uiMask )
					{
						bDoSendLnMsg = true;
					}
				}
				else
				{
					//------------------------------------------
					//	pin is configured as switch request
					//
					type = NT_Request;
				}

				//----------------------------------------------
				//	check if this message is allowed to be send
				//
				if( uiSingleMessage & uiMask )
				{
					if( 0 == (uiNewIOState & uiMask) )
					{
						bDoSendLnMsg = false;
					}
				}

				//----------------------------------------------
				//	if allowed, send message
				//
				if( bDoSendLnMsg )
				{
					//------------------------------------------
					//	send the loconet message
					//
					if( NT_Report == type )
					{
						g_clMyLoconet.SendMessage(	type,
													uiAddress,
													usOutputThrown,
													usInfo			);
					}
					else
					{
						g_clMyLoconet.SendMessage(	type,
													uiAddress,
													usInfo,
													usOutputThrown );
					}
				}
			}
		}

		//----------------------------------------------------------
		//	remove the pin from the diff list
		//
		uiDiff &= ~uiMask;

		//----------------------------------------------------------
		//	prepare to check the next pin
		//
		idx++;
		uiMask <<= 1;
	}

	g_uiLnStateSend = uiNewIOState;
}


//**************************************************************************
//	setup
//--------------------------------------------------------------------------
//
void setup()
{
	uint16_t	uiAsOutput;
	uint16_t	uiIsLowActive;
	uint16_t	uiLnStateStart;


	g_bIsProgMode = false;

#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.Init();

	g_clDebugging.PrintTitle( ARTICLE_NUMBER, VERSION_MAIN, VERSION_MINOR );
	g_clDebugging.PrintInfoLine( infoLineInit );
#endif

	//----	LNCV: Check and Init  --------------------------------------
	g_clLncvStorage.CheckEEPROM( VERSION_NUMBER );
	
	delay( 500 );

	g_clLncvStorage.Init();

	delay( 500 );

	uiAsOutput		= g_clLncvStorage.GetAsOutputs();
	uiIsLowActive	= g_clLncvStorage.GetIsLowActive();

	//----	other inits  -----------------------------------------------
//	g_clControl.Init( uiAsOutput, uiIsLowActive );
	g_clControl.Init( uiAsOutput, 0x0000 );
	g_clMyLoconet.Init();

	for( uint8_t idx = 0 ; idx < IO_NUMBERS ; idx++ )
	{
		g_arulOffDelayTimer[ idx ] = 0L;
	}

	delay( 100 );

	//----	LED check  -------------------------------------------------
	g_clControl.RedLedOn();
	
	delay( 500 );
	
	g_clControl.RedLedOff();
	g_clControl.GreenLedOn();
	
	delay( 500 );
	
	g_clControl.GreenLedOff();
	g_clControl.RedLedOn();
	
	delay( 500 );
	
	g_clControl.GreenLedOn();
	
	delay( 500 );
	
	g_clControl.RedLedOff();
	
	delay( 500 );
	
	g_clControl.GreenLedOff();


	//----	Show Configuration  ----------------------------------------
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintTitle( ARTICLE_NUMBER, VERSION_MAIN, VERSION_MINOR );
	g_clDebugging.PrintStorageConfig(	g_clLncvStorage.GetAsOutputs(),
										g_clLncvStorage.GetAsSensor(),
										g_clLncvStorage.GetIsLowActive()	);

	delay( 2000 );
#endif

	//----	Prepare Display  -------------------------------------------
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintTitle( ARTICLE_NUMBER, VERSION_MAIN, VERSION_MINOR );
	g_clDebugging.PrintInfoLine( infoLineFields );
#endif

	//------------------------------------------------------------------
	//	get the actual input and output state and set the I/O pins
	//	respective send the appropriate LN messages
	//	the trick here is to set the old state values as inverted
	//	actual states to get all I/Os set and LN messages send
	//
	g_uiIOState		 = GetIOState();	//	actual state
	g_uiLnStateSend	 = ~g_uiIOState;	//	trick to send all messages
	g_uiLnStateSend	&= ~uiAsOutput;		//	but only for inputs

	CheckToSendIOState( g_uiIOState );	//	send messages
	
	uiLnStateStart			 = g_clMyLoconet.GetOutputStatus();	//	actual state
	g_uiLnStateReceived		 = ~uiLnStateStart;	//	trick to set all pins
	g_uiLnStateReceived		&= uiAsOutput;		//	but only for outputs

	CheckLnStateAndSetOutputs( uiLnStateStart );

	//----	Start Read Timer  ------------------------------------------
	g_ulReadInputTimer = millis() + READ_INPUTS_TIME;
}


//**************************************************************************
//	loop
//--------------------------------------------------------------------------
//
void loop()
{
	//==================================================================
	//	Read Inputs
	//	-	Loconet messages
	//	-	Input signals
	//
	if( g_clMyLoconet.CheckForMessage() )
	{
		uint16_t	uiInputs = g_uiLnStateSend;

		g_uiLnStateSend	= ~uiInputs;

		CheckToSendIOState( uiInputs );
	}

	if( millis() > g_ulReadInputTimer )
	{
		g_ulReadInputTimer = millis() + READ_INPUTS_TIME;

		g_clControl.ReadInputs();
	}

	//==================================================================
	//	depending of input pins and received LN messages
	//	set output pins and send LN messages
	//
	CheckLnStateAndSetOutputs( g_clMyLoconet.GetOutputStatus() );
	CheckToSendIOState( CheckIOState( GetIOState() ) );

	//------------------------------------------------------------------
	//	Programmier-Modus
	//
	if( g_clMyLoconet.IsProgMode() != g_bIsProgMode )
	{
		if( g_bIsProgMode )
		{
			resetFunc();
		}
		else
		{
			g_bIsProgMode = true;

			g_clControl.GreenLedFlash();
		}
	}


	//==================================================================
	//	print actual status
	//
#ifdef DEBUGGING_PRINTOUT
	if( millis() > g_ulPrintStatusTimer )
	{
		g_ulPrintStatusTimer = millis() + PRINT_STATUS_TIME;

		g_clDebugging.PrintStatus(	g_clLncvStorage.GetAsOutputs(),
									g_uiLnStateReceived, g_uiIOState		);
	}
#endif
}
