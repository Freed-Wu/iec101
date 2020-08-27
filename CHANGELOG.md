# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed
- 将 SIM7600.c 改回 MG301.c

## [3.0.3] - 2020-08-26

### Added
- 增加 ModbusDigitalSet()用于遥测

### Removed
- 移除 101protocol.h，将 TimeStructure 移动到modbus.h

### Changed
- 注释了USART2中断调用ModbusIsp()的语句，改由main.c 在通过SuperviseTCP()收到服务器数据后调用ModbusIsp()。在ModbusIsp()开头与结尾增加将GPRS接收数据的数组复制到modbus接收数据的数组、将modbus发送数据的数组复制到GPRS发送数据的数组的语句。

## [3.0.2] - 2020-07-22

### Removed
- 移除测试模式

### Changed
- 将 MG301.c 改为 SIM7600.c

### Fixed
- 调试

## [3.0.1] - 2020-06-17

### Added
- 测试模式采用透传模式，设置完模式以及网络连接之后，用UART通信直接发送读取数据，作为硬件通讯测试。

### Changed
- 正常使用时采用数据模式中的缓冲器模式，利用CIPSEND发送数据，CIPRXGET读取返回信息。
- 上电时序为低电平脉冲达到一定时间，参考典型值程序中设定为500ms。将GPIOB15拉高，延时500ms后，把GPIOB15拉低。

[Unreleased]: https://github.com/Freed-Wu/iec101/compare/v3.0.3...Unreleased
[3.0.3]: https://github.com/Freed-Wu/iec101/compare/v3.0.2...v3.0.3
[3.0.2]: https://github.com/Freed-Wu/iec101/compare/v3.0.1...v3.0.2
[3.0.1]: https://github.com/Freed-Wu/iec101/releases/tag/v3.0.1
