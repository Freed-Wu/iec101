# MG301������ֲ˵��

## ������ֲ����

1. ��MG301.c�е�ATָ���MG323-B����ģ����ֲ��SIM7600CE����ģ�飻
2. ��ֲ�����й��ܱ�����ԭ��һ�£�
3. ��ֲ��ĳ���������µ�Ӳ��ƽ̨�����в������շ����ݡ�

## �ṩ����

1. MG301.c��MG301.h����Դ���룻
2. MG323-B����ģ��ATָ���ֲ᣻
3. SIM7600CE����ģ��ATָ���ֲᡣ

## �����޸�˵��

### �ϵ�ʱ��

![1-1](images/1-1.png "1-1")

![1-2](images/1-2.png "1-2")

![1-3](images/1-3.png "1-3")

�ϵ�ʱ��Ϊ�͵�ƽ����ﵽһ��ʱ�䣬�ο�����ֵ�������趨Ϊ500ms����GPIOB15���ߣ���ʱ500ms�󣬰�GPIOB15���͡�

### ����ģʽ��DEBUG\_MODE��

![2-1](images/2-1.png "2-1")

```{main.c}
#ifdef DEBUG_MODE
		GPIO_SetBits(GPIOB, GPIO_Pin_15); //����GPRSģ�鿪�����ŵ�ƽ
		DelayMs(500);
		GPIO_ResetBits(GPIOB, GPIO_Pin_15);
		USART3_InitRXbuf();
		USART3_SendDataToGPRS("AT+CSQ\r", strlen("AT+CSQ\r"));
		DelayMs(20);
		ReceiveLength = Supervise_USART3(ReceiveData);
		USART3_SendDataToGPRS("AT+CGDCONT=1,\"IP\",\"CTNET\"", strlen("AT+CGDCONT=1,\"IP\",\"CTNET\""));
		DelayMs(20);
		ReceiveLength = Supervise_USART3(ReceiveData);
		USART3_SendDataToGPRS("AT+CIPMODE=1", strlen("AT+CIPMODE=1"));
		DelayMs(20);
		ReceiveLength = Supervise_USART3(ReceiveData);
		USART3_SendDataToGPRS("AT+NETOPEN", strlen("AT+NETOPEN"));
		DelayMs(20);
		ReceiveLength = Supervise_USART3(ReceiveData);
		USART3_SendDataToGPRS("AT+CIPOPEN=0,\"TCP\",\"218.29.54.111\",20001", strlen("AT+CIPOPEN=0,\"TCP\",\"218.29.54.111\",20001"));
		DelayMs(20);
		ReceiveLength = Supervise_USART3(ReceiveData);
		USART3_SendDataToGPRS("Hello, TCP", strlen("Hello, TCP"));
		USART3_SendDataToGPRS("+++", strlen("+++"));
		DelayMs(20);
		ReceiveLength = Supervise_USART3(ReceiveData);
		USART3_SendDataToGPRS("ATO", strlen("ATO"));
		DelayMs(20);
		USART3_SendDataToGPRS("Hello, IP", strlen("Hello, IP"));
		USART3_SendDataToGPRS("+++", strlen("+++"));
		DelayMs(20);
		USART3_SendDataToGPRS("AT+CIPCLOSE=0", strlen("AT+CIPCLOSE=0"));
		DelayMs(20);
		USART3_SendDataToGPRS("AT+NETCLOSE", strlen("AT+NETCLOSE"));
		DelayMs(20);
```

����ģʽ����͸��ģʽ��������ģʽ�Լ���������֮����UARTͨ��ֱ�ӷ��Ͷ�ȡ���ݣ���ΪӲ��ͨѶ���ԡ�

����ģʽ����������ʾ��

### MG301.c�ļ��޸�

![3-1](images/3-1.png "3-1")

����ʹ��ʱ��������ģʽ�еĻ�����ģʽ������CIPSEND�������ݣ�CIPRXGET��ȡ������Ϣ��

MG301.c�ļ����޸���Ҫ��ATָ����޸��Լ��ϵ�ʱ����޸ġ�
