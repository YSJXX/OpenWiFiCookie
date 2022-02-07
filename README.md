# Open WiFi Cookie 탈취

## 개발 환경

- Ubuntu 19.04
- C/C++ (Qt C++)
- Qt Creator 

## 탈취 원리


1. 무선 통신의 특성을 이용.

- 무선 통신의 경우 각 지점 사이에 전기 전도체의 연결 없이 정보를 전송하는 특성을 가지고 있습니다. 정보 전달을 위해 정보를 공기중으로 뿌리는 형태를 가지고있는데 이 부분을 탐지하여 정보를 탈취합니다. 

2. 비밀번호가 없는 WiFi 이용.

- 비밀번호가 없는 WiFi의 경우 공유기와 휴대폰과의 통신에서 암호화하지 않은 평문 통신을 하게됩니다. 

3. 평문 통신하는 HTTP 프로토콜을 이용.

- HTTP 통신을 하는 경우 아이디,비밀번호,세션 등 정보를 평문으로 통신하게 됩니다. 

<iframe width="640" height="360" src="https://www.youtube.com/embed/6Az2cNU7gUw" frameborder="0" gesture="media" allowfullscreen=""></iframe>
