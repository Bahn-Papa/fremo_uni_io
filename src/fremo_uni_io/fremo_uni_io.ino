//##########################################################################
//#
//#	fremo_uni_io		copyright: Michael Pfeil
//#
//#	This program controls the hardware for universal boards
//#
//##########################################################################


#define VERSION_MAIN	1
#define	VERSION_MINOR	1
#define VERSION_HOTFIX	0


//##########################################################################
//#
//#		Version History:
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


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

#include "compile_options.h"

#ifdef DEBUGGING_PRINTOUT
#include "debugging.h"
#endif

#include "io_control.h"
#include "lncv_storage.h"
#include "my_loconet.h"


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
uint16_t	g_uiLnState;
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
//	CheckLnState
//--------------------------------------------------------------------------
//	The function will check the changes in the Loconet state and 
//	will switch the output(s) accordingly
//
void CheckLnState( uint16_t uiNewLnState )
{
	//------------------------------------------------------------------
	//	get difference between old and actual state ...
	//
	uint16_t	uiDiff		= g_uiLnState ^ uiNewLnState;
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

	g_uiLnState = uiNewLnState;
}


//**************************************************************************
//	GetIOState
//--------------------------------------------------------------------------
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
//	will send the appropriate Loconet messages accordingly
//
void CheckIOState( uint16_t uiNewIOState )
{
	uint32_t	ulOffTimer	= 0L;
	uint16_t	uiOffDelay	= 0;

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
				//------------------------------------------------------
				//	if the off delay timer is active stop timer
				//	and stay in 'ON' state
				//	else send the Loconet message for IO pin is ON
				//
				if( g_arulOffDelayTimer[ idx ] )
				{
					g_arulOffDelayTimer[ idx ] = 0L;
				}
				else
				{
					g_clMyLoconet.SendMessage( g_clLncvStorage.GetIOAddress( idx ),
												uiMask, 1							);
				}
			}
			else
			{
				//------------------------------------------------------
				//	the IO pin has changed to OFF, so if there is a
				//	delay time configured start the delay timer
				//	else send the loconet message for IO pin is OFF
				//
				uiOffDelay = g_clLncvStorage.GetIOOffDelay( idx );
				
				if( uiOffDelay )
				{
					g_arulOffDelayTimer[ idx ] = millis() + uiOffDelay;
				}
				else
				{
					g_clMyLoconet.SendMessage( g_clLncvStorage.GetIOAddress( idx ),
												uiMask, 0							);
				}
			}

			//----------------------------------------------------------
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
	//	and send the loconet message for IO pin OFF for that pin
	//
	uiMask = 0x0001;

	for( idx = 0 ; idx < IO_NUMBERS ; idx++ )
	{
		ulOffTimer = g_arulOffDelayTimer[ idx ];

		if( ulOffTimer && (millis() > ulOffTimer) )
		{
			g_arulOffDelayTimer[ idx ] = 0L;

			g_clMyLoconet.SendMessage( g_clLncvStorage.GetIOAddress( idx ),	uiMask, 0 );
		}

		uiMask <<= 1;
	}
}


//**************************************************************************
//	setup
//--------------------------------------------------------------------------
//
void setup()
{
	uint16_t	uiAsOutput;
	uint16_t	uiIOStateStart;
	uint16_t	uiLnStateStart;


	g_bIsProgMode = false;

#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.Init();

	g_clDebugging.PrintTitle( VERSION_MAIN, VERSION_MINOR, VERSION_HOTFIX );
	g_clDebugging.PrintInfoLine( infoLineInit );
#endif

	//----	LNCV: Check and Init  --------------------------------------
	g_clLncvStorage.CheckEEPROM();
	
	delay( 500 );

	g_clLncvStorage.Init();

	delay( 500 );

	uiAsOutput = g_clLncvStorage.GetAsOutputs();

	//----	other inits  -----------------------------------------------
	g_clControl.Init( uiAsOutput );
	g_clMyLoconet.Init();

	for( uint8_t idx = 0 ; idx < IO_NUMBERS ; idx++ )
	{
		g_arulOffDelayTimer[ idx ] = 0L;
	}

	delay( 100 );

	//----	Prepare Display  -------------------------------------------
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintTitle( VERSION_MAIN, VERSION_MINOR, VERSION_HOTFIX );
	g_clDebugging.PrintInfoLine( infoLineFields );
#endif

	//------------------------------------------------------------------
	//	get the actual input and output state and set the I/O pins
	//	respective send the appropriate LN messages
	//	the trick here is to set the old state values as inverted
	//	actual states to get all I/Os set and LN messages send
	//
	uiIOStateStart	 = GetIOState();	//	actual state
	g_uiIOState		 = ~uiIOStateStart;	//	trick to send all messages
	g_uiIOState		&= ~uiAsOutput;		//	but only for inputs

	CheckIOState( uiIOStateStart );		//	send messages
	
	uiLnStateStart	 = g_clMyLoconet.GetInputStatus();	//	actual state
	g_uiLnState		 = ~uiLnStateStart;	//	trick to set all pins
	g_uiLnState		&= uiAsOutput;		//	but only for outputs

	CheckLnState( uiLnStateStart );		//	set output pins

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
	g_clMyLoconet.CheckForMessage();

	if( millis() > g_ulReadInputTimer )
	{
		g_clControl.ReadInputs();
	}

	//==================================================================
	//	depending of input pins and received LN messages
	//	set output pins and send LN messages
	//
	CheckLnState( g_clMyLoconet.GetInputStatus() );
	CheckIOState( GetIOState() );

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
									g_uiLnState, g_uiIOState		);
	}
#endif
}
