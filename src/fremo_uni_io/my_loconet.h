
#pragma once

//##########################################################################
//#
//#		MyLoconetClass
//#
//#	This class handles all Loconet messanges
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	3		vom: 18.11.2023
//#
//#	Implementation:
//#		-	avoid missunderstanding, so rename
//#				GetInputStatus()	=>	GetOutputStatus()
//#				m_uiInputStatus		=>	m_uiOutputStatus
//#		-	add message to send status of all inputs
//#			new variable
//#				m_uiAdrSendStatus
//#				m_bSendStatus
//#			change in function
//#				CheckForMessage()
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	2		vom: 03.11.2023
//#
//#	Implementation:
//#		-	change in message handling
//#			change in function
//#				LoconetReceived()
//#				SendMessage()
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

#include "notify_types.h"


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
		bool CheckForMessage( void );
		void LoconetReceived( notify_type_t type, uint16_t uiAdr, uint8_t usDirClosed, uint8_t usOutputThrown );
		void SendMessage( notify_type_t type, uint16_t uiAdr, uint8_t usDirClosed, uint8_t usOutputThrown );

		inline uint16_t GetOutputStatus( void )
		{
			return( m_uiOutputStatus );
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
		uint16_t	m_uiOutputStatus;
		uint16_t	m_uiAdrSendStatus;
		bool		m_bIsProgMode;
		bool		m_bSendStatus;
};


//==========================================================================
//
//		E X T E R N   G L O B A L   V A R I A B L E S
//
//==========================================================================

extern MyLoconetClass		g_clMyLoconet;
