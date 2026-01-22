#include "init.h"
#include "led.h"
#include "key.h"
#include "seg.h"
#include "ultrasound.h"
#include "iic.h"
#include "onewire.h"
#include "ds1302.h"

idata unsigned long int uwTick = 0;
idata unsigned char seg_pos;
pdata unsigned char ucLed[8] = {0,0,0,0,0,0,0,0};
pdata unsigned char seg_buf[8] = {10,10,10,10,10,10,10,10};
idata unsigned char key_val,key_old,key_down,key_up;
//PWM
idata unsigned char pwm_period = 12;//循环周期
idata unsigned char pwm_cnt = 0;//轮询计数器
idata unsigned char pwm_duty = 0;//占空比
//彩灯控制
idata unsigned char seg_disp_mode = 0;//0:设置界面 1:亮度等级
idata unsigned char run_mode_num = 0;//运行模式编号
idata unsigned char run_mode_set = 0;//运行模式设置
idata unsigned char run_mode_num_disp = 0;//运行模式编号
pdata unsigned int run_time[4] = {400,400,400,400};//400-1200
bit change_mode = 0;//0-运行模式 1-流转间隔
bit flash_flage = 0;//闪烁标志位
idata unsigned int Timer_800ms = 0;
idata unsigned char bright_grade = 0;//亮度等级
bit seg_start_flag = 0;//0:关 1：开
bit led_run_start = 0;//0:关闭 1:开启
idata unsigned long int sys_ticks = 0;//控制流水灯速度
idata unsigned char led_grade_disp = 0;

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

void key_proc()
{
	unsigned char i = 0;
	key_val = key_read();
	key_down = key_val & (key_val ^ key_old);
	key_up = ~key_val & (key_val ^ key_old);
	key_old = key_val;
	
	if(key_down == 4 && seg_start_flag == 0){
		seg_start_flag = 1;
		seg_disp_mode = 1;
		for(i = 0;i <= 5;i++)
			seg_buf[i] = 10;
	}
	if(key_up == 4 && seg_disp_mode == 1){
		seg_start_flag = 0;
	}
	
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
	switch(key_down)
	{
		case 5:
			if(seg_disp_mode == 0 && seg_start_flag == 1){
				if(change_mode == 0){
					run_mode_set++;
					if(run_mode_set >= 4)
						run_mode_set = 3;
				} else {
					run_time[run_mode_set] += 100;
					if(run_time[run_mode_set] >= 1200)
						run_time[run_mode_set] = 1200;
				}
			}	
		break;
		case 4:
			if(seg_disp_mode == 0 && seg_start_flag == 1){
				if(change_mode == 0){
					run_mode_set--;
					if(run_mode_set > 200)
						run_mode_set = 0;
				} else {
					run_time[run_mode_set] -= 100;
					if(run_time[run_mode_set] <= 400)
						run_time[run_mode_set] = 400;
				}
			}
		break;
		case 7:
			led_run_start ^= 1;
			
		break;
	}
	run_mode_num_disp = run_mode_set + 1;
}

void seg_proc()
{
	unsigned char i;
	switch(seg_disp_mode)
	{
		case 0://设置状态
			seg_buf[0] = seg_buf[2] = 0;
			seg_buf[1] = run_mode_num_disp;
			seg_buf[3] = 10;
			seg_buf[4] = (run_time[run_mode_set]/1000%10 == 0) ? 10 : run_time[run_mode_set]/1000%10;
			seg_buf[5] = run_time[run_mode_set]/100%10;
			seg_buf[6] = run_time[run_mode_set]/10%10;
			seg_buf[7] = run_time[run_mode_set]%10;
			if(change_mode == 0){
				seg_buf[0] = seg_buf[2] = flash_flage ? 0 : 10;
				seg_buf[1] = flash_flage ? run_mode_num_disp : 10;
			} else {
				seg_buf[4] = flash_flage ? ((run_time[run_mode_set]/1000%10 == 0) ? 10 : run_time[run_mode_set]/1000%10) : 10;
				seg_buf[5] = flash_flage ? run_time[run_mode_set]/100%10 : 10;
				seg_buf[6] = flash_flage ? run_time[run_mode_set]/10%10 : 10;
				seg_buf[7] = flash_flage ? run_time[run_mode_set]%10 : 10;
			}
		break;
		case 1://亮度等级
			for(i = 0;i <= 5;i++)
				seg_buf[i] = 10;
			seg_buf[0] = led_grade_disp;
			seg_buf[6] = 0;
			seg_buf[7] = bright_grade;
		break;
	}
}

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

void Timer1_Isr(void) interrupt 3
{
	uwTick++;
	seg_pos = (seg_pos + 1) % 8;
	
	if(seg_start_flag == 0){
		seg_disp(seg_pos,10,0);
	} else {
		if(seg_buf[seg_pos] > 20)
			seg_disp(seg_pos,seg_buf[seg_pos] - ',',1);
		else
			seg_disp(seg_pos,seg_buf[seg_pos],0);
	}
	
	if(seg_disp_mode == 0){
		Timer_800ms++;
		if(Timer_800ms == 800){
			Timer_800ms = 0;
			flash_flage ^= 1;
		}
	} else {
		Timer_800ms = 0;
		change_mode = 0;//每次进入流转间隔设置界面就重置
	}
	//流水灯
	if(led_run_start == 1)
		sys_ticks++;
	else
		sys_ticks = 0;
}

void Timer1_Init(void)		//1毫秒@12.000MHz
{
	AUXR &= 0xBF;			//定时器时钟12T模式
	TMOD &= 0x0F;			//设置定时器模式
	TL1 = 0x18;				//设置定时初始值
	TH1 = 0xFC;				//设置定时初始值
	TF1 = 0;				//清除TF1标志
	TR1 = 1;				//定时器1开始计时
	ET1 = 1;				//使能定时器1中断
	EA = 1;
}

typedef struct{
	void (*task_func)(void);
	unsigned long int rate_ms;
	unsigned long int last_ms;
} task_t;

idata unsigned char task_num;

pdata task_t scheduler_task[] = 
{
	{led_proc,1,0},
	{key_proc,10,0},
	{seg_proc,20,0},
	{get_ad,20,0},
};

void scheduler_init()
{
	task_num = sizeof(scheduler_task)/sizeof(task_t);
}

void scheduler_run()
{
	unsigned char i;
	for(i = 0;i < task_num;i++)
	{
		unsigned long int now_time = uwTick;
		if(now_time >= scheduler_task[i].rate_ms + scheduler_task[i].last_ms){
			scheduler_task[i].last_ms = now_time;
			scheduler_task[i].task_func();
		}
	}
}

void main()
{
	system_init();
	Timer1_Init();
	load_run_time();
	scheduler_init();
	while(1)
	{
		scheduler_run();
	}
}
