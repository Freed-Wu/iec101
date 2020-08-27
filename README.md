# iec101

## 需求

- 将 MG301.c 中的 AT 指令从 MG323-B 上网模块移植到 SIM7600CE 上网模块；
- 移植后所有功能保持与原来一致；
- 移植后的程序可以在新的硬件平台上运行并正常收发数据。

## 资料

- MG301.c 和 MG301.h 程序源代码；
- MG323-B 上网模块 AT 指令手册；
- SIM7600CE 上网模块 AT 指令手册。

## 目录结构

```
.
├── doc //开发文档
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

## 开发约定

- 缩进: <kbd>Tab</kbd>
- 风格: R & K
- 日志: [CHANGELOG.md](CHANGELOG.md)
