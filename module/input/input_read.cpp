#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>  
#include <linux/input.h> 
#include <unistd.h>
#ifdef __cplusplus
extern "C" {
#endif
/*
 inux/input.h中有定义，这个文件还定义了标准按键的编码等
 struct input_event {
 struct timeval time; //按键时间
 __u16 type; //类型，在下面有定义
 __u16 code; //要模拟成什么按键
 __s32 value;//是按下还是释放
 };
code：
 事件的代码.如果事件的类型代码是EV_KEY,该代码code为设备键盘代码.代码植0~127为键盘上的按键代码,0x110~0x116 为鼠标上按键代码,其中0x110(BTN_ LEFT)为鼠标左键,0x111(BTN_RIGHT)为鼠标右键,0x112(BTN_ MIDDLE)为鼠标中键.其它代码含义请参看include/linux/input.h文件. 如果事件的类型代码是EV_REL,code值表示轨迹的类型.如指示鼠标的X轴方向REL_X(代码为0x00),指示鼠标的Y轴方向REL_Y(代码 为0x01),指示鼠标中轮子方向REL_WHEEL(代码为0x08).
type: 
EV_KEY,键盘
EV_REL,相对坐标
EV_ABS,绝对坐标
value：
事件的值.如果事件的类型代码是EV_KEY,当按键按下时值为1,松开时值为0;如果事件的类型代码是EV_ REL,value的正数值和负数值分别代表两个不同方向的值.
* * Event types
#define EV_SYN 0x00
#define EV_KEY 0x01 //按键
#define EV_REL 0x02 //相对坐标(轨迹球)
#define EV_ABS 0x03 //绝对坐标
#define EV_MSC 0x04 //其他
#define EV_SW 0x05
#define EV_LED 0x11 //LED
#define EV_SND 0x12//声音
#define EV_REP 0x14//repeat
#define EV_FF 0x15 
#define EV_PWR 0x16
#define EV_FF_STATUS 0x17
#define EV_MAX 0x1f
#define EV_CNT (EV_MAX+1)
*/ 


#define EVENT_NAME ("/dev/input/event2")
int main(void)  
{  
    struct input_event ev_temp;  
    int fd = open(EVENT_NAME, O_RDWR);  
    if(fd < 0) {  
        printf("open device failed.\n");  
        return 0;  
    }  
    printf("open successfully!\n");  

    int count;
	char up[] = "up";
	char down[] = "down";
  
    while(1) {
        count = read(fd, &ev_temp, sizeof(struct input_event));
		if(count)
		{
			if(ev_temp.type == EV_SYN)
				continue;

			printf("key:%d ", ev_temp.code);
			printf("%s\n", ev_temp.value?down:up);
			continue;
		}
    }
    return 0;  
}
#ifdef __cplusplus
}
#endif
