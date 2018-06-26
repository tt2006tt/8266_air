#include "user_config.h"




void ICACHE_FLASH_ATTR uart_send_getid(void){
	//��ģ���ȡid ��ȡ����������
	if(++uart_cnt >= 5){
		uart_poll = 9;
		uart_cnt = 0;
	}
	uint8_t textlen = 0;
	UartPackege[textlen] = UartStart;textlen++;//start
	UartPackege[textlen] = 0x65;textlen++;//Դ��ַ
	UartPackege[textlen] = 0x61;textlen++;//Ŀ���ַ
	UartPackege[textlen] = 0x01;textlen++;//����
	UartPackege[textlen] = UartGetId;textlen++;//command
#if(SENSOR_TYPE == MP_503)
	UartPackege[textlen] = 0x3C;textlen++;//stop 15
#else
	UartPackege[textlen] = UartStop;textlen++;//stop 15
#endif
	uart0_tx_buffer(UartPackege,textlen);
	//os_printf("\r\n uart_poll == %d\r\n",uart_poll);
	//os_printf("\r\n uart_cnt == %d\r\n",uart_cnt);


}

void uart_send_saveid(void){
	//�ѻ�ȡ����ID ������ģ����
	uint8_t textlen = 0,i=0;
	if(++uart_cnt >= 3){
		uart_cnt = 0;
		uart_poll = 1;
	}
	UartPackege[textlen] = UartStart;textlen++;//start0
	UartPackege[textlen] = 0x65;textlen++;//Դ��ַ1
	UartPackege[textlen] = 0x61;textlen++;//Ŀ���ַ2
	UartPackege[textlen] = 0x0B;textlen++;//����3
	UartPackege[textlen] = UartNewId_Save;textlen++;//command4
	for(i = 0;i<LENGTH_DEV_ID;i++){
		  UartPackege[textlen] = ProgramPara.Dev_ID[i];//5 6 7 8 9 10 11 12 13 14
		  textlen++;
	 }//װ������
	//textlen = textlen + LENGTH_DEV_ID;
	//os_memcpy(&UartPackege[textlen],ProgramPara.Dev_ID,LENGTH_DEV_ID);
#if(SENSOR_TYPE == MP_503)
	UartPackege[textlen] = 0x3C;textlen++;//stop 15
#else
	UartPackege[textlen] = UartStop;textlen++;//stop 15
#endif

	uart0_tx_buffer(UartPackege,textlen);
	os_memset(UartPackege,0,DataMax);

	//os_printf("\r\n uart_poll == %d\r\n",uart_poll);

}

void uart_test_ledcolor(void){
	//���Ե�����
	uint8_t textlen = 0;
	//if(++uart_cnt >= 2){
		//uart_cnt = 0;
		//uart_poll = 1;
	//}
	UartPackege[textlen] = UartStart;textlen++;//start
	UartPackege[textlen] = 0x65;textlen++;//Դ��ַ
	UartPackege[textlen] = 0x61;textlen++;//Ŀ���ַ
	UartPackege[textlen] = 0x03;textlen++;//����
	UartPackege[textlen] = Uartledcolor;textlen++;//command
	UartPackege[textlen] |= ProgramPara.Flag_Test;textlen++;//ѡ���ĸ�����
	UartPackege[textlen] |= 0xFF;textlen++;	//����
#if(SENSOR_TYPE == MP_503)
	UartPackege[textlen] = 0x3C;textlen++;//stop 15
#else
	UartPackege[textlen] = UartStop;textlen++;//stop 15
#endif
	uart0_tx_buffer(UartPackege,textlen);
	os_memset(UartPackege,0,DataMax);
	uart_poll = 1;
	//os_printf("\r\n uart_poll == %d\r\n",uart_poll);
	//os_printf("\r\n uart_cnt == %d\r\n",uart_cnt);


}
void uart_setppm(void)
{
	uint8_t textlen = 0;

	UartPackege[textlen] = UartStart;textlen++;//start
	UartPackege[textlen] = 0x65;textlen++;//Դ��ַ
	UartPackege[textlen] = 0x61;textlen++;//Ŀ���ַ
	UartPackege[textlen] = 0x05;textlen++;//����
	UartPackege[textlen] = UartSetPpm;textlen++;//command
	UartPackege[textlen] |= (ProgramPara.Alarm_H.Num >>24);textlen++;
	UartPackege[textlen] |= (ProgramPara.Alarm_H.Num >>16);textlen++;
	UartPackege[textlen] |= (ProgramPara.Alarm_H.Num >>8);textlen++;
	UartPackege[textlen] |= (ProgramPara.Alarm_H.Num >>0);textlen++;
#if(SENSOR_TYPE == MP_503)
	UartPackege[textlen] = 0x3C;textlen++;//stop 15
#else
	UartPackege[textlen] = UartStop;textlen++;//stop
#endif
	uart0_tx_buffer(UartPackege,textlen);
	os_memset(UartPackege,0,DataMax);
	//os_printf("\r\n uart_poll == %d\r\n",uart_poll);
	//os_printf("\r\n uart_cnt == %d\r\n",uart_cnt);
}

void ICACHE_FLASH_ATTR uart_waring(void){
	//����
	uint8_t textlen = 0;
	uart_cnt++;
	if(++uart_cnt >= 2){
		uart_cnt = 0;
		uart_poll = 1;
	}
	UartPackege[textlen] = UartStart;textlen++;//start
	UartPackege[textlen] = 0x65;textlen++;//Դ��ַ
	UartPackege[textlen] = 0x61;textlen++;//Ŀ���ַ  ����˭�������Ƿ����ߵ�ƽģ�鴦��
	UartPackege[textlen] = 0x01;textlen++;//����
	UartPackege[textlen] = UartWaring;textlen++;//command

#if(SENSOR_TYPE == MP_503)
	UartPackege[textlen] = 0x3C;textlen++;//stop 15
#else
	UartPackege[textlen] = UartStop;textlen++;//stop 15
#endif
	uart0_tx_buffer(UartPackege,textlen);
	os_memset(UartPackege,0,DataMax);
	//os_printf("\r\n uart_poll == %d\r\n",uart_poll);
	//os_printf("\r\n uart_cnt == %d\r\n",uart_cnt);

}

void ICACHE_FLASH_ATTR uart_smartconfig_done(void){
	//smartconfig���
	uint8_t textlen = 0;
	UartPackege[textlen] = UartStart;textlen++;//start
	UartPackege[textlen] = 0x65;textlen++;//Դ��ַ
	UartPackege[textlen] = 0x00;textlen++;//Ŀ���ַ������0x00����������ģ��
	UartPackege[textlen] = 0x02;textlen++;//����
	UartPackege[textlen] = UartSmartconfig;textlen++;//command
	UartPackege[textlen] = 0x01;textlen++;//mem �ɹ�
#if(SENSOR_TYPE == MP_503)
	UartPackege[textlen] = 0x3C;textlen++;//stop 15
#else
	UartPackege[textlen] = UartStop;textlen++;//stop 15
#endif
	uart0_tx_buffer(UartPackege,textlen);
	os_memset(UartPackege,0,DataMax);


}

void ICACHE_FLASH_ATTR uart_poll_ask(void){
	uint8_t textlen = 0;
	UartPackege[textlen] = UartStart;textlen++;//start
	UartPackege[textlen] = 0x65;textlen++;//Դ��ַ
	UartPackege[textlen] = 0x00;textlen++;//Ŀ���ַ������0x00����������ģ��
	UartPackege[textlen] = 0x01;textlen++;//����
	UartPackege[textlen] = UartPoll;textlen++;//command
#if(SENSOR_TYPE == MP_503)
	UartPackege[textlen] = 0x3C;textlen++;//stop 15
#else
	UartPackege[textlen] = UartStop;textlen++;//stop 15
#endif
	uart0_tx_buffer(UartPackege,textlen);
	//os_memset(UartPackege,0,DataMax);
	//os_printf("\r\n uart_poll == %d\r\n",uart_poll);

}

void ICACHE_FLASH_ATTR uart_airpoll_ask(void){
	uint8_t textlen = 0;
	UartPackege[textlen] = UartStart;textlen++;//start
	UartPackege[textlen] = 0x65;textlen++;//Դ��ַ
	UartPackege[textlen] = 0x00;textlen++;//Ŀ���ַ������0x00����������ģ��
	UartPackege[textlen] = 0x09;textlen++;//����
	UartPackege[textlen] = UartAirPoll;textlen++;//command
	//data
	UartPackege[textlen] = (PmData.PM25 >>24);textlen++;
	UartPackege[textlen] = (PmData.PM25 >>16);textlen++;
	UartPackege[textlen] = (PmData.PM25 >>8);textlen++;
	UartPackege[textlen] = (PmData.PM25 >>0);textlen++;
	UartPackege[textlen] = (PmData.PM10 >>24);textlen++;
	UartPackege[textlen] = (PmData.PM10 >>16);textlen++;
	UartPackege[textlen] = (PmData.PM10 >>8);textlen++;
	UartPackege[textlen] = (PmData.PM10 >>0);textlen++;

	UartPackege[textlen] = UartStop;textlen++;//stop 15

	uart0_tx_buffer(UartPackege,textlen);
	//os_memset(UartPackege,0,DataMax);
	//os_printf("\r\n uart_poll == %d  %d  %d %d\r\n",UartPackege[5],UartPackege[6],UartPackege[7],UartPackege[8]);

}

void ICACHE_FLASH_ATTR uart_stop_warning(void){
	uint8_t textlen = 0;
	if(++uart_cnt >= 2){
		uart_cnt = 0;
		uart_poll = 1;
	}
	UartPackege[textlen] = UartStart;textlen++;//start
	UartPackege[textlen] = 0x65;textlen++;//Դ��ַ
	UartPackege[textlen] = 0x61;textlen++;//Ŀ���ַ
	UartPackege[textlen] = 0x01;textlen++;//����
	UartPackege[textlen] = UartStopWarning;textlen++;//command
#if(SENSOR_TYPE == MP_503)
	UartPackege[textlen] = 0x3C;textlen++;//stop 15
#else
	UartPackege[textlen] = UartStop;textlen++;//stop 15
#endif
	uart0_tx_buffer(UartPackege,textlen);
	os_memset(UartPackege,0,DataMax);


}

//����Ϊ�ʴ�ģʽ
void ICACHE_FLASH_ATTR uart_set_answermode(void){
	uint8_t textlen = 0;
	if(++uart_cnt >= 2){
		uart_cnt = 0;
		uart_poll = 9;//������� ѯ��stm8l ����
	}
	UartPackege[textlen] = 0xFF;textlen++;//start FF 01 78 41 00 00 00 00 46
	UartPackege[textlen] = 0x01;textlen++;//����
	UartPackege[textlen] = 0x78;textlen++;//����
	UartPackege[textlen] = 0x41;textlen++;//�ʴ�
	UartPackege[textlen] = 0x00;textlen++;//����

	UartPackege[textlen] = 0x00;textlen++;//����
	UartPackege[textlen] = 0x00;textlen++;//����
	UartPackege[textlen] = 0x00;textlen++;//����
	UartPackege[textlen] = 0x46;textlen++;//У��
	uart0_tx_buffer(UartPackege,textlen);
	os_memset(UartPackege,0,DataMax);
	//os_printf("\r\nuart_set_answermode\r\n");
}

//ѯ�� sensor ģ����ֵ
void ICACHE_FLASH_ATTR uart_quespm(void){
	uint8_t textlen = 0;
	//uart_poll = 0;
	UartPackege[textlen] = 0xFF;textlen++;//start
	UartPackege[textlen] = 0x01;textlen++;//����
	UartPackege[textlen] = 0x86;textlen++;//����
	UartPackege[textlen] = 0x00;textlen++;//�ʴ�
	UartPackege[textlen] = 0x00;textlen++;//����

	UartPackege[textlen] = 0x00;textlen++;//����
	UartPackege[textlen] = 0x00;textlen++;//����
	UartPackege[textlen] = 0x00;textlen++;//����
	UartPackege[textlen] = 0x79;textlen++;//У��
	uart0_tx_buffer(UartPackege,textlen);
	os_memset(UartPackege,0,DataMax);
}
