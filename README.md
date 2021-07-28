# MS51_Analog_I2C_EEPROM
 MS51_Analog_I2C_EEPROM


update @ 2021/07/28

1. use GPIO (P1.3 for SCL , P1.4 for SDA) to emulate I2C , to initial EEPROM

2. below is log terminal 

write data , read data , and dump 

![image](https://github.com/released/MS51_Analog_I2C_EEPROM/blob/main/teraterm_log.jpg)


write data in address : 0x40 , and dump

![image](https://github.com/released/MS51_Analog_I2C_EEPROM/blob/main/teraterm_log2.jpg)

write data (256 bytes) , read data (256 bytes) , and dump 

![image](https://github.com/released/MS51_Analog_I2C_EEPROM/blob/main/teraterm_log3.jpg)

3. below is LA capture 

write data waveform

![image](https://github.com/released/MS51_Analog_I2C_EEPROM/blob/main/LA_WR.jpg)

read data waveform

![image](https://github.com/released/MS51_Analog_I2C_EEPROM/blob/main/LA_RD.jpg)

