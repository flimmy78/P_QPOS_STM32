//
#include <string.h>
#include "UART_simulation.h"
#include "stm32f10x.h"
#include "VectorTable.h"

unsigned int gpuart_halfBit;
							    
unsigned char  GP_UART_BUF[128];
unsigned char	 GP_UART_STATE=0;
void HAL_GPUart_WaitHalfBit()
{
	u32 temp;		   
	SysTick->LOAD=gpuart_halfBit;//时间加载
	SysTick->VAL =0x00;           //清空计数器
	SysTick->CTRL=0x01 ;          //开始倒数  
	do
	{
		temp=SysTick->CTRL;
	}
	while(temp&0x01&&!(temp&(1<<16)));//等待时间到达   
	SysTick->CTRL=0x00;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器	 
}

unsigned short HAL_GPUartInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	//Enable GPIO clocks 
	RCC_APB2PeriphClockCmd(TX_GPIO_PIN_EN, ENABLE);

	//半个bit的时间长度
	//系统频率除以串口频率除以2
	//gpuart_halfBit =8000000/115200/2-4; // 115200 ,1bit=8.68us
	gpuart_halfBit =(8000000/9600/2)+40;// 9600

	//RX配置
	if(1) 
{
	
	
	GPIO_InitStructure.GPIO_Pin = RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //?上拉�
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(RX_GPIO_PORT, &GPIO_InitStructure);
//}
//	else
//{


	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource12);
	
	/* Configure EXTI Line1 */
	EXTI_InitStructure.EXTI_Line = UART_SIMULATION_IRQ;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	
	/* Enable and set EXTI Line1 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}	
	//TX配置
	GPIO_InitStructure.GPIO_Pin = TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(TX_GPIO_PORT, &GPIO_InitStructure);

	TX_HIG;
	return 0;
}

static uint8_t HAL_GPUart_RevBit(void)
{
  return GPIO_ReadInputDataBit(RX_GPIO_PORT, RX_GPIO_PIN);
   //return EXTI_GetITStatus(EXTI_Line12);
}

//--------------------------------------------------------------------------------
// Send a byte of data 
//--------------------------------------------------------------------------------
unsigned char HAL_GPUart_SendByte (unsigned char x)
{
	int j; 
	unsigned int parity;  
	unsigned char data;		
	
	parity = 0; 
	
	//send start bit 
	TX_LOW;	
	HAL_GPUart_WaitHalfBit();
	HAL_GPUart_WaitHalfBit();


	//send now 8 bits
	for(j=0;j<8;j++)
	{
		if (!(x & 0x01))
		{
			TX_LOW;
		}
		else
		{
			TX_HIG;
			//parity++; 
		}
		x >>= 1; 
		HAL_GPUart_WaitHalfBit();
		HAL_GPUart_WaitHalfBit();
	}	

/*
	//send parity bit
	if (!(parity & 1))
	{
		//SPIPORT = 0; 
		HS32U2_GPDORE &= 0xF7; 
	}
	else 
	{
		//SPIPORT = 1;
		HS32U2_GPDORE |= 0x08; 
	}
	
	wait_delay (); 
*/
	//send stop bit 
	//SPIPORT = 1; 
	TX_HIG; 
	HAL_GPUart_WaitHalfBit();
	HAL_GPUart_WaitHalfBit();
//	close_pit();
	//  2stop bit
	//HAL_GPUart_WaitHalfBit();
	//HAL_GPUart_WaitHalfBit();
	//send_len++; 
    return 0;
}

//--------------------------------------------------------------------------------
// Receive a byte of data 
// 0 ok
// 1 fail
//--------------------------------------------------------------------------------
unsigned char HAL_GPUart_RevByte(unsigned char* x)
{
	//unsigned char the_bit; 
	unsigned char ret =0; 
	unsigned int i = 0; 
	unsigned char parity = 0; 

	// set flag

	
	//wait for start bit 
	//waiting 5bit 
//	if(1)
//{
	u32 temp;	//超时时间为500ms	   
	SysTick->LOAD=gpuart_halfBit*10;//时间加载
	SysTick->VAL =0x00;           //清空计数器
	SysTick->CTRL=0x01 ;          //开始倒数  
	while(1)
	{
		temp=SysTick->CTRL;
		if(!(temp&0x01&&!(temp&(1<<16))))
			{
			SysTick->CTRL=0x00;       //关闭计数器
			SysTick->VAL =0X00;       //清空计数器	 			
			return 1;
			}
		if(!HAL_GPUart_RevBit())
			break;
		
	}
	SysTick->CTRL=0x00;       //关闭计数器
	SysTick->VAL =0X00;       //清空计数器	 
//}	
//	else
//{
//	if(HAL_GPUart_RevBit())
//		return 1;
//}
	//--------------------------------------------------------------------------------

	//check the start bit
	//gpuart_halfBit=gpuart_halfBit-8;
	HAL_GPUart_WaitHalfBit();
	//gpuart_halfBit=gpuart_halfBit+8;
	if(HAL_GPUart_RevBit())
		return 1;

	
	HAL_GPUart_WaitHalfBit();

	//--------------------------------------------------------------------------------
	//receive data bits
	for (i=0; i<8; i++)
	{
		HAL_GPUart_WaitHalfBit();
		
		ret >>= 1;
		if(HAL_GPUart_RevBit())
			ret |= 0x80; 
		//parity ^= ret; 
		
		HAL_GPUart_WaitHalfBit();
	}

	//--------------------------------------------------------------------------------

	//停止位
	HAL_GPUart_WaitHalfBit();
	//HAL_GPUart_WaitHalfBit();
	//parity = (0x04 & HS32U2_EPPDR);
	
	//delay_time0(half_etu);

	//..wait_delay ();

	*x =ret;
	return 0;

}


unsigned char  HAL_GPUart_Send(unsigned char *SendBuf, unsigned short SendLen)
{
	short i = 0;
	for(i = 0; i<SendLen; i++)
	{
		HAL_GPUart_SendByte(SendBuf[i]);
	}
	return 0;

}

unsigned char  HAL_GPUart_Rev(unsigned char *RevBuf, unsigned short RevLen)
{
	unsigned short i = 0;
	unsigned char para;
	for(i = 0; i<RevLen; i++)
	{
		if(HAL_GPUart_RevByte(&para))
			return 1;
		else
			RevBuf[i]=para;
	}
	return 0;

}

//=================================
//StoreKey_Send 
//通讯格式:
//偏移 数据		格式 长度 说明
//0x00	起始符	HEX	2	0x55 0xAA
//0x02	包类型	HEX	1	0x51
//0x03	包序号	HEX	1	取值为1~255，每次交互后增加1，超过255后重新从1开始计数
//0x04	包长度	HEX	2	表示指令和参数的长度总和，低字节优先传输
//0x06	指令		HEX	2	参考指令说明
//0x08	参数		HEX	n	参考指令说明，长度n不超过1024
//0x08+n	校验和	HEX	1	从“包类型”到“参数”的数据的逐字节异或结果
//0x09+n	终止符	HEX	2	0xCC 0x33



//=================================
//StoreKey_PCtoM 
//通讯格式:
//偏移内容	格式 长度 	说明
//0x00	STX		HEX	1		0x02 起始位
//0x01	LEN		BCD	2		右对齐
//0x03	SEQ		HEX	4		序列号,每次收发为一组相同的数。
//0x07	CON	HEX	LEN-4	数据
//L+3	ETX		HEX	1		0x03 结束位
//L+4	LRC		HEX	1		校验位
//=================================

unsigned char GP_UART_CMD(void)//串口初始化
{
	unsigned char iRET;
	unsigned short cmd,len;
	
	if(GP_UART_STATE)
	{
		GP_UART_STATE=0;
		g_uiTime100ms_LCDOff_PowerOff = Time_PowerOFF;

		if(GP_UART_BUF[0]!=0x55||GP_UART_BUF[1]!=0xaa)
		return 1;
	
		len=GP_UART_BUF[5]<<8|GP_UART_BUF[4];
		cmd=GP_UART_BUF[6]<<8|GP_UART_BUF[7];
		switch(cmd)
		{
			case 0x7803:
				vLCD_GUI_ShowMid12X12("配置中...",4);
				iRET=iCMD_Factory_DeviceInit(GP_UART_BUF+8,len-2);	
			break;
			default:
				iRET=1;
			break;			
		}
			
		
		if(iRET)
		{	
			GP_UART_BUF[2]=0x45;
			GP_UART_BUF[4]=0x03;
			GP_UART_BUF[8]=iRET;
			GP_UART_BUF[10]=0xcc;
			GP_UART_BUF[11]=0x33;
			HAL_GPUart_Send(GP_UART_BUF,12);
		}
		else
		{
			GP_UART_BUF[2]=0x41;
			GP_UART_BUF[4]=0x02;
			GP_UART_BUF[9]=0xcc;
			GP_UART_BUF[10]=0x33;
			HAL_GPUart_Send(GP_UART_BUF,11);
		}
		return iRET;
	}
	return 0xff;
}

