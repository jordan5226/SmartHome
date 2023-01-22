#pragma once

// For Arduino 1.0 and earlier
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
//

#include "Utility.h"

// 定義腳位
// #define PIN_0 2
// #define PIN_g 3
// #define PIN_c 4
// #define PIN_h 5
// #define PIN_d 6
// #define PIN_e 7
// #define PIN_b 8
// #define PIN_1 9
// #define PIN_2 10
// #define PIN_f 11
// #define PIN_a 12
// #define PIN_3 13

// 共有4個七段顯示器，分別由針腳PIN_0、PIN_1、PIN_2、PIN_3控制
// 七段顯示器裡有8個LED（包含小數點）
#define POS_NUM 4
#define SEG_NUM 8

//
#define t true
#define f false


class SevenSegmentCtrl
{
// Attributes
private:
	enum PIN
	{
		PIN_0 = 2,
		PIN_g,
		PIN_c,
		PIN_h,
		PIN_d,
		PIN_e,
		PIN_b,
		PIN_1,
		PIN_2,
		PIN_f,
		PIN_a,
		PIN_3
	};

	const byte m_PosPins[POS_NUM] = {PIN_0, PIN_1, PIN_2, PIN_3};
	const byte m_SegPins[SEG_NUM] = {PIN_a, PIN_b, PIN_c, PIN_d, PIN_e, PIN_f, PIN_g, PIN_h};
	const bool m_arrNumberData[10][SEG_NUM] = {
	//	{a, b, c, d, e, f, g, h}
		{t, t, t, t, t, t, f, f}, // 0
		{f, t, t, f, f, f, f, f}, // 1
		{t, t, f, t, t, f, t, f}, // 2
		{t, t, t, t, f, f, t, f}, // 3
		{f, t, t, f, f, t, t, f}, // 4
		{t, f, t, t, f, t, t, f}, // 5
		{t, f, t, t, t, t, t, f}, // 6
		{t, t, t, f, f, f, f, f}, // 7
		{t, t, t, t, t, t, t, f}, // 8
		{t, t, t, t, f, t, t, f}, // 9
	};
	const bool m_arrAlphabetData[26][SEG_NUM] = {
	//	{a, b, c, d, e, f, g, h}
		{t, t, t, f, t, t, t, t}, // A
		{f, f, t, t, t, t, t, t}, // B
		{t, f, f, t, t, t, f, t}, // C
		{f, t, t, t, t, f, t, t}, // D
		{t, f, f, t, t, t, t, t}, // E
		{t, f, f, f, t, t, t, t}, // F
		{t, f, t, t, t, t, f, t}, // G
		{f, t, t, f, t, t, t, t}, // H
		{f, f, f, f, t, t, f, t}, // I
		{t, t, t, t, t, f, f, t}, // J
		{f, f, f, f, f, f, f, f}, // K(X)
		{f, f, f, t, t, t, f, t}, // L
		{f, f, f, f, f, f, f, f}, // M(X)
		{f, f, f, f, f, f, f, f}, // N(X)
		{t, t, t, t, t, t, f, t}, // O
		{t, t, f, f, t, t, t, t}, // P
		{t, t, t, f, f, t, t, t}, // Q
		{f, f, f, f, f, f, f, f}, // R(X)
		{t, f, t, t, f, t, t, t}, // S
		{f, f, f, f, f, f, f, f}, // T(X)
		{f, t, t, t, t, t, f, t}, // U
		{f, f, f, f, f, f, f, f}, // V(X)
		{f, f, f, f, f, f, f, f}, // W(X)
		{f, f, f, f, f, f, f, f}, // X(X)
		{f, t, t, t, f, t, t, t}, // Y
		{f, f, f, f, f, f, f, f}, // Z(X)
	};
	
	bool m_bInit; // 是否已初始化

// Methods
public:
	SevenSegmentCtrl();
	~SevenSegmentCtrl();
	
	virtual void Empty();                      // 清空七段顯示器
	virtual void Init();                       // 初始化七段顯示器
	virtual void SetDigit(int iPos, int iNum); // 控制七段顯示器每位數字的顯示
	virtual void SetNumber(int number);        // 設定數字並顯示
	virtual void SetString(String str);        // 設定字串並顯示

};
