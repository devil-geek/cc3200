#ifndef __SIMPLE_LOG_H__
#define __SIMPLE_LOG_H__

//#define DUMP_DEBUG_LOG

#ifdef DUMP_DEBUG_LOG
#define DEBUG_LOGN(x) Serial.println(x)
#define DEBUG_LOG(x) Serial.print(x)
#else
#define DEBUG_LOGN(x)
#define DEBUG_LOG(x)
#endif

#define ERROR_LOGN(x) Serial.println(x)
#define ERROR_LOG(x) Serial.print(x)


#endif

