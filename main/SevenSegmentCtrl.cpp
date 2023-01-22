#include "SevenSegmentCtrl.h"

SevenSegmentCtrl::SevenSegmentCtrl() : m_bInit(false)
{
	
}

SevenSegmentCtrl::~SevenSegmentCtrl()
{
	
}

/************************************************************************
 * 函數名稱: Empty
 * 用途描述: 清空七段顯示器的每個數字
 * 參數說明: 無
 ************************************************************************/
void SevenSegmentCtrl::Empty()
{
	this->Init();
}

/************************************************************************
 * 函數名稱: Init
 * 用途描述: 初始化七段顯示器
 * 參數說明: 無
 ************************************************************************/
void SevenSegmentCtrl::Init()
{
	for( int i = 0; i < POS_NUM ; ++i )
	{
		if( !m_bInit )
			pinMode( m_PosPins[i], OUTPUT );
		digitalWrite( m_PosPins[i], HIGH ); // 更新所有數字
	}
	for( int i = 0; i < SEG_NUM ; ++i )
	{
		if( !m_bInit )
			pinMode( m_SegPins[i], OUTPUT );
		digitalWrite( m_SegPins[i], HIGH ); // 不亮燈
	}
	
	this->m_bInit = true;
}

/************************************************************************
 * 函數名稱: SetDigit
 * 用途描述: 控制七段顯示器每位數字或字母的顯示
 * 參數說明: 
 *           (1)int iPos: 要更新的七段顯示器位數，0為最右邊
 *           (2)int iNum: 要顯示的數字或字母之ASCII碼
 ************************************************************************/
void SevenSegmentCtrl::SetDigit( int iPos, int iNum )
{
	if( iPos < 0 || 3 < iPos )
	{
		::Pf( "error pos=%d\n", iPos );
		return;
	}
	
	// 控制想要更新哪一位七段顯示器，將其腳位設為HIGH
	// 其他腳位則設為LOW，代表不更新。 
	for( int p = 0; p < POS_NUM ; ++p )
	{
		if( p == iPos )
			digitalWrite( m_PosPins[ p ], HIGH );
		else
			digitalWrite( m_PosPins[ p ], LOW );
	}

	// 在每位顯示器顯示數字或字母
	if( (0 <= iNum && iNum <= 9) || (48 <= iNum && iNum <= 57) )         // 寫入數字 
	{
		if(48 <= iNum && iNum <= 57)
			iNum -= 48;
		
		for( int i = 0; i < SEG_NUM ; ++i )
		{
			digitalWrite( m_SegPins[ i ], ( m_arrNumberData[ iNum ][ i ] == t ? LOW : HIGH ) ); // 要顯示的段位(燈)設為LOW
		}
	}
	else if( (65 <= iNum && iNum <= 90) || (97 <= iNum && iNum <= 122) ) // 寫入字母
	{
		if( 65 <= iNum && iNum <= 90 )
			iNum -= 65;
		else if( 97 <= iNum && iNum <= 122 )
			iNum -= 97;
		
		for( int i = 0; i < SEG_NUM ; ++i )
		{
			digitalWrite( m_SegPins[ i ], ( m_arrAlphabetData[ iNum ][ i ] == t ? LOW : HIGH ) ); // 要顯示的段位(燈)設為LOW
		}
	}
	else
	{
		for( int i = 0; i < SEG_NUM ; ++i )
		{
			digitalWrite( m_SegPins[ i ], LOW );
		}
		digitalWrite( PIN_h, HIGH );
		::Pf( "error pos=%d, n=%d\n", iPos, iNum );
	}
}

/************************************************************************
 * 函數名稱: SetNumber
 * 用途描述: 設定數字並顯示
 * 參數說明: 
 *           (1)int iNum: 要顯示的數字之ASCII碼
 ************************************************************************/
void SevenSegmentCtrl::SetNumber( int iNum )
{
	int n0, n1, n2, n3;
	n3   =  iNum / 1000;
	iNum %= 1000;
	n2   =  iNum / 100;
	iNum %= 100;
	n1   =  iNum / 10;
	n0   =  iNum % 10;

	// 求出每個位數的值後，分別更新
	// 注意，此處以delay(5)隔開每個位數的更新 
	SetDigit( 0, n0 ); delay(5);
	SetDigit( 1, n1 ); delay(5);
	SetDigit( 2, n2 ); delay(5);
	SetDigit( 3, n3 ); delay(5);
}

/************************************************************************
 * 函數名稱: SetString
 * 用途描述: 設定字串並顯示
 * 參數說明: 
 *           (1)String str: 要顯示的字串
 ************************************************************************/
void SevenSegmentCtrl::SetString( String str )
{
	if( !::ValidString( str ) )
		return;
	else if( str.length() > 4 )
		return;

	SetDigit( 0, (int)str.charAt(3) ); delay(5);
	SetDigit( 1, (int)str.charAt(2) ); delay(5);
	SetDigit( 2, (int)str.charAt(1) ); delay(5);
	SetDigit( 3, (int)str.charAt(0) ); delay(5);
}
 