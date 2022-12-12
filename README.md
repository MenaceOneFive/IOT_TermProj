# IOT통신시스템[A] 기말 프로젝트

## 사용 라이브러리
### SNIPE : https://github.com/codezoo-ltd/SNIPE
### Firebase-ESP8266 : https://github.com/mobizt/Firebase-ESP8266

## 프로젝트 구조
### 중계노드 (WEMOS D1 R1) 
    이 리포지토리의 코드는 중계노드의 코드입니다.
    Platform.io + Jetbarin Clion 환경에서 작성되었습니다.
### 센서노드 (Arduino) 
    IOT-PBL 폴더에 서브모듈의 형태로 포함되어 있습니다.
    리포지토리 IOT-PBL :
    https://github.com/MKSonny/IOT-PBL
### 안드로이드 앱
    파이어베이스와 통신하여 사용자에게 각 노드의 상황을 표시
    제어값을 파이어베이스에 업로드하여 노드를 제어