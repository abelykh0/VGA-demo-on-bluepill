#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define H_STEPS             64      /* Horizontal moegl. Positionen	*/
#define V_STEPS             46      /* Vertikal moegl. Positionen 	*/
#define STEP_TIME           50		/* Schrittweite in ms */

void GameInit();
void GameUpdate();

//void GlobalProtoHandler(Widget, XEvent *, String *, Cardinal *);
//void Continue();
void Pause();
void Quit();

#ifdef __cplusplus
}
#endif

#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

typedef unsigned char * Ptr;
#define NIL_POINTER			NULL

typedef int BOOL;
#define Boolean BOOL

#define BELL() (void)0
#define GWorldEntry() (void)0
#define GWorldExit() (void)0
#define GWorldExitFlyer() (void)0
#define GWorldExitEater() (void)0
#define GWorldExitRunner() (void)0

extern uint8_t GetUsbBuffer(char *buffer, uint8_t maxLength);

