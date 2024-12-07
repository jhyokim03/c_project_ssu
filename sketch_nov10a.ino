#define TRIG 11 //TRIG 핀 설정 (초음파 보내는 핀)
#define ECHO 13 //ECHO 핀 설정 (초음파 받는 핀)
#define BUZZER 8 //버저 핀 설정 

void setup() {

  Serial.begin(9600); // 시리얼 통신 시작, PC 모니터에서 초음파 센서 값을 확인하기 위해 통신 속도를 9600bps로 설정  
  pinMode(TRIG, OUTPUT); // 초음파 센서의 TRIG(초음파 보내는 핀) 핀을 출력 모드로 설정
  pinMode(ECHO, INPUT); // 초음파 센서의 ECHO 핀(초음파 받는 핀)을 입력 모드로 설정
  pinMode(BUZZER, OUTPUT); // 부저(BUZZER) 핀을 출력 모드로 설정, 의자가 기준 거리 벗어날 때 경고음을 내기 위해 사용

}

void loop() {
  long duration, distance; // 측정 시간(duration)과 계산된 거리(distance)를 저장할 변수를 선언
  
  digitalWrite(TRIG, LOW); // 초음파 발사 전 TRIG 핀을 LOW로 설정해 초기화
  delayMicroseconds(2);    // 2마이크로초 동안 대기, 안정적인 신호 전송을 위해 짧은 지연
  
  digitalWrite(TRIG, HIGH); // TRIG 핀을 HIGH로 설정하여 초음파 신호를 발생시키기 시작
  delayMicroseconds(10);    // 10마이크로초 동안 HIGH 유지하여 초음파 펄스 발생 (초음파 신호가 방출됨)
  
  digitalWrite(TRIG, LOW); // TRIG 핀을 다시 LOW로 설정하여 초음파 신호를 종료

  duration = pulseIn (ECHO, HIGH); //물체에 반사되어돌아온 초음파의 시간을 변수에 저장
  //34000*초음파가 물체로 부터 반사되어 돌아오는시간 /1000000 / 2(거리는 왕복값이 아닌 편도값이기때문에 2를 나눈다)

 //초음파센서의 거리값이 위 계산값과 동일하게 Cm로 환산되는 계산공식 
  distance = duration * 17 / 1000; 

  //PC모니터로 초음파 거리값을 확인
  Serial.println(duration ); //초음파가 반사되어 돌아오는 시간
  Serial.print("\nDIstance : ");
  Serial.print(distance); //물체와의 거리값(cm값)
  Serial.println(" Cm");

  //기준 거리(40cm) 초과 시 버저 울림 --> 정확한 실험보다 예측
  if(distance >= 40){
    digitalWrite(BUZZER, HIGH); //버저 켜기
  }
  else{
    digitalWrite(BUZZER, LOW); //버저 끄기
  }

  delay(2000); //2초마다 측정값을 보여주기
}

