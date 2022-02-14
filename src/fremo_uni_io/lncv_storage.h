
#pragma once

//##########################################################################
//#
//#		LncvStorageClass
//#
//#	This class manages the LNCVs (L oco N et C onfiguration V ariables)
//#	These variables will be saved in the EEPROM.
//#
//#	To simplify access, the variables were arranged one after the other
//#	without gaps. This makes it very easy to read and write the variables
//#	with a tool.
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	1		Date: 14.02.2022
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


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

#define IO_NUMBERS		16


////////////////////////////////////////////////////////////////////////
//	CLASS:	LncvStorageClass
//
class LncvStorageClass
{
	public:
		//----------------------------------------------------------
		//	Constructor
		//
		LncvStorageClass();

		//----------------------------------------------------------
		//
		void		CheckEEPROM( void );
		void		Init( void );
		bool		IsValidLNCVAddress( uint16_t Adresse );
		uint16_t	ReadLNCV(  uint16_t Adresse );
		void		WriteLNCV( uint16_t Adresse, uint16_t Value );

		//----------------------------------------------------------
		//
		inline uint16_t GetArticleNumber( void )
		{
			return( m_uiArticleNumber );
		};

		//----------------------------------------------------------
		//
		inline uint16_t GetModuleAddress( void )
		{
			return( m_uiModuleAddress );
		};

		//----------------------------------------------------------
		//
		inline uint16_t GetSendDelayTime( void )
		{
			return( m_uiSendDelay );
		};

		//----------------------------------------------------------
		//
		inline uint16_t	GetAsOutputs( void )
		{
			return( m_uiOutputs );
		};

		//----------------------------------------------------------
		//
		inline uint16_t	GetAsSensor( void )
		{
			return( m_uiSensors );
		};

		//----------------------------------------------------------
		//
		inline uint16_t	GetIsInverse( void )
		{
			return( m_uiInverse );
		};

		//----------------------------------------------------------
		//
		inline uint16_t	GetIOAddress( uint8_t idx )
		{
			uint16_t	uiAddress = 0;

			if( IO_NUMBERS > idx )
			{
				uiAddress = m_aruiAddress[ idx ];
			}
			
			return( uiAddress );
		}

	private:
		uint16_t	m_uiArticleNumber;
		uint16_t	m_uiModuleAddress;
//		uint16_t	m_uiConfiguration;
		uint16_t	m_uiSendDelay;
		uint16_t	m_uiOutputs;
		uint16_t	m_uiSensors;
		uint16_t	m_uiInverse;
		uint16_t	m_aruiAddress[ IO_NUMBERS ];
};


//==========================================================================
//
//		E X T E R N   G L O B A L   V A R I A B L E S
//
//==========================================================================

extern LncvStorageClass		g_clLncvStorage;
