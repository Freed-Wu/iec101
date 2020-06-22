#ifndef __101_PROTOCOL_H__
#define __101_PROTOCOL_H__
#include "stm32f10x.h"

#define INVALID_DATA 0
#define VARIABLE_DATA 1
#define STABLE_DATA 2

//�ڼ��ӷ����ϵĹ�����Ϣ
#define NULL 0 //δ����
#define M_SP_NA_1 1 //������Ϣ
#define M_SP_TA_1 2 //��ʱ��ĵ�����Ϣ
#define M_DP_NA_1 3 //˫����Ϣ
#define M_DP_TA_1 4 //��ʱ���˫����Ϣ
#define M_ST_NA_1 5 //��λ����Ϣ
#define M_ST_TA_1 6 //��ʱ��Ĳ�λ����Ϣ
#define M_BO_NA_1 7 //32���ش�
#define M_BO_TA_1 8 //��ʱ���32���ش�
#define M_ME_NA_1 9 //����ֵ����һ��ֵ
#define M_ME_TA_1 10 //����ֵ����ʱ��Ĺ�һ��ֵ
#define M_ME_NB_1 11 //����ֵ����Ȼ�ֵ
#define M_ME_TB_1 12 //����ֵ����ʱ��ı�Ȼ�ֵ
#define M_ME_NC_1 13 //����ֵ���̸�����
#define M_ME_TC_1 14 //����ֵ����ʱ��Ķ̸�����
#define M_IT_NA_1 15 //�ۼ���
#define M_IT_TA_1 16 //��ʱ����ۼ���
#define M_EP_TA_1 17 //��ʱ��ļ̵��豸�����¼�
#define M_EP_TB_1 18 //ʱ��ļ̵籣���豸���������¼�
#define M_EP_TC_1 19 //��ʱ��ļ̵籣���豸���������·��Ϣ
#define M_PS_NA_1 20 //����λ����ĳ��鵥����Ϣ
#define M_ME_ND_1 21 //����ֵ������Ʒ�������ʵĹ�һ��ֵ
#define RESERVED_22 22 //Ϊ�������ݶ��屣��
#define RESERVED_23 23 //Ϊ�������ݶ��屣��
#define RESERVED_24 24 //Ϊ�������ݶ��屣��
#define RESERVED_25 25 //Ϊ�������ݶ��屣��
#define RESERVED_26 26 //Ϊ�������ݶ��屣��
#define RESERVED_27 27 //Ϊ�������ݶ��屣��
#define RESERVED_28 28 //Ϊ�������ݶ��屣��
#define RESERVED_29 29 //Ϊ�������ݶ��屣��
#define M_SP_TB_1 30 //��CP56Time2aʱ��ĵ�����Ϣ
#define M_DP_TB_1 31 //��CP56Time2aʱ���˫����Ϣ
#define M_ST_TB_1 32 //��CP56Time2aʱ��Ĳ�λ����Ϣ
#define M_BO_TB_1 33 //��CP56Time2aʱ���32���ش�
#define M_ME_TD_1 34 //��CP56Time2aʱ��Ĳ���ֵ, ��һ��ֵ
#define M_ME_TE_1 35 //��CP56Time2aʱ��Ĳ���ֵ, ��Ȼ�ֵ
#define M_ME_TF_1 36 //��CP56Time2aʱ��Ĳ���ֵ, �̸�����
#define M_IT_TB_1 37 //��CP56Time2aʱ����ۼ���
#define M_EP_TD_1 38 //��CP56Time2aʱ��ļ̵籣���豸�¼�
#define M_EP_TE_1 39 //��CP56Time2aʱ��ļ̵籣���豸���������¼�
#define M_EP_TF_1 40 //��CP56Time2aʱ��ļ̵籣���豸���������·��Ϣ
#define RESERVED_41 41 //Ϊ�������ݶ��屣��
#define RESERVED_42 42 //Ϊ�������ݶ��屣��
#define RESERVED_43 43 //Ϊ�������ݶ��屣��
#define RESERVED_44 44 //Ϊ�������ݶ��屣��

//�ڿ��Ʒ����ϵĹ�����Ϣ
#define C_SC_NA_1 45 //��������
#define C_DC_NA_1 46 //˫������
#define C_RC_NA_1 47 //����������
#define C_SE_NA_1 48 //�趨ֵ����, ��һ��ֵ
#define C_SE_NB_1 49 //�趨ֵ����, ��Ȼ�ֵ
#define C_SE_NC_1 50 //�趨ֵ����, �̸�����
#define C_BO_NA_1 51 //32���ش�
#define RESERVED_52 52 //Ϊ�������ݶ��屣��
#define RESERVED_53 53 //Ϊ�������ݶ��屣��
#define RESERVED_54 54 //Ϊ�������ݶ��屣��
#define RESERVED_55 55 //Ϊ�������ݶ��屣��
#define RESERVED_56 56 //Ϊ�������ݶ��屣��
#define RESERVED_57 57 //Ϊ�������ݶ��屣��
#define RESERVED_58 58 //Ϊ�������ݶ��屣��
#define RESERVED_59 59 //Ϊ�������ݶ��屣��
#define RESERVED_60 60 //Ϊ�������ݶ��屣��
#define RESERVED_61 61 //Ϊ�������ݶ��屣��
#define RESERVED_62 62 //Ϊ�������ݶ��屣��
#define RESERVED_63 63 //Ϊ�������ݶ��屣��
#define RESERVED_64 64 //Ϊ�������ݶ��屣��
#define RESERVED_65 65 //Ϊ�������ݶ��屣��
#define RESERVED_66 66 //Ϊ�������ݶ��屣��
#define RESERVED_67 67 //Ϊ�������ݶ��屣��
#define RESERVED_68 68 //Ϊ�������ݶ��屣��
#define RESERVED_69 69 //Ϊ�������ݶ��屣��

//�ڼ��ӷ����ϵͳ����
#define M_EI_NA_1 70 //��ʼ������
#define RESERVED_71 71 //Ϊ�������ݶ��屣��
#define RESERVED_72 72 //Ϊ�������ݶ��屣��
#define RESERVED_73 73 //Ϊ�������ݶ��屣��
#define RESERVED_74 74 //Ϊ�������ݶ��屣��
#define RESERVED_75 75 //Ϊ�������ݶ��屣��
#define RESERVED_76 76 //Ϊ�������ݶ��屣��
#define RESERVED_77 77 //Ϊ�������ݶ��屣��
#define RESERVED_78 78 //Ϊ�������ݶ��屣��
#define RESERVED_79 79 //Ϊ�������ݶ��屣��
#define RESERVED_80 80 //Ϊ�������ݶ��屣��
#define RESERVED_81 81 //Ϊ�������ݶ��屣��
#define RESERVED_82 82 //Ϊ�������ݶ��屣��
#define RESERVED_83 83 //Ϊ�������ݶ��屣��
#define RESERVED_84 84 //Ϊ�������ݶ��屣��
#define RESERVED_85 85 //Ϊ�������ݶ��屣��
#define RESERVED_86 86 //Ϊ�������ݶ��屣��
#define RESERVED_87 87 //Ϊ�������ݶ��屣��
#define RESERVED_88 88 //Ϊ�������ݶ��屣��
#define RESERVED_89 89 //Ϊ�������ݶ��屣��
#define RESERVED_90 90 //Ϊ�������ݶ��屣��
#define RESERVED_91 91 //Ϊ�������ݶ��屣��
#define RESERVED_92 92 //Ϊ�������ݶ��屣��
#define RESERVED_93 93 //Ϊ�������ݶ��屣��
#define RESERVED_94 94 //Ϊ�������ݶ��屣��
#define RESERVED_95 95 //Ϊ�������ݶ��屣��
#define RESERVED_96 96 //Ϊ�������ݶ��屣��
#define RESERVED_97 97 //Ϊ�������ݶ��屣��
#define RESERVED_98 98 //Ϊ�������ݶ��屣��
#define RESERVED_99 99 //Ϊ�������ݶ��屣��

//�ڿ��Ʒ����ϵͳ����
#define C_IC_NA_1 100 //���ٻ�����
#define C_CI_NA_1 101 //�������ٻ�����
#define C_RD_NA_1 102 //������
#define C_CS_NA_1 103 //ʱ��ͬ������
#define C_TS_NA_1 104 //��������
#define C_RP_NA_1 105 //��λ��������
#define C_CD_NA_1 106 //��ʱ�������
#define RESERVED_107 107 //Ϊ�������ݶ��屣��
#define RESERVED_108 108 //Ϊ�������ݶ��屣��
#define RESERVED_109 109 //Ϊ�������ݶ��屣��

//�ڿ��Ʒ���Ĳ�������
#define P_ME_NA_1 110 //����ֵ����, ��һ��ֵ
#define P_ME_NB_1 111 //����ֵ����, ��Ȼ�ֵ
#define P_ME_NC_1 112 //����ֵ����, �̸�����
#define P_AC_NA_1 113 //��������
#define RESERVED_114 114 //Ϊ�������ݶ��屣��
#define RESERVED_115 115 //Ϊ�������ݶ��屣��
#define RESERVED_116 116 //Ϊ�������ݶ��屣��
#define RESERVED_117 117 //Ϊ�������ݶ��屣��
#define RESERVED_118 118 //Ϊ�������ݶ��屣��
#define RESERVED_119 119 //Ϊ�������ݶ��屣��

//�ļ�����
#define F_FR_NA_1 120 //�ļ�׼������
#define F_SR_NA_1 121 //��׼������
#define F_SC_NA_1 122 //�ٻ�Ŀ¼, ѡ���ļ�, �ٻ��ļ����ٻ���
#define F_LS_NA_1 123 //���Ľ�,���Ķ�
#define F_AF_NA_1 124 //�Ͽ��ļ�,�Ͽɽ�
#define F_SG_NA_1 125 //��
#define F_DR_TA_1 126 //Ŀ¼
#define RESERVED_127 127 //Ϊ�������ݶ��屣��

//�������������(ƽ�⴫��)
//����վ��Ӷ�վ���䱨�Ŀ�����Ĺ�����(PRM=1)
#define RST_LINK_FUNC 0x00
#define RST_PROCESS_FUNC 0x01
#define TEST_LINK_FUNC 0x02
#define USER_DATA_FUNC 0x03
#define NO_ANSWER_DATA_FUNC 0x04
#define LINK_STATUS_FUNC 0x09

//�Ӷ�վ������վ���䱨�Ŀ�����Ĺ�����(PRM=0)
#define RETURN_OK_FUNC 0x00
#define RETURN_NO_OK_FUNC 0x01
#define ACK_LINK_STATUS 0x0B

#define S2M_DIR 1 //�Ӵ�վ(Bվ)����վ(Aվ)DIRΪ1
#define M2S_DIR 0 //����վ(Aվ)����վ(Bվ)DIRΪ0

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

extern uint8_t ProtocolRxBuffer[64]; //�洢��վ������������

extern uint16_t LINK_ADDRESS; //������·��ַ

uint8_t Protocol(uint8_t* pBuffer);

uint8_t DataProcess(void);

void LinkInit(void);

uint8_t ResponseCallAll(void);

void ChangeUpdate(uint16_t InfoAdress, uint8_t Info, TimeStructure* Time);

void TempChangeUpdate(uint16_t InfoAdress, uint8_t Info, TimeStructure* Time);
#endif //__101_PROTOCOL_H__
