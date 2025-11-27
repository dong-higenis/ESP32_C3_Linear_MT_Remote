/**
*@brief SERIAL: 0번 채널 (PC <-> ESP) 의 UART: 를 115200의 Baudrate속도로 초기화 및 시작
*/
void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
}

/**
*@brief 500ms마다 test를 Serial Monitor에 출력한다.
*/
void loop() 
{
  // put your main code here, to run repeatedly:
  static uint32_t serial_timer = 0;
  
  if(millis() - serial_timer >= 500)
  {
    serial_timer = millis();
    serialPrint("test\n");
  }
}
