#include "WifiCtrl.h"
#include "SevenSegmentCtrl.h"

#define RECV_MAX_LEN         512
#define BAUDRATE             9600
#define WIFI_SERIAL          Serial1
#define WIFI_SSID            "HITRON-86E0"
#define WIFI_PASSWORD        "12345678"
#define SERVER_IP            "192.168.1.134"
#define SERVER_PORT          1000

int               n=0;
long				      g_lLastConnTime = 0; 
String				    g_strRecv       = "";     // a string to hold incoming data
WifiCtrl			    g_Wifi( WIFI_SERIAL, WIFI_SSID, WIFI_PASSWORD, SERVER_IP, SERVER_PORT);
SevenSegmentCtrl	g_7SegCtrl;

void setup()
{
  // initialize serial
  Serial.begin(BAUDRATE);
  Serial1.begin(BAUDRATE);
  //
  g_strRecv.reserve(RECV_MAX_LEN);
  //
  Serial.println("Init Wifi ...");
  // 設置HTTP請求與回應對照表
  g_Wifi.SetHttpResponseMap( "/", 200, HttpRespondHomeUrl );
  g_Wifi.SetHttpResponseMap( "/favicon.ico", 404, NULL );
  g_Wifi.SetHttpResponseMap( "", 200, HttpRespondInvalidUrl ); // url為空，代表自定義的錯誤訊息
  g_Wifi.SetHttpResponseMap( "/wifilist", 200, HttpRespondWifiList );
  
  // Initial wifi
  delay(2000);
  if( g_Wifi.InitWifiModule() )
    Serial.println("Init Wifi Success!");
  else
    Serial.println("Init Wifi Failed!");

  // Initial 7 Segment display
  g_7SegCtrl.Init();
}

void loop()
{
  /*while(1)
  {
    g_7SegCtrl.SetNumber( n ); // 於七段顯示器顯示數字
    //g_7SegCtrl.SetString("S8YU");

    if(millis() - g_lLastConnTime < 5000)
    {
      //delay(1000);
      continue;
    }

    if( !SendData( "Hi!~\r\n" ) )                // 發送數據
    { // 發送失敗
      g_7SegCtrl.Empty();                        // 清空七段顯示器
      delay(5000);
      g_Wifi.Connect( SERVER_IP, SERVER_PORT );  // 嘗試重新連線伺服器以防斷線
      break;
    }
    
    ++n;
    g_lLastConnTime = millis();
  }*/

  // TODO: 以WIFI模組接收網路消息並傳給Arduino
  if( g_Wifi.Recv( g_strRecv ) ) // 接收WIFI模組返回的數據
  {
    Serial.print( g_strRecv );   // 將WIFI模組傳來的數據由Serial輸出 (顯示WIFI模組返回內容)
	
  	if( !g_Wifi.HandleHttpRequest( g_strRecv ) ) // 根據獲取的網路數據處理HTTP請求
  	{ // 若不為HTTP請求
        // 處理硬體控制命令
  	}
  }
  
  
  // TODO: 將WIFI模組傳來的數據由Serial輸出 (顯示WIFI模組返回內容)
  /*while (Serial1.available()) {
      Serial.write(Serial1.read());
  }*/

  // TODO: 將輸入Serial的數據輸入到Serial1 (將鍵盤輸入的指令輸入到WIFI模組)
  while (Serial.available())
  {
      Serial1.write(Serial.read());
  }
  
}

/*
 * 以WIFI模組發送數據
 */
boolean SendData( String strSend )
{
  g_7SegCtrl.Empty(); // 清空七段顯示器，否則上一次所顯示的最後一位顯示器會持續點亮
  if( !g_Wifi.SendData( "Hi!~\r\n" ) )
    return false;     // 發送失敗
  return true;
}

String HttpRespondInvalidUrl()
{
  String strContent = "";
  strContent += "<!DOCTYPE HTML>\r\n";
  strContent += "<html>\r\n";
  strContent += "<head>\r\n";
  strContent += "<meta charset=\"utf-8\" />\r\n";
  strContent += "<meta name=\"viewport\" content=\"width=device-width, user-scalable=yes, initial-scale=1\">\r\n";
  strContent += "<title>Smart Home Sensor Network Configuration</title>\r\n"; 
  strContent += "</head>\r\n";
  strContent += "<body>\r\n";
  strContent += "Invalid URL!\r\n";
  strContent += "</body>\r\n";
  strContent += "</html>\r\n";

  return strContent;
}

String HttpRespondHomeUrl()
{
  String strContent = "";
  strContent += "<!DOCTYPE HTML>\r\n";
  strContent += "<html>\r\n";
  strContent += "<head>\r\n";
  strContent += "<meta charset=\"utf-8\" />\r\n";
  strContent += "<meta name=\"viewport\" content=\"width=device-width, user-scalable=yes, initial-scale=1\">\r\n";
  strContent += "<title>Smart Home Sensor Network Configuration</title>\r\n"; 
  strContent += "</head>\r\n";
  strContent += "<body>\r\n";
  strContent += "H o m e  P a g e\r\n";
  strContent += "</body>\r\n";
  strContent += "</html>\r\n";

  return strContent;
}

String HttpRespondWifiList()
{
  String strContent = "";

  g_Wifi.GetWifiAPList( strContent );
  
  return strContent;
}
