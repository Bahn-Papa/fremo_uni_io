//##########################################################################
//#
//#	fremo_uni_io		copyright: Michael Pfeil
//#
//#	This program controls the hardware for universal boards
//#
//##########################################################################


#define VERSION_MAIN	0
#define	VERSION_MINOR	9
#define VERSION_HOTFIX	1


//##########################################################################
//#
//#		Version History:
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

uint32_t	g_ulReadInputTimer		= 0L;
uint32_t	g_ulPrintStatusTimer	= 0L;
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
//
void CheckIOState( uint16_t uiNewIOState )
{
	//------------------------------------------------------------------
	//	get difference between old and actual state ...
	//
	uint16_t	uiDiff	= g_uiIOState ^ uiNewIOState;
	uint16_t	uiMask	= 0x0001;
	uint8_t		usDir	= 0;
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
				usDir = 1;
			}
			else
			{
				usDir = 0;
			}

			g_clMyLoconet.SendMessage( g_clLncvStorage.GetIOAddress( idx ), uiMask, usDir );

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

	delay( 200 );

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
									g_uiIOState, g_uiLnState		);
	}
#endif
}
