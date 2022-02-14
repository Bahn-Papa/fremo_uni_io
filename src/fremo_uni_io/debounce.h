
#pragma once

//##########################################################################
//#
//#		DebounceClass
//#
//#	This class provides functions that help debouncing digital inputs
//#	connected to the ports of an Atmel.
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	1		vom: 14.02.2022
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

#include <stdint.h>


//==========================================================================
//
//		C L A S S   D E F I N I T I O N S
//
//==========================================================================


///////////////////////////////////////////////////////////////////////
//	CLASS:	DebounceClass
//
//	This class provides functions that help debouncing digital inputs
//	connected to the ports of an Atmel.
//
class DebounceClass
{
	public:
		//--------------------------------------------------------------
		//	creates an instance of the class DebounceClass
		//
		//	Parameter:
		//		repeatMask	Specifies in a bit mask for which keys the
		//					repeat function will be switched on
		//
		DebounceClass( uint8_t repeatMask );

		//--------------------------------------------------------------
		//	This is where the actual debouncing takes place.
		//	This function must therefore be called as quickly
		//	and as often as possible.
		//
		//	Parameter:
		//		keyIn	PIN value of the port that is to be debounced
		//
		void Work( uint8_t keyIn );


		//--------------------------------------------------------------
		//	Returns the information in a bit field whether a key
		//	was pressed.
		//	Each key can only be reported ONCE.
		//
		//	Parameter:
		//		key_mask	Specifies in a bit mask which keys should
		//					be checked
		//
		uint8_t GetKeyPress( uint8_t key_mask );


		//--------------------------------------------------------------
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
		uint8_t GetKeyRepeat( uint8_t key_mask );


		//--------------------------------------------------------------
		//	Returns the information in a bit field whether a key
		//	is currently pressed.
		//
		//	Parameter:
		//		key_mask	Specifies in a bit mask which keys should
		//					be checked
		//
		uint8_t GetKeyState( uint8_t key_mask );


		//--------------------------------------------------------------
		//	Returns the information in a bit field whether a key
		//	was pressed short-time.
		//	Each key can only be reported ONCE.
		//
		//	Parameter:
		//		key_mask	Specifies in a bit mask which keys should
		//					be checked
		//
		uint8_t GetKeyShort( uint8_t key_mask );


		//--------------------------------------------------------------
		//	Returns the information in a bit field whether a key
		//	was pressed long-time.
		//	Each key can only be reported ONCE.
		//
		//	Parameter:
		//		key_mask	Specifies in a bit mask which keys should
		//					be checked
		//
		uint8_t GetKeyLong( uint8_t key_mask );

		//--------------------------------------------------------------
		//	sets the repeat mask to enable the repeat function for
		//	the inputs that are denoted in the bit field
		//
		//	Parameter:
		//		repeat_mask	Specifies in a bit mask for which keys the
		//					repeat function will be switched on
		//
		inline void SetRepeatMask( uint8_t repeat_mask )
		{
			m_usRepeatMask = repeat_mask;
		};

	private:
		//--------------------------------------------------------------
		//		m_usRepeatMask
		//
		//	contains the mask for which keys the repeat function
		//	is activated.
		//
		uint8_t	m_usRepeatMask;

		//--------------------------------------------------------------
		//		m_usKeyState
		//
		//	bit field containing the debounced current key state
		//	(bit set = key pressed)
		//
		uint8_t m_usKeyState;

		//--------------------------------------------------------------
		//		m_usKeyPress
		//
		//	bit field containing all newly pressed keys since the last
		//	call of function GetKeyPress, GetKeyShort or GetKeyLong.
		//	(bit set = key pressed)
		//
		uint8_t m_usKeyPress;

		//--------------------------------------------------------------
		//		m_usKeyRepeat
		//	bit field containing all pressed keys that have reached
		//	the repeat state
		//	(bit set = key in repeat state)
		//
		uint8_t m_usKeyRepeat;

		//--------------------------------------------------------------
		//		m_usDebounce1
		//		m_usDebounce2
		//		m_usRepeatCounter
		//	help variables to carry out the key debouncing
		//
		uint8_t m_usDebounce1;
		uint8_t m_usDebounce2;
		uint8_t m_usRepeatCounter;
};
