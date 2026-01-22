1.PWM+AD

~~~c
//PWM
idata unsigned char pwm_period = 12;//循环周期
idata unsigned char pwm_cnt = 0;//轮询计数器
idata unsigned char pwm_duty = 0;//占空比
/*
*	均分4份,用ad值去判断亮度等级,
* 再用亮度等级去判断占空比
*/
void get_ad()
{
	unsigned char ad = AD_read(0x43);
	if(ad < 64) bright_grade = 1;
	else if(ad < 128) bright_grade = 2;
	else if(ad < 192) bright_grade = 3;
	else bright_grade = 4;
	
	switch(bright_grade){
		case 1:pwm_duty = 3;break;
		case 2:pwm_duty = 6;break;
		case 3:pwm_duty = 9;break;
		case 4:pwm_duty = 12;break;
	}
}
~~~

多功能按键+把数据保存到EEPROM中

~~~c
if(key_down == 6){
    if(seg_start_flag == 0){
        seg_start_flag = 1;
        seg_disp_mode = 0;
    } else if(seg_start_flag == 1 && change_mode == 0){
        change_mode ^= 1;
    } else if(seg_start_flag == 1 && change_mode == 1){
        save_run_time();//保存数据
        seg_start_flag = 0;
        change_mode = 0;
        run_mode_set = 0;
    }
}
~~~

EEPORM函数

~~~c
void EEPROM_write_byte(unsigned char dat,unsigned char addr)
{
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	
	IIC_SendByte(addr);
	IIC_WaitAck();
	
	IIC_SendByte(dat);
	IIC_WaitAck();
	IIC_Stop();//**重点**:先stop再delay;
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
	IIC_Delay(255);
}

unsigned char EEPROM_read_byte(unsigned char addr)
{
	unsigned char temp = 0;
	IIC_Start();
	IIC_SendByte(0xa0);
	IIC_WaitAck();
	
	IIC_SendByte(addr);
	IIC_WaitAck();
	IIC_Start();
	IIC_SendByte(0xa1);
	IIC_WaitAck();
	temp = IIC_RecByte();
	IIC_SendAck(1);
	IIC_Stop();
	return temp;
}
//高八位数据跟第八位数据分开保存
void save_run_time()
{
	unsigned char i;
	for(i = 0;i < 4;i++){
		EEPROM_write_byte((unsigned char)(run_time[i] >> 8),0x10 + i*2);
		EEPROM_write_byte((unsigned char)(run_time[i] & 0x00ff),0x11 + i*2);
	}
}

void load_run_time()
{
	unsigned char i = 0;
	unsigned int val = 0;
	for(i = 0;i < 4;i++)
	{
		val = (EEPROM_read_byte(0x10 + i*2) << 8) | EEPROM_read_byte(0x11 + i*2);
		
		if(val >= 400 && val <= 1200){
			run_time[i] = val;
		} else {
			run_time[i] = 400;
		}
	}
}
~~~

花式流水灯

~~~c
void led_proc()
{
	unsigned char i;
	static unsigned char led_step = 0;
	unsigned char led_mask = 0;
	
	if(led_run_start == 0){
		for(i = 0;i < 8;i++) ucLed[i] = 0;
		led_step = 0;
		sys_ticks = 0;
	} else {
		if(sys_ticks >= run_time[run_mode_num])
		{
			sys_ticks = 0;
			led_step++;
			
			if((run_mode_num <= 1 && led_step >= 8) ||
				(run_mode_num >= 2 && led_step >= 4))
			{
				led_step = 0;
				run_mode_num++;
				if(run_mode_num >= 4) run_mode_num = 0;
			}
		}
		switch(run_mode_num)
		{
			case 0:
				led_mask = (1 << led_step);
			break;
			case 1:
				led_mask = (1 << (7 - led_step));
			break;
			case 2:
				led_mask = (1 << led_step) | (1 << (7 - led_step));
			break;
			case 3:
				led_mask = (1 << (3 - led_step)) | (1 << (4 + led_step));
			break;
			default:
				led_mask = 0;
			break;
		}
		
		for(i = 0;i < 8;i++)
		{
			ucLed[i] = (led_mask >> i) & 0x01;
		}
	}
	led_grade_disp = run_mode_num + 1;//用于数码管显示亮度等级,便于观察
	pwm_cnt++;
	if(pwm_cnt >= pwm_period) pwm_cnt = 0;
	if(pwm_cnt < pwm_duty){
		led_disp(ucLed);
	} else {
		led_off();
	}
}

~~~

