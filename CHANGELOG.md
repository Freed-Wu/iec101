# CHANGELOG

## 3.0.3

### New

- 移除 101protocol.h，将 TimeStructure 移动到modbus.h
- 增加 ModbusDigitalSet()用于遥测
- 注释了USART2中断调用ModbusIsp()的语句，改由main.c 在通过SuperviseTCP()收到服务器数据后调用ModbusIsp()。
在ModbusIsp()开头与结尾增加将GPRS接收数据的数组复制到modbus接收数据的数组、将modbus发送数据的数组复制到GPRS发送数据的数组的语句。