/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/1/1, v1.0 create this file.
*******************************************************************************/

#include "user_config.h"
#include "tcp_udp.h"

extern uint8_t uart_onoff;
extern os_timer_t uart_main;//�ô�����ѯ
extern volatile uint8_t WebSocketOK;



struct startup_config config;
struct startup_config configcmp;

void ICACHE_FLASH_ATTR time_calcu_cb(void)
{
	if((++Cnt_second >= CNT_10S) && ((Mode == Mode_Init)||(Mode == Mode_Bias))){
		SetBit(ProgramPara.init_flag_time,BIT_10S);
		Cnt_10second++;
		Cnt_second = 0;
		if(Cnt_10second == 2){
			Cnt_10second = 0;
			//os_printf("\r\nCNT_second!!!!\r\n");
			SetBit(ProgramPara.init_flag_time,BIT_30S);
		}
	}
	if(++ProgramPara.Cnt_Minite >= CNT_MINITE){
		ProgramPara.Cnt_Minite = 0;

		SetBit(ProgramPara.flag_Time,BIT_MINITE);
		if(++ProgramPara.Cnt_Hour>= CNT_HOUR){// 3
			ProgramPara.Cnt_Hour = 0;
			//os_printf("\r\nCNT_HOUR!!!!\r\n");


			ProgramPara.SendMEM = 0;//1Сʱ������ط�����
			SetBit(ProgramPara.flag_Time,BIT_HOUR);
			if(++ProgramPara.Cnt_Day >= CNT_DAY){//60
				//os_printf("\r\nCNT_DAY!!!!\r\n");
				ProgramPara.Cnt_Day = 0;
				SetBit(ProgramPara.flag_Time,BIT_DAY);
			}
		}
	}

	if(ProgramPara.Cnt_Minite >= CNT_70S && ProgramPara.SubMode != SMARTCONFIG_FINISH)//2������Ȼû����wifi �ر�smartconfig
	{
			smartconfig_stop();
			uart_onoff = 1;
			ProgramPara.SubMode = SMARTCONFIG_FINISH;
			os_timer_disarm(&gas_main);
			os_timer_setfn(&gas_main, (os_timer_func_t *)gas_main_cb,0);
			os_timer_arm(&gas_main, 100, 1);//1sɨ��һ��
			Mode = Mode_Nomal;
			uint8_t textlen = 0;
        	UartPackege[textlen] = UartStart;textlen++;//start
        	UartPackege[textlen] = 0x65;textlen++;//Դ��ַ
        	UartPackege[textlen] = 0x00;textlen++;//Ŀ���ַ������0x00����������ģ��
        	UartPackege[textlen] = 0x02;textlen++;//����
        	UartPackege[textlen] = UartSmartconfig;textlen++;//command
        	UartPackege[textlen] = 0x02;textlen++;//mem ʧ��
        	UartPackege[textlen] = UartStop;textlen++;//stop 15

        	uart0_tx_buffer(UartPackege,textlen);
        	os_memset(UartPackege,0,DataMax);
        	Mode = Mode_Nomal;
			//os_printf("\r\n smartconfig_stop \r\n");
		}
}

/******************************************************************************
 * FunctionName : Mode_init
 * Description  : ������ʼ��״̬ ���ڼ���Ƿ񿪻�������wifi���ûĬ������wifi�ͽ��� smartconfigģʽ
 * Parameters   : void
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR Mode_init(void)
{
	struct	ip_info init_info;
	uint8_t i ;
	wifi_get_ip_info(STATION_IF,&init_info);

	if((strcmp((char *)ipaddr_ntoa(&init_info.ip),"0.0.0.0") !=0) && (ProgramPara.init_flag_time & BIT(BIT_10S)) )//�Ѿ�����wifi
	{
		//ClrBit(ProgramPara.init_flag_time,BIT_30S);//���30s�ı�־
		ClrBit(ProgramPara.init_flag_time,BIT_10S);//���10s�ı�־ �������¼�ʱ

		Mode = Mode_Bias;
	}
	else if(ProgramPara.init_flag_time & BIT(BIT_30S))
	{
		ClrBit(ProgramPara.init_flag_time,BIT_10S);//���10s�ı�־ �������¼�ʱ
		ClrBit(ProgramPara.init_flag_time,BIT_30S);//���30s�ı�־
		//os_printf("\r\n22222222222222222222\r\n");
		for(i = 0;i < 2;i++){
			UartPackege[len] = UartStart;len++;//start
			UartPackege[len] = 0x65;len++;//Դ��ַ
			UartPackege[len] = 0x61;len++;//Ŀ���ַ
			UartPackege[len] = 0x01;len++;//����
			UartPackege[len] = UartFirstLink;len++;//command
#if(SENSOR_TYPE == MP_503)
	UartPackege[len] = 0x3C;len++;//stop 15
#else
	UartPackege[len] = UartStop;len++;//stop 15
#endif
			uart0_tx_buffer(UartPackege,len);
		}
		Mode = Mode_Config;//δ���ӹ�wifi ����smartconfigģʽ
	}
}


void ICACHE_FLASH_ATTR write_config(void)
{
	spi_flash_erase_sector(0x7C);
	spi_flash_write(0x7C * 4096, (uint32 *)&config, sizeof(config));
}


void ICACHE_FLASH_ATTR read_config(void)
{
	memset(&config, 0, sizeof(config));
	spi_flash_read(0x7C * 4096, (uint32 *)&config, sizeof(config));
}

void ICACHE_FLASH_ATTR InitOK(void)
{
	if((ProgramPara.init_flag_time & BIT(BIT_30S)))
	{
		ClrBit(ProgramPara.init_flag_time,BIT_10S);//���10s�ı�־ �������¼�ʱ
		ClrBit(ProgramPara.init_flag_time,BIT_30S);
		Mode = Mode_CreateSession;
	}

}


void ICACHE_FLASH_ATTR connect_test_wifi(void){
	struct station_config station_cfg;
	uint8 ssid[]="dlink_edio";         //wifi��
	uint8 password[]="epform020716";     //wifi����
	os_strcpy(station_cfg.ssid, ssid);          //ssid����
	os_strcpy(station_cfg.password, password);  //����
	wifi_station_set_config(&station_cfg);      //����WIFI�ʺź�����
	wifi_station_connect();
}

void ICACHE_FLASH_ATTR air_test(void)
{

	static uint8_t i = 0;
	static uint8_t status = 0;
	struct	ip_info init_info;
	uint8_t textlen = 0;
	wifi_get_ip_info(STATION_IF,&init_info);


	if(++i > 20)
	{
		i = 0;
		if(status == 0)
		{
			status = 1;
			connect_test_wifi();
		}
		else if(status == 1)
		{
			status = 2;

		}
		else if(status == 2)
		{

			status = 3;
		}
		else if(status == 3)
		{
			status = 4;

		}
		else if(status == 4)
		{
			status = 5;
		}
		else if(status == 5)
		{
			status = 6;
		}
		else if(status == 6)
		{
			if((strcmp((char *)ipaddr_ntoa(&init_info.ip),"0.0.0.0") !=0))//�Ѿ�����wifi
			{
				//os_printf("test 2222 done!!!\r\n");
				CheckIpStart();
			}
			else
			{
				textlen = 0;
				UartPackege[textlen] = UartStart;textlen++;//start
				UartPackege[textlen] = 0x65;textlen++;//Դ��ַ
				UartPackege[textlen] = 0x00;textlen++;//Ŀ���ַ������0x00����������ģ��
				UartPackege[textlen] = 0x02;textlen++;//����
				UartPackege[textlen] = UartSmartconfig;textlen++;//command
				UartPackege[textlen] = 0x02;textlen++;//mem ʧ��

				UartPackege[textlen] = UartStop;textlen++;//stop 15

				uart0_tx_buffer(UartPackege,textlen);
			}
			status = 0;
		}
		else if(status == 7)
		{
			status = 8;
		}
		else if(status == 8)
		{
			status = 9;
		}
		else if(status == 9)
		{
			status = 0;
			if(WebSocketOK == 1){
				//os_printf("done!!!!!\r\n");
			}
			else{
				//os_printf("error!!!!!\r\n");
				textlen = 0;
				UartPackege[textlen] = UartStart;textlen++;//start
				UartPackege[textlen] = 0x65;textlen++;//Դ��ַ
				UartPackege[textlen] = 0x00;textlen++;//Ŀ���ַ������0x00����������ģ��
				UartPackege[textlen] = 0x02;textlen++;//����
				UartPackege[textlen] = UartSmartconfig;textlen++;//command
				UartPackege[textlen] = 0x02;textlen++;//mem ʧ��

				UartPackege[textlen] = UartStop;textlen++;//stop 15

				uart0_tx_buffer(UartPackege,textlen);
			}
		}

	}
}

void ICACHE_FLASH_ATTR key_process(void)
{
	static uint8_t warning_cout = 0;

	if(ProgramPara.Key_Type & BIT(BIT_TEST)){
		os_printf("test mode!!!!\r\n");
		WebSocketOK = 0;
		ProgramPara.Key_Type = 0;
		wifi_station_disconnect();
		Mode = Mode_Test;
	}
	else if(ProgramPara.Key_Type & BIT(BIT_LONG)){
		WebSocketOK = 0;
		ProgramPara.Key_Type = 0;
		Mode = Mode_Config;
	}

}


void uart_Tx(void)
{

	uint8_t i = 0;
	//os_printf("uart_poll == %d\r\n",Mode);
	time_calcu_cb();
	if(uart_onoff == 0) return ;

	if(UartRecPackege[0] == 0xFF && UartRecPackege[1] == 0x86){//sensor
		PmData.PM25 = (uint32_t)((UartRecPackege[2] << 8) |(UartRecPackege[3]));
		PmData.PM10 = (uint32_t)((UartRecPackege[4] << 8) |(UartRecPackege[5]));
		//PmData.PM1_0 = (uint16_t)((UartRecPackege[6] << 8) |(UartRecPackege[7]));
		os_memset(UartRecPackege,0,DataMax);
		//os_printf("\r\n sensor answer %d, %d\r\n",PmData.PM25,PmData.PM10);
		uart_poll = 1;//ѯ��voc ��ʪ��
		return ;
	}
	else if(UartRecPackege[0] == 0x55 && UartRecPackege[1] == 0x61){//�ߵ�ƽģ�� ��Ҫ������
		uart_dst_addr = 0x61;
		switch(UartRecPackege[4]){
			case UartAirPoll_Ack:
				ProgramPara.PPM_H = (uint32_t)((uint32_t)(UartRecPackege[5] << 24) |(uint32_t) (UartRecPackege[6] << 16)|(uint32_t)(UartRecPackege[7] << 8) |(uint32_t) UartRecPackege[8]);
				ProgramPara.tem = (uint32_t)((uint32_t)(UartRecPackege[9] << 24) |(uint32_t) (UartRecPackege[10] << 16)|(uint32_t)(UartRecPackege[11] << 8) |(uint32_t) UartRecPackege[12]);
				ProgramPara.hum = (uint32_t)((uint32_t)(UartRecPackege[13] << 24) |(uint32_t) (UartRecPackege[14] << 16)|(uint32_t)(UartRecPackege[15] << 8) |(uint32_t) UartRecPackege[16]);
				ProgramPara.Key_Type = UartRecPackege[17];
				if(Mode == Mode_Nomal || Mode == Mode_Test){
					ProgramPara.Key_Type = 0;
				}
				//os_printf("\r\nProgramPara.Key_Type ssssss  === %d \r\n",ProgramPara.Key_Type);
				key_process();//��������
				//os_printf("\r\nProgramPara.PPM ssssss HHH === %d \r\n",ProgramPara.PPM_H);
				//os_printf("\r\nProgramPara.Key_Type  === %d  PPM_H ==%d tem ==%d hum ==%d;\r\n",ProgramPara.Key_Type,ProgramPara.PPM_H,ProgramPara.tem,ProgramPara.hum);
				uart_poll = 9;//ѯ��pm2.5��ֵ
				os_memset(UartRecPackege,0,DataMax);
				return ;
			break;
			case UartGetId_ACK:
				for(i = 0;i < LENGTH_DEV_ID;i++){
					ProgramPara.Dev_ID[i] = UartRecPackege[5+i];
				}

				//os_printf("\r\nProgramPara.ID IDDDD === %s \r\n",ProgramPara.Dev_ID);
				os_memset(UartRecPackege,0,DataMax);
				uart_poll = 9;
				return ;
			break;
		}
	}
	if(uart_poll != 0){
		switch(uart_poll){
			case 1://������ѯ��ģ������
				uart_airpoll_ask();
			break;
			case 2://����ID��eeprom
				uart_send_saveid();
			break;
			case 3://��ȡ����eeprom��ID
				uart_send_getid();
			break;
			case 4://����
				uart_waring();
			break;
			case 5://���Ե�����
				uart_test_ledcolor();
			break;
			case 6://smartconfig���
				uart_smartconfig_done();
			break;
			case 7://ֹͣ����
				uart_stop_warning();
			break;
			case 8://�����ʴ�ģʽ
				uart_set_answermode();
			break;
			case 9://ѯ��pm2.5��ֵ
				uart_quespm();
			break;
		}
	}


}
void gas_main_cb(void)
{


	switch(Mode)
	{
		case Mode_Init:				Mode_init();
		break;
		case Mode_Bias:				InitOK();
		break;
		case Mode_CreateSession:	CheckIpStart();//websocket���ӷ�����
		break;
		case Mode_Config:         	ProgramPara.Cnt_Minite = 0;gas_smartconfig();//os_printf("\r\n!!!!!text!!!\r\n");
	    break;
		case Mode_Nomal:			networking();
		break;
		case Mode_Test:				air_test();
		break;
		case Mode_Wait:
		break;
	}
}

void user_init(void)
{
	uint8_t i;
	uart_init(9600,9600);
	system_set_os_print(1);//���ش�ӡ log ���� 0�ر� 1��
	UART_SetPrintPort(1);//���ô�ӡ��Ĭ��ΪUART1  gpio2Ϊ UART1�� tx�� ����debug
    wifi_set_opmode(STATION_MODE);


    uart_poll = 8;
    uart_cnt = 0;

	system_update_cpu_freq(SYS_CPU_160MHZ);//
    //os_printf("\r\nSDK version:%s\n", system_get_sdk_version());
    ProgramPara.Cnt_ERROR = 1800;//����ƣ��Ͽ����磩180s������
    Mode = Mode_Init;


    os_memset(&config, 0, sizeof(config));
    spi_flash_read(0x7C * 4096, (uint32 *)&config, sizeof(config));
    os_memset(&configcmp, 0, sizeof(configcmp));
    spi_flash_read(0x7D * 4096, (uint32 *)&configcmp, sizeof(configcmp));

    if((os_memcmp(config.Dev_id,configcmp.Dev_id,10) == 0) && (configcmp.Dev_id[0] == 'D') && (config.Dev_id[0] == 'D'))
    {
    	os_memcpy(ProgramPara.Dev_ID,config.Dev_id,10);

    }
    else
    {
    	os_memcpy(ProgramPara.Dev_ID,"DAFXXXXXXX",10);
    	os_printf("\r\nProgramPara.Dev_ID error~~~~~~~~\r\n");
    }
	/*********************************************
	 * ���ڷ���
	 ********************************************/
	os_timer_disarm(&uart_main);
	os_timer_setfn(&uart_main, (os_timer_func_t *)uart_Tx,0);
	os_timer_arm(&uart_main, 1000, 1);//1sɨ��һ��

	/*********************************************
	 * ������
    ********************************************/
	os_timer_disarm(&gas_main);
	os_timer_setfn(&gas_main, (os_timer_func_t *)gas_main_cb,0);
	os_timer_arm(&gas_main, 100, 1);//1sɨ��һ��

   // hw_timer_init(0,1);//0:FRC1_SOURCE  1:NMI

    //hw_timer_set_func(gas_main_cb);

    //hw_timer_arm(1000000);//1s



}
