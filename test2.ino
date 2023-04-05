#include <SoftwareSerial.h>


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

String uartString = "";
unsigned int currentStep;
unsigned int oldStep;

SoftwareSerial bleSerial(A0, A1); // RX, TX

unsigned char startBit = 0xf0;
unsigned char commandBit = 0xa1;
unsigned char roll = 100;
unsigned char pitch = 100;
unsigned char yaw = 100;
unsigned char throttle = 0;
unsigned char operationBit = 0x05;
unsigned char checkSum = 0;

unsigned int firstRoll;
unsigned int firstPitch;
//-----------------------------------------------
int CTN = 0;
int button = 1;
//-----------------------------------------------

void initUart()
{
  uartString = "";
}

void initFlag()

{
  currentStep = OPERATION_STEP_1;
  oldStep = OPERATION_STEP_1;
}

void checkNextStep()
{
  oldStep = currentStep;
  currentStep = OPERATION_STEP_0;
}



void returnOldStep()

{
  currentStep = oldStep;
  currentStep++;
}

void checkCrLfProcess()
{
  while (bleSerial.available())
  {
    char inChar = bleSerial.read();
    uartString += inChar;
    if ( uartString.length() > 4
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
    //digitalread 핀 6->3 
  if (!digitalRead(3))
  {
    if (throttle > 59)
      throttle -= 20;
    else if (throttle > 3)
      throttle -= 4;
  }
  // button이 새로 정의되어 있는데 
  // 아마 버튼이 눌린 상태에서의  동작과 눌리지 않았을때의 동작이 
  // 다른것 같음 
  else if (!button)
  {
    if (throttle < 20)
      throttle = 20;
    else if (throttle < 181)
      throttle += 20;
  }
}

void checkYaw()
{
    //핀 번호 7-> 4로
  if (!digitalRead(4))
  {
    yaw = 80;
  }
  else if (!digitalRead(5))
  {
    yaw = 120;
  }
  else
  {
    yaw = 100;
  }
}



void checkEmergency()
{
  if (!digitalRead(6))
    throttle = 0;
}
// CTN도 새로운 핀 코드 이긴한데 
// 아마 ? 자이로 센서 값을 끊어서 쓰는것 같음 
// -> 좌우 / 전후 조정인데 해당 영상에서는 자이로로 했음
void checkRoll()
{
  if (CTN == 0 || CTN == 3 || CTN == 6)
    roll = 95;
  else if (CTN == 1 || CTN == 4 || CTN == 7)
    roll = 100;
  else if (CTN == 2 || CTN == 5 || CTN == 8)
    roll = 105;
}
void checkPitch()
{
  if (CTN == 6 || CTN == 7 || CTN == 8)
    pitch = 90;
  else if (CTN == 3 || CTN == 4 || CTN == 5)
    pitch = 100;
  else if (CTN == 0 || CTN == 1 || CTN == 2)
    pitch = 105;
}
void sendDroneCommand()
{
  if (throttle == 0)
  {
    roll = 100;
    pitch = 100;
    yaw = 100;
  }
  bleSerial.print("at+writeh0006");
  bleSerial.print(String(startBit, HEX));
  bleSerial.print(String(commandBit, HEX));
  if (roll < 0x10)
    bleSerial.print("0" + String(roll, HEX));
  else
    bleSerial.print(String(roll, HEX));
  if (pitch < 0x10)
    bleSerial.print("0" + String(pitch, HEX));
  else
    bleSerial.print(String(pitch, HEX));
  if (yaw < 0x10)
    bleSerial.print("0" + String(yaw, HEX));
  else
    bleSerial.print(String(yaw, HEX));
  if (throttle < 0x10)
    bleSerial.print("0" + String(throttle, HEX));
  else
    bleSerial.print(String(throttle, HEX));
  bleSerial.print("0" + String(operationBit, HEX));

  checkSum = commandBit + roll + pitch + yaw + throttle + operationBit;
  checkSum = checkSum & 0x00ff;

  if (checkSum < 0x10)
    bleSerial.print("0" + String(checkSum, HEX));
  else
    bleSerial.print(String(checkSum, HEX));
  bleSerial.print("\r");
}

void setup()
{
    // 아래 본코드 보다 간소화 되어있는데 
    //이는 아마 드론기본 코드에서 버전이 업데이트 되면서 추가된것 같음
  Serial.begin(9600);
  Serial.println("Test Started!");
  bleSerial.begin(9600);
  initFlag();
  initUart();
  //기존 5~10번 핀 사용에서 2~7번 핀 사용으로 바꿈
  for (int i = 2; i < 8; i++)
  {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH);
  }
  // 플로팅 상태 방지를 위한 풀업 사용
  //https://m.blog.naver.com/PostView.naver?isHttpsRedirect=true&blogId=yuyyulee&logNo=220285724497
  // 플로팅 상태 및 풀업사용 정의는 위에 주석 
  for (int i = 8; i <= 10; i++)
    pinMode(i, INPUT);
  pinMode(13,INPUT_PULLUP);
}
    //기존코드에서는 loop 내에 operation_step 0~10 까지 존재 했는데
    // 해당 코드에서는 setup부에 0~4 생략 되어있음 
    // 확인 필요 
    case OPERATION_STEP_5:
      if (uartString.startsWith("\r\nCONNECT "))
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
    case OPERATION_STEP_6:
      {
        checkThrottle();
        checkRoll();
        checkPitch();
        checkYaw();
        checkEmergency();
        sendDroneCommand();
        delay(10);

        if (!digitalRead(7))
        {
          Serial.println("REQUEST DISCONNECT");
          delay(300);
          initUart();
          currentStep++;
        }
      }
      break;

    case OPERATION_STEP_7:
      delay(1000);
      bleSerial.flush();
      initUart();
      while (bleSerial.available())
      {
        bleSerial.read();
      }
      initUart();
      bleSerial.print("ath\r");
      checkNextStep();
      break;
    case OPERATION_STEP_8:
      if (uartString.equals("\r\nOK\r\n"))
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
    case OPERATION_STEP_9:

      if (uartString.startsWith("\r\nDISCONNECT"))
      {
        Serial.println("DISCONNECT 1 OK");
        //본코드에는  bleSerial.print("at+btsts?\r");
        // 위에 해당구문이 있으나 여기서는 빠짐 
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
    case OPERATION_STEP_10:
      if (uartString.startsWith("\r\nREADY"))
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
      if (bleSerial.available())
        Serial.write(bleSerial.read());
      if (Serial.available())
        bleSerial.write(Serial.read());
      break;
  }
}

// 여기서 부터는 새로운 코드임
이게 왼쪽( 자이로센서랑 연결되는놈)
//x 0  -20 1 stay 10 20
//y 11 -20 110 stay 111 20  8핀부터
#include <Wire.h>


const int MPU_addr = 0x68;

int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ; 



void setup() {

  initMPU6050(); //MPU-6050 센서에 대한 초기 설정 함수3

  Serial.begin(115200); //Serial 통신 시작

  calibAccelGyro(); //센서 보정

  initDT(); //시간 간격에 대한 초기화 -> 현재 시각 저장

            //즉, 드론이 전원이 ON 되면 그떄부터 측정 시작!

  for(int i=8; i<=10; i++)

    pinMode(i,OUTPUT);

}



void loop() {
  readAccelGyro(); //가속도, 자이로 센서 값 읽어드림
  //SendDataToProcessing(); //프로세싱으로 값 전달
  calcDT(); //측정 주기 시간 계산
  calcAccelYPR();

  static int cnt;
  cnt++;
  if(cnt%2 == 0)
    SendDataToProcessing(); //위에 동일한 함수는 주석처리!
  //측정 주기 시간이 짝수(2ms 단위로 하기 위해서)이면 프로세싱으로 보낸다.
}

void initMPU6050(){
  Wire.begin(); //I2C 통신 시작 아림
  Wire.beginTransmission(MPU_addr); //0x68번지 값을 가지는 MPU-6050과 I2C 통신
  Wire.write(0x6B);
  Wire.write(0); //잠자는 MPU-6050을 깨우고 있다.
  Wire.endTransmission(true); //I2C 버스 제어권에서 손 놓음
}

void readAccelGyro(){
  Wire.beginTransmission(MPU_addr); //0x68번지 값을 가지는 MPU-6050과 I2C 통신 시작
  Wire.write(0x3B); //0x3B번지에 저장
  Wire.endTransmission(false); //데이터 전송 후 재시작 메새지 전송(연결은 계속 지속)
  Wire.requestFrom(MPU_addr, 14, true); //0x68 번지에 0x3B 부터 48까지 총 14바이트 저장
  AcX = Wire.read() << 8 | Wire.read();
  AcY = Wire.read() << 8 | Wire.read();
  AcZ = Wire.read() << 8 | Wire.read();
  Tmp = Wire.read() << 8 | Wire.read();
  GyX = Wire.read() << 8 | Wire.read();
  GyY = Wire.read() << 8 | Wire.read();
  GyZ = Wire.read() << 8 | Wire.read();
}

float dt;
float accel_angle_x, accel_angle_y, accel_angle_z;
float gyro_angle_x, gyro_angle_y, gyro_angle_z;
float filtered_angle_x, filtered_angle_y, filtered_angle_z;
float baseAcX, baseAcY, baseAcZ;  //가속도 평균값 저장 변수
float baseGyX, baseGyY, baseGyZ;  //자이로 평균값 저장 변수

void SendDataToProcessing(){
  if(accel_angle_x<20 && accel_angle_x>-20 && accel_angle_y>20)
  {
    digitalWrite(8,1);
    digitalWrite(9,1);
    digitalWrite(10,0);
  }
  else if(accel_angle_x<20 && accel_angle_x>-20 && accel_angle_y<20 && accel_angle_y>-20)
  {
    digitalWrite(8,0);
    digitalWrite(9,0);
    digitalWrite(10,1);
  }
  else if(accel_angle_x<20 && accel_angle_x>-20  && accel_angle_y<-20)
  {
    digitalWrite(8,1);
    digitalWrite(9,0);
    digitalWrite(10,1);
  }
  else if(accel_angle_x>20 && accel_angle_y>20)
  {
    digitalWrite(8,0);
    digitalWrite(9,0);
    digitalWrite(10,0);
  }
  else if(accel_angle_x>20 && accel_angle_y<20 && accel_angle_y>-20)
  {
    digitalWrite(8,1);
    digitalWrite(9,0);
    digitalWrite(10,0);
  }
  else if(accel_angle_x>20 && accel_angle_y<-20)
  {
    digitalWrite(8,0);
    digitalWrite(9,1);
    digitalWrite(10,0);
  }
  else if(accel_angle_x<-20 && accel_angle_y>20)
   {
    digitalWrite(8,0);
    digitalWrite(9,1);
    digitalWrite(10,1);
  }
  else if(accel_angle_x<-20 && accel_angle_y<20 && accel_angle_y>-20)
   {
    digitalWrite(8,1);
    digitalWrite(9,1);
    digitalWrite(10,1);
  }
  
  else if(accel_angle_x<-20 && accel_angle_y<-20)
   {
    digitalWrite(8,1);
    digitalWrite(9,1);
    digitalWrite(10,1);
  }
}

void calibAccelGyro(){
  float sumAcX = 0, sumAcY = 0, sumAcZ = 0;
  float sumGyX = 0, sumGyY = 0, sumGyZ = 0;

  readAccelGyro(); //가속도 자이로 센서 읽어들임
  //평균값 구하기
  for(int i=0; i<10; i++){
    readAccelGyro();
    sumAcX += AcX; sumAcY += AcY; sumAcZ += AcZ;
    sumGyX += GyX; sumGyY += GyY; sumGyZ += GyZ;
    delay(100);
  }
  baseAcX = sumAcX / 10; baseAcY = sumAcY / 10; baseAcZ = sumAcZ / 10;
  baseGyX = sumGyX / 10; baseGyY = sumGyY / 10; baseGyZ = sumGyZ / 10;
}

unsigned long t_now;  //현재 측정 주기 시간
unsigned long t_prev; //이전 측정 주기 시간

void initDT(){
  t_prev = millis();
}

void calcDT(){
  t_now = millis();
  dt = (t_now - t_prev) / 1000.0; //millis()로 얻은 값은 밀리초 단위이니까!!!!
  t_prev = t_now;
}

void calcAccelYPR(){
  float accel_x, accel_y, accel_z; //가속도 센서의 최종적인 보정값!!!
  float accel_xz, accel_yz;
  const float RADIANS_TO_DEGREES = 180/3.14159;

  accel_x = AcX - baseAcX; // 가속도(직선) X축에 대한 현재 값 - 가속도 센서의 평균값
  accel_y = AcY - baseAcY;
  accel_z = AcZ + (16384 - baseAcZ);

  //직석 +X축이 기울어진 각도 구함
  accel_yz = sqrt(pow(accel_y, 2) + pow(accel_z, 2));
  accel_angle_y = atan(-accel_x / accel_yz)*RADIANS_TO_DEGREES;
  accel_xz = sqrt(pow(accel_x, 2) + pow(accel_z, 2));
  accel_angle_x = atan(accel_y / accel_xz)*RADIANS_TO_DEGREES;
  accel_angle_z = 0;
}