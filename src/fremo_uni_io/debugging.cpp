//##########################################################################
//#
//#		DebuggingClass
//#
//#	This class deliveres text print outs to different output devices such
//#	as seriell interface or OLED display
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	2		vom: 18.02.2022
//#
//#	Bug fix:
//#		-	correction of mismatch of inputs and outputs in function
//#			'PrintStatus()'
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	1		vom: 14.02.2022
//#
//#	Implementation:
//#		-	OLED Display
//#			Das Display ist wie folgt aufgeteilt:
//#			 S                     1 1 1 1 1 1
//#			Z  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
//#			0      U n i   V x . x x . y y
//#			1  I n p u t   S t a t e :
//#			2  0 0 1 . . 0 . 1 . . . . . . . .
//#			3  O u t p u t   S t a t e :
//#			4  . . . 1 0 . 1 . 0 0 0 1 1 0 1 0
//#			5    <loconet message type>
//#			6  <loconet message parameter>
//#			7
//#
//##########################################################################


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

#include "compile_options.h"


#ifdef DEBUGGING_PRINTOUT
//**************************************************************************
//**************************************************************************


#include <avr/pgmspace.h>
#include <Wire.h>
#include <simple_oled_sh1106.h>

#include "debugging.h"


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define IO_NUMBERS				16

//----------------------------------------------------------------------
//	definition of display positions
//
#define INPUT_STATE_LINE		2
#define OUTPUT_STATE_LINE		4
#define STATE_COLUMN			0
#define	LOCONET_MSG_LINE		5
#define LOCONET_MSG_COLUMN		0
#define MESSAGE_LINE			7


//------------------------------------------------------------------
//	definition of print options
//
//	#define COUNT_ALL_MESSAGES
//	#define COUNT_MY_MESSAGES


//==========================================================================
//
//		G L O B A L   V A R I A B L E S
//
//==========================================================================

DebuggingClass	g_clDebugging	= DebuggingClass();

//----------------------------------------------------------------------
//	Variable für das OLED Display
//
char		g_chDebugString[ 18 ];


#if defined( COUNT_ALL_MESSAGES ) || defined( COUNT_MY_MESSAGES )

uint32_t	g_ulSensorMsgCounter	= 0L;


//==========================================================================
//
//		G L O B A L   F U N C T I O N S
//
//==========================================================================

//******************************************************************
//	PrintMsgCount
//------------------------------------------------------------------
//
void PrintMsgCount( void )
{
	g_clDisplay.ClearLine( MESSAGE_LINE );
	sprintf( g_chDebugString, "Switch: %8lu", g_ulSwitchMsgCounter );
	g_clDisplay.Print( g_chDebugString );
}

#endif	//	defined( COUNT_ALL_MESSAGES ) || defined( COUNT_MY_MESSAGES )


//==========================================================================
//
//		C L A S S   F U N C T I O N S
//
//==========================================================================


////////////////////////////////////////////////////////////////////////
//	CLASS: DebuggingClass
//

//******************************************************************
//	Constructor
//------------------------------------------------------------------
//
DebuggingClass::DebuggingClass()
{
	m_counter = 0;
}


//******************************************************************
//	Init
//------------------------------------------------------------------
//
void DebuggingClass::Init( void )
{
	g_clDisplay.Init();
	g_clDisplay.Flip( true );
}


//******************************************************************
//	PrintTitle
//------------------------------------------------------------------
//
void DebuggingClass::PrintTitle(	uint8_t versionMain,
									uint8_t versionMinor,
									uint8_t versionHotFix )
{
	g_clDisplay.Clear();
	g_clDisplay.SetInverseFont( true );
	sprintf(	g_chDebugString, "  Uni V%u.%02u.%02u  ",
				versionMain, versionMinor, versionHotFix );
	g_clDisplay.Print( g_chDebugString );
	g_clDisplay.SetInverseFont( false );
}


//******************************************************************
//	PrintInfoLine
//------------------------------------------------------------------
//
void DebuggingClass::PrintInfoLine( info_lines_t number )
{
	switch( number )
	{
		case infoLineFields:
			g_clDisplay.Print( F( "\nInput State:\n" ) );
			g_clDisplay.Print( F( "\nOutput State:"  ) );
			break;

		case infoLineInit:
			g_clDisplay.Print( F( "\n  Init:\n" ) );
			break;

		case infoLineLedTest:
			g_clDisplay.Print( F( "  LED Test\n" ) );
			break;

		default:
			break;
	}
}


//******************************************************************
//	PrintText
//------------------------------------------------------------------
//
void DebuggingClass::PrintText( char *text )
{
	g_clDisplay.ClearLine( MESSAGE_LINE );
	g_clDisplay.Print( text );
}


//******************************************************************
//	PrintCounter
//------------------------------------------------------------------
//
void DebuggingClass::PrintCounter( void )
{
	m_counter++;
	
	g_clDisplay.ClearLine( MESSAGE_LINE );
	sprintf( g_chDebugString, "Counter: %lu", m_counter );
	g_clDisplay.Print( g_chDebugString );
}


//******************************************************************
//	PrintNotifyType
//------------------------------------------------------------------
//
void DebuggingClass::PrintNotifyType( notify_type_t type )
{
	m_NotifyType = type;

#ifdef COUNT_ALL_MESSAGES

	g_ulSwitchMsgCounter++;

	PrintMsgCount();

#endif
}


//******************************************************************
//	PrintNotifyMsg
//------------------------------------------------------------------
//
void DebuggingClass::PrintNotifyMsg( uint16_t address, uint8_t dir )
{
	SetLncvMsgPos();

	switch( m_NotifyType )
	{
		case NT_Sensor:
			g_clDisplay.Print( F( "E:Sensor\n" ) );
			break;

		case NT_Request:
			g_clDisplay.Print( F( "E:Switch Reqst\n" ) );
			break;

		case NT_Report:
			g_clDisplay.Print( F( "E:Switch Report\n" ) );
			break;

		case NT_State:
			g_clDisplay.Print( F( "E:Switch State\n" ) );
			break;
	}

	sprintf( g_chDebugString, "AR:%5u - ", address );
	g_clDisplay.Print( g_chDebugString );
	g_clDisplay.Print( (dir ? "green" : "red") );

#ifdef COUNT_MY_MESSAGES

	g_ulSwitchMsgCounter++;

	PrintMsgCount();

#endif
}


//******************************************************************
//	PrintLncvDiscoverStart
//
void DebuggingClass::PrintLncvDiscoverStart( bool start, uint16_t artikel, uint16_t address )
{
	SetLncvMsgPos();

	if( start )
	{
		g_clDisplay.Print( F( "LNCV Prog Start\n" ) );
	}
	else
	{
		g_clDisplay.Print( F( "LNCV Discover\n" ) );
	}
	
	sprintf( g_chDebugString, "AR%5u AD%5u", artikel, address );
	g_clDisplay.Print( g_chDebugString );
}


//******************************************************************
//	PrintLncvStop
//
void DebuggingClass::PrintLncvStop()
{
	SetLncvMsgPos();
	g_clDisplay.Print( F( "LNCV Prog Stop" ) );
//	sprintf( g_chDebugString, "AR%5u AD%5u", ArtNr, ModuleAddress );
//	g_clDisplay.Print( g_chDebugString );
}


//******************************************************************
//	PrintLncvReadWrite
//
void DebuggingClass::PrintLncvReadWrite( bool doRead, uint16_t address, uint16_t value )
{
	SetLncvMsgPos();

	if( doRead )
	{
		g_clDisplay.Print( F( "LNCV Read\n" ) );
	}
	else
	{
		g_clDisplay.Print( F( "LNCV Write\n" ) );
	}

	sprintf( g_chDebugString, "AD%5u VA%5u", address, value );
	g_clDisplay.Print( g_chDebugString );
}


//******************************************************************
//	SetLncvMsgPos
//
void DebuggingClass::SetLncvMsgPos( void )
{
	g_clDisplay.ClearLine( LOCONET_MSG_LINE + 1 );
	g_clDisplay.ClearLine( LOCONET_MSG_LINE );
}


//******************************************************************
//	PrintStorageCheck
//
void DebuggingClass::PrintStorageCheck( uint16_t uiAddress, uint16_t uiArticle )
{
	g_clDisplay.Print( F( "Check EEPROM:\n" ) );
	sprintf( g_chDebugString, " 0:%05d 1:%05d", uiAddress, uiArticle );
	g_clDisplay.Print( g_chDebugString );
}


//******************************************************************
//	PrintStorageDefault
//
void DebuggingClass::PrintStorageDefault( void )
{
	g_clDisplay.Print( F( "\nSet default Adr" ) );
}


//******************************************************************
//	PrintStorageRead
//
void DebuggingClass::PrintStorageRead( void )
{
	g_clDisplay.Print( F( "\n  Lese LNCVs\n" ) );
}


//******************************************************************
//	PrintStatus
//
void DebuggingClass::PrintStatus(	uint16_t uiAsOutputs,
									uint16_t uiOutState,
									uint16_t uiInState )
{
	g_clDisplay.SetCursor( INPUT_STATE_LINE, STATE_COLUMN );
	PrintStatusBits( uiAsOutputs, uiInState );

	g_clDisplay.SetCursor( OUTPUT_STATE_LINE, STATE_COLUMN );
	PrintStatusBits( ~uiAsOutputs, uiOutState );
}


//******************************************************************
//	PrintStatusBits
//
void DebuggingClass::PrintStatusBits( uint16_t uiIOMask, uint16_t uiState )
{
	uint16_t	uiMask	= 0x8000;

	for( uint8_t idx = 0 ; idx < IO_NUMBERS ; idx++ )
	{
		if( uiIOMask & uiMask )
		{
			g_clDisplay.Print( "." );
		}
		else if( uiState & uiMask )
		{
			g_clDisplay.Print( "1" );
		}
		else
		{
			g_clDisplay.Print( "0" );
		}

		uiMask >>= 1;
	}
}


//**************************************************************************
//**************************************************************************
#endif	//	DEBUGGING_PRINTOUT
