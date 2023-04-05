#include <SoftwareSerial.h>

//각 단계값 정의 
#define OPERATION_STEP_0  0
#define OPERATION_STEP_1  1
#define OPERATION_STEP_2  2
#define OPERATION_STEP_3  3
#define OPERATION_STEP_4  4
#define OPERATION_STEP_5  5
#define OPERATION_STEP_6  6
#define OPERATION_STEP_7  7
#define OPERATION_STEP_8  8
#define OPERATION_STEP_9  9
#define OPERATION_STEP_10  10

#define OPERATION_STEP_ERROR  100
//uart String 타입 변수 선언 
String uartString = "";
//현재 수행 단계
unsigned int currentStep;
//이전 수행 단계
unsigned int oldStep;

SoftwareSerial bleSerial(A0, A1); // RX, TX
//roll, pitch, yaw, ... 초기값 저장 
unsigned char startBit = 0xf0;
unsigned char commandBit = 0xa1;
unsigned char roll = 100;
unsigned char pitch = 100;
unsigned char yaw = 100;
unsigned char throttle = 0;
unsigned char operationBit = 0x05;
unsigned char checkSum = 0;
//좌_우 이동의 값을 저장하기 위해 변수 선언
unsigned int firstRoll;
//전진_후진 이동의 값을 저장하기 위해 firstPitch 변수 선언
unsigned int firstPitch;

void initUart()
{
  uartString = "";
}

//각종 변수 초기값 저장
void initFlag()
{
  currentStep = OPERATION_STEP_1;
  oldStep = OPERATION_STEP_1;
}

//현재 실행되는 단계를 oldStep에 저장 및 현재 당계를 
//OPERATION_STEP_0로 변경 하는 함수
void checkNextStep()
{
  oldStep = currentStep;
  currentStep = OPERATION_STEP_0;
}
//현재단계를 oldStep에 저장되어 있는 단계로 복구 및 
// 복구 단계를 한단계 증가시키는 함수 
void returnOldStep()
{
  currentStep = oldStep;
  currentStep++;
}

// BLE로부터 수신된 데이터를 저장하는 함수
void checkCrLfProcess()
{
  while(bleSerial.available())
  {
    //BLE(read) -> inChar -> uartString으로 저장
    char inChar = bleSerial.read();
    uartString += inChar;
    //uartString 데이터의 길이, startsWith에 정의된 
    //CR, LF, endsWith에 정의된 CR,LF를 체크
    if( uartString.length() > 4
     && uartString.startsWith("\r\n")
     && uartString.endsWith("\r\n") )
    {
      returnOldStep();
      break;
    }
  }
}

void checkThrottle()
{
  //throttle: 감소시 하강, 증가시 상승
  //스위치2번 LOW일 경우 아래 사항 진행(Throttle down)
  if(!digitalRead(6))
  {
    if(throttle > 59)
      throttle -= 20;
    else if(throttle > 3)
      throttle -= 4;
  }
  //스위치 1번 LOW일 경우 아래 사항 진행(Throttle up)
  else if(!digitalRead(5))
  {
    if(throttle < 20)
      throttle = 20;
      //max 181 
    else if(throttle < 181)
      throttle += 20;
  }
}

void checkYaw()
{
  //Yaw: 감소시 좌회전, 증가시 우회전
  //3번 switch (CCW)/제자리 좌회전 
  if(!digitalRead(7))
  {
    yaw = 80;
  }
  //4번 switch (CW)/제자리 우회전 
  else if(!digitalRead(8))
  {
    yaw = 120;
  }
  // 7번,8번 포트 HIGH일 경우 yaw=100 
  else
  {
     yaw = 100;
  }
}

void checkEmergency()
{
  //비상버튼 눌리면, 모터 회전 즉시 0으로 설정.
  //5번 switch (start/emergency)
  if(!digitalRead(9))
    throttle = 0;
}

void checkRoll()
{
  //roll: 증가시 오른쪽 이동, 감소시 왼쪽 이동
  //드론 조이스틱 좌_우 (analog in)
  // case로 끊어서 조정
  unsigned int secondRoll = analogRead(4);
  //회전 각 조절 범위는 100단위 roll 값은 5단위로 끊킴 
  if(secondRoll < firstRoll - 450)
    roll = 75;
  else if(secondRoll < firstRoll - 350)
    roll = 80;
  else if(secondRoll < firstRoll - 250)
    roll = 85;
  else if(secondRoll < firstRoll - 150)
    roll = 90;
  else if(secondRoll < firstRoll - 50)
    roll = 95;
  else if(secondRoll < firstRoll + 50)
    roll = 100;
  else if(secondRoll < firstRoll + 150)
    roll = 105;
  else if(secondRoll < firstRoll + 250)
    roll = 110;
  else if(secondRoll < firstRoll + 350)
    roll = 115;
  else if(secondRoll < firstRoll + 450)
    roll = 120;
  else
    roll = 125;
}

void checkPitch()
{
  //pitch: 증가시 전진, 감소시 후진
  //조이스틱 X 축 드론 전_후진 제어 
  //회전 각 조절 범위는 100단위 roll 값은 5단위로 끊킴
  unsigned int secondPitch = analogRead(5);

  if(secondPitch < firstPitch - 450)
    pitch = 75;
  else if(secondPitch < firstPitch - 350)
    pitch = 80;
  else if(secondPitch < firstPitch - 250)
    pitch = 85;
  else if(secondPitch < firstPitch - 150)
    pitch = 90;
  else if(secondPitch < firstPitch - 50)
    pitch = 95;
  else if(secondPitch < firstPitch + 50)
    pitch = 100;
  else if(secondPitch < firstPitch + 150)
    pitch = 105;
  else if(secondPitch < firstPitch + 250)
    pitch = 110;
  else if(secondPitch < firstPitch + 350)
    pitch = 115;
  else if(secondPitch < firstPitch + 450)
    pitch = 120;
  else
    pitch = 125;
}
//드론 커맨드 제어 함수
void sendDroneCommand()
{
  //하강 0일 경우 아래 고정값 
  if(throttle == 0)
  {
    roll = 100;
    pitch = 100;
    yaw = 100;
  }
  //
  bleSerial.print("at+writeh000d");
  bleSerial.print(String(startBit,HEX));
  bleSerial.print(String(commandBit,HEX));
  //HEX -> 16자리 아래서 해당 값이 16자리 이하경우 앞에 0 을 붙여서 16자리를 충족시켜서 전송 
  if(roll < 0x10)
    bleSerial.print("0" + String(roll,HEX));
  else 
    bleSerial.print(String(roll,HEX));

  if(pitch < 0x10)
    bleSerial.print("0" + String(pitch,HEX));
  else 
    bleSerial.print(String(pitch,HEX));
  if(yaw < 0x10)
    bleSerial.print("0" + String(yaw,HEX));
  else 
    bleSerial.print(String(yaw,HEX));
  if(throttle < 0x10)
    bleSerial.print("0" + String(throttle,HEX));
  else 
    bleSerial.print(String(throttle,HEX));
  bleSerial.print("0" + String(operationBit,HEX));
  
  //-------------------------------------------------------------------------------------------------------------------------------------------

  checkSum = commandBit + roll + pitch + yaw + throttle + operationBit;

  checkSum = checkSum & 0x00ff;
  if(checkSum < 0x10)
    bleSerial.print("0" + String(checkSum,HEX));
  else 
    bleSerial.print(String(checkSum,HEX));
  //
  bleSerial.print("\r");
}
//-----------------------------------------------
void setup()
{
  Serial.begin(9600);
  Serial.println("Test Started!");
  bleSerial.begin(9600);
  bleSerial.print("at\r");
  // 직렬데이터를 2초간 대기
  bleSerial.setTimeout(2);
  bleSerial.readString();
  //각각 Flag , uartString 문자열 초기화 
  initFlag();
  initUart();
  // 디지털 5~10번 포트 INPUT 설정및 초기값 HIGH
  for(int i = 5; i < 11; i++)
  {
    pinMode(i,INPUT);
    digitalWrite(i,HIGH);
  }
}

void loop()
{
  switch(currentStep)
  {
    case OPERATION_STEP_0:
      //아래 함수는 BLE로부터 수신 받은 데이터를 저장 
      checkCrLfProcess();
    break;
    //최초 수행단계 
    case OPERATION_STEP_1:
      delay(2000);
      //SoftwareSerial 포트로 입력된 데이터가 있는경우 초기화
      bleSerial.flush();
      //Flag, UartString문자열 초기화 
      initFlag();
      initUart();
      //SoftwareSerial 포트 사용가능일 경우 다음 스탭으로 넘어감
      while(bleSerial.available())
      {
        bleSerial.read();
      }
      currentStep++;
    break;
    case OPERATION_STEP_2:
      //9번 포트가 LOW일 경우 아래를 진행(연결 진행시)
      if(!digitalRead(9))
      {
        //roll,pitch각각 좌,우/상,하 조이스틱 초기값 설정 
        //후 3번 스탭으로 이동 
        firstRoll = analogRead(4);
        firstPitch = analogRead(5);
        currentStep++;
      }
    break;
    
    case OPERATION_STEP_3:
    //ATD -> 가장 최근 연결했던 블루투스 장치로 재연결
      bleSerial.print("atd");
      //드론 Address
      bleSerial.print("083A5C1F2053");
      bleSerial.print("\r");
      //현재 단계를 oldStep에 저장하고 OPERATIOM_STEP_0를 실행
      // STEP_0에서 수신 받아 진행 단계가 복구되면 STEP_4로 
      checkNextStep();
    break;
    case OPERATION_STEP_4:
      //BLE로부터 수신메시지가 OK 일경우 실행 
      if(uartString.equals("\r\nOK\r\n"))
      {
        Serial.println("Wait Connect");
        delay(300);
        initUart();
        checkNextStep();
        //현재 단계를 oldStep에 저장하고 OPERATIOM_STEP_0를 실행
        // STEP_0에서 수신 받아 진행 단계가 복구되면 STEP_4로 
      }
      else
      {
        Serial.println("CONNECT 1 ERROR");
        initUart();
        currentStep = OPERATION_STEP_ERROR;
      }
    break;
    
    case OPERATION_STEP_5:
      //BLE에서 정상적으로 연결시 CONNECT~~뜨게 되어 
      // CONNECT로 시작하는 문자를 수신할 경우 아래를 진행함
      if(uartString.startsWith("\r\nCONNECT "))
      {
        Serial.println("CONNECT OK");
        delay(300);
        initUart();
        currentStep++;
      }
      else
      {
        Serial.println("CONNECT 2 ERROR");
        initUart();
        currentStep = OPERATION_STEP_ERROR;
      }
    break;
    //Throttle , Roll, Pitch, Yaw ,Emergency 제어를 진행
    case OPERATION_STEP_6:
    {
      checkThrottle();
      checkRoll();
      checkPitch();
      checkYaw();
      checkEmergency();
      //
      sendDroneCommand();
      delay(10);
      //연결 해지 버튼이 LOW 일경우 초기화 시키고 다음단계로 진행됨
      if(!digitalRead(10))
      {
        Serial.println("REQUEST DISCONNECT");
        delay(300);
        initUart();
        currentStep++;
      }
    }
    break;
    // 모든 입력데이터를 초기화 시키고 
    // softwareSerial에 ath--> 연결해지 시킴  
    case OPERATION_STEP_7:
      delay(1000);
      bleSerial.flush();
      initUart();
      while(bleSerial.available())
      {
        bleSerial.read();
      }
      initUart();
      bleSerial.print("ath\r");
      checkNextStep();
    break;
    //정상적으로 연결 해지명령이 전달 되면 
    // Wait Disconnect 출력 
    case OPERATION_STEP_8:
      if(uartString.equals("\r\nOK\r\n"))
      {
        Serial.println("Wait Disconnect");
        delay(300);
        initUart();
        checkNextStep();
      }
      else
      {
        Serial.println("DISCONNECT 1 ERROR");
        initUart();
        currentStep = OPERATION_STEP_ERROR;
      }
    break;
    //DISCONNECT 메시지 수신시 -> 정상적 연결해지시 
    // 연결해지가 되었다고 메시지로 알려주고 
    case OPERATION_STEP_9:
      if(uartString.startsWith("\r\nDISCONNECT"))
      {
        Serial.println("DISCONNECT 1 OK");
        bleSerial.print("at+btsts?\r");
        delay(300);
        initUart();
        checkNextStep();
      }
      else
      {
        Serial.println("DISCONNECT 2 ERROR");
        initUart();
        currentStep = OPERATION_STEP_ERROR;
      }
    break;
    //STANDBY 수신시 아래 진행 
    case OPERATION_STEP_10:
      if(uartString.startsWith("\r\nSTANDBY"))
      {
        Serial.println("DISCONNECT 2 OK");
        delay(300);
        initUart();
        currentStep = OPERATION_STEP_2;
      }
      else
      {
        Serial.println("DISCONNECT 3 ERROR");
        initUart();
        currentStep = OPERATION_STEP_ERROR;
      }
    break;
    default:
      if(bleSerial.available())
        Serial.write(bleSerial.read());
      if(Serial.available())
        bleSerial.write(Serial.read());
    break;
  }
}
