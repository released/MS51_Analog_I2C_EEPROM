
#ifndef _I2C_ANALOG_H_
#define _I2C_ANALOG_H_

//#include "MS51_16K.h"
//#include "Function_define_MS51_16K.h"
/*------------------------------------------

------------------------------------------*/
#define I2C_WR                  					(0x00)
#define I2C_RD                  					(0x01)

#define SET_SDA              					(P14 = 1)
#define RESET_SDA         						(P14 = 0)
#define SET_SCL              					(P13 = 1)
#define RESET_SCL        						(P13 = 0)

#define I2C_ANALOG_SDA_STATE 					(P14 ? 1 : 0)
#define I2C_ANALOG_SCL_STATE 					(P13 ? 1 : 0)
#define I2C_ANALOG_DELAY     					(I2C_ANALOG_Delay())

enum I2C_ANALOG_REPLAY_ENUM
{
     I2C_ANALOG_NACK = 0,
     I2C_ANALOG_ACK = 1
};

enum I2C_ANALOG_BUS_STATE_ENUM
{
     I2C_ANALOG_BUS_READY = 0,
     I2C_ANALOG_BUS_BUSY=1,
     I2C_ANALOG_BUS_ERROR=2
};



#endif


