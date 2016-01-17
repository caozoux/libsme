#include <stdio.h>

#define IRQ_DIS() 
#define IRQ_EN() 

#define CYCL_MAX 128
#define CYCL_OVER(s) ((s == (CYCL_MAX-1)) ? 1 : 0)

#define CTCK_CHECK 
#define CYCL_INCREASE(s) 				((s == (CYCL_MAX-1)) ? 0 : s+1)
#define CYCL_SET(buf, s, e, a) 			do {if (e == (CYCL_MAX-1)) { buf[e] =a ;e=0;} else{ buf[e++] = a; }\
											}while(0)
#define CYCL_GET(buf, s, e, val) 		do {if (s == (CYCL_MAX-1)) { s= 0; val = buf[CYCL_MAX-1]; } else{ val = buf[s++]; }\
											}while(0) 
#define CYCL_IS_FULL() 					((cycl_start == CYCL_INCREASE(cycl_end)) ? 1 : 0)
#define CYCL_IS_empty() 				((cycl_start == cycl_end))
static unsigned char cyclBuf[CYCL_MAX];
static unsigned char cycl_start;
static unsigned char cycl_end;

void cyclAdd(unsigned char ch)
{
	if (CYCL_IS_FULL())
		return;
	IRQ_DIS();
	CYCL_SET(cyclBuf, cycl_start, cycl_end, ch);
	IRQ_EN();
}

unsigned char cyclGet(void)
{
	unsigned char ch;
	if (CYCL_IS_empty())
		return 0;
	IRQ_DIS();
	CYCL_GET(cyclBuf, cycl_start, cycl_end, ch);
	IRQ_EN();
	return ch;
}

void dump(void)
{
	unsigned char i;
	for (i =0; i <CYCL_MAX; i++) {
		printf("%d:%d ", i, cyclBuf[i]);
	}
	printf("\n");
	printf("s:%d e:%d\n", cycl_start, cycl_end);
}

int main(int argc, char *args[])
{
	unsigned char i;
	cycl_start = 0;
	cycl_end = 0;

	for (i =0; i <CYCL_MAX; i ++) {
		cyclAdd(i);
	}

	dump();
	printf("%d \n", cyclGet());
	printf("%d \n", cyclGet());
	printf("%d \n", cyclGet());
	cyclAdd(4);
	cyclAdd(4);
	cyclAdd(4);
	cyclAdd(4);
	dump();
	printf("<------------test dynam add\n");
	for (i =0; i <120; i ++) {
		cyclGet();
	}
	dump();
}
