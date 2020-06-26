#ifndef __101_PROTOCOL_H__
#define __101_PROTOCOL_H__
#include "stm32f10x.h"

#define INVALID_DATA 0
#define VARIABLE_DATA 1
#define STABLE_DATA 2

//在监视方向上的过程信息
#define NULL 0 //未定义
#define M_SP_NA_1 1 //单点信息
#define M_SP_TA_1 2 //带时标的单点信息
#define M_DP_NA_1 3 //双点信息
#define M_DP_TA_1 4 //带时标的双点信息
#define M_ST_NA_1 5 //步位置信息
#define M_ST_TA_1 6 //带时标的步位置信息
#define M_BO_NA_1 7 //32比特串
#define M_BO_TA_1 8 //带时标的32比特串
#define M_ME_NA_1 9 //测量值，规一化值
#define M_ME_TA_1 10 //测量值，带时标的规一化值
#define M_ME_NB_1 11 //测量值，标度化值
#define M_ME_TB_1 12 //测量值，带时标的标度化值
#define M_ME_NC_1 13 //测量值，短浮点数
#define M_ME_TC_1 14 //测量值，带时标的短浮点数
#define M_IT_NA_1 15 //累计量
#define M_IT_TA_1 16 //带时标的累计量
#define M_EP_TA_1 17 //带时标的继电设备保护事件
#define M_EP_TB_1 18 //时标的继电保护设备成组启动事件
#define M_EP_TC_1 19 //带时标的继电保护设备成组输出电路信息
#define M_PS_NA_1 20 //带变位检出的成组单点信息
#define M_ME_ND_1 21 //测量值，不带品质描述词的规一化值
#define RESERVED_22 22 //为将来兼容定义保留
#define RESERVED_23 23 //为将来兼容定义保留
#define RESERVED_24 24 //为将来兼容定义保留
#define RESERVED_25 25 //为将来兼容定义保留
#define RESERVED_26 26 //为将来兼容定义保留
#define RESERVED_27 27 //为将来兼容定义保留
#define RESERVED_28 28 //为将来兼容定义保留
#define RESERVED_29 29 //为将来兼容定义保留
#define M_SP_TB_1 30 //带CP56Time2a时标的单点信息
#define M_DP_TB_1 31 //带CP56Time2a时标的双点信息
#define M_ST_TB_1 32 //带CP56Time2a时标的步位置信息
#define M_BO_TB_1 33 //带CP56Time2a时标的32比特串
#define M_ME_TD_1 34 //带CP56Time2a时标的测量值, 规一化值
#define M_ME_TE_1 35 //带CP56Time2a时标的测量值, 标度化值
#define M_ME_TF_1 36 //带CP56Time2a时标的测量值, 短浮点数
#define M_IT_TB_1 37 //带CP56Time2a时标的累计量
#define M_EP_TD_1 38 //带CP56Time2a时标的继电保护设备事件
#define M_EP_TE_1 39 //带CP56Time2a时标的继电保护设备成组启动事件
#define M_EP_TF_1 40 //带CP56Time2a时标的继电保护设备成组输出电路信息
#define RESERVED_41 41 //为将来兼容定义保留
#define RESERVED_42 42 //为将来兼容定义保留
#define RESERVED_43 43 //为将来兼容定义保留
#define RESERVED_44 44 //为将来兼容定义保留

//在控制方向上的过程信息
#define C_SC_NA_1 45 //单点命令
#define C_DC_NA_1 46 //双点命令
#define C_RC_NA_1 47 //步调节命令
#define C_SE_NA_1 48 //设定值命令, 规一化值
#define C_SE_NB_1 49 //设定值命令, 标度化值
#define C_SE_NC_1 50 //设定值命令, 短浮点数
#define C_BO_NA_1 51 //32比特串
#define RESERVED_52 52 //为将来兼容定义保留
#define RESERVED_53 53 //为将来兼容定义保留
#define RESERVED_54 54 //为将来兼容定义保留
#define RESERVED_55 55 //为将来兼容定义保留
#define RESERVED_56 56 //为将来兼容定义保留
#define RESERVED_57 57 //为将来兼容定义保留
#define RESERVED_58 58 //为将来兼容定义保留
#define RESERVED_59 59 //为将来兼容定义保留
#define RESERVED_60 60 //为将来兼容定义保留
#define RESERVED_61 61 //为将来兼容定义保留
#define RESERVED_62 62 //为将来兼容定义保留
#define RESERVED_63 63 //为将来兼容定义保留
#define RESERVED_64 64 //为将来兼容定义保留
#define RESERVED_65 65 //为将来兼容定义保留
#define RESERVED_66 66 //为将来兼容定义保留
#define RESERVED_67 67 //为将来兼容定义保留
#define RESERVED_68 68 //为将来兼容定义保留
#define RESERVED_69 69 //为将来兼容定义保留

//在监视方向的系统命令
#define M_EI_NA_1 70 //初始化结束
#define RESERVED_71 71 //为将来兼容定义保留
#define RESERVED_72 72 //为将来兼容定义保留
#define RESERVED_73 73 //为将来兼容定义保留
#define RESERVED_74 74 //为将来兼容定义保留
#define RESERVED_75 75 //为将来兼容定义保留
#define RESERVED_76 76 //为将来兼容定义保留
#define RESERVED_77 77 //为将来兼容定义保留
#define RESERVED_78 78 //为将来兼容定义保留
#define RESERVED_79 79 //为将来兼容定义保留
#define RESERVED_80 80 //为将来兼容定义保留
#define RESERVED_81 81 //为将来兼容定义保留
#define RESERVED_82 82 //为将来兼容定义保留
#define RESERVED_83 83 //为将来兼容定义保留
#define RESERVED_84 84 //为将来兼容定义保留
#define RESERVED_85 85 //为将来兼容定义保留
#define RESERVED_86 86 //为将来兼容定义保留
#define RESERVED_87 87 //为将来兼容定义保留
#define RESERVED_88 88 //为将来兼容定义保留
#define RESERVED_89 89 //为将来兼容定义保留
#define RESERVED_90 90 //为将来兼容定义保留
#define RESERVED_91 91 //为将来兼容定义保留
#define RESERVED_92 92 //为将来兼容定义保留
#define RESERVED_93 93 //为将来兼容定义保留
#define RESERVED_94 94 //为将来兼容定义保留
#define RESERVED_95 95 //为将来兼容定义保留
#define RESERVED_96 96 //为将来兼容定义保留
#define RESERVED_97 97 //为将来兼容定义保留
#define RESERVED_98 98 //为将来兼容定义保留
#define RESERVED_99 99 //为将来兼容定义保留

//在控制方向的系统命令
#define C_IC_NA_1 100 //总召唤命令
#define C_CI_NA_1 101 //计数量召唤命令
#define C_RD_NA_1 102 //读命令
#define C_CS_NA_1 103 //时钟同步命令
#define C_TS_NA_1 104 //测试命今
#define C_RP_NA_1 105 //复位进程命令
#define C_CD_NA_1 106 //延时获得命今
#define RESERVED_107 107 //为将来兼容定义保留
#define RESERVED_108 108 //为将来兼容定义保留
#define RESERVED_109 109 //为将来兼容定义保留

//在控制方向的参数命令
#define P_ME_NA_1 110 //测量值参数, 规一化值
#define P_ME_NB_1 111 //测量值参数, 标度化值
#define P_ME_NC_1 112 //测量值参数, 短浮点数
#define P_AC_NA_1 113 //参数激活
#define RESERVED_114 114 //为将来兼容定义保留
#define RESERVED_115 115 //为将来兼容定义保留
#define RESERVED_116 116 //为将来兼容定义保留
#define RESERVED_117 117 //为将来兼容定义保留
#define RESERVED_118 118 //为将来兼容定义保留
#define RESERVED_119 119 //为将来兼容定义保留

//文件传输
#define F_FR_NA_1 120 //文件准备就绪
#define F_SR_NA_1 121 //节准备就绪
#define F_SC_NA_1 122 //召唤目录, 选择文件, 召唤文件，召唤节
#define F_LS_NA_1 123 //最后的节,最后的段
#define F_AF_NA_1 124 //认可文件,认可节
#define F_SG_NA_1 125 //段
#define F_DR_TA_1 126 //目录
#define RESERVED_127 127 //为将来兼容定义保留

//定义控制域功能码(平衡传输)
//启动站向从动站传输报文控制域的功能码(PRM=1)
#define RST_LINK_FUNC 0x00
#define RST_PROCESS_FUNC 0x01
#define TEST_LINK_FUNC 0x02
#define USER_DATA_FUNC 0x03
#define NO_ANSWER_DATA_FUNC 0x04
#define LINK_STATUS_FUNC 0x09

//从动站向启动站传输报文控制域的功能码(PRM=0)
#define RETURN_OK_FUNC 0x00
#define RETURN_NO_OK_FUNC 0x01
#define ACK_LINK_STATUS 0x0B

#define S2M_DIR 1 //从从站(B站)到主站(A站)DIR为1
#define M2S_DIR 0 //从主站(A站)到从站(B站)DIR为0

typedef struct {
	uint8_t TypeID;
	uint8_t Qualifier;
	uint8_t Reason;
	uint16_t ASDU_Address;
	uint16_t InfoAddress;
	uint8_t InfoData[16];
} ASDU_DataStructure;

typedef struct {
	uint8_t Year;
	uint8_t Month;
	uint8_t Day;
	uint8_t Hour;
	uint8_t Min;
	uint8_t Sec;
	uint16_t MilliSec;
} TimeStructure;

extern unsigned char Time_Data[8];

extern uint8_t ProtocolRxBuffer[64]; //存储主站发过来的命令

extern uint16_t LINK_ADDRESS; //定义链路地址

uint8_t Protocol(uint8_t* pBuffer);

uint8_t DataProcess(void);

void LinkInit(void);

uint8_t ResponseCallAll(void);

void ChangeUpdate(uint16_t InfoAdress, uint8_t Info, TimeStructure* Time);

void TempChangeUpdate(uint16_t InfoAdress, uint8_t Info, TimeStructure* Time);
#endif //__101_PROTOCOL_H__
