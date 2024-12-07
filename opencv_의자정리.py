import cv2
import numpy as np
import serial  # pySerial 추가
import time

# 시리얼 포트 설정 (예: COM6, /dev/ttyUSB0 등)
arduino = serial.Serial('COM6', 9600)  # 아두이노와 연결된 포트 설정
time.sleep(2)  # 아두이노와의 연결 대기

# 임계값 설정
THRESHOLD = 30
url = 'http://192.168.110.73:81/stream'  # ESP32 카메라 IP 주소로 변경

# 카메라 열기
#cap = cv2.VideoCapture(url) #esp32 cam 흔들림, 조명 문제로 실행 불가
cap = cv2.VideoCapture(0)


# 얼굴 탐지용 Haar Cascade 분류기 로드
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

# 첫 번째 프레임 읽기
ret, frame_a = cap.read()

if not ret:
    print("프레임을 읽을 수 없습니다.")
    cap.release()
    cv2.destroyAllWindows()
    exit()

# 첫 번째 프레임을 그레이스케일로 변환
gray_start = cv2.cvtColor(frame_a, cv2.COLOR_BGR2GRAY)

# 얼굴 탐지 상태 및 타이머 설정
face_detected = False
face_detected_time = 0
change_detected_time = 0  # 변화 감지 시작 시간
change_detected = False  # 변화 감지 여부
face_last_sent_time = 0  # 마지막으로 UING 메시지를 전송 시간
arrange_last_sent_time = 0  # 마지막 ARRANGE  메시지 전송 시간



while True:
    ret, frame_c = cap.read()  # 새로운 프레임 읽기

    if not ret:
        break

    # 현재 프레임을 그레이스케일로 변환
    gray_current = cv2.cvtColor(frame_c, cv2.COLOR_BGR2GRAY)

    # 얼굴 탐지
    faces = face_cascade.detectMultiScale(gray_current, scaleFactor=1.1, minNeighbors=5)
   
    if len(faces) > 0:
        # 얼굴이 처음 탐지되었거나 마지막 전송 후 2초가 지난 경우에만 메시지 전송
        if not face_detected or (time.time() - face_last_sent_time > 2):
            arduino.write(b'USING')  # 아두이노로 메시지 전송
            print("USING 메시지 전송")  # 디버깅용 출력
            face_detected = True
            face_last_sent_time = time.time()  # 전송 시간 갱신

    # 얼굴이 탐지되지 않은 경우에만 diff_cnt 계산
    if len(faces) == 0:
        # 첫 번째 프레임과 현재 프레임의 차이를 이진화
        diff = cv2.absdiff(gray_start, gray_current)
        _, diff = cv2.threshold(diff, THRESHOLD, 255, cv2.THRESH_BINARY)

        # 형태학적 연산으로 노이즈 제거
        kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
        diff = cv2.morphologyEx(diff, cv2.MORPH_OPEN, kernel)

        # 변화 영역 카운트
        diff_cnt = cv2.countNonZero(diff)

        # 흑백 화면을 생성
        diff_display = cv2.cvtColor(diff, cv2.COLOR_GRAY2BGR)  # 그레이스케일 이미지를 컬러로 변환
        diff_display = cv2.resize(diff_display, (frame_c.shape[1], frame_c.shape[0]))  # 크기 조정

        # 변화 영역 기준
        if diff_cnt > 40000:
            if not change_detected:  # 변화가 처음 감지되었을 때
                change_detected_time = time.time()  # 현재 시간 기록
                change_detected = True
        else:
            change_detected = False  # 변화가 감지되지 않으면 초기화

        # 2초 동안 변화가 감지된 경우
        if change_detected and (time.time() - change_detected_time >= 2):
            print("큰 변화 감지: ARRANGE 메시지 전송")
            arduino.write(b'ARRANGE')  # 아두이노로 메시지 전송
            arrange_last_sent_time = time.time()  # 마지막 전송 시간 기록
            time.sleep(2)

        

    else:
        diff_cnt = 0 #얼굴 탐지할 경우 프레임차이 계산 중지 및 초기화
        diff_display = np.zeros_like(frame_c)  # 얼굴이 탐지되면 빈 화면을 표시

        # 탐지된 얼굴 사각형 그리기
        for (x, y, w, h) in faces:
            cv2.rectangle(frame_c, (x, y), (x + w, y + h), (255, 0, 0), 2)

     # 원본 프레임과 흑백 변화 화면을 나란히 결합
    combined = cv2.hconcat([frame_c, diff_display])

    # 변화 영역 카운트 표시하기
    cv2.putText(combined, f'Changes: {diff_cnt}', (frame_c.shape[1] - 200, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)

    # 프레임 표시
    cv2.imshow('Camera and Change Detection', combined)


    # 'q' 키를 누르면 루프 종료
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# 자원 해제
cap.release()
arduino.close()  # 아두이노와의 연결 해제
cv2.destroyAllWindows()
