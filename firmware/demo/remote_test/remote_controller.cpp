#include "remote_controller.h"

uint32_t remote_addresses[MAX_REMOTE_CONTROLLER_NUM] = {0};  
uint32_t remote_buttons[MAX_REMOTE_CONTROLLER_NUM][MAX_BTN_NUM] = {0}; 

Preferences preferences;

int remote_count = 0;

void remoteSetup(void)
{
  loadAllRemotes();
}

void loadAllRemotes(void) 
{
  Serial.println("\n=== loadAllRemotes() 시작 ===");
  remote_count = 0;
  
  for (int slot = 1; slot <= MAX_REMOTE_CONTROLLER_NUM; slot++) 
  {
    char namespace_name[20];
    sprintf(namespace_name, "remote_%d", slot);
    
    Serial.printf("[로드] 슬롯 %d (%s) 확인 중...\n", slot, namespace_name);
    
    bool opened = preferences.begin(namespace_name, true);
    Serial.printf("  preferences.begin() 결과: %s\n", opened ? "성공" : "실패");
    
    uint32_t address = preferences.getUInt("address", 0);
    Serial.printf("  읽은 address: 0x%08X\n", address);
    
    if (address == 0) 
    {
      Serial.printf("  -> 빈 슬롯 (address==0), 로드 중단\n");
      preferences.end();
      break;
    }
    
    // 리모컨 데이터 로드
    remote_addresses[slot-1] = address;
    remote_buttons[slot-1][0] = preferences.getUInt("up_btn", 0);
    remote_buttons[slot-1][1] = preferences.getUInt("down_btn", 0);
    remote_buttons[slot-1][2] = preferences.getUInt("lock_btn", 0);
    remote_buttons[slot-1][3] = preferences.getUInt("stop_btn", 0);
    
    preferences.end();
    
    remote_count = slot;
    
    Serial.printf("  -> 슬롯 %d 로드 완료: 주소 0x%08X\n", slot, address);
  }
  
  Serial.printf("=== 총 %d개 리모컨 로드 완료 ===\n\n", remote_count);
}

/**
* @brief 새 리모컨 학습
**/
uint8_t learnNewRemote(uint32_t code) 
{
  uint32_t address = 0;

  bool is_A_type = false;
  bool is_B_type = false;

  if(code >= 0x23900000)
  {
    is_A_type = true;
  }
  else
  {
    is_B_type = true;
  }

  
  if (remote_count >= MAX_REMOTE_CONTROLLER_NUM) 
  {
    Serial.println("리모컨 슬롯 가득 참");
    return RF_MEMORY_FULL;
  }
  
  if(is_A_type)
  {
    address = code & 0xFFFFF0FF;
  }
  else if(is_B_type)
  {
    address = code & 0xFFFFFFF0;
  }

  for (int i = 0; i < remote_count; i++) 
  {
    if (remote_addresses[i] == address) 
    {
      Serial.printf("이미 슬롯 %d에 등록된 리모컨\n", i + 1);
      return RF_ALREADY_EXIST;
    }
  }
  
  int new_slot = remote_count + 1;
  char namespace_name[20];
  sprintf(namespace_name, "remote_%d", new_slot);
  
        
  preferences.begin(namespace_name, false);

  if (is_A_type)
  {
    // NVS 저장
    preferences.putUInt("address", address);
    preferences.putUInt("up_btn", address | (ATYPE_REMOTE_UP_BTN << 8));
    preferences.putUInt("down_btn", address | (ATYPE_REMOTE_DOWN_BTN << 8));
    preferences.putUInt("lock_btn", address | (ATYPE_REMOTE_LOCK_BTN << 8));
    preferences.putUInt("stop_btn", address | (ATYPE_REMOTE_STOP_BTN << 8));
    
    // RAM 저장
    remote_addresses[new_slot-1] = address;
    remote_buttons[new_slot-1][0] = address | (ATYPE_REMOTE_UP_BTN << 8);
    remote_buttons[new_slot-1][1] = address | (ATYPE_REMOTE_DOWN_BTN << 8);
    remote_buttons[new_slot-1][2] = address | (ATYPE_REMOTE_LOCK_BTN << 8);
    remote_buttons[new_slot-1][3] = address | (ATYPE_REMOTE_STOP_BTN << 8);

    is_A_type = false;
  }
  else if(is_B_type)
  {
    // NVS 저장
    preferences.putUInt("address", address);
    preferences.putUInt("up_btn", address | BTYPE_REMOTE_A_BTN); 
    preferences.putUInt("down_btn", address | BTYPE_REMOTE_B_BTN);
    preferences.putUInt("lock_btn", address | BTYPE_REMOTE_C_BTN);
    preferences.putUInt("stop_btn", address | BTYPE_REMOTE_D_BTN);
    
    // RAM 저장
    remote_addresses[new_slot-1] = address;
    remote_buttons[new_slot-1][0] = address | 0x01;
    remote_buttons[new_slot-1][1] = address | 0x02;
    remote_buttons[new_slot-1][2] = address | 0x04;
    remote_buttons[new_slot-1][3] = address | 0x08;
    
    is_B_type = false;
  }

  preferences.end();
  
  remote_count++;
  
  Serial.printf("리모컨 슬롯 %d에 저장! (총 %d개)\n", new_slot, remote_count);
  
  return SUCCEED;
}


// 빠른 검색
int findRemoteSlot(uint32_t receivedCode) 
{
  for (int i = 0; i < remote_count; i++) 
  {
    for (int btn = 0; btn < MAX_BTN_NUM; btn++) 
    {
      if (receivedCode == remote_buttons[i][btn]) 
      {
        return i + 1;  // 슬롯 번호 반환
      }
    }
  }
  return RF_UNKNOWN_REMOTE_CONTROLLER;
}

// 마지막 리모컨 삭제 (순차 구조 유지)
void deleteLastRemote(void) 
{
  if (remote_count == 0) 
  {
    Serial.println("삭제할 리모컨 없음");
    return;
  }
  
  char ns[20];
  sprintf(ns, "remote_%d", remote_count);
  preferences.begin(ns, false);
  preferences.clear();
  preferences.end();
  
  // 메모리 삭제
  remote_addresses[remote_count-1] = 0;
  for (int i = 0; i < MAX_BTN_NUM; i++) 
  {
    remote_buttons[remote_count-1][i] = 0;
  }
  
  remote_count--;
  
  Serial.printf("슬롯 %d 삭제, 현재 %d개 리모컨\n", remote_count + 1, remote_count);
}

// 전체 삭제 (공장 초기화)
void deleteAllRemotes(void) 
{
  for (int slot = 1; slot <= remote_count; slot++) 
  {
    char ns[20];
    sprintf(ns, "remote_%d", slot);
    preferences.begin(ns, false);
    preferences.clear();
    preferences.end();
  }
  
  memset(remote_addresses, 0, sizeof(remote_addresses));
  memset(remote_buttons, 0, sizeof(remote_buttons));
  remote_count = 0;
}