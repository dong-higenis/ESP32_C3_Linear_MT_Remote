#include "remote_wifi.h"

WiFiServer server(80);

/**
* @brief wifi ap설정
*/
const char* ssid = "ESP32-C3-Linear-MT-board";
const char* password = "";  // 오픈 네트워크로 테스트

void wifiSetup(void)
{
  if (WiFi.softAP(ssid, password)) 
  {
    IPAddress IP = WiFi.softAPIP();
  } 
  else 
  {
    Serial.println("AP Start FAILED!");
  }
  
  server.begin();
}

btn_decoded_t wifiTask(void)
{
  WiFiClient client = server.available();
  btn_decoded_t which_web_btn_accessed = BTN_NONE;
  String header = "";

  if (client) 
  {
    String currentLine = "";

    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        
        header += c;

        if (c == '\n') 
        {
          if (currentLine.length() == 0)
          {
            if (header.indexOf("GET /motor_up") >= 0)
            {
              which_web_btn_accessed = BTN_A_PRESSED;
            }
            else if (header.indexOf("GET /motor_down") >= 0)
            {
              which_web_btn_accessed = BTN_B_PRESSED;
            }
            else if (header.indexOf("GET /motor_lock") >= 0)
            {
              which_web_btn_accessed = BTN_C_PRESSED;
            }
            else if (header.indexOf("GET /motor_stop") >= 0)
            {
              which_web_btn_accessed = BTN_D_PRESSED;
            }

            httpMakeWeb(client, ssid);
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

    // client.stop();
  }
  
  return which_web_btn_accessed;
}

/**
 * @brief 모터 리모컨 웹페이지 전송
 * @details 디자인/HTML만 담당, 로직은 wifiTask()에서 분리
 */
void httpMakeWeb(WiFiClient &client, const char* ssid)
{
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
  client.println("<title>ESP32_C3_Linear_MT_Remote 웹 서버</title>");

  // CSS 스타일 정의
  client.println("<style>");
  client.println("body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif;");
  client.println("      margin: 0; padding: 0; background: linear-gradient(135deg, #1e3c72, #2a5298); }");
  client.println(".container { max-width: 700px; margin: 40px auto; background: #ffffff;");
  client.println("            padding: 24px 20px 28px 20px; border-radius: 16px;");
  client.println("            box-shadow: 0 10px 25px rgba(0,0,0,0.25); }");
  client.println(".title { text-align: center; margin-bottom: 8px; color: #222; }");
  client.println(".subtitle { text-align: center; color: #666; font-size: 14px; margin-bottom: 18px; }");
  client.println(".info { background-color: #f3f7ff; padding: 14px 16px; border-radius: 10px;");
  client.println("        margin: 16px 0 22px 0; border: 1px solid #d0ddff; font-size: 14px; }");
  client.println(".info h3 { margin: 0 0 6px 0; color: #2a4fa3; font-size: 15px; }");
  client.println(".info p { margin: 2px 0; }");
  client.println(".section-title { margin: 6px 0 12px 0; font-size: 15px; color: #333; font-weight: 600; }");
  client.println(".btn-grid { display: grid; grid-template-columns: repeat(2, minmax(0, 1fr));");
  client.println("            gap: 12px; }");
  client.println(".btn { display: flex; align-items: center; justify-content: center;");
  client.println("       padding: 16px 10px; border-radius: 12px; border: none;");
  client.println("       font-size: 16px; font-weight: 600; letter-spacing: 0.5px;");
  client.println("       color: #fff; text-decoration: none; cX`ursor: pointer;");
  client.println("       box-shadow: 0 4px 10px rgba(0,0,0,0.18);");
  client.println("       transition: transform 0.08s ease, box-shadow 0.08s ease, filter 0.12s; }");
  client.println(".btn span { margin-left: 6px; }");
  client.println(".btn-up { background: linear-gradient(135deg, #00b09b, #96c93d); }");
  client.println(".btn-down { background: linear-gradient(135deg, #ff512f, #dd2476); }");
  client.println(".btn-lock { background: linear-gradient(135deg, #2c3e50, #4ca1af); }");
  client.println(".btn-stop { background: linear-gradient(135deg, #606c88, #3f4c6b); }");
  client.println(".btn:active { transform: translateY(1px) scale(0.98);");
  client.println("             box-shadow: 0 2px 6px rgba(0,0,0,0.25); filter: brightness(0.96); }");
  client.println(".footer { margin-top: 18px; text-align: center; font-size: 12px; color: #888; }");
  client.println("@media (max-width: 480px) {");
  client.println("  .container { margin: 16px; padding: 18px 14px 22px 14px; }");
  client.println("  .btn { padding: 14px 8px; font-size: 15px; }");
  client.println("}");
  client.println("</style>");
  client.println("</head>");

  // HTML BODY
  client.println("<body>");
  client.println("<div class='container'>");
  client.println("<h1 class='title'>ESP32_C3 모터 리모컨</h1>");
  client.println("<p class='subtitle'>아래 버튼을 눌러 모터를 제어하세요.</p>");

  // 연결 정보
  client.println("<div class='info'>");
  client.println("<h3>연결 정보</h3>");
  client.println("<p><strong>네트워크:</strong> " + String(ssid) + "</p>");
  client.println("<p><strong>IP 주소:</strong> " + WiFi.softAPIP().toString() + "</p>");
  client.println("<p><strong>연결된 시간:</strong> " + String(millis()/1000) + "초</p>");
  client.println("</div>");

  // 버튼 섹션
  client.println("<p class='section-title'>모터 제어</p>");
  client.println("<div class='btn-grid'>");
  client.println("  <a href='/motor_up' class='btn btn-up'>");
  client.println("    ⬆️<span>MOTOR_UP</span>");
  client.println("  </a>");
  client.println("  <a href='/motor_down' class='btn btn-down'>");
  client.println("    ⬇️<span>MOTOR_DOWN</span>");
  client.println("  </a>");
  client.println("  <a href='/motor_lock' class='btn btn-lock'>");
  client.println("    🔒<span>MOTOR_LOCK</span>");
  client.println("  </a>");
  client.println("  <a href='/motor_stop' class='btn btn-stop'>");
  client.println("    ⏹️<span>MOTOR_STOP</span>");
  client.println("  </a>");
  client.println("</div>");

  client.println("<div class='footer'>");
  client.println("로컬 AP 모드로 동작 중이며 인터넷 연결은 제공하지 않습니다.");
  client.println("</div>");

  client.println("</div>");
  client.println("</body>");
  client.println("</html>");
}