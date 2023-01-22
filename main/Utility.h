#pragma once

#define ARDUINO_MEGA2560     1               // 使用ARDUINO MEGA2560

#ifdef  ARDUINO_MEGA2560
#define _DEBUG_              1               // 允許Debug信息
#endif

/*
 * 驗證字串是否合法
 */
static boolean ValidString(String str)
{
	str.replace(" ", "");   // 去除空格符
	if( str == "" )         // 若字串無內容
		return false;       // 錯誤
	return true;
}

/*
 * 將DEBUG信息輸出到Serial
 */
inline void Pf(const char *fmt, ... )
{
#ifdef _DEBUG_
	char tmp[128]; // max is 128 chars
	va_list args;
	va_start (args, fmt );
	vsnprintf(tmp, 128, fmt, args);
	va_end (args);
	Serial.print(tmp);
#endif
}