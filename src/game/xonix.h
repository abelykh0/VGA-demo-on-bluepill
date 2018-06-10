/* Hey emacs, make this use -*- Mode: C++; tab-width: 4 -*- */
#ifndef XONIX_H
#define XONIX_H

/*
 * xonix global definitions
 *
 * xonix.h,v 1.39 1995/09/08 09:51:30 j Exp
 */

/*
 * Copyright (c) 1995
 *   Torsten Schoenitz <torsten_schoenitz@bonnie.heep.sax.de>
 *   Joerg Wunsch <joerg_wunsch@uriah.heep.sax.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE DEVELOPERS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE DEVELOPERS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Typedefs.*/

struct point {
  short h, v;
};

typedef struct point myPoint;

struct rectangle {
	short top, left, bottom, right;
};

typedef struct rectangle myRect;
							
struct Player   {
  int       x, y;                           /* Koordinaten der Figur        */
  int       sx, sy;                         /* Groesse der Figur            */
  int       dx, dy;                         /* Schrittweite der Figur       */
  myRect	rr;								/* Redraw-Rechteck				*/
};

typedef struct Player Player;

struct Segment	{
	int		y;
	int		xl;
	int		xr;
	int		dy;
};

typedef struct Segment mySegment;

enum attribute  {
            EMPTY       = 0x00,             /* Ungefuellt                   */
            WAY         = 0x01,             /* Spur des Runners             */
            TESTFILLED  = 0x02,             /* Probefuellung                */
            FILLED      = 0x04,             /* Endgueltig gefuellt          */
            BORDER      = 0x08,             /* Rahmen                       */
            RUNNER      = 0x80,             /* Spielfigur                   */
            FLYER       = 0x40,             /* Flieger                      */
            EATER       = 0x20              /* Fresser                      */
            };


/* Andere Konstanten. */

/* Die Spieldimensionen werden durch die "Fresser"-Groesse bestimmt.        */
/* H_STEPS und V_STEPS sollten multipliziert mit EATER_SIZE jeweils         */
/* etwa 600 bzw. 400 ergeben                                                */
									     
#define EATER_SIZE          4               /* Groesse der "Fresser"        */
#define RATIO               1               /* Faktor von Eater zu Runner   */

#define PERCENT_THRESHOLD	75				/* Fuellstand - Schwelle		*/
#define LEVEL_BONUS_FACTOR	100.0        /* Bonus fuer geschafftes Level */
#define LOOP_FACTOR			10.0			/* Faktor fuer Punktberechnung	*/

#define FLYER_SIZE          RATIO*EATER_SIZE   /* Groesse der "Flieger"     */
#define FLYER_STEP          RATIO              /* Schrittweite der "Flieger"*/
#define RUNNER_SIZE         RATIO*EATER_SIZE   /* Groesse der Spielfigur    */
#define FIELD_WIDTH         H_STEPS*EATER_SIZE /* Breite des Spielfeldes    */
#define FIELD_HEIGHT        V_STEPS*EATER_SIZE /* Hoehe des Spielfeldes     */

#define MAX_FLYER           10
#define MAX_EATER           4

/* ------------------------------------------------------------------------ */
/* Deklaration der globalen Variablen.                                      */
/* ------------------------------------------------------------------------ */

extern Boolean      gQuit,                  /* Wenn gQuit auf true gesetzt  */
                                            /* wird, terminiert die Appli-	*/
                                            /* kation.                      */
                    gRun,                   /* Ist gRun auf true gesetzt,	*/
                                            /* laeuft das Spiel, sonst sind */
                                            /* Einstellungen usw. moeglich  */
                    gPause,					/* Spiel unterbrochen           */
					gEndOfGame;				/* Runner sind alle 			*/
extern int          gFlyerCount;            /* Anzahl der Flieger           */
									    
extern Player       gFlyer[MAX_FLYER];      /* Alle Flieger                 */

extern int          gEaterCount;            /* Anzahl der Fresser           */
									    
extern Player       gEater[MAX_EATER];      /* Alle Fresser                 */
									    
extern Player       gMyRunner;              /* Spielfigur                   */
									    
extern Ptr          gMyStatusArea;          /* Status-Area                  */

extern unsigned     gHighScore;	            /* Punktestand                  */
extern int			gLevel;					/* Es geht mit Level 1 los		*/

/* ------------------------------------------------------------------------ */
/* Funktionen								     							*/
/* ------------------------------------------------------------------------ */

extern void Animate (void);
extern void DrawRunnerToGWorld (void);
extern void DrawWayToGWorld (int xPos, int yPos);
extern void DrawEmptyToGWorld (int xPos, int yPos);
extern void DrawFlyerToGWorld (int xPos, int yPos);
extern void DrawEaterToGWorld (int xPos, int yPos);
extern void DrawFilledToGWorld (int xPos, int yPos);
extern void DrawSmallFilledToGWorld (int xPos, int yPos);
extern void DrawCompleteBorder (void);
extern void SetRunner (int x, int y);
extern void ClearWay (void);
extern Boolean FillUp (int x, int y);
extern void ExitXonix (int number);
extern void ScorePercentage (int num);
extern void ScoreLevel (int num);
extern void ScoreRunner (int num);
extern void DisplayHighScore (void);
extern void DrawComplete();
extern void ScorePoints(int points);
extern void Do_New (void);

#ifdef __cplusplus
}
#endif

#endif /* XONIX_H */
