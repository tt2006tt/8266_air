#include "networking.h"
#include "user_config.h"

uint8_t DatePackege[DataMax] = {0};

uint8_t primaryKey[16] = {0};//除注册和登录，其他的都用私钥
volatile uint8_t Receive = 0;
const uint8_t hardversion = 0x02;
const uint32_t softversion = 171011;
uint8_t Postdata[33] = "action=saveDevid&devid=";
extern uint8_t gassmartconfig;
extern char ubWebRxBuf[RXMAX];
extern uint8_t map_addlen;
extern volatile uint8_t HttpOK;
extern struct espconn http_tcp_conn;
void Reflash(uint16_t command,uint8_t receive,uint16_t waittime)
{
    DataFormat.command = command;
    Receive = receive;
    os_memset(ubWebRxBuf,0,RXMAX);
    //os_delay_us(1000);
}

void ICACHE_FLASH_ATTR Post_Id(void)
{
	uint8_t postdata1[33] = "action=saveDevid&devid=";
	char pbuf[194] = {0};
	postdata1[23] = ProgramPara.Dev_ID[0];
	postdata1[24] = ProgramPara.Dev_ID[1];
	postdata1[25] = ProgramPara.Dev_ID[2];
	postdata1[26] = ProgramPara.Dev_ID[3];
	postdata1[27] = ProgramPara.Dev_ID[4];
	postdata1[28] = ProgramPara.Dev_ID[5];
	postdata1[29] = ProgramPara.Dev_ID[6];
	postdata1[30] = ProgramPara.Dev_ID[7];
	postdata1[31] = ProgramPara.Dev_ID[8];
	postdata1[32] = ProgramPara.Dev_ID[9];
	os_sprintf(pbuf,"POST /%s HTTP/1.1\r\nAccept: */*\r\nContent-Length: %d\r\nContent-Type: application/x-www-form-urlencoded\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n%s",
			"baas/tianhesm",sizeof(postdata1),"flytronlink.net",postdata1);//
	//os_printf("\r\npost 2222222  %s !!!!!!!!! %d\r\n",pbuf,os_strlen(pbuf));
	espconn_send(&http_tcp_conn, pbuf, 194);
}
void networking(void)
{
	static uint8_t i = 0;
	static y = 0;
	uint8_t x = 0;

	if(WebSocketOK)
	{
		if(ProgramPara.Cnt_ERROR >0) //超过3分钟依然发送上心跳
		{
			ProgramPara.Cnt_ERROR--;
		}
		else if(ProgramPara.Cnt_ERROR == 0)
		{
			ProgramPara.Cnt_ERROR = 1800;
			espconn_disconnect(&websocket_tcp_conn);
			//os_printf("disconnect 3333\r\n");
			Cnt_second = 0;
			Cnt_10second = 0;
			ClrBit(ProgramPara.init_flag_time,BIT_10S);
			ClrBit(ProgramPara.init_flag_time,BIT_30S);
			WebSocketOK = 0;
			HttpOK = 0;
			Mode = Mode_Bias;
			return ;
		}
		if(gassmartconfig == 1
				&& ProgramPara.Dev_ID[2] != 'F')
		{
			gassmartconfig = 1;
			Post_Id();
		}
		ESP8266_Monitor();
		if((ProgramPara.Status & BIT(BIT_LOGINOK)))
		{//登录成功if(programpara->Flag_LogInOK)
			//os_printf("\r\n BIT_LOGINOK \r\n");
			if((ProgramPara.flag_Time & BIT(BIT_MINITE)))
			{
		//  每两分钟发一次心跳（Debug中只用20s）
				ClrBit(ProgramPara.flag_Time,BIT_MINITE);
				DataFormat.command = COMMAND_HEARTRATE;
			}
			if((ProgramPara.flag_Time & BIT(BIT_HOUR)))
			{
				//每小时上传一次PPM（Debug中只用60s）
				ClrBit(ProgramPara.flag_Time,BIT_HOUR);
				DataFormat.command = COMMAND_GETTIME;
			}
			if((ProgramPara.flag_Time & BIT(BIT_DAY)))
			{
				//两天检查一次固件(Debug中只用60分钟)
				ClrBit(ProgramPara.flag_Time,BIT_DAY);
				DataFormat.command = COMMAND_GETTIME;
				//Reflash(COMMAND_GETTIME,0xE0,0);
			}

		}
		else
		{
		//登录不成功
			if(ProgramPara.Cnt_ERROR >0)
			{
				ProgramPara.Cnt_ERROR --;
			}
			else
			{
				//ProgramPara.Cnt_ERROR = 3000;
				//Mode = Mode_Init;
				ProgramPara.Cnt_ERROR = 1800;
				espconn_disconnect(&websocket_tcp_conn);
				//os_printf("disconnect 111111111\r\n");
				Cnt_second = 0;
				Cnt_10second = 0;
				ClrBit(ProgramPara.init_flag_time,BIT_10S);
				ClrBit(ProgramPara.init_flag_time,BIT_30S);
				WebSocketOK = 0;
				HttpOK = 0;
				Mode = Mode_Bias;
			}
		}
		//os_printf("\r\n wwz  ==== %s \r\n",test);

	}
	else//没有连接上websocket
	{
		if((ProgramPara.Status & BIT(BIT_HASNET)))
		{//如果曾经连上过网络则在蓝灯闪烁后重连
			if(ProgramPara.Cnt_ERROR >0)
			{
			ProgramPara.Cnt_ERROR --;
			SetBit(ProgramPara.Status,BIT_RECONNECT);//重连标志位
			}
			else
			{
				ProgramPara.Cnt_ERROR = 1800;
				espconn_disconnect(&websocket_tcp_conn);
				os_printf("disconnect 222222\r\n");
				Cnt_second = 0;
				Cnt_10second = 0;
				ClrBit(ProgramPara.init_flag_time,BIT_10S);
				ClrBit(ProgramPara.init_flag_time,BIT_30S);
				WebSocketOK = 0;
				HttpOK = 0;
				Mode = Mode_Bias;

			}
		}
		else
		{//从未连接上网络
			ClrBit(ProgramPara.Status,BIT_RECONNECT);//清重连标志位
		}
	}
}



void ESP8266_Monitor(void)
{

	static uint8_t  Cnt_Reboot;

	  uint8_t length;//要发送的数据包长度
	  uint32_t AddrBias = 0x00000000; //地址偏移长度
	  uint8_t dst[] = "SSFFFFFFFF";//目标ID
	  uint8_t i;

	  switch(Receive){
	      case 0x01:DataFormat.command = COMMAND_GETNEWID;       break;//获得新的设备ID
	      case 0x02:DataFormat.command = COMMAND_NEWIDACK;       break;//获得新的设备ID ACK
	      case 0x04:DataFormat.command = COMMAND_LOGIN;          break;//Login和在成功建立websocket会话后置位
	      case 0x08:DataFormat.command = COMMAND_LOGINACK;       break;//LoginACK
	      case 0x10:DataFormat.command = COMMAND_READAIRDETECTOR;        break;//APP读Gas的 PPM值
	      case 0x11:DataFormat.command = COMMAND_READAIRDETECTORACK;        break;//APP读Gas的 PPM值
	      case 0x20:DataFormat.command = COMMAND_SETPPM;         break;//设置PPM上限值
	      case 0x30:DataFormat.command = COMMAND_LEDCOLOR;       break;//LED测试灯
	      case 0x40:DataFormat.command = COMMAND_SELFTEST;       break;//自测
	      case 0x50:DataFormat.command = COMMAND_SELFTESTACK;    break;//自测ACK
	      case 0x60:DataFormat.command = COMMAND_MAPDATA;    break;
	      case 0xE0:DataFormat.command = COMMAND_GETTIME;      break;//获取时间
	      case 0xF0:DataFormat.command = COMMAND_GETTIMEACK;   break;//获取时间ACK
	      case 0xF1:DataFormat.command = COMMAND_UPDATEAIRPPM;   break;//获取时间ACK
	      default:break;
	    }
	  //os_printf("\r\nCOMMAND_=== 0x%x\r\n",DataFormat.command);
	  switch(DataFormat.command){
	  	  case COMMAND_MAPDATA:
				ProcessPackege(dst,ProgramPara.Dev_ID,PRIMARYKEY,88,COMMAND_MAPDATA);
				for(i = 0;i<4;i++)//装载城市代码
				{
					DataFormat.mems[3-i] = MapFormat.city_code>>(8*i);
				}
				for(i = 0;i < 16;i++)
				{
					DataFormat.mems[4+i] = MapFormat.x[i];
					DataFormat.mems[20+i]= MapFormat.y[i];
				}
				for(i = 0; i < map_addlen;i++)
				{
					DataFormat.mems[36+i] = MapFormat.addr[i];
				}
				length = WebSocket_Data();
				//Reflash(0x0000,0xFF,0);
				espconn_send(&websocket_tcp_conn, DatePackege, length);
				Reflash(COMMAND_GETTIME,0xE0,0);
	  		  break;
	  	  case COMMAND_READAIRDETECTOR:
	  		  Reflash(COMMAND_READAIRDETECTORACK,0x11,0);
	  		//os_printf("\r\nCOMMAND_READGAS === %d\r\n",DataFormat.command);
				break;
	  	  case COMMAND_READAIRDETECTORACK:
				for(i = 0;i<10;i++){

				  dst[i] = DataFormat.SRC_ID[i];//读服务器发来的数据
				}
				ProcessPackege(dst,ProgramPara.Dev_ID,OPENKEY,0x00016,COMMAND_READAIRDETECTORACK);

				for(i = 0;i<4;i++){//4
				  DataFormat.mems[3-i] = PmData.PM25>>(8*i);
				}
				//DataFormat.mems[3] = PmData.PM25>>(8*0);
				//DataFormat.mems[2] = PmData.PM25>>(8*1);
			//	DataFormat.mems[1] = PmData.PM10>>(8*0);
				//DataFormat.mems[0] = PmData.PM10>>(8*1);

				for(i = 0;i<4;i++){//4+4
						  DataFormat.mems[7-i] = ProgramPara.PPM_H>>(8*i);
				}
				for(i = 0;i<4;i++){
						  DataFormat.mems[11-i] = ProgramPara.tem>>(8*i);
				}
				for(i = 0;i<4;i++){//16
						  DataFormat.mems[15-i] = ProgramPara.hum>>(8*i);
				}
				for(i = 0;i<4;i++){//4
				  DataFormat.mems[19-i] = PmData.PM10>>(8*i);
				}
				//os_printf("\r\nCOMMAND_READGAS_ACK === %d\r\n",DataFormat.command);
				length = WebSocket_Data();

				Reflash(0x0000,0xFF,0);

				//ClrBit(ProgramPara.Status,BIT_SELFTEST);

				espconn_send(&websocket_tcp_conn, DatePackege, length);
	  		  	break;

	  	  case COMMAND_WARNING:
				ProcessPackege(dst,ProgramPara.Dev_ID,PRIMARYKEY,0x0006,COMMAND_WARNING);//目的ID,源ID，加密类型，命令数据长度，命令

				ProgramPara.SendMEM = 1;

				for(i = 0;i<LENGTH_PPM;i++){
				  DataFormat.mems[3-i] = ProgramPara.PPM_H>>(8*i);
				}
				length = WebSocket_Data();

				Reflash(0x0000,0xFF,0);


				espconn_send(&websocket_tcp_conn, DatePackege, length);
				break;
	  	  case COMMAND_HEARTRATE:
	  		   // os_printf("\r\n  COMMAND_HEARTRATE~~~~ %s   %d\r\n",ProgramPara.Dev_ID);
				ProcessPackege(dst,ProgramPara.Dev_ID,PRIMARYKEY,0x0002,COMMAND_HEARTRATE);

				length = WebSocket_Data();
				DataFormat.command = 0x0000;

				os_memset(ubWebRxBuf,0,RXMAX);

				espconn_send(&websocket_tcp_conn, DatePackege, length);


				break;
	  	  case COMMAND_HEARTRATEACK:

				Cnt_Reboot = 0;
	  		  //DataFormat.command = 0;
	  		  //Receive = 0xFF;
				ProgramPara.Cnt_ERROR = 1800;
				//Reflash(COMMAND_GETTIME,0xE0,0);
				Reflash(0x0000,0xFF,0);
				 //os_printf("\r\n  COMMAND_HEARTRATEACK~~~~ %s \r\n",ProgramPara.Dev_ID);
				break;
	  	  case COMMAND_UPDATEAIRPPM:
	  		  	ProgramPara.Cnt_ERROR = 1800;
				ProcessPackege(dst,ProgramPara.Dev_ID,OPENKEY,0x0016,COMMAND_UPDATEAIRPPM);
				for(i = 0;i<4;i++){//4
				  DataFormat.mems[3-i] = PmData.PM25>>(8*i);
				}
				for(i = 0;i<4;i++){//4+4
						  DataFormat.mems[7-i] = ProgramPara.PPM_H>>(8*i);
				}
				for(i = 0;i<4;i++){
						  DataFormat.mems[11-i] = ProgramPara.tem>>(8*i);
				}
				for(i = 0;i<4;i++){//16
						  DataFormat.mems[15-i] = ProgramPara.hum>>(8*i);
				}
				for(i = 0;i<4;i++){//4
				  DataFormat.mems[19-i] = PmData.PM10>>(8*i);
				}
				length = WebSocket_Data();

				Reflash(0x0000,0xFF,0);

				espconn_send(&websocket_tcp_conn, DatePackege, length);
				/*
				os_printf("\r\n  COMMAND_UPDATEAIRPPM~~~~ %d \r\n",DataFormat.mems[3]);
				os_printf("\r\n  COMMAND_UPDATEAIRPPM~~~~ %d \r\n",DataFormat.mems[2]);
				os_printf("\r\n  COMMAND_UPDATEAIRPPM~~~~ %d \r\n",DataFormat.mems[1]);
				os_printf("\r\n  COMMAND_UPDATEAIRPPM~~~~ %d \r\n",DataFormat.mems[0]);
				os_printf("\r\n  COMMAND_UPDATEAIRPPM~~~~ %d \r\n",DataFormat.mems[7]);
				os_printf("\r\n  COMMAND_UPDATEAIRPPM~~~~ %d \r\n",DataFormat.mems[6]);
				os_printf("\r\n  COMMAND_UPDATEAIRPPM~~~~ %d \r\n",DataFormat.mems[5]);
				os_printf("\r\n  COMMAND_UPDATEAIRPPM~~~~ %d \r\n",DataFormat.mems[4]);
				os_printf("\r\n  COMMAND_UPDATEAIRPPM~~~~ %d \r\n",DataFormat.mems[11]);
				os_printf("\r\n  COMMAND_UPDATEAIRPPM~~~~ %d \r\n",DataFormat.mems[10]);
				os_printf("\r\n  COMMAND_UPDATEAIRPPM~~~~ %d \r\n",DataFormat.mems[9]);
				os_printf("\r\n  COMMAND_UPDATEAIRPPM~~~~ %d \r\n",DataFormat.mems[8]);
				*/
				break;
	  	  case COMMAND_GETNEWID:
				ProcessPackege(dst,ProgramPara.Dev_ID,OPENKEY,0x000c,COMMAND_GETNEWID);

				os_memcpy(DataFormat.mems,ProgramPara.Dev_ID,LENGTH_DEV_ID);
				length = WebSocket_Data();

				Reflash(0x0000,0xFF,0);

				espconn_send(&websocket_tcp_conn, DatePackege, length);
				break;
	  	  case COMMAND_NEWIDACK:
	  		//os_printf("\r\n  COMMAND_NEWIDACK~~~~\r\n");

				for(i = 0;i<10;i++){

				  ProgramPara.Dev_ID[i] = DataFormat.mems[i];//读服务器发来的数据
				}
				os_memcpy(config.Dev_id,ProgramPara.Dev_ID,10);
				os_memcpy(configcmp.Dev_id,ProgramPara.Dev_ID,10);
				if(gassmartconfig == 1)
				{
					Post_Id();
				}
				//os_printf("ProgramPara.Dev_ID == %s \r\n",ProgramPara.Dev_ID);
			    spi_flash_erase_sector(0x7C);
			    spi_flash_write(0x7C * 4096, (uint32 *)&config, sizeof(config));
			    spi_flash_erase_sector(0x7D);
			    spi_flash_write(0x7D * 4096, (uint32 *)&configcmp, sizeof(configcmp));


				/*
				uint8_t textlen = 0;
				for(i = 0;i<2;i++){
					UartPackege[textlen] = UartStart;textlen++;//start0
					UartPackege[textlen] = 0x65;textlen++;//源地址1
					UartPackege[textlen] = 0x61;textlen++;//目标地址2
					UartPackege[textlen] = 0x0B;textlen++;//长度3
					UartPackege[textlen] = UartNewId_Save;textlen++;//command4
					for(i = 0;i<LENGTH_DEV_ID;i++){
						  UartPackege[textlen] = ProgramPara.Dev_ID[i];//5 6 7 8 9 10 11 12 13 14
						  textlen++;
					 }//装载数据
					UartPackege[textlen] = UartStop;textlen++;//stop 15
					uart0_tx_buffer(UartPackege,textlen);
					os_memset(UartPackege,0,DataMax);
				}
				uart_cnt = 0;
				uart_poll = 2;
				*/
				//os_printf("COMMAND_NEWIDACK\r\n");
				Reflash(COMMAND_LOGIN,0x04,0);

				break;
	  	  case COMMAND_LOGIN:
				ProcessPackege(dst,ProgramPara.Dev_ID,OPENKEY,0x000A,COMMAND_LOGIN);
				ProgramPara.HWVersion.Num = hardversion;
				ProgramPara.SWVersion.Num = softversion;

				for(i = 0;i<4;i++){
				  DataFormat.mems[3-i] = ProgramPara.HWVersion.Num>>(8*i);
				}
				for(i = 0;i<4;i++){
				  DataFormat.mems[7-i] = ProgramPara.SWVersion.Num>>(8*i);
				}
				length = WebSocket_Data();

				Reflash(0x0000,0xFF,0);
				//os_printf("\r\nCOMMAND_LOGIN === %d\r\n",COMMAND_LOGIN);
				//os_printf("\r\nCOMMAND_LOGIN  playload_length === %d\r\n", DataFormat.payloadlength);
				espconn_send(&websocket_tcp_conn, DatePackege, length);

	  		  	  /*  8266由于未知原因使用memcpy zalloc 和malloc 的buff有问题 因此 此处字符串使用数组*/



				break;
	  	  case COMMAND_LOGINACK:
				os_memcpy(primaryKey,DataFormat.mems,LENGTH_KEY);
				//os_printf("\r\nprimaryKeyprimaryKey === %s  \r\n ",primaryKey);
				SetBit(ProgramPara.Status,BIT_LOGINOK);
				//Reflash(0x0000,0xFF,0);
				Reflash(COMMAND_GETTIME,0xE0,0);
				//Reflash(COMMAND_MAPDATA,0x60,0);
				break;

	  	  case COMMAND_GETTIME:

	  		   // os_printf("\r\n  COMMAND_HEARTRATE~~~~ %s   %d\r\n",ProgramPara.Dev_ID);
				ProcessPackege(dst,ProgramPara.Dev_ID,OPENKEY,0x0002,COMMAND_GETTIME);

				length = WebSocket_Data();
				DataFormat.command = 0x0000;

				os_memset(ubWebRxBuf,0,RXMAX);

				espconn_send(&websocket_tcp_conn, DatePackege, length);
				break;

	  	  case COMMAND_GETTIMEACK:
				i = 0;
				for(length = 0;length < 2;length++){
					UartPackege[i] = UartStart;i++;//start
					UartPackege[i] = 0x65;i++;//源地址
					UartPackege[i] = 0x61;i++;//目标地址
					UartPackege[i] = 0x07;i++;//长度
					UartPackege[i] = UartAirSetTime;i++;//command
					UartPackege[i] = DataFormat.mems[2];i++;//月
					UartPackege[i] = DataFormat.mems[3];i++;//日
					UartPackege[i] = DataFormat.mems[4];i++;//时
					UartPackege[i] = DataFormat.mems[5];i++;//分
					UartPackege[i] = DataFormat.mems[6];i++;//秒
					UartPackege[i] = DataFormat.mems[7];i++;//周

					UartPackege[i] = UartStop;i++;//stop
					uart0_tx_buffer(UartPackege,i);

				}
				os_memset(UartPackege,0,DataMax);
				Reflash(COMMAND_UPDATEAIRPPM,0xF1,0);

				break;

	  	  case COMMAND_SETPPM:
				os_memcpy(dst,DataFormat.SRC_ID,LENGTH_SRC_ID);
				for(i = 0;i<4;i++){
					ProgramPara.Alarm_H.Arr[i]= DataFormat.mems[3-i];
				}


				if(ProgramPara.Alarm_H.Num > PPM_MAX){
					ProgramPara.Alarm_H.Num= PPM_MAX;
				}
				//else if(ProgramPara.Alarm_H.Num< PPM_MIN){
					//ProgramPara.Alarm_H.Num= PPM_MIN;
				//}


				ProcessPackege(dst,ProgramPara.Dev_ID,PRIMARYKEY,0x0002,COMMAND_SETPPMACK);
				length = WebSocket_Data();

				Reflash(0x0000,0xFF,0);
				uart_setppm();
				espconn_send(&websocket_tcp_conn, DatePackege, length);
				break;
	  	  case COMMAND_LEDCOLOR:
	  		  	os_memcpy(dst,DataFormat.SRC_ID,LENGTH_SRC_ID);
	  		  	//os_printf("COMMAND_LEDCOLOR dst == %s  \r\n",dst);
				if(DataFormat.mems[2] == 0xff)
				{
					if(DataFormat.mems[1] == 0xff)
					{
						if(DataFormat.mems[0] == 0xff)
						{//蜂鸣器
							os_printf("COMMAND_LEDCOLOR BEEEEEEEEEEP！！！！  \r\n");
							ProgramPara.Flag_Test =0;
							SetBit(ProgramPara.Flag_Test,BIT_BEEP);
						}
					}
					else
					{//蓝
						os_printf("COMMAND_LEDCOLOR BLUEEEEEEEE！！！！  \r\n");
						ProgramPara.Flag_Test =0;
						SetBit(ProgramPara.Flag_Test,BIT_BLUE);
					}
				}
				else
				{
					if(DataFormat.mems[1] == 0xff)
					{//绿
						os_printf("COMMAND_LEDCOLOR GREEEEEEEEENNNNNN！！！！  \r\n");
						ProgramPara.Flag_Test =0;
						SetBit(ProgramPara.Flag_Test,BIT_GREEN);
					}
					else
					{//红
						os_printf("COMMAND_LEDCOLOR REDDDDDDD！！！！  \r\n");
						ProgramPara.Flag_Test =0;
						SetBit(ProgramPara.Flag_Test,BIT_RED);
					}
				}

				ProcessPackege(dst,ProgramPara.Dev_ID,PRIMARYKEY,0x0002,COMMAND_LEDCOLORACK);
				length = WebSocket_Data();

				Reflash(0x0000,0xFF,0);

				ClrBit(ProgramPara.Status,BIT_SELFTEST);
				espconn_send(&websocket_tcp_conn, DatePackege, length);
				uart_poll = 5;
				uart_cnt = 0;
				break;
	  	  case COMMAND_STOPALARM:
				SetBit(ProgramPara.Status,BIT_ALARMOFF);
				ClrBit(ProgramPara.flag_Time,BIT_MINITE);
				ProgramPara.Cnt_Minite = 0;
				Reflash(0x0000,0xFF,0);
				uart_poll = 7;//停止报警

				break;
	  	  default:
				break;
}
}

void WebRecvData_Proc(char * buf,uint8 index)
{
	uint16_t i = index;
	rxcount = 0; //记录标志 置0
	uint8_t open_key[]      = "Ftl_201207166688";//私钥，用在最开始的注册、登录
	//if(buf[i] != 0x82) //添加这一句可以增强程序的安全性，但是会出现有些命令被抛弃的情况
	//return;
	i++;//1
	//len = buf[i%RXMAX];//%max
	i++;//2
	//if(buf[i%RXMAX] != 'F')//
	{
		//return ;
	}
	i++;//3
	//------------------获取DST_ID------------------
	os_memcpy(DataFormat.DST_ID,&buf[i],10);
	i += 10;
	//------------------获取SRC_ID------------------
	os_memcpy(DataFormat.SRC_ID,&buf[i],10);
	i += 10;
	//------------------获取加密类型--------------
	DataFormat.encrypt= buf[i];
	i++;
	//----------------------获取包序-----------------
	DataFormat.idx = (uint16_t)((buf[i]<<8) | (buf[(i+1)]));
	i += 2;
	//------------------获取数据部分长度----------
	DataFormat.payloadlength = (uint16_t)(buf[i]<<8) | (buf[(i+1)]);
	i += 2;
	if(DataFormat.encrypt == 1)//公钥加密
	{
		RC4(&buf[i],open_key,DataFormat.payloadlength);
	}
	else if(DataFormat.encrypt == 3)//私钥加密
	{
		RC4(&buf[i],primaryKey,DataFormat.payloadlength);
	}

	//----------------------获取命令-----------------
	DataFormat.command = (uint16_t)(buf[i]<<8) | (buf[(i+1)]);
	i = i + 2;
	//----------------------数据部分-----------------
	if((DataFormat.payloadlength <= (MESMAX + 2)) && (DataFormat.payloadlength >2))
	{
		os_memcpy(DataFormat.mems,&buf[i],DataFormat.payloadlength - 2);
	}
	os_memset(ubWebRxBuf,0,RXMAX);

	switch(DataFormat.command)
	{
		case COMMAND_NEWIDACK:      Receive = 0x02;     break;
		case COMMAND_LOGINACK:      Receive = 0x08;		break;
		case COMMAND_READAIRDETECTOR:       Receive = 0x10;     break;
		case COMMAND_SETPPM:        Receive = 0x20;     break;
		case COMMAND_LEDCOLOR:      Receive = 0x30;     break;
		//case COMMAND_SELFTEST:      Receive = 0x40;     break;
		//case COMMAND_UPDATEFIR:     Receive = 0xE0;     break;
		case COMMAND_GETTIMEACK:  Receive = 0xF0;     break;
		default:break;
	}

}


/******************************************************************************
 * FunctionName : ProcessPackege
 * Description  : RC4加密
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
void RC4(uint8_t* message,uint8_t* key,uint8_t msglen)//
{

  uint8_t iS[256] ={0};
  uint16_t i = 0;
  uint8_t keylen = LENGTH_KEY;//(sizeof(key)/sizeof(key[0]))
  uint8_t temp = 0;

  uint8_t j = 0;
  uint8_t x = 0;

  uint8_t *iInputChar = 0;

  uint8_t iOutputChar[DataMax] = {0};

  uint8_t t = 0 ;

  iInputChar = message;

  for ( i = 0; i < 256; i++)
  {
    iS[i] = i;
  }
  for ( i = 0; i < 256; i++)
  {
    j = (j + iS[i] + key[i%keylen]) % 256;
    temp = iS[i];
    iS[i] = iS[j];
    iS[j] = temp;
  }
  for (x = 0,i=0,j=0; x < msglen; x++)
  {
    i = (i + 1) % 256;
    j = (j + iS[i]) % 256;
    temp = iS[i];
    iS[i] = iS[j];
    iS[j] = temp;
    t = (iS[i] + (iS[j] % 256)) % 256;
    iOutputChar[x] = (uint8_t) (iInputChar[x] ^ iS[t]);
  }
  os_memset( message,0,msglen);
  os_memcpy( message,iOutputChar,msglen);
}

/******************************************************************************
 * FunctionName : ProcessPackege
 * Description  : 目标地址接口
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
void ProcessPackege(uint8_t *dst,uint8_t *src,uint8_t encrypt,uint16_t length,uint16_t command)
{
  os_memcpy(DataFormat.DST_ID,dst,LENGTH_DST_ID);
  os_memcpy(DataFormat.SRC_ID,src,LENGTH_SRC_ID);
  DataFormat.encrypt = encrypt;
  DataFormat.payloadlength = length;
  DataFormat.command = command;

}

/******************************************************************************
 * FunctionName : WebSocket_Data
 * Description  : websocket 数据打包
 * Parameters   : pespconn -- the espconn used to connetion with the host
 * Returns      : none
*******************************************************************************/
uint8_t WebSocket_Data(void)
{
  uint8_t sendlen = 0;
  uint8_t i;
  uint8_t open_key[]      = "Ftl_201207166688";//私钥，用在最开始的注册、登录
  for(i=0;i<DataMax;i++){
    DatePackege[i] = 0;
  }
  //-----------规定的数据格式头----------
  DatePackege[0] = 0x82;
  DatePackege[2] = 0x00;
  DatePackege[3] = 0x00;
  DatePackege[4] = 0x00;
  DatePackege[5] = 0x00;
  //--------------------Flag-------------------
  DatePackege[6] = 'F';
  sendlen = 7;
  //--------------------DST_ID-----------------
  os_memcpy(&DatePackege[sendlen],DataFormat.DST_ID,LENGTH_DST_ID);
  sendlen += LENGTH_DST_ID;//17
  //--------------------SRC_ID-----------------

  os_memcpy(&DatePackege[sendlen],DataFormat.SRC_ID,LENGTH_SRC_ID);
  sendlen += LENGTH_SRC_ID;//27
  //------------------加密类型---------------

  DatePackege[sendlen] = DataFormat.encrypt;
  ///os_memcpy(&DatePackege[sendlen],(uint8_t*)&DataFormat.encrypt,LENGTH_ENCRYPT);
  sendlen += LENGTH_ENCRYPT;//28
  //--------------------包序------------------

  DataFormat.idx +=1;
  DatePackege[sendlen]      |= ((DataFormat.idx >> 8 )&0xFF);
  DatePackege[sendlen+1]    |= ((DataFormat.idx >> 0 )&0xFF);
 // memcpy(&DatePackege[sendlen],(uint16_t*)&DataFormat.idx,LENGTH_IDX);

  sendlen += LENGTH_IDX;//30
  //-------------命令数据长度--------------

  DatePackege[sendlen]      |= ((DataFormat.payloadlength >> 8 )&0xFF);
  DatePackege[sendlen+1]    |= ((DataFormat.payloadlength >> 0 )&0xFF);
  sendlen += LENGTH_PAYLOADLENGTH;//32

  //-------------------命令-------------------
  DatePackege[sendlen]      |= ((DataFormat.command >> 8 )&0xFF);
  DatePackege[sendlen+1]    |= ((DataFormat.command >> 0 )&0xFF);
  //memcpy(&DatePackege[sendlen],(uint16_t*)&DataFormat.command,LENGTH_COMMAND);
  sendlen += LENGTH_COMMAND;//34

  //-------------------数据-------------------
  for(i = 0;i < (DataFormat.payloadlength - 2);i++)
  {
	  DatePackege[sendlen + i] = DataFormat.mems[i];
  }

  if(DataFormat.encrypt == 1){
    RC4(&DatePackege[sendlen - LENGTH_COMMAND],open_key,DataFormat.payloadlength);
  }
  else if(DataFormat.encrypt == 2){
    RC4(&DatePackege[sendlen - LENGTH_COMMAND],primaryKey,DataFormat.payloadlength);
  }

  sendlen += DataFormat.payloadlength;

  DatePackege[1] = 0x80 | (sendlen - 6-2);

  return (sendlen - 2);
}
