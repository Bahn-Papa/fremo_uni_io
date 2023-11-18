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

#include "compile_options.h"


//----------------------------------------------------------------------
//	The main version is defined by PLATINE_VERSION (compile_options.h)
//
//#define VERSION_MAIN	1
#define	VERSION_MINOR	7
#define VERSION_HOTFIX	0

#define VERSION_NUMBER		((PLATINE_VERSION * 10000) + (VERSION_MINOR * 100) + VERSION_HOTFIX)


//##########################################################################
//#
//#		Version History:
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


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

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
	uint16_t		isInverse		= g_clLncvStorage.GetIsInverse();
	uint16_t		uiAddress		= 0;
	uint8_t			usInfo;
	uint8_t			usOutputThrown	= 0;
	bool			bIsGerade;
	bool			bMsgPending		= false;

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
				if( uiNewIOState & uiMask )
				{
					usInfo = 1;
				}
				else
				{
					usInfo = 0;
				}

				if( isInverse & uiMask )
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
					//
					type	= NT_Report;

					if( 0 == ((idx + 1) % 2) )
					{
						bIsGerade	= true;
					}
					else
					{
						bIsGerade		= false;
						usOutputThrown	= usInfo;
					}
				}
				else
				{
					//------------------------------------------
					//	pin is configured as switch request
					//
					type = NT_Request;
				}

				if( uiDiff & uiMask )
				{
					if( (NT_Report == type) && !bIsGerade )
					{
						//------------------------------------------
						//	if the notifiy type is NT_Report
						//	we need 2 infos to send the msg.
						//	so we can send the msg only when we
						//	are at even count, ergo set the flag
						//	that we still must send a msg
						//
						bMsgPending = true;
					}
					else
					{
						//------------------------------------------
						//	in all other cases just send the msg
						//
						g_clMyLoconet.SendMessage( type, uiAddress, usInfo, usOutputThrown );

						bMsgPending = false;
					}

					//------------------------------------------------------
					//	this change was handled,
					//	so clear the corresponding bit
					//
					uiDiff &= ~uiMask;
				}
				else if( bMsgPending )
				{
					g_clMyLoconet.SendMessage( type, uiAddress, usInfo, usOutputThrown );

					bMsgPending = false;
				}
			}
		}

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
	uint16_t	uiLnStateStart;


	g_bIsProgMode = false;

#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.Init();

	g_clDebugging.PrintTitle( PLATINE_VERSION, VERSION_MINOR, VERSION_HOTFIX );
	g_clDebugging.PrintInfoLine( infoLineInit );
#endif

	//----	LNCV: Check and Init  --------------------------------------
	g_clLncvStorage.CheckEEPROM( VERSION_NUMBER );
	
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
	g_clDebugging.PrintTitle( PLATINE_VERSION, VERSION_MINOR, VERSION_HOTFIX );
	g_clDebugging.PrintStorageConfig(	g_clLncvStorage.GetAsOutputs(),
										g_clLncvStorage.GetAsSensor(),
										g_clLncvStorage.GetIsInverse()	);

	delay( 2000 );
#endif

	//----	Prepare Display  -------------------------------------------
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintTitle( PLATINE_VERSION, VERSION_MINOR, VERSION_HOTFIX );
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
