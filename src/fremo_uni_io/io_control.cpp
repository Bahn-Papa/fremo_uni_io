//##########################################################################
//#
//#		IO_ControlClass
//#
//#-------------------------------------------------------------------------
//#
//#	This class operates the ports of the Atmel chip.
//#	There are functions to
//#		-	check digital inputs
//#		-	set digital outputs
//#
//#	The assignment of the IO pins to the port pins is shown
//#	in the following table:
//#		Head	Pin		IO Pin	Port Pin
//#		SV 6	6		15			PF 0
//#		SV 6	5		14			PF 1
//#		SV 7	6		13			PF 4
//#		SV 7	5		12			PF 5
//#		SV 8	6		11			PF 6
//#		SV 8	5		10			PF 7
//#		SV 1	6		 9			PB 5
//#		SV 1	5		 8			PB 6
//#		SV 3	6		 7			PC 6
//#		SV 3	5		 6			PC 7
//#		SV 5	6		 5			PE 2
//#		SV 5	5		 4			PB 4
//#		SV 2	6		 3			PB 7
//#		SV 2	5		 2			PD 7
//#		SV 4	6		 1			PD 5
//#		SV 4	5		 0			PD 6
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

#include "io_control.h"
#include "debounce.h"


//==========================================================================
//
//		F U N C T I O N   D E C L A R A T I O N
//
//==========================================================================

uint8_t GetKeyStatePortB( uint8_t usMask );
uint8_t GetKeyStatePortC( uint8_t usMask );
uint8_t GetKeyStatePortD( uint8_t usMask );
uint8_t GetKeyStatePortE( uint8_t usMask );
uint8_t GetKeyStatePortF( uint8_t usMask );


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define	INIT_READ_INPUT_COUNT	6
#define FLASH_TIME				250

/*
#define MASK_PORT_BIT_0			0x01
#define MASK_PORT_BIT_1			0x02
#define MASK_PORT_BIT_2			0x04
#define MASK_PORT_BIT_3			0x08
#define MASK_PORT_BIT_4			0x10
#define MASK_PORT_BIT_5			0x20
#define MASK_PORT_BIT_6			0x40
#define MASK_PORT_BIT_7			0x80
*/


//---------------------------------------------------------------------
//	Port B
//		PB0		LED GREEN
//		PB1		LED RED
//		PB2		N/A
//		PB3		N/A
//		PB4		Input Output
//		PB5		Input Output
//		PB6		Input Output
//		PB7		Input Output
//
#define LED_GREEN			0
#define LED_RED				1


//---------------------------------------------------------------------
//	Port C
//		PC0		N/A
//		PC1		N/A
//		PC2		N/A
//		PC3		N/A
//		PC4		N/A
//		PC5		N/A
//		PC6		Input Output
//		PC7		Input Output
//


//---------------------------------------------------------------------
//	Port D
//		PD0		N/A
//		PD1		N/A
//		PD2		N/A
//		PD3		N/A
//		PD4		N/A
//		PD5		Input Output
//		PD6		Input Output
//		PD7		Input Output
//


//---------------------------------------------------------------------
//	Port E
//		PE0		N/A
//		PE1		N/A
//		PE2		Input Output
//		PE3		N/A
//		PE4		N/A
//		PE5		N/A
//		PE6		N/A
//		PE7		N/A
//


//---------------------------------------------------------------------
//	Port F
//		PF0		Input Output
//		PF1		Input Output
//		PF2		N/A
//		PF3		N/A
//		PF4		Input Output
//		PF5		Input Output
//		PF6		Input Output
//		PF7		Input Output
//


//==========================================================================
//
//		M A C R O S
//
//==========================================================================

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif


//==========================================================================
//
//		G L O B A L   V A R I A B L E S
//
//==========================================================================

IO_ControlClass		g_clControl	= IO_ControlClass();

DebounceClass		g_clPortB( 0x00 );
DebounceClass		g_clPortC( 0x00 );
DebounceClass		g_clPortD( 0x00 );
DebounceClass		g_clPortE( 0x00 );
DebounceClass		g_clPortF( 0x00 );

volatile uint8_t	g_usPortBInputs;
volatile uint8_t	g_usPortBOutputs;
volatile uint8_t	g_usPortCInputs;
volatile uint8_t	g_usPortCOutputs;
volatile uint8_t	g_usPortDInputs;
volatile uint8_t	g_usPortDOutputs;
volatile uint8_t	g_usPortEInputs;
volatile uint8_t	g_usPortEOutputs;
volatile uint8_t	g_usPortFInputs;
volatile uint8_t	g_usPortFOutputs;

uint32_t			g_ulMillisFlash	= 0L;

typedef uint8_t (*func_ptr_t)( uint8_t );

func_ptr_t	g_arFunctions[ IO_NUMBERS ] =
{
	GetKeyStatePortD,
	GetKeyStatePortD,
	GetKeyStatePortD,
	GetKeyStatePortB,
	GetKeyStatePortB,
	GetKeyStatePortE,
	GetKeyStatePortC,
	GetKeyStatePortC,
	GetKeyStatePortB,
	GetKeyStatePortB,
	GetKeyStatePortF,
	GetKeyStatePortF,
	GetKeyStatePortF,
	GetKeyStatePortF,
	GetKeyStatePortF,
	GetKeyStatePortF
};


uint8_t	g_arPortPins[ IO_NUMBERS ] =
{
	PB6, PB5, PB7, PB7, PB4, PB2, PB7, PB6, PB6, PB5, PB7, PB6, PB5, PB4, PB1, PB0
};

volatile uint8_t * g_arPorts[ IO_NUMBERS ] =
{
	&PORTD,
	&PORTD,
	&PORTD,
	&PORTB,
	&PORTB,
	&PORTE,
	&PORTC,
	&PORTC,
	&PORTB,
	&PORTB,
	&PORTF,
	&PORTF,
	&PORTF,
	&PORTF,
	&PORTF,
	&PORTF
};

volatile uint8_t * g_arInputMasks[ IO_NUMBERS ] =
{
	&g_usPortDInputs,
	&g_usPortDInputs,
	&g_usPortDInputs,
	&g_usPortBInputs,
	&g_usPortBInputs,
	&g_usPortEInputs,
	&g_usPortCInputs,
	&g_usPortCInputs,
	&g_usPortBInputs,
	&g_usPortBInputs,
	&g_usPortFInputs,
	&g_usPortFInputs,
	&g_usPortFInputs,
	&g_usPortFInputs,
	&g_usPortFInputs,
	&g_usPortFInputs
};

volatile uint8_t * g_arOutputMasks[ IO_NUMBERS ] =
{
	&g_usPortDOutputs,
	&g_usPortDOutputs,
	&g_usPortDOutputs,
	&g_usPortBOutputs,
	&g_usPortBOutputs,
	&g_usPortEOutputs,
	&g_usPortCOutputs,
	&g_usPortCOutputs,
	&g_usPortBOutputs,
	&g_usPortBOutputs,
	&g_usPortFOutputs,
	&g_usPortFOutputs,
	&g_usPortFOutputs,
	&g_usPortFOutputs,
	&g_usPortFOutputs,
	&g_usPortFOutputs
};


//==========================================================================
//
//		F U N C T I O N S
//
//==========================================================================

uint8_t GetKeyStatePortB( uint8_t usMask )
{
	return( g_clPortB.GetKeyState( usMask ) );
}

uint8_t GetKeyStatePortC( uint8_t usMask )
{
	return( g_clPortC.GetKeyState( usMask ) );
}

uint8_t GetKeyStatePortD( uint8_t usMask )
{
	return( g_clPortD.GetKeyState( usMask ) );
}

uint8_t GetKeyStatePortE( uint8_t usMask )
{
	return( g_clPortE.GetKeyState( usMask ) );
}

uint8_t GetKeyStatePortF( uint8_t usMask )
{
	return( g_clPortF.GetKeyState( usMask ) );
}


//==========================================================================
//
//		C L A S S   F U N C T I O N S
//
//==========================================================================


////////////////////////////////////////////////////////////////////////
//	CLASS: IO_ControlClass
//

//******************************************************************
//	Constructor
//------------------------------------------------------------------
IO_ControlClass::IO_ControlClass()
{
	g_usPortBInputs		= 0;
	g_usPortBOutputs	= 0;
	g_usPortCInputs		= 0;
	g_usPortCOutputs	= 0;
	g_usPortDInputs		= 0;
	g_usPortDOutputs	= 0;
	g_usPortEInputs		= 0;
	g_usPortEOutputs	= 0;
	g_usPortFInputs		= 0;
	g_usPortFOutputs	= 0;
}


//******************************************************************
//	Init
//------------------------------------------------------------------
//	here for all Ports the relevant I/O pins will be configured.
//
void IO_ControlClass::Init( uint16_t uiOutputs )
{
	uint16_t	uiMask = 0x0001;


	m_uiOutputs = uiOutputs;

	for( uint8_t idx = 0 ; idx < IO_NUMBERS ; idx++ )
	{
		if( uiOutputs & uiMask )
		{
			*g_arOutputMasks[ idx ] |= _BV( g_arPortPins[ idx ] );
		}
		else
		{
			*g_arInputMasks[ idx ] |= _BV( g_arPortPins[ idx ] );
		}

		uiMask <<= 1;
	}

/*
	if( uiOutputs & MASK_IO_BIT_0 )
	{
		m_usPortDOutputs |= MASK_PORT_BIT_6;
	}
	else
	{
		m_usPortDInputs |= MASK_PORT_BIT_6;
	}

	if( uiOutputs & MASK_IO_BIT_1 )
	{
		m_usPortDOutputs |= MASK_PORT_BIT_5;
	}
	else
	{
		m_usPortDInputs |= MASK_PORT_BIT_5;
	}

	if( uiOutputs & MASK_IO_BIT_2 )
	{
		m_usPortDOutputs |= MASK_PORT_BIT_7;
	}
	else
	{
		m_usPortDInputs |= MASK_PORT_BIT_7;
	}

	if( uiOutputs & MASK_IO_BIT_3 )
	{
		m_usPortBOutputs |= MASK_PORT_BIT_7;
	}
	else
	{
		m_usPortBInputs |= MASK_PORT_BIT_7;
	}

	if( uiOutputs & MASK_IO_BIT_4 )
	{
		m_usPortBOutputs |= MASK_PORT_BIT_4;
	}
	else
	{
		m_usPortBInputs |= MASK_PORT_BIT_4;
	}

	if( uiOutputs & MASK_IO_BIT_5 )
	{
		m_usPortEOutputs |= MASK_PORT_BIT_2;
	}
	else
	{
		m_usPortEInputs |= MASK_PORT_BIT_2;
	}

	if( uiOutputs & MASK_IO_BIT_6 )
	{
		m_usPortCOutputs |= MASK_PORT_BIT_7;
	}
	else
	{
		m_usPortCInputs |= MASK_PORT_BIT_7;
	}

	if( uiOutputs & MASK_IO_BIT_7 )
	{
		m_usPortCOutputs |= MASK_PORT_BIT_6;
	}
	else
	{
		m_usPortCInputs |= MASK_PORT_BIT_6;
	}

	if( uiOutputs & MASK_IO_BIT_8 )
	{
		m_usPortBOutputs |= MASK_PORT_BIT_6;
	}
	else
	{
		m_usPortBInputs |= MASK_PORT_BIT_6;
	}

	if( uiOutputs & MASK_IO_BIT_9 )
	{
		m_usPortBOutputs |= MASK_PORT_BIT_5;
	}
	else
	{
		m_usPortBInputs |= MASK_PORT_BIT_5;
	}

	if( uiOutputs & MASK_IO_BIT_10 )
	{
		m_usPortFOutputs |= MASK_PORT_BIT_7;
	}
	else
	{
		m_usPortFInputs |= MASK_PORT_BIT_7;
	}

	if( uiOutputs & MASK_IO_BIT_11 )
	{
		m_usPortFOutputs |= MASK_PORT_BIT_6;
	}
	else
	{
		m_usPortFInputs |= MASK_PORT_BIT_6;
	}

	if( uiOutputs & MASK_IO_BIT_12 )
	{
		m_usPortFOutputs |= MASK_PORT_BIT_5;
	}
	else
	{
		m_usPortFInputs |= MASK_PORT_BIT_5;
	}

	if( uiOutputs & MASK_IO_BIT_13 )
	{
		m_usPortFOutputs |= MASK_PORT_BIT_4;
	}
	else
	{
		m_usPortFInputs |= MASK_PORT_BIT_4;
	}

	if( uiOutputs & MASK_IO_BIT_14 )
	{
		m_usPortFOutputs |= MASK_PORT_BIT_1;
	}
	else
	{
		m_usPortFInputs |= MASK_PORT_BIT_1;
	}

	if( uiOutputs & MASK_IO_BIT_15 )
	{
		m_usPortFOutputs |= MASK_PORT_BIT_0;
	}
	else
	{
		m_usPortFInputs |= MASK_PORT_BIT_0;
	}
*/

	//----	Port B  ------------------------------------------------
	//
	if( g_usPortBInputs )
	{
		DDRB	&= ~g_usPortBInputs;	//	configure as Input
		PORTB	|=  g_usPortBInputs;	//	Pull-Up on
	}

	if( g_usPortBOutputs )
	{
		DDRB	|=  (g_usPortBOutputs | LED_GREEN | LED_RED);	//	configure as Output
		PORTB	&= ~(g_usPortBOutputs | LED_GREEN | LED_RED);	//	switch off
	}

	//----	Port C  ------------------------------------------------
	//
	if( g_usPortCInputs )
	{
		DDRC	&= ~g_usPortCInputs;	//	configure as Input
		PORTC	|=  g_usPortCInputs;	//	Pull-Up on
	}

	if( g_usPortCOutputs )
	{
		DDRC	|=  g_usPortCOutputs;	//	configure as Output
		PORTC	&= ~g_usPortCOutputs;	//	switch off
	}

	//----	Port D  ------------------------------------------------
	//
	if( g_usPortDInputs )
	{
		DDRD	&= ~g_usPortDInputs;	//	configure as Input
		PORTD	|=  g_usPortDInputs;	//	Pull-Up on
	}

	if( g_usPortDOutputs )
	{
		DDRD	|=  g_usPortDOutputs;	//	configure as Output
		PORTD	&= ~g_usPortDOutputs;	//	switch off
	}

	//----	Port E  ------------------------------------------------
	//
	if( g_usPortEInputs )
	{
		DDRE	&= ~g_usPortEInputs;	//	configure as Input
		PORTE	|=  g_usPortEInputs;	//	Pull-Up on
	}

	if( g_usPortEOutputs )
	{
		DDRE	|=  g_usPortEOutputs;	//	configure as Output
		PORTE	&= ~g_usPortEOutputs;	//	switch off
	}

	//----	Port F  ------------------------------------------------
	//
	if( g_usPortFInputs )
	{
		DDRF	&= ~g_usPortFInputs;	//	configure as Input
		PORTF	|=  g_usPortFInputs;	//	Pull-Up on
	}

	if( g_usPortFOutputs )
	{
		DDRF	|=  g_usPortFOutputs;	//	configure as Output
		PORTF	&= ~g_usPortFOutputs;	//	switch off
	}

	//----	Read actual Inputs  ------------------------------------
	//
	for( uint8_t idx = 0 ; idx < INIT_READ_INPUT_COUNT ; idx++ )
	{
		delay( 20 );

		ReadInputs();
	}

	delay( 20 );
}


//******************************************************************
//	ReadInputs
//------------------------------------------------------------------
//	This function should be called at regular intervals,
//	for example: every 20 ms
//
void IO_ControlClass::ReadInputs( void )
{
	//----------------------------------------------------------
	//	get the inputs from the ports
	//
	if( g_usPortBInputs )
	{
		g_clPortB.Work( PINB );
	}

	if( g_usPortCInputs )
	{
		g_clPortC.Work( PINC );
	}

	if( g_usPortDInputs )
	{
		g_clPortD.Work( PIND );
	}

	if( g_usPortEInputs )
	{
		g_clPortE.Work( PINE );
	}

	if( g_usPortFInputs )
	{
		g_clPortF.Work( PINF );
	}

	//----------------------------------------------------------
	//	let the LED(s) flash
	//
	if( m_bLedGreen || m_bLedRed )
	{
		if( millis() > g_ulMillisFlash )
		{
			g_ulMillisFlash = millis() + FLASH_TIME;

			if( m_bLedGreen )
			{
				if( IsGreenLedOn() )
				{
					//----	switch LED off  ----
					cbi( PORTB, LED_GREEN );
				}
				else
				{
					//----	switch LED on  ----
					sbi( PORTB, LED_GREEN );
				}
			}

			if( m_bLedRed )
			{
				if( IsRedLedOn() )
				{
					//----	switch LED off  ----
					cbi( PORTB, LED_RED );
				}
				else
				{
					//----	switch LED on  ----
					sbi( PORTB, LED_RED );
				}
			}
		}
	}
}


//******************************************************************
//	IsInputSet
//------------------------------------------------------------------
//
bool IO_ControlClass::IsInputSet( uint8_t usIOPin )
{
	uint8_t	usMask	= 0x01;
	bool	retval	= false;

	if( IO_NUMBERS > usIOPin )
	{
		usMask <<=  g_arPortPins[ usIOPin ];
		retval	 = (0 != (*g_arFunctions[ usIOPin ])( usMask ));
	}

/*
	if( uiMask & uiInputs )
	{
		switch( uiMask )
		{
			case MASK_IO_BIT_0:
				retval = ( 0 != g_clPortD.GetKeyState( MASK_PORT_BIT_6 ) );
				break;
				
			case MASK_IO_BIT_1:
				retval = ( 0 != g_clPortD.GetKeyState( MASK_PORT_BIT_5 ) );
				break;
				
			case MASK_IO_BIT_2:
				retval = ( 0 != g_clPortD.GetKeyState( MASK_PORT_BIT_7 ) );
				break;
				
			case MASK_IO_BIT_3:
				retval = ( 0 != g_clPortB.GetKeyState( MASK_PORT_BIT_7 ) );
				break;
				
			case MASK_IO_BIT_4:
				retval = ( 0 != g_clPortB.GetKeyState( MASK_PORT_BIT_4 ) );
				break;
				
			case MASK_IO_BIT_5:
				retval = ( 0 != g_clPortE.GetKeyState( MASK_PORT_BIT_2 ) );
				break;
				
			case MASK_IO_BIT_6:
				retval = ( 0 != g_clPortC.GetKeyState( MASK_PORT_BIT_7 ) );
				break;
				
			case MASK_IO_BIT_7:
				retval = ( 0 != g_clPortC.GetKeyState( MASK_PORT_BIT_6 ) );
				break;
				
			case MASK_IO_BIT_8:
				retval = ( 0 != g_clPortB.GetKeyState( MASK_PORT_BIT_6 ) );
				break;
				
			case MASK_IO_BIT_9:
				retval = ( 0 != g_clPortB.GetKeyState( MASK_PORT_BIT_5 ) );
				break;
				
			case MASK_IO_BIT_10:
				retval = ( 0 != g_clPortF.GetKeyState( MASK_PORT_BIT_7 ) );
				break;
				
			case MASK_IO_BIT_11:
				retval = ( 0 != g_clPortF.GetKeyState( MASK_PORT_BIT_6 ) );
				break;
				
			case MASK_IO_BIT_12:
				retval = ( 0 != g_clPortF.GetKeyState( MASK_PORT_BIT_5 ) );
				break;
				
			case MASK_IO_BIT_13:
				retval = ( 0 != g_clPortF.GetKeyState( MASK_PORT_BIT_4 ) );
				break;
				
			case MASK_IO_BIT_14:
				retval = ( 0 != g_clPortF.GetKeyState( MASK_PORT_BIT_1 ) );
				break;
				
			case MASK_IO_BIT_15:
				retval = ( 0 != g_clPortF.GetKeyState( MASK_PORT_BIT_0 ) );
				break;
				
			default:
				break;
		}
	}
*/

	return( retval );
}


//******************************************************************
//	SetOutput
//------------------------------------------------------------------
//
void IO_ControlClass::SetOutput( uint8_t usIOPin, bool bOn )
{
	if( bOn )
	{
		sbi( *g_arPorts[ usIOPin ], g_arPortPins[ usIOPin ] );
	}
	else
	{
		cbi( *g_arPorts[ usIOPin ], g_arPortPins[ usIOPin ] );
	}

/*
		switch( uiMask )
		{
			case MASK_IO_BIT_0:
				if( bOn )
				{
					sbi( PORTD, MASK_PORT_BIT_6 );
				}
				else
				{
					cbi( PORTD, MASK_PORT_BIT_6 );
				}
				break;

			case MASK_IO_BIT_1:
				if( bOn )
				{
					sbi( PORTD, MASK_PORT_BIT_5 );
				}
				else
				{
					cbi( PORTD, MASK_PORT_BIT_5 );
				}
				break;

			case MASK_IO_BIT_2:
				if( bOn )
				{
					sbi( PORTD, MASK_PORT_BIT_7 );
				}
				else
				{
					cbi( PORTD, MASK_PORT_BIT_7 );
				}
				break;

			case MASK_IO_BIT_3:
				if( bOn )
				{
					sbi( PORTB, MASK_PORT_BIT_7 );
				}
				else
				{
					cbi( PORTB, MASK_PORT_BIT_7 );
				}
				break;

			case MASK_IO_BIT_4:
				if( bOn )
				{
					sbi( PORTB, MASK_PORT_BIT_4 );
				}
				else
				{
					cbi( PORTB, MASK_PORT_BIT_4 );
				}
				break;

			case MASK_IO_BIT_5:
				if( bOn )
				{
					sbi( PORTE, MASK_PORT_BIT_2 );
				}
				else
				{
					cbi( PORTE, MASK_PORT_BIT_2 );
				}
				break;

			case MASK_IO_BIT_6:
				if( bOn )
				{
					sbi( PORTC, MASK_PORT_BIT_7 );
				}
				else
				{
					cbi( PORTC, MASK_PORT_BIT_7 );
				}
				break;

			case MASK_IO_BIT_7:
				if( bOn )
				{
					sbi( PORTC, MASK_PORT_BIT_6 );
				}
				else
				{
					cbi( PORTC, MASK_PORT_BIT_6 );
				}
				break;

			case MASK_IO_BIT_8:
				if( bOn )
				{
					sbi( PORTB, MASK_PORT_BIT_6 );
				}
				else
				{
					cbi( PORTB, MASK_PORT_BIT_6 );
				}
				break;

			case MASK_IO_BIT_9:
				if( bOn )
				{
					sbi( PORTB, MASK_PORT_BIT_5 );
				}
				else
				{
					cbi( PORTB, MASK_PORT_BIT_5 );
				}
				break;

			case MASK_IO_BIT_10:
				if( bOn )
				{
					sbi( PORTF, MASK_PORT_BIT_7 );
				}
				else
				{
					cbi( PORTF, MASK_PORT_BIT_7 );
				}
				break;

			case MASK_IO_BIT_11:
				if( bOn )
				{
					sbi( PORTF, MASK_PORT_BIT_6 );
				}
				else
				{
					cbi( PORTF, MASK_PORT_BIT_6 );
				}
				break;

			case MASK_IO_BIT_12:
				if( bOn )
				{
					sbi( PORTF, MASK_PORT_BIT_5 );
				}
				else
				{
					cbi( PORTF, MASK_PORT_BIT_5 );
				}
				break;

			case MASK_IO_BIT_13:
				if( bOn )
				{
					sbi( PORTF, MASK_PORT_BIT_4 );
				}
				else
				{
					cbi( PORTF, MASK_PORT_BIT_4 );
				}
				break;

			case MASK_IO_BIT_14:
				if( bOn )
				{
					sbi( PORTF, MASK_PORT_BIT_1 );
				}
				else
				{
					cbi( PORTF, MASK_PORT_BIT_1 );
				}
				break;

			case MASK_IO_BIT_15:
				if( bOn )
				{
					sbi( PORTF, MASK_PORT_BIT_0 );
				}
				else
				{
					cbi( PORTF, MASK_PORT_BIT_0 );
				}
				break;

			default:
				break;
		}
	}
*/
}


//******************************************************************
//	GreenLedOn
//------------------------------------------------------------------
//
void IO_ControlClass::GreenLedOn( void )
{
	m_bLedGreen = false;

	sbi( PORTB, LED_GREEN );
}


//******************************************************************
//	GreenLedOff
//------------------------------------------------------------------
//
void IO_ControlClass::GreenLedOff( void )
{
	m_bLedGreen = false;

	cbi( PORTB, LED_GREEN );
}


//******************************************************************
//	GreenLedFlash
//------------------------------------------------------------------
//
void IO_ControlClass::GreenLedFlash( void )
{
	m_bLedGreen = true;
}


//******************************************************************
//	IsGreenLedOn
//------------------------------------------------------------------
//
bool IO_ControlClass::IsGreenLedOn( void )
{
	return( bit_is_set( PINB, LED_GREEN ) );
}


//******************************************************************
//	RedLedOn
//------------------------------------------------------------------
//
void IO_ControlClass::RedLedOn( void )
{
	m_bLedRed = false;

	sbi( PORTB, LED_RED );
}


//******************************************************************
//	RedLedOff
//------------------------------------------------------------------
//
void IO_ControlClass::RedLedOff( void )
{
	m_bLedRed = false;

	cbi( PORTB, LED_RED );
}


//******************************************************************
//	RedLedFlash
//------------------------------------------------------------------
//
void IO_ControlClass::RedLedFlash( void )
{
	m_bLedRed = true;
}


//******************************************************************
//	IsRedLedOn
//------------------------------------------------------------------
//
bool IO_ControlClass::IsRedLedOn( void )
{
	return( bit_is_set( PINB, LED_RED ) );
}
