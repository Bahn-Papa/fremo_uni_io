
#pragma once

//##########################################################################
//#
//#		DebuggingClass
//#
//#	This class will deliver text output to different output devices.
//#	e.g.: OLED display
//#
//#-------------------------------------------------------------------------
//#
//#	File version:	2		vom: 29.01.2023
//#
//#	Implementation:
//#		-	add function to print the configuration
//#			new function
//#				PrintStorageConfig()
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


//==========================================================================
//
//		D E F I N I T I O N S
//
//==========================================================================

typedef enum info_lines
{
	infoLineFields = 1,
	infoLineInit,
	infoLineLedTest

}	info_lines_t;


typedef enum notify_type
{
	NT_Sensor = 0,
	NT_Request,
	NT_Report,
	NT_State

}	notify_type_t;


//==========================================================================
//
//		C L A S S   D E F I N I T I O N S
//
//==========================================================================


////////////////////////////////////////////////////////////////////////
//	CLASS:	DebuggingClass
//
class DebuggingClass
{
	public:
		DebuggingClass();

		void Init( void );

		void PrintTitle(	uint8_t versionMain,
							uint8_t versionMinor,
							uint8_t versionHotFix );
		void PrintInfoLine( info_lines_t number );

		void PrintNotifyType( notify_type_t type );
		void PrintNotifyMsg( uint16_t address, uint8_t dir );

		void PrintLncvDiscoverStart(	bool start,
										uint16_t artikel,
										uint16_t address	);
		void PrintLncvStop();
		void PrintLncvReadWrite(	bool doRead,
									uint16_t address,
									uint16_t value		);

		void PrintStorageCheck( uint16_t uiAddress, uint16_t uiArticle );
		void PrintStorageDefault( void );
		void PrintStorageRead( void );
		void PrintStorageConfig( uint16_t uiAsOutputs, uint16_t uiAsSensors, uint16_t uiIsInverse );

		void PrintStatus(	uint16_t uiAsOutputs,
							uint16_t uiOutState,
							uint16_t uiInState		);

		void PrintText( char *text );
		void PrintCounter( void );

	private:
		notify_type_t	m_NotifyType;
		uint32_t		m_counter;

		void SetLncvMsgPos( void );
		void PrintStatusBits( uint16_t uiIOMask, uint16_t uiState );
};


//==========================================================================
//
//		E X T E R N   G L O B A L   V A R I A B L E S
//
//==========================================================================

extern DebuggingClass	g_clDebugging;
