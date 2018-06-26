#include "user_config.h"




void ICACHE_FLASH_ATTR uart_send_getid(void){
	//从模块获取id 获取多次以免出错
	if(++uart_cnt >= 5){
		uart_poll = 9;
		uart_cnt = 0;
	}
	uint8_t textlen = 0;
	UartPackege[textlen] = UartStart;textlen++;//start
	UartPackege[textlen] = 0x65;textlen++;//源地址
	UartPackege[textlen] = 0x61;textlen++;//目标地址
	UartPackege[textlen] = 0x01;textlen++;//长度
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
	//把获取到的ID 保存在模块中
	uint8_t textlen = 0,i=0;
	if(++uart_cnt >= 3){
		uart_cnt = 0;
		uart_poll = 1;
	}
	UartPackege[textlen] = UartStart;textlen++;//start0
	UartPackege[textlen] = 0x65;textlen++;//源地址1
	UartPackege[textlen] = 0x61;textlen++;//目标地址2
	UartPackege[textlen] = 0x0B;textlen++;//长度3
	UartPackege[textlen] = UartNewId_Save;textlen++;//command4
	for(i = 0;i<LENGTH_DEV_ID;i++){
		  UartPackege[textlen] = ProgramPara.Dev_ID[i];//5 6 7 8 9 10 11 12 13 14
		  textlen++;
	 }//装载数据
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
	//测试灯亮灭
	uint8_t textlen = 0;
	//if(++uart_cnt >= 2){
		//uart_cnt = 0;
		//uart_poll = 1;
	//}
	UartPackege[textlen] = UartStart;textlen++;//start
	UartPackege[textlen] = 0x65;textlen++;//源地址
	UartPackege[textlen] = 0x61;textlen++;//目标地址
	UartPackege[textlen] = 0x03;textlen++;//长度
	UartPackege[textlen] = Uartledcolor;textlen++;//command
	UartPackege[textlen] |= ProgramPara.Flag_Test;textlen++;//选择哪个灯亮
	UartPackege[textlen] |= 0xFF;textlen++;	//亮度
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
	UartPackege[textlen] = 0x65;textlen++;//源地址
	UartPackege[textlen] = 0x61;textlen++;//目标地址
	UartPackege[textlen] = 0x05;textlen++;//长度
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
	//报警
	uint8_t textlen = 0;
	uart_cnt++;
	if(++uart_cnt >= 2){
		uart_cnt = 0;
		uart_poll = 1;
	}
	UartPackege[textlen] = UartStart;textlen++;//start
	UartPackege[textlen] = 0x65;textlen++;//源地址
	UartPackege[textlen] = 0x61;textlen++;//目标地址  无论谁报警都是发给高电平模块处理
	UartPackege[textlen] = 0x01;textlen++;//长度
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
	//smartconfig完成
	uint8_t textlen = 0;
	UartPackege[textlen] = UartStart;textlen++;//start
	UartPackege[textlen] = 0x65;textlen++;//源地址
	UartPackege[textlen] = 0x00;textlen++;//目标地址，这里0x00代表发给所有模块
	UartPackege[textlen] = 0x02;textlen++;//长度
	UartPackege[textlen] = UartSmartconfig;textlen++;//command
	UartPackege[textlen] = 0x01;textlen++;//mem 成功
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
	UartPackege[textlen] = 0x65;textlen++;//源地址
	UartPackege[textlen] = 0x00;textlen++;//目标地址，这里0x00代表发给所有模块
	UartPackege[textlen] = 0x01;textlen++;//长度
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
	UartPackege[textlen] = 0x65;textlen++;//源地址
	UartPackege[textlen] = 0x00;textlen++;//目标地址，这里0x00代表发给所有模块
	UartPackege[textlen] = 0x09;textlen++;//长度
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
	UartPackege[textlen] = 0x65;textlen++;//源地址
	UartPackege[textlen] = 0x61;textlen++;//目标地址
	UartPackege[textlen] = 0x01;textlen++;//长度
	UartPackege[textlen] = UartStopWarning;textlen++;//command
#if(SENSOR_TYPE == MP_503)
	UartPackege[textlen] = 0x3C;textlen++;//stop 15
#else
	UartPackege[textlen] = UartStop;textlen++;//stop 15
#endif
	uart0_tx_buffer(UartPackege,textlen);
	os_memset(UartPackege,0,DataMax);


}

//设置为问答模式
void ICACHE_FLASH_ATTR uart_set_answermode(void){
	uint8_t textlen = 0;
	if(++uart_cnt >= 2){
		uart_cnt = 0;
		uart_poll = 9;//设置完后 询问stm8l 数据
	}
	UartPackege[textlen] = 0xFF;textlen++;//start FF 01 78 41 00 00 00 00 46
	UartPackege[textlen] = 0x01;textlen++;//保留
	UartPackege[textlen] = 0x78;textlen++;//命令
	UartPackege[textlen] = 0x41;textlen++;//问答
	UartPackege[textlen] = 0x00;textlen++;//保留

	UartPackege[textlen] = 0x00;textlen++;//保留
	UartPackege[textlen] = 0x00;textlen++;//保留
	UartPackege[textlen] = 0x00;textlen++;//保留
	UartPackege[textlen] = 0x46;textlen++;//校验
	uart0_tx_buffer(UartPackege,textlen);
	os_memset(UartPackege,0,DataMax);
	//os_printf("\r\nuart_set_answermode\r\n");
}

//询问 sensor 模块数值
void ICACHE_FLASH_ATTR uart_quespm(void){
	uint8_t textlen = 0;
	//uart_poll = 0;
	UartPackege[textlen] = 0xFF;textlen++;//start
	UartPackege[textlen] = 0x01;textlen++;//保留
	UartPackege[textlen] = 0x86;textlen++;//命令
	UartPackege[textlen] = 0x00;textlen++;//问答
	UartPackege[textlen] = 0x00;textlen++;//保留

	UartPackege[textlen] = 0x00;textlen++;//保留
	UartPackege[textlen] = 0x00;textlen++;//保留
	UartPackege[textlen] = 0x00;textlen++;//保留
	UartPackege[textlen] = 0x79;textlen++;//校验
	uart0_tx_buffer(UartPackege,textlen);
	os_memset(UartPackege,0,DataMax);
}
