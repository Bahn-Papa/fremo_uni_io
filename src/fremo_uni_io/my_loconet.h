
#pragma once

//##########################################################################
//#
//#		MyLoconetClass
//#
//#	This class handles all Loconet messanges
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

#include <stdint.h>


//==========================================================================
//
//		C L A S S   D E F I N I T I O N S
//
//==========================================================================


///////////////////////////////////////////////////////////////////////
//	CLASS:	MyLoconetClass
//
//	Diese Klasse enthält Funktionen zum Entprellen von Eingängen.
//
class MyLoconetClass
{
	public:
		MyLoconetClass();

		void Init( void );
		void CheckForMessage( void );
		void LoconetReceived( bool isSensor, uint16_t adr, uint8_t dir, uint8_t output );
		void SendMessage( uint16_t adr, uint16_t mask, uint8_t dir );

		inline uint16_t GetInputStatus( void )
		{
			return( m_uiInputStatus );
		}

		inline void SetProgMode( bool bMode )
		{
			m_bIsProgMode = bMode;
		};

		inline bool IsProgMode( void )
		{
			return( m_bIsProgMode );
		};

	private:
		uint16_t	m_uiInputStatus;
		bool		m_bIsProgMode;
};


//==========================================================================
//
//		E X T E R N   G L O B A L   V A R I A B L E S
//
//==========================================================================

extern MyLoconetClass		g_clMyLoconet;
