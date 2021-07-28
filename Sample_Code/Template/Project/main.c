/*_____ I N C L U D E S ____________________________________________________*/
#include "MS51_16K.h"

#include	"project_config.h"
#include "i2c_analog.h"


/*_____ D E C L A R A T I O N S ____________________________________________*/
volatile uint8_t u8TH0_Tmp = 0;
volatile uint8_t u8TL0_Tmp = 0;

//UART 0
bit BIT_TMP;
bit BIT_UART;
bit uart0_receive_flag=0;
unsigned char uart0_receive_data;

// I2C
#define EEPROM_SLAVE_ADDR    					(0xA0)
uint8_t buffer[256] = {0};
uint8_t u8SlaveAddr = EEPROM_SLAVE_ADDR >>1;

uint8_t demo_state = 0;

/*_____ D E F I N I T I O N S ______________________________________________*/
volatile uint32_t BitFlag = 0;
volatile uint32_t counter_tick = 0;

/*_____ M A C R O S ________________________________________________________*/
#define SYS_CLOCK 								(24000000ul)


/*_____ F U N C T I O N S __________________________________________________*/

extern uint8_t I2C_ANALOG_WriteData(uint8_t SlaveAddress,uint16_t REG_Address,uint8_t* REG_data, uint16_t count);
extern uint8_t I2C_ANALOG_ReadData(uint8_t SlaveAddress,uint16_t REG_Address,uint8_t* REG_data, uint16_t count);
extern void I2C_ANALOG_SW_open(uint32_t u32BusClock);
extern void I2C_ANALOG_Stop(void);

void tick_counter(void)
{
	counter_tick++;
}

uint32_t get_tick(void)
{
	return (counter_tick);
}

void set_tick(uint32_t t)
{
	counter_tick = t;
}

void compare_buffer(uint8_t *src, uint8_t *des, int nBytes)
{
    uint16_t i = 0;	
	
    for (i = 0; i < nBytes; i++)
    {
        if (src[i] != des[i])
        {
            printf("error idx : %4d : 0x%2X , 0x%2X\r\n", i , src[i],des[i]);
			set_flag(flag_error , Enable);
        }
    }

	if (!is_flag_set(flag_error))
	{
    	printf("compare_buffer finish \r\n");	
		set_flag(flag_error , Disable);
	}

}

void reset_buffer(void *dest, unsigned int val, unsigned int size)
{
    uint8_t *pu8Dest;
//    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;

	#if 1
	while (size-- > 0)
		*pu8Dest++ = val;
	#else
	memset(pu8Dest, val, size * (sizeof(pu8Dest[0]) ));
	#endif
	
}

void copy_buffer(void *dest, void *src, unsigned int size)
{
    uint8_t *pu8Src, *pu8Dest;
    unsigned int i;
    
    pu8Dest = (uint8_t *)dest;
    pu8Src  = (uint8_t *)src;


	#if 0
	  while (size--)
	    *pu8Dest++ = *pu8Src++;
	#else
    for (i = 0; i < size; i++)
        pu8Dest[i] = pu8Src[i];
	#endif
}

void dump_buffer(uint8_t *pucBuff, int nBytes)
{
    uint16_t i = 0;
    
    printf("dump_buffer : %2d\r\n" , nBytes);    
    for (i = 0 ; i < nBytes ; i++)
    {
        printf("0x%2X," , pucBuff[i]);
        if ((i+1)%8 ==0)
        {
            printf("\r\n");
        }            
    }
    printf("\r\n\r\n");
}

void  dump_buffer_hex(uint8_t *pucBuff, int nBytes)
{
    int     nIdx, i;

    nIdx = 0;
    while (nBytes > 0)
    {
        printf("0x%02X  ", nIdx);
        for (i = 0; i < 16; i++)
            printf("%2BX ", pucBuff[nIdx + i]);
        printf("  ");
        for (i = 0; i < 16; i++)
        {
            if ((pucBuff[nIdx + i] >= 0x20) && (pucBuff[nIdx + i] < 127))
                printf("%c", pucBuff[nIdx + i]);
            else
                printf(".");
            nBytes--;
        }
        nIdx += 16;
        printf("\n");
    }
    printf("\n");
}

void delay(uint16_t dly)
{
/*
	delay(100) : 14.84 us
	delay(200) : 29.37 us
	delay(300) : 43.97 us
	delay(400) : 58.5 us	
	delay(500) : 73.13 us	
	
	delay(1500) : 0.218 ms (218 us)
	delay(2000) : 0.291 ms (291 us)	
*/

	while( dly--);
}



void send_UARTString(uint8_t* Data)
{
	#if 1
	uint16_t i = 0;

	while (Data[i] != '\0')
	{
		#if 1
		SBUF = Data[i++];
		#else
		UART_Send_Data(UART0,Data[i++]);		
		#endif
	}

	#endif

	#if 0
	uint16_t i = 0;
	
	for(i = 0;i< (strlen(Data)) ;i++ )
	{
		UART_Send_Data(UART0,Data[i]);
	}
	#endif

	#if 0
    while(*Data)  
    {  
        UART_Send_Data(UART0, (unsigned char) *Data++);  
    } 
	#endif
}

void send_UARTASCII(uint16_t Temp)
{
    uint8_t print_buf[16];
    uint16_t i = 15, j;

    *(print_buf + i) = '\0';
    j = (uint16_t)Temp >> 31;
    if(j)
        (uint16_t) Temp = ~(uint16_t)Temp + 1;
    do
    {
        i--;
        *(print_buf + i) = '0' + (uint16_t)Temp % 10;
        (uint16_t)Temp = (uint16_t)Temp / 10;
    }
    while((uint16_t)Temp != 0);
    if(j)
    {
        i--;
        *(print_buf + i) = '-';
    }
    send_UARTString(print_buf + i);
}


void I2Cx_WriteMulti(uint8_t address, uint16_t REG_Address, uint8_t *REG_data, uint16_t len)
{	
	I2C_ANALOG_WriteData(address << 1 , REG_Address , REG_data , len);
}

void I2Cx_ReadMulti(uint8_t address, uint16_t REG_Address, uint8_t *REG_data, uint16_t len)
{ 
	I2C_ANALOG_ReadData(address << 1 , REG_Address , REG_data , len);
}

void I2Cx_Init(void)	// P13 :SCL , P14 : SDA
{
	I2C_ANALOG_SW_open(100000);
}


void state_machine(void)
{
	switch(demo_state)
	{
		case 0:
			set_flag(flag_WriteAddr , Enable);
			set_flag(flag_Dump , Enable);
			break;
		case 1:
			set_flag(flag_WriteData , Enable);
			set_flag(flag_Dump , Enable);			
			break;
		case 2:
			set_flag(flag_WriteData1 , Enable);
			set_flag(flag_Dump , Enable);
			break;
		case 3:
			set_flag(flag_WriteData2 , Enable);
			set_flag(flag_Dump , Enable);			
			break;			
		
	}

	if (demo_state <= 3 )
	{
		demo_state++;
	}	
}


void EEPROM_clear(void)
{
	uint16_t i = 0;	
	uint8_t temp = 0xFF;
	
	
	printf("clear EEPROM ..\r\n");	

	for (i = 0 ; i < 256 ; i++)
	{
		I2Cx_WriteMulti(u8SlaveAddr , i , &temp , 1);
		Timer1_Delay_10us(100);
		printf(".");
		if ((i+1)%64 ==0)
        {
            printf("\r\n");
        }		
	}

	printf("done\r\n");	

}

void EEPROM_dump(void)
{

	reset_buffer(buffer , 0x00 , 256);
	
	printf("dump EEPROM\r\n");

	#if 1
	I2Cx_ReadMulti(u8SlaveAddr , 0x00 , buffer , 256);
	dump_buffer_hex(buffer , 256);
	
	#else
	for (i = 0 ; i < 0x100 ; i++ )
	{	
		I2Cx_ReadMulti(u8SlaveAddr , 0x01 , &buffer[i] , 1);			
		printf("0x%2X," ,buffer[i]);

		if ((i+1)%8 ==0)
        {
            printf("\r\n");
        }
	}	
	#endif
}

void EEPROM_test(void)
{
	uint8_t value = 0;
	uint16_t reg = 0;	
//	uint8_t array[2] = {0};
//	uint16_t i = 0;
	const uint32_t delay = 10;

	#if 1	//clear EEPROM
	EEPROM_clear();
	#endif

	Timer1_Delay_10us(delay);
	reg = 0x01;
	value = 0xA1;
	I2Cx_WriteMulti(u8SlaveAddr , reg , &value , 1);
	Timer1_Delay_10us(delay);
	printf("WR : 0x%02X : 0x%BX \r\n" ,reg , (uint8_t) value);

	value = 0;
	I2Cx_ReadMulti(u8SlaveAddr , reg , &value , 1);
	printf("RD : 0x%02X : 0x%BX \r\n" ,reg ,value);

	reg = 0x02;		
	value = 0xB2;
	I2Cx_WriteMulti(u8SlaveAddr , reg , &value , 1);
	Timer1_Delay_10us(delay);
	printf("WR : 0x%02X : 0x%BX \r\n" ,reg ,value);

	value = 0;
	I2Cx_ReadMulti(u8SlaveAddr , reg , &value , 1);
	printf("RD : 0x%02X : 0x%BX \r\n" ,reg ,value);

	reg = 0x03;	
	value = 0xC3;		
	I2Cx_WriteMulti(u8SlaveAddr , reg , &value , 1);
	Timer1_Delay_10us(delay);
	printf("WR : 0x%02X : 0x%BX \r\n" ,reg ,value);

	value = 0;
	I2Cx_ReadMulti(u8SlaveAddr , reg , &value , 1);
	printf("RD : 0x%02X : 0x%BX \r\n" ,reg ,value);

	reg = 0x04;		
	value = 0xD4;
	I2Cx_WriteMulti(u8SlaveAddr , reg , &value , 1);
	Timer1_Delay_10us(delay);
	printf("WR : 0x%02X : 0x%BX \r\n" ,reg ,value);

	value = 0;
	I2Cx_ReadMulti(u8SlaveAddr , reg , &value , 1);
	printf("RD : 0x%02X : 0x%BX \r\n" ,reg ,value);	

//	array[1] = 0x12;
//	array[0] = 0x46;	
//	reg = 0x1320;	
//	I2Cx_WriteMulti(u8SlaveAddr , reg , array , 2);
//	Timer1_Delay_100us(delay);
//	printf("WR : 0x%2X : 0x%2X , 0x%2X \r\n" ,reg ,array[0],array[1]);

//	value = 0;
//	I2Cx_ReadMulti(u8SlaveAddr , reg , &value , 1);
//	printf("RD : 0x%2X : 0x%2X \r\n" ,reg , value);
//	value = 0;	
//	I2Cx_ReadMulti(u8SlaveAddr , reg+1 , &value , 1);
//	printf("RD : 0x%2X : 0x%2X \r\n" ,reg+1 ,value);

	#if 0	//dump EEPROM
	EEPROM_dump();
	#endif
	
}

void EEPROM_process(void)
{
	uint16_t i = 0;
	uint8_t value = 0;
	static uint8_t addr = 0;
	static uint8_t temp = 0;
		
	const uint8_t data1[16] = 
	{
		0x23 , 0x16 , 0x80 , 0x49 , 0x56 , 0x30 , 0x17 , 0x22 ,
		0x33 , 0x46 , 0x55 , 0x27 , 0x39 , 0x48 , 0x57 , 0x60			
	};

	state_machine();
	
	if (is_flag_set(flag_WriteAddr))		// fix vaule , to incr address
	{
		set_flag(flag_WriteAddr , Disable);
		printf("write fix data in address\r\n");
		
		value = 0x01;
		I2Cx_WriteMulti(u8SlaveAddr , addr , &value , 1);
		printf("WR : 0x%2BX : 0x%2BX \r\n" , addr++ , value);
	}

	if (is_flag_set(flag_WriteData))		// incr vaule , to fix address
	{
		set_flag(flag_WriteData , Disable);
		printf("write increase data in increase address (one)\r\n");
		
		value = temp++;
		addr = 0x10;
		I2Cx_WriteMulti(u8SlaveAddr , addr , &value , 1);
		printf("WR : 0x%2BX : 0x%2BX \r\n" , addr++ , value);	
	}

	if (is_flag_set(flag_WriteData1))
	{
		set_flag(flag_WriteData1 , Disable);
		printf("write array data in address 0x40\r\n");
		
		addr = 0x40;
		I2Cx_WriteMulti(u8SlaveAddr , addr ,(uint8_t*) data1 , 16 );

		for ( i = 0 ; i < 16; i++)
		{
			printf("WR : 0x%2BX : 0x%2BX \r\n" , addr , data1[i]);				
		}		
	}

	if (is_flag_set(flag_WriteData2))
	{
		set_flag(flag_WriteData2 , Disable);
		printf("write increase data in increase address (256)\r\n");
		
		addr = 0x00;
		for ( i = 0 ; i < 0x100; i++)
		{
			value = i; 
			addr = i; 			
			I2Cx_WriteMulti(u8SlaveAddr , addr , &value , 1);
			Timer1_Delay_10us(10);			
//			printf("WR : 0x%2BX : 0x%2BX \r\n" , addr , value);

		}		
	}	

	if (is_flag_set(flag_Dump))
	{
		set_flag(flag_Dump , Disable);
		EEPROM_dump();
	}

	if (is_flag_set(flag_Erase))
	{
		set_flag(flag_Erase , Disable);
		EEPROM_clear();
	}	
}

void GPIO_Init(void)
{
	P17_QUASI_MODE;		
	P30_PUSHPULL_MODE;	
}

void Timer1_Delay_10us(UINT32 u32CNT)
{
    clr_CKCON_T1M;                                    	//T1M=0, Timer1 Clock = Fsys/12
    TMOD |= 0x10;                                		//Timer1 is 16-bit mode
    set_TCON_TR1;                                    	//Start Timer1
    while (u32CNT != 0)
    {
        TL1 = LOBYTE(TIMER_DIV12_VALUE_10us);    		//Find  define in "Function_define.h" "TIMER VALUE"
        TH1 = HIBYTE(TIMER_DIV12_VALUE_10us);
        while (TF1 != 1);                        		//Check Timer1 Time-Out Flag
        clr_TCON_TF1;
        u32CNT --;
    }
    clr_TCON_TR1;                                     	//Stop Timer1
}

void Timer1_Delay_100us(UINT32 u32CNT)
{
    clr_CKCON_T1M;                                    	//T1M=0, Timer1 Clock = Fsys/12
    TMOD |= 0x10;                                		//Timer1 is 16-bit mode
    set_TCON_TR1;                                    	//Start Timer1
    while (u32CNT != 0)
    {
        TL1 = LOBYTE(TIMER_DIV12_VALUE_100us);    		//Find  define in "Function_define.h" "TIMER VALUE"
        TH1 = HIBYTE(TIMER_DIV12_VALUE_100us);
        while (TF1 != 1);                        		//Check Timer1 Time-Out Flag
        clr_TCON_TF1;
        u32CNT --;
    }
    clr_TCON_TR1;                                     	//Stop Timer1
}

void Timer0_IRQHandler(void)
{

	tick_counter();

	if ((get_tick() % 1000) == 0)
	{
//    	printf("%s : %4d\r\n",__FUNCTION__,LOG++);
		
	}

	if ((get_tick() % 50) == 0)
	{

	}		
}

void Timer0_ISR(void) interrupt 1        // Vector @  0x0B
{
    _push_(SFRS);	
	
    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;
    clr_TCON_TF0;
	
	Timer0_IRQHandler();

    _pop_(SFRS);	
}

void TIMER0_Init(void)
{
	uint16_t res = 0;

	/*
		formula : 16bit 
		(0xFFFF+1 - target)  / (24MHz/psc) = time base 

	*/	
	const uint16_t TIMER_DIV12_1ms_FOSC_240000 = 65536-2000;

	ENABLE_TIMER0_MODE1;	// mode 0 : 13 bit , mode 1 : 16 bit
    TIMER0_FSYS_DIV12;
	
	u8TH0_Tmp = HIBYTE(TIMER_DIV12_1ms_FOSC_240000);
	u8TL0_Tmp = LOBYTE(TIMER_DIV12_1ms_FOSC_240000); 

    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;

    ENABLE_TIMER0_INTERRUPT;                       //enable Timer0 interrupt
    ENABLE_GLOBAL_INTERRUPT;                       //enable interrupts
  
    set_TCON_TR0;                                  //Timer0 run
}


void Serial_ISR (void) interrupt 4 
{
    _push_(SFRS);

    if (RI)
    {   
      uart0_receive_flag = 1;
      uart0_receive_data = SBUF;
      clr_SCON_RI;                                         // Clear RI (Receive Interrupt).
    }
    if  (TI)
    {
      if(!BIT_UART)
      {
          TI = 0;
      }
    }

    _pop_(SFRS);	
}

void UART0_Init(void)
{
	#if 1
	const unsigned long u32Baudrate = 115200;
	P06_QUASI_MODE;    //Setting UART pin as Quasi mode for transmit
	
	SCON = 0x50;          //UART0 Mode1,REN=1,TI=1
	set_PCON_SMOD;        //UART0 Double Rate Enable
	T3CON &= 0xF8;        //T3PS2=0,T3PS1=0,T3PS0=0(Prescale=1)
	set_T3CON_BRCK;        //UART0 baud rate clock source = Timer3

	RH3    = HIBYTE(65536 - (SYS_CLOCK/16/u32Baudrate));  
	RL3    = LOBYTE(65536 - (SYS_CLOCK/16/u32Baudrate));  
	
	set_T3CON_TR3;         //Trigger Timer3
	set_IE_ES;

	ENABLE_GLOBAL_INTERRUPT;

	set_SCON_TI;
	BIT_UART=1;
	#else	
    UART_Open(SYS_CLOCK,UART0_Timer3,115200);
    ENABLE_UART0_PRINTF; 
	#endif
}


void MODIFY_HIRC_24(void)
{
	unsigned char u8HIRCSEL = HIRC_24;
    unsigned char data hircmap0,hircmap1;
//    unsigned int trimvalue16bit;
    /* Check if power on reset, modify HIRC */
    SFRS = 0 ;
	#if 1
    IAPAL = 0x38;
	#else
    switch (u8HIRCSEL)
    {
      case HIRC_24:
        IAPAL = 0x38;
      break;
      case HIRC_16:
        IAPAL = 0x30;
      break;
      case HIRC_166:
        IAPAL = 0x30;
      break;
    }
	#endif
	
    set_CHPCON_IAPEN;
    IAPAH = 0x00;
    IAPCN = READ_UID;
    set_IAPTRG_IAPGO;
    hircmap0 = IAPFD;
    IAPAL++;
    set_IAPTRG_IAPGO;
    hircmap1 = IAPFD;
    clr_CHPCON_IAPEN;

	#if 0
    switch (u8HIRCSEL)
    {
		case HIRC_166:
		trimvalue16bit = ((hircmap0 << 1) + (hircmap1 & 0x01));
		trimvalue16bit = trimvalue16bit - 15;
		hircmap1 = trimvalue16bit & 0x01;
		hircmap0 = trimvalue16bit >> 1;

		break;
		default: break;
    }
	#endif
	
    TA = 0xAA;
    TA = 0x55;
    RCTRIM0 = hircmap0;
    TA = 0xAA;
    TA = 0x55;
    RCTRIM1 = hircmap1;
    clr_CHPCON_IAPEN;
    PCON &= CLR_BIT4;
}


void SYS_Init(void)
{
    MODIFY_HIRC_24();

    ALL_GPIO_QUASI_MODE;
    ENABLE_GLOBAL_INTERRUPT;                // global enable bit	
}

void main (void) 
{
    SYS_Init();

    UART0_Init();
	GPIO_Init();
	TIMER0_Init();

	I2Cx_Init();
	EEPROM_test();

	demo_state = 0;
		
    while(1)
    {
		EEPROM_process();
		
    }
}



