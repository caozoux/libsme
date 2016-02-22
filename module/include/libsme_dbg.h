#ifndef _LIBSME_DBG_H_
#define _LIBSME_DBG_H_
#define _LDBGD(fmt, ...) do {printf(fmt,## __VA_ARGS__);} while(0)
#define _LDBGW(fmt, ...) do {printf(fmt,## __VA_ARGS__);} while(0)
#define _LDBGI(fmt, ...) do {printf(fmt,## __VA_ARGS__);} while(0)
#define _LDBGF(fmt, ...) do {printf(fmt,## __VA_ARGS__);} while(0)
#endif
