# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Changed
- �� SIM7600.c �Ļ� MG301.c

## [3.0.3] - 2020-08-26

### Added
- ���� ModbusDigitalSet()����ң��

### Removed
- �Ƴ� 101protocol.h���� TimeStructure �ƶ���modbus.h

### Changed
- ע����USART2�жϵ���ModbusIsp()����䣬����main.c ��ͨ��SuperviseTCP()�յ����������ݺ����ModbusIsp()����ModbusIsp()��ͷ���β���ӽ�GPRS�������ݵ����鸴�Ƶ�modbus�������ݵ����顢��modbus�������ݵ����鸴�Ƶ�GPRS�������ݵ��������䡣

## [3.0.2] - 2020-07-22

### Removed
- �Ƴ�����ģʽ

### Changed
- �� MG301.c ��Ϊ SIM7600.c

### Fixed
- ����

## [3.0.1] - 2020-06-17

### Added
- ����ģʽ����͸��ģʽ��������ģʽ�Լ���������֮����UARTͨ��ֱ�ӷ��Ͷ�ȡ���ݣ���ΪӲ��ͨѶ���ԡ�

### Changed
- ����ʹ��ʱ��������ģʽ�еĻ�����ģʽ������CIPSEND�������ݣ�CIPRXGET��ȡ������Ϣ��
- �ϵ�ʱ��Ϊ�͵�ƽ����ﵽһ��ʱ�䣬�ο�����ֵ�������趨Ϊ500ms����GPIOB15���ߣ���ʱ500ms�󣬰�GPIOB15���͡�

[Unreleased]: https://github.com/Freed-Wu/iec101/compare/v3.0.3...Unreleased
[3.0.3]: https://github.com/Freed-Wu/iec101/compare/v3.0.2...v3.0.3
[3.0.2]: https://github.com/Freed-Wu/iec101/compare/v3.0.1...v3.0.2
[3.0.1]: https://github.com/Freed-Wu/iec101/releases/tag/v3.0.1
