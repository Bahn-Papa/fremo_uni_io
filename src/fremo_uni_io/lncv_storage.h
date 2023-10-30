
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
//#	File version:	5		vom: 26.10.2023
//#
//#	Implementation:
//#		-	add switch report handling
//#			new variable
//#				m_uiSwitchReport
//#			new function
//#				GetAsReport()
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	4		vom: 27.01.2023
//#
//#	Implementation:
//#		-	add version number to EEPROM
//#			change in function
//#				CheckEEPROM()
//#		-	move definition into header file
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	3		Date: 18.02.2022
//#
//#	Implementation:
//#		-	add off delay time array
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	2		Date: 15.02.2022
//#
//#	Implementation:
//#		-	add function GetAsInputs()
//#			the function will return a bit mask where each '1' bit
//#			stands for an input
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


//----------------------------------------------------------------------
//	my artikle number
#define ARTIKEL_NUMMER	1512


//----------------------------------------------------------------------
//	address definitions for config informations
//
#define LNCV_ADR_MODULE_ADDRESS			0
#define LNCV_ADR_ARTIKEL_NUMMER			1
#define LNCV_ADR_VERSION_NUMBER			2
#define LNCV_ADR_SWITCH_AS_REPORT		3
#define LNCV_ADR_SEND_DELAY				4

#define LNCV_ADR_FIRST_IO_ADDRESS		11
#define LNCV_ADR_LAST_IO_ADDRESS		26
#define LNCV_ADR_FIRST_DELAY_ADDRESS	31
#define LNCV_ADR_LAST_DELAY_ADDRESS		46


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
		void		CheckEEPROM( uint16_t uiVersionNumber );
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
		inline uint16_t	GetAsReport( void )
		{
			return( m_uiSwitchReport );
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
		inline uint16_t	GetAsInputs( void )
		{
			return( ~m_uiOutputs );
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

		//----------------------------------------------------------
		//
		inline uint16_t	GetIOOffDelay( uint8_t idx )
		{
			uint16_t	uiOffDelay = 0;

			if( IO_NUMBERS > idx )
			{
				uiOffDelay = m_aruiOffDelay[ idx ];
			}
			
			return( uiOffDelay );
		}

	private:
		uint16_t	m_uiArticleNumber;
		uint16_t	m_uiModuleAddress;
		uint16_t	m_uiSwitchReport;
		uint16_t	m_uiSendDelay;
		uint16_t	m_uiOutputs;
		uint16_t	m_uiSensors;
		uint16_t	m_uiInverse;
		uint16_t	m_aruiAddress[  IO_NUMBERS ];
		uint16_t	m_aruiOffDelay[ IO_NUMBERS ];
};


//==========================================================================
//
//		E X T E R N   G L O B A L   V A R I A B L E S
//
//==========================================================================

extern LncvStorageClass		g_clLncvStorage;
