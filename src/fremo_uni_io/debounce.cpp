//##########################################################################
//#
//#		DebounceClass
//#
//#	This class provides functions that help debouncing digital inputs
//#	connected to the ports of an Atmel.
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	1	vom: 14.02.2022
//#
//#	Implementation:
//#		-	first version
//#			based on the algorithm of Peter Dannegger
//#
//##########################################################################


//==========================================================================
//
//		I N C L U D E S
//
//==========================================================================

#include "debounce.h"


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define	REPEAT_START	100
#define REPEAT_NEXT		 20


//==========================================================================
//
//		C L A S S   F U N C T I O N S
//
//==========================================================================


////////////////////////////////////////////////////////////////////////
//	CLASS: DebounceClass
//

//******************************************************************
//	Constructor
//------------------------------------------------------------------
//	creates an instance of the class DebounceClass
//
//	Parameter:
//		repeatMask	Specifies in a bit mask for which keys the
//					repeat function will be switched on
//
DebounceClass::DebounceClass( uint8_t repeatMask )
{
	m_usRepeatMask		= repeatMask;

	m_usKeyState		= 0;
	m_usKeyPress		= 0;
	m_usKeyRepeat		= 0;

	m_usDebounce1		= 0xFF;
	m_usDebounce2		= 0xFF;
	m_usRepeatCounter	= 0;
}

//******************************************************************
//	Work
//------------------------------------------------------------------
//	This is where the actual debouncing takes place.
//	This function must therefore be called as quickly and
//	as often as possible.
//
//	Parameter:
//		keyIn	PIN value of the port that is to be debounced
//
void DebounceClass::Work( uint8_t keyIn )
{
	uint8_t	help	 =	m_usKeyState ^ ~keyIn;

	m_usDebounce1	 = ~(m_usDebounce1 & help);
	m_usDebounce2	 =   m_usDebounce1 ^ (m_usDebounce2 & help);
	help			&=  (m_usDebounce1 &  m_usDebounce2);

	m_usKeyState	^=  help;
	m_usKeyPress	|= (m_usKeyState & help);

	if( 0 == (m_usKeyState & m_usRepeatMask) )
	{
		m_usRepeatCounter = REPEAT_START;
	}

	if( --m_usRepeatCounter == 0 )
	{
		m_usRepeatCounter	 = REPEAT_NEXT;
		m_usKeyRepeat		|= (m_usKeyState & m_usRepeatMask);
	}
}


//******************************************************************
//	GetKeyPress
//------------------------------------------------------------------
//	Returns the information in a bit field whether a key
//	was pressed.
//	Each key can only be reported ONCE.
//
//	Parameter:
//		key_mask	Specifies in a bit mask which keys should
//					be checked
//
uint8_t DebounceClass::GetKeyPress( uint8_t key_mask )
{
	key_mask		&=	m_usKeyPress;
	m_usKeyPress	^=	key_mask;

	return( key_mask );
}


//******************************************************************
//	GetKeyRepeat
//------------------------------------------------------------------
//	Returns the information in a bit field whether a key
//	was pressed long enough to activate the REPEAT function.
//	After a start-up delay, the respective key is reported
//	again and again in successive function calls as long as
//	it remains pressed.
//
//	Parameter:
//		key_mask	Specifies in a bit mask which keys should
//					be checked
//
uint8_t DebounceClass::GetKeyRepeat( uint8_t key_mask )
{
	key_mask		&=	m_usKeyRepeat;
	m_usKeyRepeat	^=	key_mask;

	return( key_mask );
}


//******************************************************************
//	GetKeyState
//------------------------------------------------------------------
//	Returns the information in a bit field whether a key is
//	currently pressed.
//
//	Parameter:
//		key_mask	Specifies in a bit mask which keys should
//					be checked
//
uint8_t DebounceClass::GetKeyState( uint8_t key_mask )
{
	key_mask &= m_usKeyState;

	return( key_mask );
}


//******************************************************************
//	GetKeyShort
//------------------------------------------------------------------
//	Returns the information in a bit field whether a key
//	was pressed short-time.
//	Each key can only be reported ONCE.
//
//	Parameter:
//		key_mask	Specifies in a bit mask which keys should
//					be checked
//
uint8_t DebounceClass::GetKeyShort( uint8_t key_mask )
{
	return( GetKeyPress( ~m_usKeyState & key_mask ) );
}


//******************************************************************
//	GetKeyLong
//------------------------------------------------------------------
//	Returns the information in a bit field whether a key
//	was pressed long-time.
//	Each key can only be reported ONCE.
//
//	Parameter:
//		key_mask	Specifies in a bit mask which keys should
//					be checked
//
uint8_t DebounceClass::GetKeyLong( uint8_t key_mask )
{
	return( GetKeyPress( GetKeyRepeat( key_mask ) ) );
}
