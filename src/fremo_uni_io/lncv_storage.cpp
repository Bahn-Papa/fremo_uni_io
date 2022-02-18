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
//#	File version:	2		vom: 18.02.2022
//#
//#	Implementation:
//#		-	add off delay time array
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

#include <Arduino.h>

#include "compile_options.h"

#include "lncv_storage.h"

#ifdef DEBUGGING_PRINTOUT
#include "debugging.h"
#endif


//==========================================================================
//
//		G L O B A L   V A R I A B L E S
//
//==========================================================================

LncvStorageClass	g_clLncvStorage = LncvStorageClass();


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

//----------------------------------------------------------------------
//	my artikle number
#define ARTIKEL_NUMMER	1512


//----------------------------------------------------------------------
//	address definitions for config informations
//
#define LNCV_ADR_MODULE_ADDRESS			0
#define LNCV_ADR_ARTIKEL_NUMMER			1
#define LNCV_ADR_CONFIGURATION			2
#define LNCV_ADR_SEND_DELAY				3
#define LNCV_ADR_OUTPUTS				4
#define LNCV_ADR_SENSORS				5
#define LNCV_ADR_INVERSE				6
#define LNCV_ADR_FIRST_IO_ADDRESS		11
#define LNCV_ADR_LAST_IO_ADDRESS		26
#define LNCV_ADR_FIRST_DELAY_ADDRESS	31
#define LNCV_ADR_LAST_DELAY_ADDRESS		46


//----------------------------------------------------------------------
//	delay times
//
#define	MIN_SEND_DELAY_TIME				 5
#define DEFAULT_SEND_DELAY_TIME			10


////////////////////////////////////////////////////////////////////////
//	CLASS: LncvStorageClass
//

//**********************************************************************
//	Constructor
//----------------------------------------------------------------------
//
LncvStorageClass::LncvStorageClass()
{
}


//**********************************************************************
//	CheckEEPROM
//----------------------------------------------------------------------
//	This function checks if the EEPROM is empty (0xFF in the cells).
//	If so, then the EEPROM will be filled with default config infos
//	and all addresses will be set to zero.
//
void LncvStorageClass::CheckEEPROM( void )
{
	uint8_t	byte0	= eeprom_read_byte( (uint8_t *)0 );
	uint8_t	byte1	= eeprom_read_byte( (uint8_t *)1 );
	uint8_t	idx		= LNCV_ADR_LAST_DELAY_ADDRESS;
	

#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintStorageCheck( byte0, byte1 );
#endif

	if( (0xFF == byte0) && (0xFF == byte1) )
	{
		//----------------------------------------------------------
		//	the EEPROM is empty, so write default config info ...
		//

#ifdef DEBUGGING_PRINTOUT
		g_clDebugging.PrintStorageDefault();
#endif

		WriteLNCV( LNCV_ADR_MODULE_ADDRESS, 0x0001 );				//	Module Adress 0x0001
		WriteLNCV( LNCV_ADR_ARTIKEL_NUMMER,	ARTIKEL_NUMMER );		//	Artikel-Nummer
		WriteLNCV( LNCV_ADR_CONFIGURATION, 0 );						//	no configuration
		WriteLNCV( LNCV_ADR_SEND_DELAY, DEFAULT_SEND_DELAY_TIME );	//	Send Delay Timer
		WriteLNCV( LNCV_ADR_OUTPUTS, 0 );							//	all Inputs
		WriteLNCV( LNCV_ADR_SENSORS, 0 );							//	all Switch messages
		WriteLNCV( LNCV_ADR_INVERSE, 0 );							//	all not inverse
		
		//----------------------------------------------------------
		//	set all I/O addresses and delay times to '0'
		//
		while( LNCV_ADR_INVERSE < idx )
		{
			WriteLNCV( idx, 0 );
			idx--;
		}
	}
}


//**********************************************************************
//	Init
//----------------------------------------------------------------------
//	This function will read the saved informations from the EEPROM.
//
void LncvStorageClass::Init( void )
{
#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintStorageRead();
#endif

	//--------------------------------------------------------------
	//	read config information
	//
	m_uiArticleNumber	= ReadLNCV( LNCV_ADR_ARTIKEL_NUMMER );
	m_uiModuleAddress	= ReadLNCV( LNCV_ADR_MODULE_ADDRESS );
//	m_uiConfiguration	= ReadLNCV( LNCV_ADR_CONFIGURATION );
	m_uiOutputs			= ReadLNCV( LNCV_ADR_OUTPUTS );
	m_uiSensors			= ReadLNCV( LNCV_ADR_SENSORS );
	m_uiInverse			= ReadLNCV( LNCV_ADR_INVERSE );

	//--------------------------------------------------------------
	//	read send delay time
	//	and make sure it is not shorter than MIN_SEND_DELAY_TIME ms
	//
	m_uiSendDelay = ReadLNCV( LNCV_ADR_SEND_DELAY );

	if( MIN_SEND_DELAY_TIME > m_uiSendDelay )
	{
		m_uiSendDelay = MIN_SEND_DELAY_TIME;
	}

	//--------------------------------------------------------------
	//	read IO addresses and delay times
	//
	for( uint8_t idx = 0 ; idx < IO_NUMBERS ; idx++ )
	{
		m_aruiAddress[  idx ] = ReadLNCV( LNCV_ADR_FIRST_IO_ADDRESS    + idx );
		m_aruiOffDelay[ idx ] = ReadLNCV( LNCV_ADR_FIRST_DELAY_ADDRESS + idx );
	}
}


//**********************************************************************
//	IsValidLNCVAdress
//
bool LncvStorageClass::IsValidLNCVAddress( uint16_t Adresse )
{
	if( LNCV_ADR_LAST_DELAY_ADDRESS >= Adresse )
	{
		return( true );
	}

	return( false );
}


//**********************************************************************
//	ReadLNCV
//
uint16_t LncvStorageClass::ReadLNCV( uint16_t Adresse )
{
	uint16_t	value;

	//--------------------------------------------------------------
	//	because of uint16 values the address has to be shifted
	//	by '1' (this will double the address).
	//
	value = eeprom_read_word( (uint16_t *)(Adresse << 1) );

	return( value );
}


//**********************************************************************
//	WriteLNCV
//
void LncvStorageClass::WriteLNCV( uint16_t Adresse, uint16_t Value )
{
	//--------------------------------------------------------------
	//	because of uint16 values the address has to be shifted
	//	by '1' (this will double the address).
	//
	eeprom_write_word( (uint16_t *)(Adresse << 1), Value );
}
