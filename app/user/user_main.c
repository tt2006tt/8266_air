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
extern os_timer_t uart_main;//用串口轮询
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


			ProgramPara.SendMEM = 0;//1小时后可以重发警报
			SetBit(ProgramPara.flag_Time,BIT_HOUR);
			if(++ProgramPara.Cnt_Day >= CNT_DAY){//60
				//os_printf("\r\nCNT_DAY!!!!\r\n");
				ProgramPara.Cnt_Day = 0;
				SetBit(ProgramPara.flag_Time,BIT_DAY);
			}
		}
	}

	if(ProgramPara.Cnt_Minite >= CNT_70S && ProgramPara.SubMode != SMARTCONFIG_FINISH)//2分钟依然没连上wifi 关闭smartconfig
	{
			smartconfig_stop();
			uart_onoff = 1;
			ProgramPara.SubMode = SMARTCONFIG_FINISH;
			os_timer_disarm(&gas_main);
			os_timer_setfn(&gas_main, (os_timer_func_t *)gas_main_cb,0);
			os_timer_arm(&gas_main, 100, 1);//1s扫描一次
			Mode = Mode_Nomal;
			uint8_t textlen = 0;
        	UartPackege[textlen] = UartStart;textlen++;//start
        	UartPackege[textlen] = 0x65;textlen++;//源地址
        	UartPackege[textlen] = 0x00;textlen++;//目标地址，这里0x00代表发给所有模块
        	UartPackege[textlen] = 0x02;textlen++;//长度
        	UartPackege[textlen] = UartSmartconfig;textlen++;//command
        	UartPackege[textlen] = 0x02;textlen++;//mem 失败
        	UartPackege[textlen] = UartStop;textlen++;//stop 15

        	uart0_tx_buffer(UartPackege,textlen);
        	os_memset(UartPackege,0,DataMax);
        	Mode = Mode_Nomal;
			//os_printf("\r\n smartconfig_stop \r\n");
		}
}

/******************************************************************************
 * FunctionName : Mode_init
 * Description  : 开机初始化状态 用于检测是否开机连接上wifi如果没默认连上wifi就进入 smartconfig模式
 * Parameters   : void
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR Mode_init(void)
{
	struct	ip_info init_info;
	uint8_t i ;
	wifi_get_ip_info(STATION_IF,&init_info);

	if((strcmp((char *)ipaddr_ntoa(&init_info.ip),"0.0.0.0") !=0) && (ProgramPara.init_flag_time & BIT(BIT_10S)) )//已经连上wifi
	{
		//ClrBit(ProgramPara.init_flag_time,BIT_30S);//清除30s的标志
		ClrBit(ProgramPara.init_flag_time,BIT_10S);//清除10s的标志 让其重新计时

		Mode = Mode_Bias;
	}
	else if(ProgramPara.init_flag_time & BIT(BIT_30S))
	{
		ClrBit(ProgramPara.init_flag_time,BIT_10S);//清除10s的标志 让其重新计时
		ClrBit(ProgramPara.init_flag_time,BIT_30S);//清除30s的标志
		//os_printf("\r\n22222222222222222222\r\n");
		for(i = 0;i < 2;i++){
			UartPackege[len] = UartStart;len++;//start
			UartPackege[len] = 0x65;len++;//源地址
			UartPackege[len] = 0x61;len++;//目标地址
			UartPackege[len] = 0x01;len++;//长度
			UartPackege[len] = UartFirstLink;len++;//command
#if(SENSOR_TYPE == MP_503)
	UartPackege[len] = 0x3C;len++;//stop 15
#else
	UartPackege[len] = UartStop;len++;//stop 15
#endif
			uart0_tx_buffer(UartPackege,len);
		}
		Mode = Mode_Config;//未连接过wifi 进入smartconfig模式
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
		ClrBit(ProgramPara.init_flag_time,BIT_10S);//清除10s的标志 让其重新计时
		ClrBit(ProgramPara.init_flag_time,BIT_30S);
		Mode = Mode_CreateSession;
	}

}


void ICACHE_FLASH_ATTR connect_test_wifi(void){
	struct station_config station_cfg;
	uint8 ssid[]="dlink_edio";         //wifi名
	uint8 password[]="epform020716";     //wifi密码
	os_strcpy(station_cfg.ssid, ssid);          //ssid名称
	os_strcpy(station_cfg.password, password);  //密码
	wifi_station_set_config(&station_cfg);      //设置WIFI帐号和密码
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
			if((strcmp((char *)ipaddr_ntoa(&init_info.ip),"0.0.0.0") !=0))//已经连上wifi
			{
				//os_printf("test 2222 done!!!\r\n");
				CheckIpStart();
			}
			else
			{
				textlen = 0;
				UartPackege[textlen] = UartStart;textlen++;//start
				UartPackege[textlen] = 0x65;textlen++;//源地址
				UartPackege[textlen] = 0x00;textlen++;//目标地址，这里0x00代表发给所有模块
				UartPackege[textlen] = 0x02;textlen++;//长度
				UartPackege[textlen] = UartSmartconfig;textlen++;//command
				UartPackege[textlen] = 0x02;textlen++;//mem 失败

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
				UartPackege[textlen] = 0x65;textlen++;//源地址
				UartPackege[textlen] = 0x00;textlen++;//目标地址，这里0x00代表发给所有模块
				UartPackege[textlen] = 0x02;textlen++;//长度
				UartPackege[textlen] = UartSmartconfig;textlen++;//command
				UartPackege[textlen] = 0x02;textlen++;//mem 失败

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
		uart_poll = 1;//询问voc 温湿度
		return ;
	}
	else if(UartRecPackege[0] == 0x55 && UartRecPackege[1] == 0x61){//高电平模块 需要处理按键
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
				key_process();//按键处理
				//os_printf("\r\nProgramPara.PPM ssssss HHH === %d \r\n",ProgramPara.PPM_H);
				//os_printf("\r\nProgramPara.Key_Type  === %d  PPM_H ==%d tem ==%d hum ==%d;\r\n",ProgramPara.Key_Type,ProgramPara.PPM_H,ProgramPara.tem,ProgramPara.hum);
				uart_poll = 9;//询问pm2.5数值
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
			case 1://串口轮询问模块数据
				uart_airpoll_ask();
			break;
			case 2://保存ID到eeprom
				uart_send_saveid();
			break;
			case 3://获取存在eeprom的ID
				uart_send_getid();
			break;
			case 4://报警
				uart_waring();
			break;
			case 5://测试灯亮灭
				uart_test_ledcolor();
			break;
			case 6://smartconfig完成
				uart_smartconfig_done();
			break;
			case 7://停止警告
				uart_stop_warning();
			break;
			case 8://设置问答模式
				uart_set_answermode();
			break;
			case 9://询问pm2.5数值
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
		case Mode_CreateSession:	CheckIpStart();//websocket连接服务器
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
	system_set_os_print(1);//开关打印 log 功能 0关闭 1打开
	UART_SetPrintPort(1);//设置打印口默认为UART1  gpio2为 UART1的 tx脚 用于debug
    wifi_set_opmode(STATION_MODE);


    uart_poll = 8;
    uart_cnt = 0;

	system_update_cpu_freq(SYS_CPU_160MHZ);//
    //os_printf("\r\nSDK version:%s\n", system_get_sdk_version());
    ProgramPara.Cnt_ERROR = 1800;//错误灯（断开网络）180s后重启
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
	 * 串口发送
	 ********************************************/
	os_timer_disarm(&uart_main);
	os_timer_setfn(&uart_main, (os_timer_func_t *)uart_Tx,0);
	os_timer_arm(&uart_main, 1000, 1);//1s扫描一次

	/*********************************************
	 * 主程序
    ********************************************/
	os_timer_disarm(&gas_main);
	os_timer_setfn(&gas_main, (os_timer_func_t *)gas_main_cb,0);
	os_timer_arm(&gas_main, 100, 1);//1s扫描一次

   // hw_timer_init(0,1);//0:FRC1_SOURCE  1:NMI

    //hw_timer_set_func(gas_main_cb);

    //hw_timer_arm(1000000);//1s



}
