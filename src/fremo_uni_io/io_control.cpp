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
//#	in the following tables:
//#
//#	Platine Version 1 und Version 2:
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
//#	Platine Version 4:
//#		Head	Pin		IO Pin	Port Pin
//#		SV 6	6		15			PF 0
//#		SV 6	5		14			PF 1
//#		SV 1	6		13			PB 5
//#		SV 1	5		12			PB 6
//#		SV 7	6		11			PF 4
//#		SV 7	5		10			PF 5
//#		SV 8	6		 9			PF 6
//#		SV 8	5		 8			PF 7
//#		SV 3	6		 7			PC 6
//#		SV 3	5		 6			PC 7
//#		SV 5	6		 5			PE 2
//#		SV 5	5		 4			PB 4
//#		SV 2	6		 3			PB 7
//#		SV 2	5		 2			PD 7
//#		SV 4	6		 1			PD 5
//#		SV 4	5		 0			PD 6
//#
//#	To simplify the program code the mapping is done with the help
//#	of a few arrays. The trick is to use the universal pin number (IO pin)
//#	as array index for the different mapping arrays.
//#	The following mappings are needed:
//#		-	pin of port
//#		-	input  mask for data direction
//#		-	output mask for data direction
//#		-	read an input
//#		-	set an output
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	3		vom: 05.02.2023
//#
//#	Implementation:
//#		-	add support for board version 4
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	2		vom: 18.02.2022
//#
//#	Implementation:
//#		-	add support for board version 2
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
//	Port B		V1				V2 + V4
//	PB0			LED GREEN		Relais
//	PB1			LED RED			LED RED
//	PB2			N/A				LED GREEN
//	PB3			N/A				N/A
//	PB4			I/O (SV5.5)		I/O (SV5.5)
//	PB5, OC1A	I/O (SV1.6)		I/O (SV1.6)
//	PB6, OC1B	I/O (SV1.5)		I/O (SV1.5)
//	PB7, OC1C	I/O (SV2.6)		I/O (SV2.6)
//
#if PLATINE_VERSION == 1

	#define LED_GREEN		_BV( 0 )

#else

	#define RELAIS			_BV( 0 )
	#define LED_GREEN		_BV( 2 )

#endif

#define LED_RED				_BV( 1 )


//---------------------------------------------------------------------
//	Port C		V1, V2 + V4
//	PC0			N/A
//	PC1			N/A
//	PC2			N/A
//	PC3			N/A
//	PC4			N/A
//	PC5			N/A
//	PC6, OC3A	I/O (SV3.6)
//	PC7, OC4A	I/O (SV3.5)
//


//---------------------------------------------------------------------
//	Port D		V1, V2 + V4
//	PD0			N/A
//	PD1			N/A
//	PD2			N/A
//	PD3			N/A
//	PD4			N/A
//	PD5			I/O (SV4.6)
//	PD6			I/O (SV4.5)
//	PD7, OC4D	I/O (SV2.5)
//


//---------------------------------------------------------------------
//	Port E		V1, V2 + V4
//	PE0			N/A
//	PE1			N/A
//	PE2			I/O (SV5.6)
//	PE3			N/A
//	PE4			N/A
//	PE5			N/A
//	PE6			N/A
//	PE7			N/A
//


//---------------------------------------------------------------------
//	Port F		V1, V2 + V4
//	PF0			I/O (SV6.6)
//	PF1			I/O (SV6.5)
//	PF2			N/A
//	PF3			N/A
//	PF4			I/O (SV7.6)
//	PF5			I/O (SV7.5)
//	PF6			I/O (SV8.6)
//	PF7			I/O (SV8.5)
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

//----------------------------------------------------------------------
//	this array contains the mapping	universal pin numbering to
//	pin of port
//
#if PLATINE_VERSION == 4

	uint8_t	g_arPortPins[ IO_NUMBERS ] =
	{
		PB6, PB5, PB7, PB7, PB4, PB2, PB7, PB6, PB7, PB6, PB5, PB4, PB6, PB5, PB1, PB0
	};

#else

	uint8_t	g_arPortPins[ IO_NUMBERS ] =
	{
		PB6, PB5, PB7, PB7, PB4, PB2, PB7, PB6, PB6, PB5, PB7, PB6, PB5, PB4, PB1, PB0
	};

#endif

//----------------------------------------------------------------------
//	this array contains the mapping universal pin numbering to
//	address of variable containing the input mask of a port
//
#if PLATINE_VERSION == 4

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
		&g_usPortFInputs,
		&g_usPortFInputs,
		&g_usPortFInputs,
		&g_usPortFInputs,
		&g_usPortBInputs,
		&g_usPortBInputs,
		&g_usPortFInputs,
		&g_usPortFInputs
	};

#else

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

#endif

//----------------------------------------------------------------------
//	this array contains the mapping universal pin numbering to
//	address of variable containing the output mask of a port
//
#if PLATINE_VERSION == 4

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
		&g_usPortFOutputs,
		&g_usPortFOutputs,
		&g_usPortFOutputs,
		&g_usPortFOutputs,
		&g_usPortBOutputs,
		&g_usPortBOutputs,
		&g_usPortFOutputs,
		&g_usPortFOutputs
	};

#else

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

#endif

//----------------------------------------------------------------------
//	this array contains the mapping universal pin numbering to
//	address of function for reading the inputs of a port
//
#if PLATINE_VERSION == 4

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
		GetKeyStatePortF,
		GetKeyStatePortF,
		GetKeyStatePortF,
		GetKeyStatePortF,
		GetKeyStatePortB,
		GetKeyStatePortB,
		GetKeyStatePortF,
		GetKeyStatePortF
	};

#else

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

#endif

//----------------------------------------------------------------------
//	this array contains the mapping universal pin numbering to
//	address of port to set an output
//
#if PLATINE_VERSION == 4

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
		&PORTF,
		&PORTF,
		&PORTF,
		&PORTF,
		&PORTB,
		&PORTB,
		&PORTF,
		&PORTF
	};

#else

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

#endif


//==========================================================================
//
//		F U N C T I O N S
//
//==========================================================================

//----------------------------------------------------------------------
//	the functions just serve as mapping functions to
//	the class functions for each port
//
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

	//--------------------------------------------------------------
	//	identify the input and output mask for each port
	//
	//	the trick here is that the addresses of the variables
	//	holding the mask are stored in an array just as the
	//	port pins. The mapping of universal pin numbering to ports
	//	is done by storing the right variable address into
	//	the right array place and storing the right pin number
	//	into the right arry place
	//	So the only thing to do here is to find out if a universal
	//	pin is configured as output or as input
	//
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

	//----	Port B  ------------------------------------------------
	//
	if( g_usPortBInputs )
	{
		DDRB	&= ~g_usPortBInputs;	//	configure as Input
		PORTB	|=  g_usPortBInputs;	//	Pull-Up on
	}

#if PLATINE_VERSION == 1

	DDRB	|=  (g_usPortBOutputs | LED_GREEN | LED_RED);	//	configure as Output
	PORTB	&= ~(g_usPortBOutputs | LED_GREEN | LED_RED);	//	switch off

#else

	DDRB	|=  (g_usPortBOutputs | LED_GREEN | LED_RED | RELAIS);	//	configure as Output
	PORTB	&= ~(g_usPortBOutputs | LED_GREEN | LED_RED | RELAIS);	//	switch off

#endif

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
	//	handle the inputs for each port
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
