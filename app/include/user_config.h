#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "ets_sys.h"
#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "mem.h"
#include "gas_uart.h"
#include "user_json.h"
#include "user_interface.h"
#include "smartconfig.h"
#include "airkiss.h"
#include "tcp_udp.h"
#include  "eagle_soc.h"
#include  "pwm.h"
#include "key.h"
#include "uart.h"
#include "networking.h"

//#define USE_OPTIMIZE_PRINTF	//���ڴ�ӡ��

/* ��ͬ�������� IDǰ��λ��ͬ */
#define TEXXXTT   	0//ʹ��������  �����õĴ�������mp4
#define MP_4		1//��ȼ���� GS
#define MP_502		2//HC ��ȩ	������
#define MP_9		3
#define MP_7		4	 // CO
#define MP_503		5//VO	������
#define MP_901		6
#define MP_2		7	//���� SM


#define SENSOR_TYPE		MP_502   //ѡ�񴫸�������


struct startup_config{
char Dev_id[12];
};


#if(SENSOR_TYPE == MP_502 || SENSOR_TYPE == MP_503)
	#define PPM_MAX 3500
	#define PPM_MIN 2500
#endif



//#define DEBUG  //������Ҫʹ��debug ��Ϊ��������3���������Զ��ػ�
#ifdef DEBUG
	#define	CNT_10S	   10	//10S
	#define	CNT_30S	   20	//30S
    #define CNT_MINITE 120//120s
    #define CNT_HOUR   2 //2min
    #define CNT_DAY    15//30min
    #define CNT_2DAY	  120
	#define CNT_70S	 60
#else
	#define	CNT_10S	   10	//10S
	#define	CNT_30S	   20	//30S
    #define CNT_MINITE 120//2min
    #define CNT_HOUR   30//
    #define CNT_DAY    24//1day
    #define CNT_2DAY		48	//48hour
	#define CNT_70S	 60
#endif

#define BIT_MINITE      0
#define BIT_HOUR 	1
#define BIT_DAY 	2
#define BIT_10S      3
#define BIT_30S      4

#define BIT_RECONNECT   7  //���������ȴ�״̬
#define BIT_ALARMOFF    6  //ǿ�ƽ�������
#define BIT_SELFTEST    5  //�Բ�
//#define BIT_KEYOK       4
#define BIT_LOGINOK     3  //��ɵ�¼
#define BIT_HASBIAS     2  //��У�����´ξͲ�����У���ˣ�
#define BIT_HASNET      1  //�������ӹ�����(������������������)
#define BIT_ONLINE      0  //��ǰ������wifi


#define BIT_SHORT   	0  //�̰�
#define BIT_LONG   		1  //����
#define BIT_TEST   		2  //����
#define BIT_WARNING		3	//����
/************************************************
 * uart_command
 ************************************************/
#define UartStop		0x56
#define	UartStart		0x55
#define UartPoll		0x81
#define UartPoll_Ack	0x01
#define UartSmartconfig	0x82
#define Uartledcolor	0x83
#define Uartledcolor_Ack 0x02
#define UartStopWarning	0x84
#define UartGetId		0x85
#define UartGetId_ACK	0x03
#define UartNewId_Save	0x86
#define UartNewId_Save_ACK	0x04
#define UartWaring		0x87
#define UartWaring_Ack	0x05
#define UartFirstLink	0x88
#define UartFirstLink_Ack	0x06
#define UartSetPpm		0x89
#define UartSetPpm_Ack	0x07

#define UartAirPoll_Ack	0x08
#define UartAirSetTime	0x90
#define UartAirPoll		0x91


#define BIT_RED      0
#define BIT_GREEN    1
#define BIT_BLUE     2
#define BIT_BEEP     3

#define SMARTCONFIG_INIT        0x00
#define SMARTCONFIG_START       0x01
#define SMARTCONFIG_WAIT        0x02
#define SMARTCONFIG_IP          0x03
#define SMARTCONFIG_BROAD       0x04
#define SMARTCONFIG_TRAN        0x05
#define SMARTCONFIG_SEND        0x06
#define SMARTCONFIG_FINISH      0x07
#define SMARTCONFIG_ERROR       0xFF

/***************************
 	 	 ���ݳ���
 ***************************/
#define LENGTH_KEY              16
#define LENGTH_DEV_ID           10
#define LENGTH_DST_ID           10
#define LENGTH_SRC_ID           10
#define LENGTH_ENCRYPT          1
#define LENGTH_IDX               2
#define LENGTH_PAYLOADLENGTH   2
#define LENGTH_COMMAND          2
#define LENGTH_PPM              4
#define LENGTH_KEY              16
#define LENGTH_SOFTNUM          2
#define LENGTH_DEVVER           4
#define LENGTH_FIRDATA          50
#define LENGTH_UPDATE LENGTH_COMMAND + LENGTH_SOFTNUM + LENGTH_DEVVER
#define LENGTH_VERSION  4
#define LENGTH_ALARMVAL 4
#define LENGTH_NETMASK  4
#define LENGTH_IPADDR   4


#define MESMAX          128


#define RESENDCNT       10
#define WebMaxSize      128
#define PATTERN_SIZE	128
#define RXMAX           128

/***************************
 	 	 ��������
 ***************************/
#define NONEKEY         0
#define OPENKEY         1
#define PRIMARYKEY      2



extern char ubWebRxBuf[RXMAX];

typedef union

{
    uint32_t    Num;
    uint8_t     Arr[LENGTH_IPADDR];
}Typdef_MyIPAddress;

typedef union
{
    uint32_t    Num;         //�汾��
    uint8_t     Arr[LENGTH_VERSION];      //�豸�汾��
}Typdef_Version;
typedef union
{
    uint32_t    Num;   //����ֵ
    uint8_t     Arr[LENGTH_ALARMVAL];
}Typdef_AlarmValue;


typedef union
{
    uint32_t    Num;   //ƫ��ֵ
    uint8_t     Arr[LENGTH_ALARMVAL];
}Typdef_PpmOffset;



typedef union
{
    uint32_t    Num;
    uint8_t     Arr[LENGTH_IPADDR];
}Typdef_IPAddress;

typedef struct
{
    uint8_t DST_ID[LENGTH_DST_ID];         //Ŀ��ID
    uint8_t SRC_ID[LENGTH_SRC_ID];         //ԴID
    uint8_t mems[MESMAX];           //���ݳ���
    uint8_t encrypt;            //��������
    uint16_t idx;               //����
    uint16_t payloadlength;     //�������ݳ���
    uint16_t command;           //����
}Typdef_DataFormat;

typedef struct
{
    uint16_t payloadlength;     //�������ݳ���
    uint16_t command;           //����
    uint8_t mems[MESMAX];           //���ݳ���
}Typdef_UartFormat;

typedef struct
{
	uint32_t city_code;
	char	x[16];	//����
	char 	y[16];	//γ��
	char	addr[48];
}Typdef_MapFormat;

typedef struct
{
	uint32_t PM25;
	uint32_t PM10;
	uint16_t PM1_0;
}Typdef_PmData;

typedef struct
{
    uint16_t  Cnt_ERROR;        //��ʱ��������״̬����ʱ
    uint16_t  Cnt_Minite;       //��ʱ��������
    uint16_t  Cnt_Hour;       //��ʱ��������
    uint16_t  Cnt_Day;       //��ʱ��������
    uint8_t   flag_revise;			 //У׼��־λ  0 λУ׼   1ΪУ׼���
    uint8_t   SubMode;          //��ģʽɢת
    uint8_t   flag_Time;	//ʱ�� bit0 ����,bit1 Сʱ, bit2 �졣
    uint8_t	  init_flag_time;	//�����ж�mode_initʱ���ӷ���������ʱ

    uint8_t   Status;           //bit 0 �������ӣ�    bit 1����������   ��bit 2 ���У�� ,   ��bit 3 �ɹ���½ ,
                                //bit 4 �ɹ����˽Կ ,bit 5 �Բ�        , bit 6 �ֶ�ֹͣ���� , bit 7 ������   ��bit4�Ѿ������ˣ�
    uint8_t SendMEM;            //���Ͷ��Ŵ�����־λ ÿ�η�����  1Сʱ�����
    uint8_t Updating;           //������־λ������ʱ �������е�����������
    uint8_t Flag_Test;          //���Ե�����   bit0 ��� bit1 �̵� bit2 ���� bit3 ������
    uint8_t flag_Alarm;         //������־λ������λΪ 1 ʱ ����
    uint8_t Dev_ID[LENGTH_DEV_ID];       //�ɷ�����������豸ID
    uint32_t PPM_H;               //������PPM��ֵ  �ߵ�ƽģ��
    uint32_t PM25;               //pm2.5��ֵ
    uint32_t tem;               //�¶�
    uint32_t hum;               //ʪ��
    Typdef_AlarmValue Alarm_H;    //����ֵ �ߵ�ƽģ��
    Typdef_AlarmValue Alarm_L;    //����ֵ �͵�ƽģ��
    Typdef_Version   Version;   //�豸�汾��
    Typdef_Version   HWVersion; //�豸Ӳ���汾��
    Typdef_Version   SWVersion; //�豸����汾��
    uint8_t Key_Type;			//��¼����״̬�ͱ���״̬
}Typdef_ProgramPara;

extern Typdef_ProgramPara ProgramPara;
extern Typdef_PmData PmData;
extern Typdef_MapFormat MapFormat;
extern Typdef_DataFormat DataFormat;
extern Typdef_UartFormat UartFormat;
extern uint8_t UartPackege[DataMax];
extern uint8_t UartRecPackege[DataMax];
extern struct startup_config config;
extern struct startup_config configcmp;

extern char ubWebRxBuf[RXMAX];//����������������
/***************************
 	 	 ϵͳ״̬
 ***************************/
uint8 Mode;	//ϵͳ״̬

#define Mode_Init    		0x01
#define Mode_Wait           0x02	//��ģʽ�����ȴ�  �������� gas_main_cb ���ϵ���
#define Mode_Bias			0x03
#define Mode_Config  		0x04	//smartconfig״̬
#define Mode_Nomal			0x08	//��������״̬
#define Mode_Detect			0x10
#define Mode_Warning		0x20
#define Mode_CreateSession  0x40	//websocket����
#define Mode_Test           0x80
#define Mode_Connect        0xA0

/***************************
 	 	 Э������
 ***************************/

#define COMMAND_READGAS         0x3001
#define COMMAND_READGASACK      0x3002

#define COMMAND_WARNING         0x3003

#define COMMAND_HEARTRATE       0x3005
#define COMMAND_HEARTRATEACK    0x3006
#define COMMAND_UPDATEPPM       0x3007
#define COMMAND_GETNEWID        0x300b
#define COMMAND_NEWIDACK        0x300c
#define COMMAND_LOGIN           0x300d
#define COMMAND_LOGINACK        0x300e
#define	COMMAND_MAPDATA			0x3013
//#define COMMAND_UPDATEFIR       0x220f
//#define COMMAND_UPDATEFIRACK    0x2210
#define COMMAND_SETPPM          0x300f
#define COMMAND_SETPPMACK       0x3010

#define COMMAND_LEDCOLOR        0x2213
#define COMMAND_LEDCOLORACK     0x2214
#define COMMAND_SELFTEST        0x2215
#define COMMAND_SELFTESTACK     0x2216
#define COMMAND_STOPALARM       0x2219
#define COMMAND_STOPALARMACK    0x221a

#define COMMAND_READAIRDETECTOR 	0x3200
#define COMMAND_READAIRDETECTORACK 	0x3201
#define COMMAND_UPDATEAIRPPM 		0x3202
#define COMMAND_GETTIME 	0x3206
#define COMMAND_GETTIMEACK 	0x3207

#define SetBit(VAR,Place)         ( (VAR) |= (uint8_t)((uint8_t)1<<(uint8_t)(Place)) )
#define ClrBit(VAR,Place)         ( (VAR) &= (uint8_t)((uint8_t)((uint8_t)1<<(uint8_t)(Place))^(uint8_t)255))

void ICACHE_FLASH_ATTR smartconfig_done(sc_status status, void *pdata);
void ICACHE_FLASH_ATTR time_calcu_cb(void);
void gas_main_cb(void);
void uart_Tx(void);

#endif

