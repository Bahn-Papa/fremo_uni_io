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
//#	File version:	9		vom: 26.10.2023
//#
//#	Implementation:
//#		-	add switch report handling
//#			new variable
//#				m_uiSwitchReport
//#			change in function
//#				Init()
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	8		vom: 27.01.2023
//#
//#	Bug Fix:
//#		-	remove of the error that I put in the last time
//#			change in function
//#				Init()
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	7		vom: 27.01.2023
//#
//#	Bug Fix:
//#		-	the interpretation of the config was wrong
//#			change in function
//#				Init()
//#			new definition
//#				CONFIG_ACTIVE_GREEN
//#				CONFIG_INPUT
//#				CONFIG_SENSOR
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	6		vom: 27.01.2023
//#
//#	Implementation:
//#		-	add version number to EEPROM
//#			change in function
//#				CheckEEPROM()
//#		-	move definition into header file
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	5		vom: 09.11.2022
//#
//#	Bug Fix:
//#		-	in function 'WriteLNCV()' Address was not declared
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	4		vom: 06.11.2022
//#
//#	Implementation:
//#		-	only write to EEPROM if the new value is different
//#			than the old one
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	3		vom: 19.07.2022
//#
//#	Implementation:
//#		-	new configuration method:
//#			address, input, output, switch, sensor, green and red
//#         is configured in one word.
//#         The word has the following format:
//#			xxxx m	-	xxxx	address
//#						m		mode
//#								0	-	output switch msg RED   (0) active
//#								1	-	output switch msg GREEN (1) active
//#								2	-	output sensor LOW  (0) active
//#								3	-	output sensor HIGH (1) active
//#								4	-	input  switch msg RED   (0) active
//#								5	-	input  switch msg GREEN (1) active
//#								6	-	input  sensor LOW  (0) active
//#								7	-	input  sensor HIGH (1) active
//#         output means: lissening on Loconet and set IO pins
//#         input  means: check state of IO pins and send loconet msg
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
//	delay times
//
#define	MIN_SEND_DELAY_TIME				 5
#define DEFAULT_SEND_DELAY_TIME			10

//----------------------------------------------------------------------
//	configuration masks
//
#define CONFIG_INPUT			0x0004
#define CONFIG_SENSOR			0x0002
#define CONFIG_ACTIVE_GREEN		0x0001


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
void LncvStorageClass::CheckEEPROM( uint16_t uiVersionNumber )
{
	uint16_t	uiAddress	= ReadLNCV( LNCV_ADR_MODULE_ADDRESS );
	uint16_t	uiArticle	= ReadLNCV( LNCV_ADR_ARTIKEL_NUMMER );
	uint8_t		idx			= LNCV_ADR_LAST_DELAY_ADDRESS;


#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintStorageCheck( uiAddress, uiArticle );
#endif

	if( (0xFFFF == uiAddress) || (0x0000 == uiAddress) )
	{
		//----------------------------------------------------------
		//	the EEPROM is empty, so write default config info ...
		//

#ifdef DEBUGGING_PRINTOUT
		g_clDebugging.PrintStorageDefault();
#endif

		WriteLNCV( LNCV_ADR_MODULE_ADDRESS, 0x0001 );				//	Module Adress 0x0001
		WriteLNCV( LNCV_ADR_ARTIKEL_NUMMER,	ARTIKEL_NUMMER );		//	Artikel-Nummer
		WriteLNCV( LNCV_ADR_VERSION_NUMBER, uiVersionNumber );		//	Version Number

		WriteLNCV( LNCV_ADR_CONFIGURATION, 0 );						//	no configuration
		WriteLNCV( LNCV_ADR_SEND_DELAY, DEFAULT_SEND_DELAY_TIME );	//	Send Delay Timer
		
		//----------------------------------------------------------
		//	set all I/O addresses and delay times to '0'
		//
		while( LNCV_ADR_SEND_DELAY < idx )
		{
			WriteLNCV( idx, 0 );
			idx--;
		}
	}
	else
	{
		WriteLNCV( LNCV_ADR_VERSION_NUMBER, uiVersionNumber );
	}

	delay( 250 );
}


//**********************************************************************
//	Init
//----------------------------------------------------------------------
//	This function will read the saved informations from the EEPROM.
//
void LncvStorageClass::Init( void )
{
    uint16_t    uiHelper;
    uint16_t    uiMask      = 0x0001;


#ifdef DEBUGGING_PRINTOUT
	g_clDebugging.PrintStorageRead();
#endif

	//--------------------------------------------------------------
	//	read config information
	//
	m_uiArticleNumber	= ReadLNCV( LNCV_ADR_ARTIKEL_NUMMER );
	m_uiModuleAddress	= ReadLNCV( LNCV_ADR_MODULE_ADDRESS );
	m_uiSwitchReport	= ReadLNCV( LNCV_ADR_SWITCH_AS_REPORT );
	m_uiOutputs			= 0x0000;
	m_uiSensors			= 0x0000;
	m_uiInverse			= 0x0000;

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
    //  for the IO addresses find out if it is
    //      input or output
    //      switch or sensor
    //      react on RED or GREEN
	//
	for( uint8_t idx = 0 ; idx < IO_NUMBERS ; idx++ )
	{
		m_aruiOffDelay[ idx ]	= ReadLNCV( LNCV_ADR_FIRST_DELAY_ADDRESS + idx );

        uiHelper				 = ReadLNCV( LNCV_ADR_FIRST_IO_ADDRESS + idx );
        m_aruiAddress[ idx ]	 = uiHelper / 10;
		uiHelper				-= (m_aruiAddress[ idx ] * 10);

        if( 0 == (uiHelper & CONFIG_INPUT) )
        {
            //------------------------------------------------------
            //  this is an output
            //
            m_uiOutputs |= uiMask;
        }

        if( uiHelper & CONFIG_SENSOR )
        {
            //------------------------------------------------------
            //  this is a sensor
            //
            m_uiSensors |= uiMask;
        }

        if( 0 == (uiHelper & CONFIG_ACTIVE_GREEN) )
        {
            m_uiInverse |= uiMask;
        }

        uiMask <<= 1;
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
void LncvStorageClass::WriteLNCV( uint16_t Address, uint16_t Value )
{
	//--------------------------------------------------------------
	//	because of uint16 values the address has to be shifted
	//	by '1' (this will double the address).
	//
	uint16_t *	puiAdr	= (uint16_t *)(Address << 1);
	uint16_t	uiValue	= eeprom_read_word( puiAdr );
	
	if( uiValue != Value )
	{
		eeprom_write_word( puiAdr, Value );
	}
}
