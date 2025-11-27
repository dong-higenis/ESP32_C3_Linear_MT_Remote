
/**
* @note 아래 라이브러리들은 ESP32 보드 매니저 설치 시, 자동으로 포함되는 내장 라이브러리임.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEClient.h>

/**
* @brief Pin 번호
*/
#define BTN1_PIN 0
#define BTN2_PIN 4
#define MOTOR_IN1_PIN 18
#define MOTOR_IN2_PIN 19
#define RF_DAT_PIN 10

/**
* @brief 모터 게인 값
*/
#define MOTOR_GAIN 0.554f

/**
* @brief 모터 속도 정의
*/
#define MOTOR_MIN_SPEED 255
#define MOTOR_STOP_SPEED 0 

/**
* @pClient: BLE 클라이언트 객체 포인터
* @pServer: BLE 서버 객체 포인터
* @pCharacteristic: BLE 특성 객체 포인터 (데이터 송신)
* @pReceiveCharacteristic: BLE 특성 객체 포인터 (데이터 수신)
* 그외 연결상태 플래그
*/
BLEClient* pClient;
BLEServer* pServer;
BLECharacteristic* pCharacteristic;
BLECharacteristic* pReceiveCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

bool btn1_state = HIGH;
bool btn2_state = HIGH;

/**
* @brief 이벤트 기반 핸들러 ( 콜백 이용 ), 데이터 수신, 송신시 발생하는 이벤트를 이용하였음.
*        실제 BLE로 입력할 데이터를 정하여 입력해야함. 
*/
class MyCallbacks: public BLECharacteristicCallbacks 
{
  void onWrite(BLECharacteristic *pCharacteristic) 
  {
    String value = pCharacteristic->getValue();
    
    // 수신한 데이터 출력
    Serial.print("[BLE] Received Value: ");
    Serial.println(value);
    Serial.print("[BLE] Value Length: ");
    Serial.println(value.length());

    std::string stdValue(value.c_str());
    
    if (stdValue.compare("1") == 0) // Custom value
    {
      ledcWrite(MOTOR_IN1_PIN, MOTOR_MIN_SPEED);
      ledcWrite(MOTOR_IN2_PIN, MOTOR_STOP_SPEED);
      Serial.println("[MOTOR] Forward - IN1: ON, IN2: OFF");
    } 
    else if (stdValue.compare("2") == 0) // Custom value
    {
      ledcWrite(MOTOR_IN1_PIN, MOTOR_STOP_SPEED);
      ledcWrite(MOTOR_IN2_PIN, MOTOR_MIN_SPEED);
      Serial.println("[MOTOR] Reverse - IN1: OFF, IN2: ON");
    } 
    else if (stdValue.compare("3") == 0) // Custom value
    {
      ledcWrite(MOTOR_IN1_PIN, MOTOR_STOP_SPEED);
      ledcWrite(MOTOR_IN2_PIN, MOTOR_STOP_SPEED);
      Serial.println("[MOTOR] Stop - IN1: OFF, IN2: OFF");
    }
    else
    {
      Serial.print("[BLE] Unknown command: ");
      Serial.println(value);
    }
  }
};

/**
* @brief 클라이언트가 연결되면 자동으로 발생하는 이벤트를 이용함.
* startAdvertising() 함수를 이용하여 주변 기기에 ESP정보를 뿌려 연결가능하게 함.
*/
class MyServerCallbacks: public BLEServerCallbacks 
{
  void onConnect(BLEServer* pServer) 
  {
    deviceConnected = true;
    Serial.println("[BLE] Device Connected!");
    BLEDevice::startAdvertising();
  }

  void onDisconnect(BLEServer* pServer) 
  {
    deviceConnected = false;
    Serial.println("[BLE] Device Disconnected!");
  }
};

void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  btnSetup();
  motorSetup();
  bleSetup();
}

void loop() 
{
  // put your main code here, to run repeatedly:
  btnTask();
  motorTask();
}

/**
* @brief BLE 디바이스 초기화(init("ESP_NAME")) 이후 서버 객체 및 콜백을 등록,
* 그 외는 표준서비스나 특성등을 등록하며, 표준 프로필을 사용함.
* 클라이언트 객체도 생성했으며, 필요시 활용 가능하다.
* @details 클라이언트가 연결되거나, 명령을 전송하거나 등등 각 콜백이 자동으로 호출되는 이벤트 기반 구조라고 할 수 있음.
*/
void bleSetup(void)
{
  BLEDevice::init("MyESP32");
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService("0000180d-0000-1000-8000-00805f9b34fb");
  pCharacteristic = pService->createCharacteristic(
      "00002a37-0000-1000-8000-00805f9b34fb",
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );

  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setCallbacks(new MyCallbacks());

  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  
  pClient = BLEDevice::createClient();
}

void btnSetup(void)
{
  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);
}

void btnTask(void)
{
  btn1_state = digitalRead(BTN1_PIN);
  btn2_state = digitalRead(BTN2_PIN);
}

void motorSetup(void)
{
  pinMode(MOTOR_IN1_PIN, OUTPUT);
  pinMode(MOTOR_IN2_PIN, OUTPUT);

  ledcAttach(MOTOR_IN1_PIN, 5000, 8);
  ledcAttach(MOTOR_IN2_PIN, 5000, 8);
}

void motorTask(void)
{
  if (!btn1_state && btn2_state) 
  {
    ledcWrite(MOTOR_IN1_PIN, MOTOR_MIN_SPEED); 
    ledcWrite(MOTOR_IN2_PIN, MOTOR_STOP_SPEED); 
  } 
  else if (!btn2_state && btn1_state) 
  {
    ledcWrite(MOTOR_IN1_PIN, MOTOR_STOP_SPEED);
    ledcWrite(MOTOR_IN2_PIN, MOTOR_MIN_SPEED); 
  } 
  else if (btn1_state && btn2_state)
  {
    ledcWrite(MOTOR_IN1_PIN, MOTOR_STOP_SPEED);
    ledcWrite(MOTOR_IN2_PIN, MOTOR_STOP_SPEED);
  }
}