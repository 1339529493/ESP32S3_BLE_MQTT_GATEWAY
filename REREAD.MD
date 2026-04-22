这是一个个人学习项目

# BLE-MQTT Gateway on ESP32-S3

## 功能
- BLE GATT服务：自定义Service，支持RX/TX双向通信
- MQTT客户端：连接云端Mosquitto，支持发布/订阅
- 双向数据通道：手机BLE ↔ ESP32 ↔ 云端MQTT

## 测试记录
| 测试项 | 结果 |
|--------|------|

## 模块结构
- main.c: 入口，任务创建
- wifi.c: WiFi连接管理
- mqtt.c: MQTT客户端，发布/订阅
- ble.c: BLE GATT服务，RX/TX特征