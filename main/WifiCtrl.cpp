/************************************************************************
 * Title:			      Wifi Module Controller For ESP8266(firmware version 0.9.2.2)
 * Author:          Jordan Yeh
 * Date(YY/MM/DD):	2016/11/13
 ************************************************************************/
// For Arduino 1.0 and earlier
#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
//

#include "WifiCtrl.h"


WifiCtrl::WifiCtrl( HardwareSerial& hwSerial, String strWifi_SSID, String strWifi_Password, String strServer_IP, int iServer_Port ) : 
	m_bInitWifi(false), m_sStatusInitWifi(0), m_iSessionID_Server(0), m_nMode(1), m_nResponseMapCnt(0), 
	m_strComma(","), m_strCrLn("\r\n"), m_strQuote("\""), m_strRecv(""), m_hwSerial(hwSerial), m_pResponseMap(NULL)
{
	if( !::ValidString( strWifi_SSID ) || !::ValidString( strWifi_Password ) || !::ValidString( strServer_IP ) || (iServer_Port <= 0) )
	{
		this->m_strWifi_SSID     = "";
		this->m_strWifi_Password = "";
		this->m_strServer_IP     = "192.168.1.134";
		this->m_iServer_Port     = 5566;
		return;
	}
	
	this->m_strWifi_SSID     = strWifi_SSID;
	this->m_strWifi_Password = strWifi_Password;
	this->m_strServer_IP     = strServer_IP;
	this->m_iServer_Port     = iServer_Port;
}

WifiCtrl::~WifiCtrl()
{
	if( m_pResponseMap )
		delete[] m_pResponseMap;
}

/************************************************************************
 * 函數名稱: InitWifiModule
 * 用途描述: Wifi模組初始化程序
 * 參數說明: 無
 ************************************************************************/
bool WifiCtrl::InitWifiModule()
{
	bool    bCmdHasSent  = false;                                   // 命令是否為已發送並正在等候回覆
	long    lLastChkTime = 0;
	String  strPrint     = "";

	while( !this->m_bInitWifi )
	{
		if( millis() - lLastChkTime > 10000 )                       // 若初始化程序逾時無反應
		{	// 重啟初始化階段
			m_sStatusInitWifi = WIFI_CMD_SETMODE;
			bCmdHasSent     = false;
			m_strRecv         = "";   
		}
    
		// 若Wifi模組有傳來消息，則持續接收
		while( m_hwSerial.available() )
		{
			char chIn = (char)m_hwSerial.read();  // 接收WIFI模組返回的消息

			m_strRecv += chIn;
      
			if (chIn == '\n')
			{ // 若讀取到換行符
#ifdef _DEBUG_
				Serial.print( m_strRecv );  // 將所讀取的消息字串由Serial輸出
#endif
				m_strRecv.replace("\r\n", "");  // 對字串消除所有\r\n字元
        
				if( (m_strRecv == "Link") || (m_strRecv == "Linked") )
				{	// 若返回消息內容為 Link 或 Linked
					m_bInitWifi = true; // 設置狀態:初始化WIFI模組成功
					return true;
				}
				else if( (m_strRecv == "OK") || (m_strRecv == "no change") || (m_strRecv == "link is builded") )
				{	// 若返回消息內容為 OK 或 no change 或 已經處於多重連線型態
					if( m_sStatusInitWifi != WIFI_CMD_CONTOSERVER ) 
					{ // 若當前初始化階段不為"與伺服器連線"
						bCmdHasSent = false;  // 設置命令未發送
						++m_sStatusInitWifi;  // 進入下一個初始化階段
					}
				}
				else if( m_strRecv == "ALREAY CONNECT")
				{	// 若返回消息內容為 ALREAY CONNECT
					this->Close( m_iSessionID_Server );
					delay(100);
					bCmdHasSent = false;  // 設置命令未發送
				}
				else if( (m_strRecv == "ERROR") )
				{	// 若返回消息內容為 ERROR
					bCmdHasSent = false;  // 設置命令未發送
				}
        
				m_strRecv = ""; // 清空接收字串
			}
		}

		// 若命令未發送，則可發送命令；命令已發送，則不進入state machine，等待Wifi模組回應
		if( !bCmdHasSent )
		{	// 根據初始化階段發送不同的命令
			switch( m_sStatusInitWifi )
			{
			case WIFI_CMD_SETMODE:      // 模式設定成 3:Station+AP混合模式
				m_hwSerial.write("AT+CWMODE=3\r\n");
				m_nMode = 3;
#ifdef _DEBUG_
				strPrint = "Step ";
				strPrint.concat( WIFI_CMD_SETMODE );
				strPrint.concat( ":Set mode to \"Station+AP\"" );
				Serial.println( strPrint );
#endif
				bCmdHasSent = true; // 設置命令已發送
				break;
				
			case WIFI_CMD_MULTICON:     // 允許一對多的多重連線
				m_hwSerial.write("AT+CIPMUX=1\r\n");
#ifdef _DEBUG_
				strPrint = "Step ";
				strPrint.concat( WIFI_CMD_MULTICON );
				strPrint.concat( ":Set as allow one to multi connectin" );
				Serial.println( strPrint );
#endif
				bCmdHasSent = true; // 設置命令已發送
				break;
				
			case WIFI_CMD_ONSERVER:     // 啟動服務器監聽Port
				if( !this->CreateServer(80) )
					return false;
#ifdef _DEBUG_
				strPrint = "Step ";
				strPrint.concat( WIFI_CMD_ONSERVER );
				strPrint.concat( ":Start listen port on 80" );
				Serial.println( strPrint );
#endif
				bCmdHasSent = true; // 設置命令已發送
				break;
				
			case WIFI_CMD_CONTOAP:      // 連線到已存在的無線網路
				if( !this->ConnectToAP() )
					return false;
#ifdef _DEBUG_
				strPrint = "Step ";
				strPrint.concat( WIFI_CMD_CONTOAP );
				strPrint.concat( ":Connect to Wifi AP" );
				Serial.println( strPrint );
#endif
				bCmdHasSent = true; // 設置命令已發送
				break;
				
			case WIFI_CMD_CONTOSERVER:  // 讓 ESP8266扮演客戶端，對伺服器進行連線
				if( !this->InitConnection( m_strServer_IP, m_iServer_Port ) )
					return false;
#ifdef _DEBUG_
				strPrint = "Step ";
				strPrint.concat( WIFI_CMD_CONTOSERVER );
				strPrint.concat( ":Connect to server" );
				Serial.println( strPrint );
#endif
				bCmdHasSent = true; // 設置命令已發送
				break;
				
			default:
#ifdef _DEBUG_
				Serial.println("Init Wifi Error...");
#endif
				return false;
			}
			
			if( bCmdHasSent ) // 若設置命令已發送
				lLastChkTime = millis();  // 則啟動逾時計時

			delay(100);
		}
	}
	
	return false;
}

/************************************************************************
 * 函數名稱: SendData
 * 用途描述: 發送數據
 * 參數說明: 
 *           (1)String strSend:  要發送的數據內容
 *           (2)int    iSession: 指定要通信的對象之Session ID
 * 補充說明: 
 *           AT command example:
 *           AT+CIPSEND=<id>,<length> // 傳送訊息
 *           等待出現">"後，才可接著發送消息字串
 ************************************************************************/
bool WifiCtrl::SendData(String strSend, int iSession)
{
	if( !::ValidString( strSend ) )
	{
#ifdef _DEBUG_
		Serial.println( "[SendData] Invalid data" );
#endif
		return false; // 取消發送
	}

	bool   bSuccess = false;
	String strATCmd = "AT+CIPSEND=";
	
	if( -1 == iSession )
		iSession = m_iSessionID_Server;
	
	strATCmd.concat( iSession + m_strComma + (strSend.length()-2) + m_strCrLn );
#ifdef _DEBUG_
	Serial.print( strATCmd );
#endif
	m_hwSerial.print( strATCmd );
	delay(100);

	char strFind[] = ">";
	if( m_hwSerial.find( strFind ) )        // 從WIFI模組返回的消息中查找字串
	{
#ifdef _DEBUG_
		Serial.print( strFind );
		Serial.print( strSend );
#endif
		m_hwSerial.print( strSend );        // 發送數據
		bSuccess = true;
	}
	else
	{
		this->Close( iSession );            // 關閉連線
		bSuccess = false;
	}

	delay(100);
	char strFindOK[] = "OK";
	if( m_hwSerial.find( strFindOK ) )      // 從WIFI模組返回的消息中查找字串
	{
#ifdef _DEBUG_
		Serial.println( "[SendData] OK" );
#endif
		bSuccess = true;
	}
	else
	{
#ifdef _DEBUG_
		Serial.println( "[SendData] Error\n[SendData] Exit SendData" );
#endif
		bSuccess = false;
	}

	return bSuccess;
}

/************************************************************************
 * 函數名稱: Recv
 * 用途描述: 接收數據
 * 參數說明: 
 *           (1)String& strRecv:       儲存接收到的數據內容
 *           (2)unsigned int nTimeout: 指定接收數據的逾時時間長度
 ************************************************************************/
bool WifiCtrl::Recv(String& strRecv, unsigned int nTimeout)
{
	char   cIn          = '\0';
	bool   bRecv        = false;
	long   lLastChkTime = 0;
	String strTmpRecv   = "";    // 宣告一個暫存接收Buffer
	strRecv             = "";    // 清空字串Buffer
	
	while( true )
	{
		if( ( millis() - lLastChkTime ) > nTimeout && bRecv ) // 若接收數據程序逾時無反應
		{
			break;
		}
		
		strTmpRecv = "";
		
		while ( m_hwSerial.available() )          // 若WIFI序列埠緩衝區還有數據
		{
			cIn = (char)m_hwSerial.read();        // get the new byte:
			strTmpRecv += cIn;
		}
		delay(50);                                // 短延遲，防止迴圈太快接收不到數據
		if( strTmpRecv != "")                     // WIFI序列埠接收到數據
		{
			bRecv = true;                         // 接收成功
			lLastChkTime = millis();              // 啟動逾時計時
			strRecv += strTmpRecv;                // 把接收到的數據寫入Buffer
			
			if( strTmpRecv.indexOf( "OK" ) >= 0 || 
				strTmpRecv.indexOf( "ERROR" ) >= 0 || 
				strTmpRecv.indexOf( "Unlink" ) >= 0 ) // 若接收完畢
				break;                            // 退出接收數據程序
		}
		else if( !bRecv )                         // 不曾接收成功
		{
			break;                                // 退出接收數據程序
		}
	}
	
	return bRecv;
}

/************************************************************************
 * 函數名稱: Close
 * 用途描述: 關閉連線會話
 * 參數說明: 
 *           (1)int iSession: 指定要關閉的對象之Session ID
 * 補充說明: 
 *           AT command example:
 *           AT+CIPCLOSE=0 // 中斷指定的連線。適用於 CIPMUX=1 的情況
 ************************************************************************/
bool WifiCtrl::Close(int iSession)
{
	String strATCmd = "AT+CIPCLOSE=";
	strATCmd.concat( iSession + m_strCrLn );
#ifdef _DEBUG_
	Serial.print( strATCmd );
#endif
	m_hwSerial.print( strATCmd ); // 關閉連線
	delay(100);
	char strFindOK[] = "OK";
	if( m_hwSerial.find( strFindOK ) )        // 從WIFI模組返回的消息中查找字串
	{
		delay(100);
		char strFindUnlink[] = "Unlink";
		if( m_hwSerial.find( strFindUnlink ) )  // 從WIFI模組返回的消息中查找字串
		{
#ifdef _DEBUG_
			String str = "[Close] Close Session ";
			str.concat( iSession );
			Serial.println( str );
#endif
			return true;
		}
	}
#ifdef _DEBUG_
	Serial.println( "[Close] Close failed" );
#endif
	return false;
}

/************************************************************************
 * 函數名稱: Connect
 * 用途描述: 連線到目標服務器
 * 參數說明: 
 *           (1)String strIP:   目標IP
 *           (2)int iPort:      目標Port
 *           (3)String strType: 連線型態，TCP或UDP協定
 * 補充說明: 
 *           AT command example:
 *           AT+CIPSTART=0,"TCP","192.168.1.134",8087 // 讓 ESP8266扮演客戶端，對伺服器進行連線
 ************************************************************************/
bool WifiCtrl::Connect(String strIP, int iPort, String strType)
{
	if( !::ValidString( strIP ) || (iPort <= 0))
		return false;                        // 取消連線
	
	String strATCmd = "AT+CIPSTART=";
	strATCmd.concat( m_iSessionID_Server + m_strComma 
		+ m_strQuote + strType + m_strQuote + m_strComma 
		+ m_strQuote + strIP   + m_strQuote + m_strComma + iPort + m_strCrLn );
#ifdef _DEBUG_
	Serial.print( strATCmd );
#endif
	m_hwSerial.print( strATCmd );
	delay(100);
	char strFindOK[] = "OK";
	if( m_hwSerial.find( strFindOK ) )      // 從WIFI模組返回的消息中查找字串
	{
		delay(100);
		char strFindLink[] = "Link";
		if( m_hwSerial.find( strFindLink ) )  // 從WIFI模組返回的消息中查找字串
		{
#ifdef _DEBUG_
			Serial.println( "[Connect to server] Linked" );
#endif
			return true;
		}
	}
#ifdef _DEBUG_
	Serial.println( "[Connect to server] Link Error" );
#endif
	
	return false;
}

/************************************************************************
 * 函數名稱: ConnectToAP
 * 用途描述: 連線到Wifi AP
 * 參數說明: 
 *           (1)String strWifi_SSID:     目標Wifi AP的SSID
 *           (2)String strWifi_Password: 目標Wifi AP的密碼
 * 補充說明: 
 *           AT command example:
 *           AT+CWJAP="D-Link_DIR-809","12345678" // 連線到已存在的無線網路
 ************************************************************************/
bool WifiCtrl::ConnectToAP( String strWifi_SSID, String strWifi_Password )
{
	String strATCmd = "";
	if( !::ValidString( strWifi_SSID ) || !::ValidString( strWifi_Password ) ) // 若傳入的SSID字串或密碼字串為空或不合法
	{	// 使用初始化Wifi所指定的SSID與密碼連線
		if( !::ValidString( this->m_strWifi_SSID ) || !::ValidString( this->m_strWifi_Password ) ) // 若初始化Wifi所指定的SSID字串或密碼字串為空或不合法
			return false; // 取消連線
		
		strATCmd = "AT+CWJAP=\"" + this->m_strWifi_SSID + m_strQuote + m_strComma + m_strQuote + this->m_strWifi_Password + m_strQuote + m_strCrLn;
	}
	else
	{
		strATCmd = "AT+CWJAP=\"" + strWifi_SSID + m_strQuote + m_strComma + m_strQuote + strWifi_Password + m_strQuote + m_strCrLn;
	}
	
	m_hwSerial.print( strATCmd );
	return true;
}

/************************************************************************
 * 函數名稱: InitConnection
 * 用途描述: 於Wifi初始化階段對指定的目標服務器連線
 *           (注意與Connect函數的差異)
 * 參數說明: 
 *           (1)String strIP:   目標IP
 *           (2)int iPort:      目標Port
 *           (3)String strType: 連線型態，TCP或UDP協定
 * 補充說明: 
 *           AT command example:
 *           AT+CIPSTART=0,"TCP","192.168.1.134",8087 // 讓 ESP8266扮演客戶端，對服務器進行連線
 ************************************************************************/
bool WifiCtrl::InitConnection(String strIP, int iPort, String strType)
{
	if( !::ValidString( strIP ) || (iPort <= 0))
		return false;      // 取消連線
	
	String strATCmd = "AT+CIPSTART=";
	strATCmd.concat( m_iSessionID_Server + m_strComma 
		+ m_strQuote + strType + m_strQuote + m_strComma 
		+ m_strQuote + strIP   + m_strQuote + m_strComma + iPort + m_strCrLn );
	m_hwSerial.print( strATCmd );
	
	return true;
}

/************************************************************************
 * 函數名稱: CreateServer
 * 用途描述: 啟動服務器於Port並監聽
 * 參數說明: 
 *           (1)int iPort: 要監聽的Port
 * 補充說明: 
 *           AT command example:
 *           AT+CIPSERVER=1,80 // 讓 ESP8266扮演服務器，等候客戶端的連線
 ************************************************************************/
bool WifiCtrl::CreateServer(int iPort)
{
	if( (iPort <= 0) || (m_nMode < 2) )
		return false;      // 取消連線
	
	String strATCmd = "AT+CIPSERVER=1,";
	strATCmd.concat( iPort + m_strCrLn );
	m_hwSerial.print( strATCmd );
	
	return true;
}

/************************************************************************
 * 函數名稱: HandleHttpRequest
 * 用途描述: 根據獲取的網路數據處理HTTP請求
 * 參數說明: 
 *           (1)String strRequest: 傳入使用Recv函數接收到的數據內容
 ************************************************************************/
bool WifiCtrl::HandleHttpRequest(String strRequest)
{
	if( strRequest.indexOf( ":GET " ) < 0 && strRequest.indexOf( ":POST " ) < 0 )  // 接收到的字串中未包含HTTP Request消息
		return false;
	
	String strUrl = "", strResponse = "";
	int    iUrlIdx       = strRequest.indexOf( "GET " );
	int    iHttpProtoIdx = strRequest.indexOf( " HTTP/" );
	int    iSession      = strRequest.indexOf( "+IPD," );

	if( iUrlIdx < 0 )
		iUrlIdx = strRequest.indexOf( "POST " );

	if( iUrlIdx < 0 || iHttpProtoIdx < 0 || iSession < 0 )               // 接收到的字串中未包含HTTP Request消息
		return false;
	
	iSession += 5;
	iSession = strRequest.substring( iSession, (iSession + 1) ).toInt(); // 獲取與客戶端連線的Session ID
	strUrl   = strRequest.substring( (iUrlIdx + 4) , iHttpProtoIdx );    // 得到客戶端請求的URL字串

	// 從HTTP請求與回應對照表找出對應於客戶端請求的URL之回應，並發送HTTP回應
	int i = 0;
	for( ; i < m_nResponseMapCnt ; ++i )
	{
		if( strUrl == m_pResponseMap[ i ].m_strUrl )
		{
			this->SendHttpResponse( iSession, i );
			break;
		}
	}
	// 若找不到符合請求的回應，則回應自定義的錯誤訊息
	if( i >= m_nResponseMapCnt )
	{
		for( i=0 ; i < m_nResponseMapCnt ; ++i )
		{
			if( 0 == m_pResponseMap[ i ].m_strUrl.length() )
			{				
				this->SendHttpResponse( iSession, i );
				break;
			}
		}
		
		// 若HTTP請求與回應對照表中找不到自定義的錯誤訊息，則採用預設的錯誤訊息
		if( i >= m_nResponseMapCnt )
		{
			strUrl = ""; // 用以暫存HTTP回應內容
			strUrl.concat(
				"<!DOCTYPE HTML>" + m_strCrLn + 
				"<html>" + m_strCrLn + 
				"<head>" + m_strCrLn + 
				"<title>Smart Home Sensor AP Configuration</title>" + m_strCrLn + 
				"</head>" + m_strCrLn + 
				"<body>" + m_strCrLn + 
				"Invalid URL!" + m_strCrLn + 
				"</body>" + m_strCrLn + 
				"</html>" + m_strCrLn );
			
			strResponse.concat( 
				"HTTP/1.1 200 OK" + m_strCrLn + 
				"Content-Type: text/html" + m_strCrLn + 
				"Content-Length: " + strUrl.length() + m_strCrLn + 
				"Connection: close" + m_strCrLn + 
				m_strCrLn + 
				strUrl + m_strCrLn ); // 注意最後要多加一個CRLF

			this->SendData( strResponse, iSession );
		}
	}
	
	//this->Close( iSession );
	
	return true;
}

/************************************************************************
 * 函數名稱: SendHttpResponse
 * 用途描述: 發送HTTP回應
 * 參數說明: 
 *           (1)int iSession:     指定要通信的對象之Session ID
 *           (2)int iResponseIdx: 要發送的HTTP請求與回應對照表中的回應之索引
 ************************************************************************/
bool WifiCtrl::SendHttpResponse(int iSession, int iResponseIdx)
{
	String strResponse = "HTTP/1.1 ";
	
	if( 200 == m_pResponseMap[ iResponseIdx ].m_iStatusCode )
	{
		strResponse.concat( "200 OK" + m_strCrLn );
		
		if( NULL == m_pResponseMap[ iResponseIdx ].m_pResponse )
			return false;
	
		String strContent = m_pResponseMap[ iResponseIdx ].m_pResponse(); // 以Callback函數取得HTTP回應內容

		strResponse.concat( 
			"Content-Type: text/html" + m_strCrLn + 
			"Content-Length: " + strContent.length() + m_strCrLn + 
			"Connection: close" + m_strCrLn + 
			m_strCrLn + 
		  strContent + m_strCrLn ); // 注意最後要多加一個CRLF
	}
	else if( 404 == m_pResponseMap[ iResponseIdx ].m_iStatusCode )
	{
		strResponse.concat( "404 " + m_strCrLn + m_strCrLn );
	}

	this->SendData( strResponse, iSession );
	
	return true;
}

/************************************************************************
 * 函數名稱: SetHttpResponseMap
 * 用途描述: 設置HTTP請求與回應對照表
 * 參數說明: 
 *           (1)String strUrl:             請求的URL
 *           (2)int iStatusCode:           要回應的HTTP狀態碼
 *           (3)ResponseCallback callback: 生成回應內容的Callback函數
 ************************************************************************/
bool WifiCtrl::SetHttpResponseMap( String strUrl, int iStatusCode, ResponseCallback pCallback )
{
	LPRESPONSEMAP pNewMap = new RESPONSEMAP[ ++m_nResponseMapCnt ];
	if( m_nResponseMapCnt > 1)
	{
		for( int i = 0 ; i < (m_nResponseMapCnt - 1) ; ++i)
		{
			pNewMap[i] = this->m_pResponseMap[i];
		}
		delete[] this->m_pResponseMap;
	}
	this->m_pResponseMap = pNewMap;
	this->m_pResponseMap[ m_nResponseMapCnt-1 ].m_strUrl      = strUrl;
	this->m_pResponseMap[ m_nResponseMapCnt-1 ].m_iStatusCode = iStatusCode;
	this->m_pResponseMap[ m_nResponseMapCnt-1 ].m_pResponse   = pCallback;
}

/************************************************************************
 * 函數名稱: GetWifiAPList
 * 用途描述: 獲得可連線的Wifi AP列表
 * 參數說明: 
 *           (1)String& strWifiList: 儲存接收到的Wifi AP列表
 ************************************************************************/
bool WifiCtrl::GetWifiAPList( String& strWifiList )
{
	bool   bMode      = true;
  bool   bHtmlHead  = false;  // Has generated HTML head and body tags
  char   cEnc       = 0;
	int    iEnc       = 0;
  int    iRSSI      = 0;
	int    iHeadIdx   = -1;
	int    iTailIdx   = -1;
	int    iQuote1Idx = -1;
	int    iQuote2Idx = -1;
	String strSSID    = "";
  String strMAC     = "";
  String strChannel = "";
	String strATCmd   = "AT+CWLAP\r\n";
	String strTmpLine = "";
  String arrEnc[]   = { "OPEN", "WEP", "WPA-PSK(TKIP)", "WPA2-PSK(AES)", "WPA-WPA2-PSK", "WPA2_Enterprise(Not support)" }; 
	
	m_hwSerial.print( strATCmd );              // 發送獲取Wifi AP列表命令
#ifdef _DEBUG_
	Serial.println( "[GetWifiAPList] Send Command: AT+CWLAP" );
#endif
	
  strATCmd = "";

	if( this->Recv( strATCmd, TIMEOUT_LONG ) ) // 接收WIFI模組返回的數據
	{
#ifdef _DEBUG_
	  Serial.println( strATCmd );
#endif

		// 處理接收到的字串
		while( true )
		{
			iHeadIdx   = strATCmd.indexOf( "+CWLAP:(", ( iTailIdx + 1 ) );
			iTailIdx   = strATCmd.indexOf( ")", ( iTailIdx + 1 ) );
      
			if( iHeadIdx < 0 || iTailIdx < 0 )
				break;
			
			// 取得單筆WIFI AP數據
			strTmpLine = strATCmd.substring( ( iHeadIdx + 8 ), iTailIdx );

			// [1] 獲取加密方式
      cEnc = strTmpLine.charAt( 0 );
			iEnc = ::atoi( &cEnc );
      if( iEnc < 0 || iEnc > 5 )
        iEnc = 0;
      
			// 搜尋SSID之前後雙引號索引
			iQuote1Idx = strTmpLine.indexOf( "\"", 2 );
			iQuote2Idx = strTmpLine.indexOf( "\"", 3 );
			// [2] 獲取SSID
			strSSID    = strTmpLine.substring( ( iQuote1Idx + 1 ), iQuote2Idx );
      
			// 搜尋訊號強度之逗號索引
			iQuote1Idx = iQuote2Idx; // 用iQuote1Idx暫存後引號之索引
			iQuote2Idx = strTmpLine.indexOf( ",", ( iQuote1Idx + 2 ) ); // 取得後逗號索引
			// [3] 獲取訊號強度
			iRSSI      = strTmpLine.substring( ( iQuote1Idx + 2 ), iQuote2Idx ).toInt();
      
      // 搜尋MAC地址之前後雙引號索引
			iQuote1Idx = strTmpLine.indexOf( "\"", ( iQuote2Idx + 1 ) );
			iQuote2Idx = strTmpLine.indexOf( "\"", ( iQuote1Idx + 1 ) );
			// [4] 獲取MAC地址
      strMAC     = strTmpLine.substring( ( iQuote1Idx + 1 ), iQuote2Idx );

      // 搜尋頻道之逗號索引
      iQuote1Idx = iQuote2Idx; // 用iQuote1Idx暫存後引號之索引
      iQuote2Idx = strTmpLine.length();
      // [5] 獲取頻道
      strChannel = strTmpLine.substring( ( iQuote1Idx + 2 ), iQuote2Idx );

      // 將資訊寫入Wifi AP列表
      if( !bHtmlHead )
      {
        strWifiList += "<!DOCTYPE HTML>";
        strWifiList += "<html>";
        strWifiList += "<head>";
        strWifiList += "<meta charset=\"utf-8\" />\r\n";
        strWifiList += "<meta name=\"viewport\" content=\"width=device-width, user-scalable=yes, initial-scale=1\">\r\n";
        strWifiList += "<title>Smart Home Sensor Network Configuration</title>\r\n";
        strWifiList += "<style type=\"text/css\"> td{width: 176.812px; text-align: center;} table{border-collapse: collapse; width: 100%; border: 1;}</style>\r\n";
        strWifiList += "</head>";
        strWifiList += "<body>";
        /*
        strWifiList += "<table>";
        strWifiList += "<tbody>";
        strWifiList += "<tr>";
        strWifiList += "<td>Encryption</td>";
        strWifiList += "<td>SSID</td>";
        strWifiList += "<td>Signal RSSI</td>";
        strWifiList += "<td>MAC Address</td>";
        strWifiList += "<td>Channel</td>";
        strWifiList += "</tr>\r\n";
        */

        bHtmlHead = true;
      }
      /*
      strWifiList += "<tr>";
      strWifiList += "<td>" + arrEnc[ iEnc ] + "</td>";
      strWifiList += "<td>" + strSSID +"</td>";
      if( iRSSI > -50 )
        strWifiList += "<td>|||||</td>";
      else if( iRSSI > -60 )
        strWifiList += "<td>||||</td>";
      else if( iRSSI > -70 )
        strWifiList += "<td>|||</td>";
      else if( iRSSI > -80 )
        strWifiList += "<td>||</td>";
      else
        strWifiList += "<td>|</td>";
      strWifiList += "<td>" + strMAC + "</td>";
      strWifiList += "<td>" + strChannel + "</td>";
      strWifiList += "</tr>\r\n";
      */

      if( iRSSI > -50 )
        strWifiList += "|||||";
      else if( iRSSI > -60 )
        strWifiList += "||||&nbsp;";
      else if( iRSSI > -70 )
        strWifiList += "|||&nbsp;&nbsp;";
      else if( iRSSI > -80 )
        strWifiList += "||&nbsp;&nbsp;&nbsp;";
      else
        strWifiList += "|&nbsp;&nbsp;&nbsp;&nbsp;";
      strWifiList += ", ";
      strWifiList += strSSID;
      strWifiList += "<br />";
		}

    if( bHtmlHead )
    {
      /*
      strWifiList += "</tbody>";
      strWifiList += "</table>";
      */
      strWifiList += "</body>";
      strWifiList += "</html>\r\n";      
    }

		return true;
	}

	return false; // 接收失敗
}
