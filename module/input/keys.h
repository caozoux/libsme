#ifndef __MEKEYS_H__
#define __MEKEYS_H__
struct MEKey_data {
const char *name;
int keycode;
unsigned char hid_scan_code;
int usec;
int sec;
};

struct MEKey_data ubuntu_key[] = 
	{{"ESC",4, 0x29},
	 {"F1",59, 0x3a},
	 {"F2",60, 0x3b},
	 {"F3",61, 0x3c},
	 {"F4",62, 0x3d},
	 {"F5",63, 0x3e},
	 {"F6",64, 0x3f},
	 {"F7",65, 0x40},
	 {"F8",66, 0x41},
	 {"F9",67, 0x42},
	 {"F10",68, 0x43},
	 {"F11",87, 0x44},
	 {"F12",88, 0x45},
	 {"`",41, 0x35},
	 {"1",2, 0x1e},
	 {"2",3, 0x1f},
	 {"3",4, 0x20},
	 {"4",5, 0x21},
	 {"5",6, 0x22},
	 {"6",7, 0x23},
	 {"7",8, 0x24},
	 {"8",9, 0x25},
	 {"9",10, 0x26},
	 {"0",11, 0x27},
	 {"-",12, 0x2d},
	 {"=",13, 0x2e},
	 {"DEL",14, 0x4c},
	 {"TAB",15, 0x2b},
	 {"q",16, 0x14},
	 {"w",17, 0x1a},
	 {"e",18, 0x08},
	 {"r",19, 0x15},
	 {"t",20, 0x17},
	 {"y",21, 0x1c},
	 {"u",22, 0x18},
	 {"i",23, 0x0c},
	 {"o",24, 0x12},
	 {"p",25, 0x13},
	 {"[",26, 0x2f},
	 {"]",27, 0x30},
	 {"\\",43, 0x31},
	 {"CL",28, 0x39},
	 {"a",30, 0x04},
	 {"s",31, 0x16},
	 {"d",32, 0x07},
	 {"f",33, 0x09},
	 {"g",34, 0x0a},
	 {"h",35, 0x0b},
	 {"j",36, 0x0d},
	 {"k",37, 0x0e},
	 {"l",38, 0x0f},
	 {";",39, 0x33},
	 {"'",40, 0x34},
	 {"CR",28, 0x28},
	 {"SHL",42, 0x02}, //shift在hid传输缓存buf[8]中	, buf[0] =2，然后在buf[3]中设置key scan code
	 {"x",45, 0x1b},
	 {"c",46, 0x06},
	 {"v",47, 0xa19},
	 {"b",48, 0x05},
	 {"n",49, 0x11},
	 {"m",50, 0x10},
	 {",",51, 0x36},
	 {".",52, 0x37},
	 {"/",53, 0x38},
	 {"SHR",54, 0x20}, //the same as SHL, buf buf[0] = 0x20
	 {"CTRL",29, 0x01}, //the same as SHL, buf buf[0] = 0x01
	 {"WINL",125, 0x08}, //the same as SHL, buf buf[0] = 0x08
	 {"ALTL",56, 0x04}, //the same as SHL, buf buf[0] = 0x04
	 {"SP",57, 0x2c},
	 {"ALTR",100, 0x40},  //the same as SHL, buf buf[0] = 0x40
	 {"PAST",126, 0x65},
	 {"CTRR",97, 0x20},   //the same as SHL, buf buf[0] = 0x01
	 {"UP",103, 0x52},
	 {"LEFT",105, 0x50},
	 {"DOWN",108, 0x51},
	 {"RIHT",106, 0x4f},
	 {"prts",99, 0x0},
	 {"", 0,0}
     };
 #endif
