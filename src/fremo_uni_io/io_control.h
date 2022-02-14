
#pragma once

//##########################################################################
//#
//#		IO_ControlClass
//#
//#	This class operates the ports of the Atmel chip.
//#	There are functions to
//#		-	check digital inputs
//#		-	set digital outputs
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

#include <stdint.h>


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define IO_NUMBERS			16


////////////////////////////////////////////////////////////////////////
//	CLASS:	IO_ControlClass
//
//	This class operates the ports of the Atmel chip.
//	There are functions to
//		-	check digital inputs
//		-	set digital outputs
//
class IO_ControlClass
{
	public:
		IO_ControlClass();

		void Init( uint16_t uiOutputs );
		void ReadInputs( void );

		bool IsInputSet( uint8_t usIOPin );
		void SetOutput( uint8_t usIOPin, bool bOn );

		void GreenLedOn(    void );
		void GreenLedOff(   void );
		void GreenLedFlash( void );
		bool IsGreenLedOn(  void );

		void RedLedOn(    void );
		void RedLedOff(   void );
		void RedLedFlash( void );
		bool IsRedLedOn(  void );

	private:
		uint16_t	m_uiOutputs;
		bool		m_bLedGreen;
		bool		m_bLedRed;
};


//==========================================================================
//
//		E X T E R N   G L O B A L   V A R I A B L E S
//
//==========================================================================

extern IO_ControlClass	g_clControl;
