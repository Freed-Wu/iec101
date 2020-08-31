# iec101

## 程序移植

### 需求

1. 将MG301.c中的AT指令从MG323-B上网模块移植到SIM7600CE上网模块；
2. 移植后所有功能保持与原来一致；
3. 移植后的程序可以在新的硬件平台上运行并正常收发数据。

## 需求

- 将 MG301.c 中的 AT 指令从 MG323-B 上网模块移植到 SIM7600CE 上网模块；
- 移植后所有功能保持与原来一致；
- 移植后的程序可以在新的硬件平台上运行并正常收发数据。

## 资料

- [SIM7500_SIM7600 Series_AT Command Manual_V2.00.pdf](https://cn.simcom.com/service-929.html)
- [SIM7500_SIM7600_SIM7800 Series_TCPIP_Application Note_V2.00](https://cn.simcom.com/service-909.html)
- [SIM7100_SIM7500_SIM7600 Series_UART_Application Note_V2.00](https://cn.simcom.com/service-923.html)
- [SIM7600CE_硬件设计手册_V1.09](https://cn.simcom.com/service-669.html)
- [SIM7600CE_产品规格书_20200427](https://cn.simcom.com/service-728.html)
- [SIM7600 Reference Design V1.02](https://cn.simcom.com/service-697.html)
- MG323-B 上网模块 AT 指令手册；

## 目录结构

```
.
├── doc //开发文档
│   └── ...
├── images //图片
│   └── ...
├── Project //工程文件
│   ├── ARM-MDK
│   │   └── ...
│   ├── EWARM
│   │   └── ...
│   └── SI
│       └── ...
├── README.md
└── Source //代码
    ├── STM32F10x_StdPeriph_Lib_V3.5.0 //库
    │   └── ...
    └── User //用户代码
        └── ...
```

### 开发约定

- 缩进: <kbd>Tab</kbd>
- 风格: R & K
