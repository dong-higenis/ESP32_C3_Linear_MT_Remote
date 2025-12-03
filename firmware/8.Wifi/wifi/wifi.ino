
/**
* @note 아래 라이브러리들은 ESP32 보드 매니저 설치 시, 자동으로 포함되는 내장 라이브러리임.
*/
#include <WiFi.h>

/**
* @brief AP 설정
*/
const char* ssid = "ESP32-C3-Linear-MT-board";
const char* password = "";  // 오픈 네트워크로 테스트

/**
 * @brief HTTP 웹서버 포트 설정
 * @details 80번 포트는 HTTP 기본 포트로, 브라우저에서 IP 주소만 입력해도 접속 가능
 */
WiFiServer server(80);

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  wifiSetup();
}

void loop() 
{
  // put your main code here, to run repeatedly:
  wifiTask();
}

/**
 * @brief WiFi AP 모드 초기화 및 웹서버 시작
 * @details ESP32를 AP(Access Point) 모드로 설정하여 자체 WiFi 네트워크 생성
 *          기본 IP 주소는 192.168.4.1로 자동 할당됨
 */
void wifiSetup(void)
{
    // 디버그 메시지 추가
  Serial.println("WiFi AP Starting...");
  
  if (WiFi.softAP(ssid, password)) 
  {
    Serial.println("AP Started Successfully!");

    IPAddress IP = WiFi.softAPIP();

    Serial.print("AP IP address: ");
    Serial.println(IP);
  } 
  else 
  {
    Serial.println("AP Start FAILED!");
  }
  
  server.begin();
  Serial.println("HTTP server started");
}

/**
 * @brief 클라이언트 연결 처리 및 HTTP 응답 생성
 * @details 클라이언트(브라우저)가 연결되면 HTTP 요청을 읽고 HTML 페이지를 전송
 *          loop()에서 계속 호출되어 새로운 연결을 감지함
 */
void wifiTask(void)
{
  // 새로운 클라이언트 연결 대기 (연결 없으면 즉시 리턴)
  WiFiClient client = server.available();

  if (client) {
    Serial.println("New Client connected");
    String currentLine = "";
    
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        
        if (c == '\n') {
          if (currentLine.length() == 0) 
          {
            // HTTP 응답 헤더 전송 (한글을 위해 UTF-8 사용)
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html; charset=utf-8");
            client.println("Connection: close");
            client.println();
            
            // HTML 웹페이지 (HEAD) 
            client.println("<!DOCTYPE html>");
            client.println("<html lang='ko'>");
            client.println("<head>");
            client.println("<meta charset='UTF-8'>");
            client.println("<meta name='viewport' content='width=device-width, initial-scale=1.0'>");
            client.println("<title>Welcome ESP32_C3_Linear_MT_Remote 웹 서버</title>");

            // CSS 스타일 정의
            client.println("<style>");
            client.println("body { font-family: Arial, sans-serif; margin: 40px; background-color: #f0f0f0; }");
            client.println(".container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 0 10px rgba(0,0,0,0.1); }");
            client.println("h1 { color: #333; text-align: center; }");
            client.println("p { font-size: 18px; line-height: 1.6; }");
            client.println(".info { background-color: #e7f3ff; padding: 15px; border-radius: 5px; margin: 20px 0; }");
            client.println("</style>");
            client.println("</head>");

            // HTML BODY
            client.println("<body>");
            client.println("<div class='container'>");
            client.println("<h1>웹서버 연결 성공!</h1>");
            client.println("<p>안녕하세요! ESP32_C3_Linear_MT_Remote에서 실행중인 웹서버입니다.</p>");
            client.println("<div class='info'>");
            client.println("<h3>연결 정보</h3>");
            client.println("<p><strong>네트워크:</strong> " + String(ssid) + "</p>");
            client.println("<p><strong>IP 주소:</strong> " + WiFi.softAPIP().toString() + "</p>");
            client.println("<p><strong>연결된 시간:</strong> " + String(millis()/1000) + "초</p>");
            client.println("</div>");
            client.println("<p>이 페이지는 ESP32가 AP 모드로 동작하며 핸드폰에서 직접 접속할 수 있습니다.</p>");
            client.println("<p>추가 기능이 필요하시면 코드를 수정해서 사용하세요!</p>");
            client.println("</div>");
            client.println("</body>");
            client.println("</html>");
            
            break;
          } 
          else 
          {
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {
          currentLine += c;
        }
      }
    }
    
    // 클라이언트 연결 종료
    client.stop();
    Serial.println("Client disconnected");
  }
}