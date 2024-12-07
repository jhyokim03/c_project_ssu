#include <dummy.h>

#include <dummy.h>

//esp 카메라 웹서버 코드
#include "esp_camera.h" //esp cam 모듈의 카메라 기능 사용
#include <WiFi.h> //wifi 사용하기 위한 라이브러리

#define CAMERA_MODEL_AI_THINKER // 카메라 모델 선택

#include "camera_pins.h" //카메라 모델에 적합한 핀 설정

// ===========================
// 와이파이 비밀번호 입력
// ===========================
const char *ssid = "hyospot";
const char *password = "jhyokim03";

void startCameraServer(); // 카메라 웹 서버를 시작하는 함수 선언 (아래에서 정의된 함수가 호출될 예정)
void setupLedFlash(int pin); // LED 플래시를 설정하는 함수 선언 (특정 핀을 설정하여 LED 제어)

// 시리얼 통신 초기화
Serial.begin(115200); // 시리얼 통신 속도를 115200bps로 설정하여 PC 모니터와의 통신을 시작
Serial.setDebugOutput(true); // 디버그 출력을 활성화하여 상세한 로그 메시지 출력
Serial.println(); // 빈 줄 출력하여 초기화 메시지 구분

// 카메라 설정 구조체 선언 및 초기화
camera_config_t config; // 카메라 설정을 담는 구조체 선언, 이후 카메라의 해상도, 프레임 속도 등의 설정을 이 구조체에 저장하여 사용
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  //데이터핀 설정
  //이미지 전송(속도, 해상도)을 위한 많은 수의 데이터 핀이 필요
  config.pin_d0 = Y2_GPIO_NUM; //데이터핀 0 설정
  config.pin_d1 = Y3_GPIO_NUM; //데이터핀 1 설정
  config.pin_d2 = Y4_GPIO_NUM; //데이터핀2 설정
  config.pin_d3 = Y5_GPIO_NUM;//데이터핀3 설정
  config.pin_d4 = Y6_GPIO_NUM;//데이터핀4 설정
  config.pin_d5 = Y7_GPIO_NUM;//데이터핀5 설정
  config.pin_d6 = Y8_GPIO_NUM;//데이터핀6 설정
  config.pin_d7 = Y9_GPIO_NUM;//데이터핀7 설정
  //카메라 모듈 성능과 작동 방식 정의
  config.pin_xclk = XCLK_GPIO_NUM; //카메라 동작 주기 설정 핀
  config.pin_pclk = PCLK_GPIO_NUM; //이미지 전송 시 읽어들이는 속도 결정 핀
  config.pin_vsync = VSYNC_GPIO_NUM; //수직 동기화 핀(새로운 프레임 시작 알림)
  config.pin_href = HREF_GPIO_NUM;//수평 참조 신호(데이터 전송 동기화 )
  config.pin_sccb_sda = SIOD_GPIO_NUM; //카메라 설정 등 명령 제어
  config.pin_sccb_scl = SIOC_GPIO_NUM; //데이터 전송 타이밍 제공
  config.pin_pwdn = PWDN_GPIO_NUM; //전원다운 핀(카메라 전원 차단 상태 전환)
  config.pin_reset = RESET_GPIO_NUM; //리셋 핀(카메라 리셋)
  config.xclk_freq_hz = 20000000; //주파수 설정 20MHz
  config.frame_size = FRAMESIZE_UXGA; //프레임 크기 설정(1600x1200픽셀)
  config.pixel_format = PIXFORMAT_JPEG;  // 스트리밍 용도 픽셀 포멧 설정
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY; //프레임 잡는 모드 설정
  config.fb_location = CAMERA_FB_IN_PSRAM; //프레임 버퍼 위치 설정
  config.jpeg_quality = 12; //jpeg 품질 설정(1(높음)~63(낮음))
  config.fb_count = 1; //프레임 버퍼 개수 설정

// PSRAM IC가 존재하는 경우, UXGA 해상도와 더 높은 JPEG 품질로 초기화하여 
// 더 큰 미리 할당된 프레임 버퍼를 사용
  if (config.pixel_format == PIXFORMAT_JPEG) { // 픽셀 포맷이 JPEG인 경우
    if (psramFound()) { // PSRAM이 발견되면
      config.jpeg_quality = 10; //높은 품질 설정
      config.fb_count = 2; //버퍼 개수 2로 설정, 더 많은 프레임 저장 가능
      config.grab_mode = CAMERA_GRAB_LATEST; //최신 프레임 잡도록 설정
    } else {
      // PSRAM이 없을 경우 크기 제한
      config.frame_size = FRAMESIZE_SVGA; //해상도 낮춤
      config.fb_location = CAMERA_FB_IN_DRAM; //프레임 버퍼 dram에 저장
    }
  } else {
    // 얼굴인식에 가장 괜찮은 옵션
    config.frame_size = FRAMESIZE_240X240; //해상도 240x 240
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
 
#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

#if defined(CAMERA_MODEL_ESP32S3_EYE)
  s->set_vflip(s, 1);
#endif

// led setup
#if defined(LED_GPIO_NUM)
  setupLedFlash(LED_GPIO_NUM);
#endif

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  delay(10000);
}
