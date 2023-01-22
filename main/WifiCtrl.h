/******************************************************************************************
 * Title:           Wifi Module Controller For ESP8266(firmware version 0.9.2.2)
 * Author:          Jordan Yeh
 * Date(YY/MM/DD):  2016/11/13
 ******************************************************************************************  
 * Usage:
 * [1]Declare a WifiCtrl object
 *  WifiCtrl g_Wifi( [Serial Port], "[SSID]", "[Password]", "[IP]", [Port] );
 *  Ex: WifiCtrl g_Wifi( Serial1, "D-Link_DIR-809", "12345678", "192.168.1.134", 5566 );
 *
 * [2]Initial Wifi Configuration
 *  Ex: g_Wifi.InitWifiModule();
 *
 * [3]Send a String to server or client ( Note that the string must contain \r\n )
 *  g_Wifi.SendData( [Data] );
 *  Ex: g_Wifi.SendData( "Hi!~\r\n" );
 *
 * [4]Receive data and store it into a String buffer
 *  g_Wifi.Recv( [String Buffer Reference], [Timeout(ms)] );
 *  Ex: g_Wifi.Recv( g_strRecv ); // 6 milliseconds
 *  Ex: g_Wifi.Recv( g_strRecv, 10000 ); // 10 milliseconds
 *
 * [5]Close the Socket session to server or client
 *  g_Wifi.Close( [Session ID] );
 *  Ex: g_Wifi.Close( 0 );
 *
 * [6]Create a connection ( Default protocol: TCP )
 *  g_Wifi.Connect( [IP], [Port], [Protocol] );
 *  Ex: g_Wifi.Connect( "192.168.1.134", 5566, "UDP" ); // use UDP protocol
 *  Ex: g_Wifi.Connect( "192.168.1.134", 5566);         // use default protocol(TCP)
 ******************************************************************************************/
#pragma once
#include "Utility.h"

#ifndef _WIFICTRL_H_
#define _WIFICTRL_H_

#define TIMEOUT_NORMAL 6000
#define TIMEOUT_LONG   10000


typedef String (*ResponseCallback)(); // 定義生成回應內容的Callback函數之型態

// 定義HTTP請求與回應對照表結構體
struct HTTP_RESPONSE_MAP
{
	String           m_strUrl;      // 請求的URL
	int              m_iStatusCode; // 要回應的HTTP狀態碼
	ResponseCallback m_pResponse;   // 生成回應內容的Callback函數
	
	HTTP_RESPONSE_MAP() : m_strUrl(""), m_iStatusCode(200), m_pResponse(NULL)
	{
		
	}
	
	~HTTP_RESPONSE_MAP()
	{
		m_pResponse = NULL;
	}
};
typedef HTTP_RESPONSE_MAP  RESPONSEMAP;
typedef HTTP_RESPONSE_MAP* LPRESPONSEMAP;


class WifiCtrl
{
// Attributes
private:
	enum WIFI_CMD
	{
		WIFI_CMD_SETMODE = 0, // 模式設定成 3:Station+AP混合模式
		WIFI_CMD_MULTICON,    // 允許一對多的多重連線
		WIFI_CMD_ONSERVER,    // 啟動服務器監聽Port
		WIFI_CMD_CONTOAP,     // 連線到已存在的無線網路
		WIFI_CMD_CONTOSERVER, // 讓 ESP8266扮演客戶端，對伺服器進行連線
		WIFI_CMD_COUNT,
	};


	bool             m_bInitWifi;                        // Is initial Wifi complete?
	short            m_sStatusInitWifi;                  // 當前初始化階段
	int              m_iServer_Port;                     // 目標服務器的PORT
	int              m_iSessionID_Server;                // Wifi Module 網路連線所使用的會話ID
	unsigned int     m_nMode;                            // Wifi連線模式
	unsigned int     m_nResponseMapCnt;                  // HTTP請求與回應對照表計數
	String           m_strComma;                         // 逗號分隔符
	String           m_strCrLn;                          // 回車換行符
	String           m_strQuote;                         // 引號符
	String           m_strServer_IP;                     // 目標服務器的IP
	String           m_strRecv;                          // a string to hold incoming data
	String           m_strWifi_Password;                 // Wifi AP的密碼
	String           m_strWifi_SSID;                     // Wifi AP的SSID
	HardwareSerial&  m_hwSerial;                         // Wifi所使用的序列埠reference
	LPRESPONSEMAP    m_pResponseMap;                     // HTTP請求與回應對照表的指針

// Methods
public:
	WifiCtrl( HardwareSerial& hwSerial, String strWifi_SSID, String strWifi_Password, String strServer_IP, int iServer_Port );
	~WifiCtrl();
	
	virtual bool Close(int iSession);																// 關閉連線會話
	virtual bool Connect(String strIP, int iPort, String strType="TCP");							// 連線到目標服務器
	virtual bool ConnectToAP( String strWifi_SSID="", String strWifi_Password="" );					// 連線到Wifi AP
	virtual bool CreateServer(int iPort);															// 啟動服務器於Port並監聽
	virtual bool GetWifiAPList( String& pWifiList );												// 獲得可連線的Wifi AP列表
	virtual bool HandleHttpRequest(String strRequest);												// 根據獲取的網路數據處理HTTP請求
	virtual bool InitConnection(String strIP, int iPort, String strType="TCP");						// 於Wifi初始化階段對指定的目標服務器連線
	virtual bool InitWifiModule();																	// Wifi模組初始化程序
	virtual bool Recv(String& strRecv, unsigned int nTimeout=TIMEOUT_NORMAL);						// 接收數據
	virtual bool SendData(String strSend, int iSession = -1);										// 發送數據
	virtual bool SetHttpResponseMap( String strUrl, int iStatusCode, ResponseCallback pCallback );	// 設置HTTP請求與回應對照表
	
protected:
	virtual bool SendHttpResponse(int iSession, int iResponseIdx);									// 發送HTTP回應
	
};
#endif
