/* Hey emacs, this is -*- Mode: C++; tab-width: 4 -*-                       */
/* ------------------------------------------------------------------------ */
/*  NAME                                                                    */
/*  xonix.c                                                                 */
/*                                                                          */
/*  BESCHREIBUNG                                                            */
/*  xonix ist ein Spielprogramm, dessen Idee aus den Anfangszeiten der	    */
/*  DOSEN stammt. Es wurde auf Basis von framework erstellt.                */
/* ------------------------------------------------------------------------ */
              
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

/* xonix.c,v 1.51 1995/09/14 11:52:10 j Exp */

#include <stdlib.h>         /* Wegen der Zufaelligkeiten                    */
#include <string.h>
//#include <time.h>
#include "xonix.h"
#include "stack.h"

#define AKT_STATUS	(*(gMyStatusArea+H_STEPS*gMyRunner.y+gMyRunner.x))

/* ------------------------------------------------------------------------ */
/* Deklaration der globalen Variablen.                                      */
/* ------------------------------------------------------------------------ */

Boolean             gQuit,                  /* Wenn gQuit auf true gesetzt  */
                                            /* wird, terminiert die 	    */
                                            /* Applikation.                 */
                    gRun,                   /* Ist gRun auf true gesetzt    */
                                            /* laeuft das Spiel, sonst sind */
                                            /* Einstellungen usw. moeglich  */
                    gPause,					/* Spiel unterbrochen           */
					gEndOfGame;				/* Runner sind alle				*/
int					gPlayer;				/* Soviele Runner kann ich		*/
											/* verheizen					*/
int					gLevel;					/* Es geht mit Level 1 los		*/

int                 gFlyerCount;            /* Anzahl der Flieger           */

int		    		gFillCount;             /* Anzahl gefuellter Flaechen-  */
				            				/* stuecke (Runner-Groesse)     */
											
Player              gFlyer[MAX_FLYER];      /* Alle Flieger                 */
																			
int                 gEaterCount;            /* Anzahl der Fresser           */
																			
Player              gEater[MAX_EATER];      /* Alle Fresser                 */
																			
Player              gMyRunner;              /* Spielfigur                   */
																			
Ptr                 gMyStatusArea;          /* Status-Area                  */

unsigned            gHighScore;             /* Punktestand                  */

int 				gLoops;					/* Schleifenzaehler im Animate	*/

/* ------------------------------------------------------------------------ */
/* Forward-Declarations.                                                    */
/* ------------------------------------------------------------------------ */

void Do_New (void);
void NewFlyer (void);
void NewRunner (void);
void NewEater (void);
void NewPlayRoom (void);
void GetNewPlayer (void);
void DrawCompleteBorder (void);
void ResetGlobals (void);
void SetPlayerToStatus (int xPos, int yPos, unsigned char figur);
Boolean HorizontalBounceCheck (int x, int y, int size, 
							   unsigned char *bouncePartner);
Boolean VerticalBounceCheck (int x, int y, int size, 
							 unsigned char *bouncePartner);

/* ------------------------------------------------------------------------ */
/* Do_New startet ein komplett neues Spiel                                  */
/* ------------------------------------------------------------------------ */

void Do_New (void)
{
	ResetGlobals ();							/* Frisch an's Werk			*/

	NewPlayRoom ();								/* Neue Spielwiese anlegen	*/

    gRun = TRUE;                                /* ... und ab gehts         */

    return;
}


/* ------------------------------------------------------------------------ */
/* ResetGlobals setzt alle spielbestimmenden Globalen in den Ausgangs-		*/
/* zustand																	*/
/* ------------------------------------------------------------------------ */

void ResetGlobals (void)
{
	gFlyerCount = 0;
	gFillCount	= 0;
	gEaterCount = 0;
	gLevel 		= 1;
	gPlayer		= 5;
	gPause		= FALSE;
	gEndOfGame	= FALSE;
	gHighScore	= 0;
	gLoops 		= 0;
}


/* ------------------------------------------------------------------------ */
/* TestFree kontrolliert, ob die uebergebene Position eine freie Position	*/
/* ist. In diesem Fall wird TRUE zurueckgegeben. Beim ersten Auftreten		*/
/* eines unfreien Platzes erfolgt die Rueckkehr mit FALSE.					*/
/* ------------------------------------------------------------------------ */

Boolean TestFree (int xPos, int yPos)
{
    int i, j;

    for (i = 0; i < RATIO; i ++)
        for (j = 0; j < RATIO; j ++)
            if (*(gMyStatusArea + (yPos + i) * H_STEPS + (xPos + j)) != EMPTY)
            {
                return FALSE;
            }
   
    return TRUE;
}  


/* ------------------------------------------------------------------------ */
/* RandomPosition errechnet eine zufaellige Position fuer einen neuen Flyer.*/
/* ------------------------------------------------------------------------ */
   
myPoint RandomPosition (void)
{
    myPoint   newPosition;
    
    //srand ((unsigned int) time (0));       /* Zufallsgenerator initialis.	*/

    newPosition.h = ((unsigned) rand ()  % ((H_STEPS / RATIO) - 4) + 2) * RATIO;
    newPosition.v = ((unsigned) rand ()  % ((V_STEPS / RATIO) - 4) + 2) * RATIO;

    return newPosition;
}


/* ------------------------------------------------------------------------ */
/* NewRunner erzeugt einen neuen Runner an der Ausgangsposition links oben	*/
/* im Rahmenbereich und traegt ihn in das Statusfeld ein                    */
/* ------------------------------------------------------------------------ */
   
void NewRunner (void)
{  
	gLoops = 0;
	
    gMyRunner.x = 0;
    gMyRunner.y = 0;
    gMyRunner.sx = RUNNER_SIZE;
    gMyRunner.sy = RUNNER_SIZE;
    gMyRunner.dx = 0;
    gMyRunner.dy = 0;
   
    SetPlayerToStatus (gMyRunner.x, gMyRunner.y, (unsigned char) RUNNER);
   
	ScoreRunner (gPlayer);
	
    return;
}  


/* ------------------------------------------------------------------------ */
/* NewEater erzeugt einen neuen Fresser im Rahmenbereich und traegt ihn in 	*/
/* das Statusfeld ein											            */
/* ------------------------------------------------------------------------ */

void NewEater (void)
{
	if (gEaterCount < MAX_EATER)
	{
    	gEater[gEaterCount].sx = EATER_SIZE;
    	gEater[gEaterCount].sy = EATER_SIZE;
    	gEater[gEaterCount].dx = 1;
    	gEater[gEaterCount].dy = 1;

    	switch (gEaterCount % 4)			/* Anfangsrichtung der Fresser	*/
    	{
			case 0: 
				gEater[gEaterCount].x = H_STEPS - RATIO;
				gEater[gEaterCount].y = V_STEPS - RATIO - 1 - gEaterCount * 4;
				break;

        	case 1: 
				gEater[gEaterCount].x = H_STEPS - RATIO - gEaterCount * 4;
                gEater[gEaterCount].y = V_STEPS - RATIO;
				gEater[gEaterCount].dy *= -1;
                break;

        	case 2: 
				gEater[gEaterCount].x = H_STEPS - RATIO - gEaterCount * 4;
                gEater[gEaterCount].y = 0;
				gEater[gEaterCount].dx *= -1;
                break;

        	case 3: 
				gEater[gEaterCount].x = 0;
                gEater[gEaterCount].y = V_STEPS - RATIO;
				gEater[gEaterCount].dx *= -1;
                gEater[gEaterCount].dy *= -1;
                break;
    	}
		/* Fresser in Statusfeld eintragen */

    	*(gMyStatusArea
        	+ gEater[gEaterCount].y * H_STEPS
        	+ gEater[gEaterCount].x)            |= (unsigned char) EATER;
   
		gEaterCount ++;
	}
   	return;
}



/* ------------------------------------------------------------------------ */
/* ResetStatus erzeugt einen definierten Anfangszustand im StatusArea		*/
/* ------------------------------------------------------------------------ */

void ResetStatus (void)
{
	int i, j, k;
	
    i = H_STEPS * V_STEPS;            /* Also z.B. 150 x 100    */
    j = H_STEPS * 2 + 2;              /* Beginn EMPTY-Bereich	*/
    k = H_STEPS - 4;                  /* Breite EMPTY-Bereiches	*/
   																			 
    memset (gMyStatusArea, (char) FILLED, i);   /* erstmal ist alles Rahmen */
   																			 
    memset (gMyStatusArea + j, (char) EMPTY, k);    /* Eine Zeile mit Rand  */

    for (i = 1; i < V_STEPS - 4; i ++)
	{
        memcpy (gMyStatusArea + j + (i * H_STEPS),
                gMyStatusArea + j, k);          /* Erste EMPTY-Zeile auf 	*/
                                                /* alle  anderen kopieren   */
	}
}


/* ------------------------------------------------------------------------ */
/* SetPlayerToStatus traegt den Status der uebergebenen Spielfigur (Runner, */
/* Flyer oder Way) an der uebergebenen Position in das Statusfeld ein.      */
/* ------------------------------------------------------------------------ */
   
void SetPlayerToStatus (int xPos, int yPos, unsigned char figur)
{  
    int i, j;
   
    for (i = 0; i < RATIO; i ++)
        for (j = 0; j < RATIO; j ++)
            *(gMyStatusArea + (yPos + i) * H_STEPS + (xPos + j)) |= figur;
   
    return;
}  


/* ------------------------------------------------------------------------ */
/* UnsetPlayerToStatus loescht den Status der uebergebenen Spielfigur		*/
/* (Runner, Flyer oder Way) an der uebergebenen Position aus dem Statusfeld.*/
/* ------------------------------------------------------------------------ */
   
void UnsetPlayerToStatus (int xPos, int yPos, unsigned char figur)
{  
    int i, j;
   
    for (i = 0; i < RATIO; i ++)
        for (j = 0; j < RATIO; j ++)
            *(gMyStatusArea + (yPos + i) * H_STEPS
                            + (xPos + j)) &= ~figur;
   
    return;
}  


/* ------------------------------------------------------------------------ */
/* ChangeToFilled wechselt den Status von TESTFILLED zu FILLED              */
/* ------------------------------------------------------------------------ */

void ChangeToFilled (int xPos, int yPos)
{
    int i, j;

    for (i = 0; i < RATIO; i ++)
        for (j = 0; j < RATIO; j ++)
        {
            *(gMyStatusArea + (yPos + i) * H_STEPS
                            + (xPos + j)) &= ~((unsigned char) TESTFILLED);
            *(gMyStatusArea + (yPos + i) * H_STEPS
                            + (xPos + j)) |= (unsigned char) FILLED;
        }
}


/* ------------------------------------------------------------------------ */
/* SetEaterToStatus traegt einen Eater an der uebergebenen Position in das	*/
/* Statusfeld ein															*/
/* ------------------------------------------------------------------------ */

void SetEaterToStatus (int xPos, int yPos)
{
	*(gMyStatusArea + (yPos * H_STEPS) + xPos) |= (unsigned char) EATER;
}


/* ------------------------------------------------------------------------ */
/* UnsetEaterToStatus loescht einen Eater an der uebergebenen Position aus	*/
/* dem Statusfeld															*/
/* ------------------------------------------------------------------------ */

void UnsetEaterToStatus (int xPos, int yPos)
{
	*(gMyStatusArea + (yPos * H_STEPS) + xPos) &= ~((unsigned char) EATER);
}


/* ------------------------------------------------------------------------ */
/* ClearWay loescht die Spur des Runners									*/
/* ------------------------------------------------------------------------ */

void ClearWay (void)
{
	int i, j;
	
	for (i = 0; i < V_STEPS; i += RATIO)
		for (j = 0; j < H_STEPS; j += RATIO)
		{
			if (*(gMyStatusArea + (i * H_STEPS) + j) == (unsigned char) WAY)
			{
				UnsetPlayerToStatus (j, i, (unsigned char) WAY);
				DrawEmptyToGWorld (j, i);
			}
		}
}


/* ------------------------------------------------------------------------ */
/* FillWay fuellt die Spur des Runners										*/
/* ------------------------------------------------------------------------ */

void FillWay (void)
{
	int i, j;
	
	for (i = 0; i < V_STEPS; i += RATIO)
		for (j = 0; j < H_STEPS; j += RATIO)
		{
			if (*(gMyStatusArea + (i * H_STEPS) + j) == (unsigned char) WAY)
			{
				UnsetPlayerToStatus (j, i, (unsigned char) WAY);
				SetPlayerToStatus (j, i, (unsigned char) FILLED);
				
				DrawFilledToGWorld (j, i);
				gFillCount ++;
			}
		}
}


/* ------------------------------------------------------------------------ */
/* NewFlyer erzeugt einen neuen Flyer an einer zufaelligen Position und     */
/* uebergibt ihn der Spielverwaltung                                        */
/* ------------------------------------------------------------------------ */
    
void NewFlyer (void)
{
    myPoint   flyerStartPoint;

    if (gFlyerCount < MAX_FLYER)
    {
    	do
    	{
        	flyerStartPoint = RandomPosition (); /* Zufaellige Startposition*/
		}
    	while (!(TestFree (flyerStartPoint.h, flyerStartPoint.v)));

        gFlyer[gFlyerCount].x = flyerStartPoint.h;
        gFlyer[gFlyerCount].y = flyerStartPoint.v;
        gFlyer[gFlyerCount].sx = FLYER_SIZE;
        gFlyer[gFlyerCount].sy = FLYER_SIZE;
        gFlyer[gFlyerCount].dx = FLYER_STEP;
        gFlyer[gFlyerCount].dy = FLYER_STEP;
   
        switch (gFlyerCount % 4)            /* Anfangsrichtung festlegen	*/
        {
            case 1: gFlyer[gFlyerCount].dx *= -1;
                    break;
            case 2: gFlyer[gFlyerCount].dy *= -1;
                    break;
            case 3: gFlyer[gFlyerCount].dx *= -1;
                    gFlyer[gFlyerCount].dy *= -1;
                    break;
            default:
                    break;
        }
        SetPlayerToStatus (gFlyer[gFlyerCount].x, gFlyer[gFlyerCount].y,
                            (unsigned char) FLYER);
   
        gFlyerCount ++;                     /* Hallo, wieder einer mehr !!!	*/
    }
   
    return;
}  


/* ------------------------------------------------------------------------ */
/* NewPlayRoom erzeugt eine komplett leere Spielumgebung mit Rand und		*/
/* leerem Innenraum	sowie die dem Level entsprechende Anzahl an Stoeren-	*/
/* frieden																	*/
/* ------------------------------------------------------------------------ */

void NewPlayRoom (void)
{
    int i;

	GWorldEntry ();

	ResetStatus ();

	DrawComplete ();

	ScorePercentage (0);

	ScorePoints (gHighScore);

	ScoreLevel (gLevel);

	for (i = 0, gFlyerCount = 0; i < gLevel + 1; i ++)
	{
		NewFlyer ();
	}

	NewRunner ();

	gFillCount = 0;

	for (i = 0, gEaterCount = 0; i < gLevel / 5 + 1; i ++)
	{
		NewEater ();
	}

	GWorldExit ();
}


/* ------------------------------------------------------------------------ */
/* NewLevel schaltet auf das naechste Level und fordert eine neue Spiel-	*/
/* flaeche an																*/
/* ------------------------------------------------------------------------ */

void NewLevel (void)
{
	gHighScore += (unsigned int)(gLevel * LEVEL_BONUS_FACTOR);

	ScorePoints (gHighScore);

	gLevel ++;

	if (gLevel % 2 == 1)
	{
		gPlayer ++;                               /* Einen Runner als Bonus */
	}

 	NewPlayRoom ();							/* Eine neue Spielwiese muss her*/
    
    return;
}

/* ------------------------------------------------------------------------ */
/* SetRunner setzt die Schrittweite eines Runners; wird beim Tastendruck 	*/
/* gerufen 																	*/
/* ------------------------------------------------------------------------ */

void SetRunner (int x, int y)
{
	gMyRunner.dx = x;
	gMyRunner.dy = y;
}


/* ------------------------------------------------------------------------ */
/* SetOldRect setzt das Refresh-Rect eines Players auf die aktuelle Posi-	*/
/* tion																		*/
/* ------------------------------------------------------------------------ */

void SetOldRect (Player *pl)
{
	pl -> rr.top 	= (pl -> y) * EATER_SIZE;
	pl -> rr.bottom	= (pl -> rr.top) + (pl -> sy);
	pl -> rr.left	= (pl -> x) * EATER_SIZE;
	pl -> rr.right	= (pl -> rr.left) + (pl -> sx);
}


/* ------------------------------------------------------------------------ */
/* SetUnionRect setzt das Refresh-Rect eines Players auf das gemeinsame 	*/
/* Rechteck aus dem bisherigen Refresh-Rect und der aktuellen Position		*/
/* ------------------------------------------------------------------------ */

void SetUnionRect (Player *pl)
{
	int x, y;
	
	x = (pl -> x) * EATER_SIZE;
	y = (pl -> y) * EATER_SIZE;
	
	pl -> rr.top	= (pl -> rr.top) < y ? pl -> rr.top : y;
						
	pl -> rr.bottom	= (pl -> rr.bottom) > (y + (pl -> sy)) ? 
						pl -> rr.bottom : (y + (pl -> sy));
						
	pl -> rr.left	= (pl -> rr.left) < x ? pl -> rr.left : x;
						
	pl -> rr.right	= (pl -> rr.right) > (x + (pl -> sx)) ? 
						pl -> rr.right : (x + (pl -> sx));
						
}


/* ------------------------------------------------------------------------ */
/* FindStartPoints sucht zwei m�gliche Startpunkte fuer das Fuellen			*/
/* ------------------------------------------------------------------------ */

void FindStartPoints (int *x1Start, int *y1Start, int *x2Start, int *y2Start)
{
	if (gMyRunner.dx)						/* horizontale Bewegung			*/
	{
		if (gMyRunner.y >= (2 * RATIO))
		{
			if (*(gMyStatusArea + gMyRunner.x - gMyRunner.dx
							+ ((gMyRunner.y - RATIO) * H_STEPS)) 
				== (unsigned char) EMPTY)
			{
				*x1Start = gMyRunner.x - gMyRunner.dx;
				*y1Start = gMyRunner.y - RATIO;
			}
		}
		
		if (gMyRunner.y < V_STEPS - (2 * RATIO))
		{
			if (*(gMyStatusArea + gMyRunner.x - gMyRunner.dx
							+ ((gMyRunner.y + RATIO) * H_STEPS))
				== (unsigned char) EMPTY)
			{
				*x2Start = gMyRunner.x - gMyRunner.dx;
				*y2Start = gMyRunner.y + RATIO;
			}
		}
	}
	else if (gMyRunner.dy)					/* vertikale Bewegung			*/
	{
		if (gMyRunner.x >= (2 * RATIO))
		{
			if (*(gMyStatusArea + gMyRunner.x - RATIO
							+ ((gMyRunner.y - gMyRunner.dy) * H_STEPS)) 
				== (unsigned char) EMPTY)
			{
				*x1Start = gMyRunner.x - RATIO;
				*y1Start = gMyRunner.y - gMyRunner.dy;
			}
		}
		
		if (gMyRunner.x < H_STEPS - (2 * RATIO))
		{
			if (*(gMyStatusArea + gMyRunner.x + RATIO
							+ ((gMyRunner.y - gMyRunner.dy) * H_STEPS))
				== (unsigned char) EMPTY)
			{
				*x2Start = gMyRunner.x + RATIO;
				*y2Start = gMyRunner.y - gMyRunner.dy;
			}
		}
	}
		
	return;
}


/* ------------------------------------------------------------------------ */
/* FillUp fuellt den Bereich beginnend beim uebergebenen Startpunkt und  	*/
/* liefert TRUE zurueck, wenn kein Flieger im Wege war (sonst FALSE)	 	*/
/* ------------------------------------------------------------------------ */

Boolean	FillUp (int xStart, int yStart)
{
	stack *fillStack;
	myPoint	pt;
	Boolean runFlag;
	unsigned char nb;
	int i, j;
	
	if ((fillStack = InitStack ()) == NIL_POINTER)	/* Ich bekomme nicht,	*/
	{												/* was ich will !!!		*/
		ExitXonix (1);
	}
	
	Push (fillStack, xStart, yStart);		/* Startpunkt in den Stack		*/
	runFlag = TRUE;
	while ((Pop (fillStack, &pt)) && (runFlag == TRUE))
	{
		SetPlayerToStatus (pt.h, pt.v, (unsigned char) TESTFILLED);
													
		if (pt.v >= (2 * RATIO))			/* oberen Nachbar testen		*/
		{
			nb = *(gMyStatusArea + pt.h + (H_STEPS * (pt.v - RATIO)));
			
			if (nb == (unsigned char) EMPTY)
			{
				Push (fillStack, pt.h, pt.v - RATIO);
			}
			else if (nb == (unsigned char) FLYER)
			{
				runFlag = FALSE;
			}
		}
		
		if (pt.v < (V_STEPS - (2 * RATIO)))	/* unteren Nachbar testen	 	*/
		{
			nb = *(gMyStatusArea + pt.h + (H_STEPS * (pt.v + RATIO)));
			
			if (nb == (unsigned char) EMPTY)
			{
				Push (fillStack, pt.h, pt.v + RATIO);
			}
			else if (nb == (unsigned char) FLYER)
			{
				runFlag = FALSE;
			}
		}

		if (pt.h >= (2 * RATIO))			/* linken Nachbar testen		*/
		{
			nb = *(gMyStatusArea + (pt.h - RATIO) + (H_STEPS * pt.v));
			
			if (nb == (unsigned char) EMPTY)
			{
				Push (fillStack, pt.h - RATIO, pt.v);
			}
			else if (nb == (unsigned char) FLYER)
			{
				runFlag = FALSE;
			}
		}
		
		if (pt.h < (H_STEPS - (2 * RATIO)))	/* rechten Nachbar testen	 	*/
		{
			nb = *(gMyStatusArea + (pt.h + RATIO) + (H_STEPS * pt.v));
			
			if (nb == (unsigned char) EMPTY)
			{
				Push (fillStack, pt.h + RATIO, pt.v);
			}
			else if (nb == (unsigned char) FLYER)
			{
				runFlag = FALSE;
			}
		}
	}

    if (runFlag)
    {
        for (i = RATIO; i <= V_STEPS - RATIO; i += RATIO)
            for (j = RATIO; j <= H_STEPS - RATIO; j += RATIO)
            {
                if (*(gMyStatusArea + (i * H_STEPS) + j) ==
                                                (unsigned char) TESTFILLED)
                {
                        ChangeToFilled (j, i);
                        DrawFilledToGWorld (j, i);
                        gFillCount ++;
                }
            }
    }
    else
    {
        for (i = RATIO; i <= V_STEPS - RATIO; i += RATIO)
            for (j = RATIO; j <= H_STEPS - RATIO; j += RATIO)
            {
                UnsetPlayerToStatus (j, i, (unsigned char) TESTFILLED);
            }
    }

	DeinitStack (fillStack);
	
	return runFlag;
}


/* ------------------------------------------------------------------------ */
/* SeedFillUp fuellt einen Bereich segmentweise beginnend beim uebergebenen */
/* Startpunkt und liefert TRUE zurueck, wenn kein Flieger im Wege war 		*/
/* (sonst FALSE)	 														*/
/* ------------------------------------------------------------------------ */

Boolean	SeedFillUp (int xStart, int yStart, Boolean wayToFill)
{
	segStack *fillStack;
	mySegment	sg;
	Boolean runFlag;
	int l;
	unsigned char ov, av=0, nv, wv;
	int i, j, m, n;

	if ((fillStack = InitSegmentStack ()) == NIL_POINTER)
													/* Ich bekomme nicht,	*/
	{												/* was ich will !!!		*/
		ExitXonix(1);
	}
	
	runFlag = TRUE;

	nv = (unsigned char) TESTFILLED;
	wv = (unsigned char) WAY;
	ov = *(gMyStatusArea + yStart * H_STEPS + xStart);

	PushSeg (fillStack, yStart, xStart, xStart, RATIO);
	PushSeg (fillStack, yStart + RATIO, xStart, xStart, -RATIO);

	while ((PopSeg (fillStack, &sg)) && (runFlag == TRUE))
	{
		for (xStart = sg.xl; xStart >= RATIO && 
			(av = *(gMyStatusArea + H_STEPS * sg.y + xStart)) == ov;
			xStart -= RATIO)
		{
			/* SetPlayerToStatus (xStart, sg.y, nv); */
			/* Eigentlich reicht auch eine Markierung ! */

			*(gMyStatusArea + H_STEPS * sg.y + xStart) |= nv;
		}

		if (av == (unsigned char) FLYER)
			runFlag = FALSE;

		if (xStart >= sg.xl)	goto skip;

		l = xStart + RATIO;
		
		if (l < sg.xl) 
			PushSeg (fillStack, sg.y, l, sg.xl - RATIO, -sg.dy);

		xStart = sg.xl + RATIO;

		do 
		{
			for (; xStart <= H_STEPS - RATIO &&
				(av = *(gMyStatusArea + H_STEPS * sg.y + xStart)) == ov;
				xStart += RATIO)
			{
				/* SetPlayerToStatus (xStart, sg.y, nv); */
				/* Eigentlich reicht auch eine Markierung ! */

				*(gMyStatusArea + H_STEPS * sg.y + xStart) |= nv;
			}
			
			if (av == (unsigned char) FLYER)
				runFlag = FALSE;

			PushSeg (fillStack, sg.y, l, xStart - RATIO, sg.dy);

			if (xStart > sg.xr + RATIO)
				PushSeg (fillStack, sg.y, sg.xr + RATIO, xStart - RATIO, 
									-sg.dy);

skip:                  
    		for (xStart += RATIO; xStart <= sg.xr &&
				*(gMyStatusArea + H_STEPS * sg.y + xStart) != ov;
				xStart += RATIO) {}

			l = xStart;
		}
		while (xStart <= sg.xr);
	}

    if ((runFlag) && (!wayToFill))
    {
        for (i = RATIO; i <= V_STEPS - RATIO; i += RATIO)
            for (j = RATIO; j <= H_STEPS - RATIO; j += RATIO)
            {
                if (*(gMyStatusArea + (i * H_STEPS) + j) == nv)
                {
                    /* ChangeToFilled (j, i); */
					*(gMyStatusArea + H_STEPS * i + j) &= ~nv;

					for (m = 0; m < RATIO; m ++)
						for (n = 0; n < RATIO; n ++)
							*(gMyStatusArea + (i + m) * H_STEPS + j + n) 
							|= (unsigned char) FILLED;

                    DrawFilledToGWorld (j, i);
                    gFillCount ++;
                }
            }
    }
    else if ((!runFlag) && (!wayToFill))
    {
        for (i = RATIO; i <= V_STEPS - RATIO; i += RATIO)
            for (j = RATIO; j <= H_STEPS - RATIO; j += RATIO)
            {
                /* UnsetPlayerToStatus (j, i, nv); */
				/* Eigentlich reicht auch eine Markierung ! */

				*(gMyStatusArea + H_STEPS * i + j) &= ~nv;
            }
    }
	else if ((runFlag) && (wayToFill))
	{
        for (i = RATIO; i <= V_STEPS - RATIO; i += RATIO)
            for (j = RATIO; j <= H_STEPS - RATIO; j += RATIO)
            {
                if (*(gMyStatusArea + (i * H_STEPS) + j) & (nv | wv))
                {
					for (m = 0; m < RATIO; m ++)
						for (n = 0; n < RATIO; n ++)
						{
							*(gMyStatusArea + (i + m) * H_STEPS + j + n) 
							&= ~(nv | wv);
							*(gMyStatusArea + (i + m) * H_STEPS + j + n) 
							|= (unsigned char) FILLED;
						}

                    DrawFilledToGWorld (j, i);
                    gFillCount ++;
                }
            }
	} 
	else if ((!runFlag) && (wayToFill))
	{
        for (i = RATIO; i <= V_STEPS - RATIO; i += RATIO)
            for (j = RATIO; j <= H_STEPS - RATIO; j += RATIO)
            {
                if (*(gMyStatusArea + (i * H_STEPS) + j) == wv)
                {
					for (m = 0; m < RATIO; m ++)
						for (n = 0; n < RATIO; n ++)
						{
							*(gMyStatusArea + (i + m) * H_STEPS + j + n) 
							&= ~wv;

							*(gMyStatusArea + (i + m) * H_STEPS + j + n) 
							|= (unsigned char) FILLED;
						}

                    DrawFilledToGWorld (j, i);
                    gFillCount ++;
                }
				else
				{
					for (m = 0; m < RATIO; m ++)
						for (n = 0; n < RATIO; n ++)
						{
							*(gMyStatusArea + (i + m) * H_STEPS + j + n) 
							&= ~nv;
						}
				}

            }
	}

	DeinitSegmentStack (fillStack);
	
	return runFlag;
}


/* ------------------------------------------------------------------------ */
/* NewRunnerPosition berechnet die neue Position der Spielfigur, zeichnet	*/
/* die Schleifspur und gibt im Falle eines Bummses den bouncePartner       	*/
/* zurueck (ansonsten EMPTY)                                             	*/
/* ------------------------------------------------------------------------ */

unsigned char NewRunnerPosition (void)
{
    Boolean     bounced;
    unsigned char bouncePartner;

	GWorldEntry (); 

    bounced = FALSE;

	SetOldRect (&gMyRunner);
   
    if (gMyRunner.dx < 0)                   /* nach links              		*/
    {
        if (gMyRunner.x == 0)
            gMyRunner.dx = 0;
        else
            bounced = VerticalBounceCheck (gMyRunner.x - 1, gMyRunner.y,
                                        RATIO, &bouncePartner)  &&
                      ((AKT_STATUS & (unsigned char) FILLED) !=
                                        (unsigned char) FILLED);
    }
    else if (gMyRunner.dx > 0)              /* nach rechts					*/
    {
        if (gMyRunner.x == H_STEPS - RATIO)
            gMyRunner.dx = 0;
        else
            bounced = VerticalBounceCheck (gMyRunner.x + RATIO, gMyRunner.y,
                                        RATIO, &bouncePartner)  &&
                     ((AKT_STATUS & (unsigned char) FILLED) !=
                                        (unsigned char) FILLED);
    }
    else if (gMyRunner.dy < 0)              /* nach oben                    */
    {
        if (gMyRunner.y == 0)
            gMyRunner.dy = 0;
        else
            bounced = HorizontalBounceCheck (gMyRunner.x, gMyRunner.y - 1,
                                        RATIO, &bouncePartner)  &&
                     ((AKT_STATUS & (unsigned char) FILLED) !=
                                        (unsigned char) FILLED);
    }
    else if (gMyRunner.dy > 0)              /* nach unten                   */
    {
        if (gMyRunner.y == V_STEPS - RATIO)
            gMyRunner.dy = 0;
        else
            bounced = HorizontalBounceCheck (gMyRunner.x, gMyRunner.y + RATIO,
                                        RATIO, &bouncePartner)  &&
                     ((AKT_STATUS & (unsigned char) FILLED) !=
                                        (unsigned char) FILLED);
    }
    
    if (bounced == TRUE)                    /* irgendwas liegt im Weg       */
    {
        switch (bouncePartner)
        {
            case (unsigned char) FILLED:	/* Geschafft					*/

                if ((AKT_STATUS & (WAY + FILLED)) == (unsigned char) EMPTY)
                {
                    UnsetPlayerToStatus (gMyRunner.x, gMyRunner.y,
                                        (unsigned char) RUNNER);
                    SetPlayerToStatus (gMyRunner.x, gMyRunner.y,
                                        (unsigned char) WAY);
                    DrawWayToGWorld (gMyRunner.x, gMyRunner.y);
   
                    gMyRunner.x += gMyRunner.dx;
                    gMyRunner.y += gMyRunner.dy;

                    SetPlayerToStatus (gMyRunner.x, gMyRunner.y,
                                        (unsigned char) RUNNER);
                    DrawRunnerToGWorld ();
                }
				else 
				{
					bouncePartner = (unsigned char) EMPTY;
				}
                break;

            case (unsigned char) WAY:       /* Pech gehabt !                */

                break;

            case (unsigned char) FLYER:     /* Boing !                      */

                break;
        }																	 
    }																		 
    else                                    /* Hinterlasse eine Spur        */
    {
        UnsetPlayerToStatus (gMyRunner.x, gMyRunner.y,
                        (unsigned char) RUNNER);

        if (TestFree (gMyRunner.x, gMyRunner.y))
        {
            SetPlayerToStatus (gMyRunner.x, gMyRunner.y,
                                (unsigned char) WAY);
            DrawWayToGWorld (gMyRunner.x, gMyRunner.y);
        }
        else
        {
            DrawFilledToGWorld (gMyRunner.x, gMyRunner.y);
        }
        gMyRunner.x += gMyRunner.dx;        /* Auf zur naechsten Position   */
        gMyRunner.y += gMyRunner.dy;

        SetPlayerToStatus (gMyRunner.x, gMyRunner.y,
                        (unsigned char) RUNNER);

        DrawRunnerToGWorld ();
        bouncePartner = (unsigned char) EMPTY;
    }
    
    SetUnionRect (&gMyRunner);
    
	GWorldExitRunner();

    return bouncePartner;
}


/* ------------------------------------------------------------------------ */
/* FillNewArea fuellt den abgegrenzten Bereich, in dem keine Flieger herum- */
/* geistern                                                                 */
/* ------------------------------------------------------------------------ */

void FillNewArea (void)
{
	int x1, y1, x2, y2;
	int percent, oldFillCount;
	Boolean	wayToFill;
	
	GWorldEntry ();
	
	FindStartPoints (&x1, &y1, &x2, &y2);	/* Wo geht die Fuellerei los ?	*/
	
	wayToFill = TRUE;

	oldFillCount = gFillCount;

	if ((x1 >= RATIO) && (x1 < H_STEPS - RATIO) &&
		(y1 >= RATIO) && (y1 < V_STEPS - RATIO))
	{
		SeedFillUp (x1, y1, wayToFill);		/* Fuelle erste Region			*/
		wayToFill = FALSE;
	}
	if ((x2 >= RATIO) && (x2 < H_STEPS - RATIO) &&
		(y2 >= RATIO) && (y2 < V_STEPS - RATIO))
	{
		SeedFillUp (x2, y2, wayToFill);		/* Fuelle zweite Region			*/
		wayToFill = FALSE;
	}

	if (wayToFill == TRUE)
		FillWay ();							/* Fuelle wenigstens die Spur	*/

	gMyRunner.dx = 0;						/* Hier wird gestoppt			*/
	gMyRunner.dy = 0;
	
	percent = (gFillCount * 100) /
	  ((H_STEPS / RATIO - 2) * (V_STEPS / RATIO - 2));

	ScorePercentage (percent);

    gHighScore += (unsigned int)((gFillCount - oldFillCount) * gLevel * LOOP_FACTOR / gLoops);

    ScorePoints (gHighScore);
    
    gLoops = 0;

	GWorldExit ();
		
	return;
}

/* ------------------------------------------------------------------------ */
/* NewEaterPosition setzt die Fresser auf die naechste Position und kon-	*/
/* trolliert evtl. moegliche Knabber-Angriffe auf die Spielfigur. Wurde die	*/
/* Spielfigur angeknabbert, wird TRUE zurueckgegeben.                    	*/
/* ------------------------------------------------------------------------ */

Boolean NewEaterPosition (void)
{
	int	i;
	int nextX, nextY;
	Boolean hbFlag, vbFlag, returnFlag;
	unsigned char bouncePartner;

	GWorldEntry ();

	returnFlag = FALSE;
	hbFlag = FALSE;
	vbFlag = FALSE;

	for (i = 0; i < gEaterCount; i ++)
	{
		nextX = gEater[i].x + gEater[i].dx;
		nextY = gEater[i].y + gEater[i].dy;

		if ((nextX >= 0) && (nextX < H_STEPS))	/* Alles bleibt im Rahmen	*/
		{
			if ((bouncePartner = *(gMyStatusArea + gEater[i].y * H_STEPS +
				nextX)) != (unsigned char) FILLED)	/* Bumm !!!				*/
			{
				if ((bouncePartner & (unsigned char) RUNNER) == 
										(unsigned char) RUNNER)	/* Aetsch !	*/
				{
					returnFlag = TRUE;
				}

				hbFlag = TRUE;
				gEater[i].dx *= -1;
			}
		}
		else								/* Hier darf keiner 'raus		*/
		{
			hbFlag = TRUE;
			gEater[i].dx *= -1;				/* Wende !!!					*/
		}

        if ((nextY >= 0) && (nextY < V_STEPS)) 	/* Alles bleibt im Rahmen   */
        {   
            if ((bouncePartner = *(gMyStatusArea + nextY * H_STEPS +
                gEater[i].x)) != (unsigned char) FILLED)  		/* Bumm !   */
            {
				if ((bouncePartner & (unsigned char) RUNNER) == 
										(unsigned char) RUNNER)	/* Aetsch !	*/
                {
                    returnFlag = TRUE;
                }
            
                vbFlag = TRUE;
                gEater[i].dy *= -1;         
            }
        }
        else                                /* Hier darf keiner 'raus       */
        {
            vbFlag = TRUE;
            gEater[i].dy *= -1;             /* Wende !!!                    */
        }

		if ((vbFlag == FALSE) && (hbFlag == FALSE))	/* evtl. nur diagonal	*/
		{
			if ((bouncePartner = *(gMyStatusArea + nextY * H_STEPS +
                nextX)) != (unsigned char) FILLED)        		/* Bumm !   */
			{
				gEater[i].dx *= -1;
				gEater[i].dy *= -1;

				if ((bouncePartner & (unsigned char) RUNNER) == 
										(unsigned char) RUNNER)	/* Aetsch !	*/
				{
					returnFlag = TRUE;
				}
			}
		}
	}

	/* Alle Anstoessigkeiten sind abgeklaert, nun mal los !					*/


	for (i = 0; i < gEaterCount; i ++)
	{
		SetOldRect (&(gEater[i]));

		DrawSmallFilledToGWorld (gEater[i].x, gEater[i].y);

		UnsetEaterToStatus (gEater[i].x, gEater[i].y);

		gEater[i].x += gEater[i].dx;
		gEater[i].y += gEater[i].dy;

		SetEaterToStatus (gEater[i].x, gEater[i].y);

		DrawEaterToGWorld (gEater[i].x, gEater[i].y);

		SetUnionRect (&(gEater[i]));
	}

	GWorldExitEater ();

    return returnFlag;
}


/* ------------------------------------------------------------------------ */
/* CheckPercentage liefert TRUE, wenn der Schwellwert des Fuellstands		*/
/* ueberschritten wird														*/
/* ------------------------------------------------------------------------ */

Boolean CheckPercentage (void)
{
    int percent;

	percent = (gFillCount * 100) /
	  ((H_STEPS / RATIO - 2) * (V_STEPS / RATIO - 2));

	if (percent >= PERCENT_THRESHOLD)
		return TRUE;
	else
		return FALSE;
}


/* ------------------------------------------------------------------------ */
/* GetNewPlayer setzt die Spielfigur wieder an den Ausgangspunkt zurueck	*/
/* und kassiert dafuer ein Leben                                            */
/* ------------------------------------------------------------------------ */

void GetNewPlayer (void)
{
	GWorldEntry();
	
	UnsetPlayerToStatus (gMyRunner.x, gMyRunner.y, (unsigned char) RUNNER);

	if (TestFree (gMyRunner.x, gMyRunner.y))
		DrawEmptyToGWorld (gMyRunner.x, gMyRunner.y);
	else
		DrawFilledToGWorld (gMyRunner.x, gMyRunner.y);

	DrawCompleteBorder ();
	
	ClearWay ();

	if (!(-- gPlayer))
	{
		gRun = FALSE;
		gQuit = TRUE;
		gEndOfGame = TRUE;
		DisplayHighScore();
	}
	else
	{
		NewRunner ();
    
		BELL ();
	}

    GWorldExit();

    return;
}

/* ------------------------------------------------------------------------ */
/* HorizontalBounceCheck testet ab der uebergebenen Position ueber die 		*/
/* Breite von size, ob der Bereich unbelegt (EMPTY) ist. Andernfalls wird 	*/
/* TRUE zurueckgegeben, um zu signalisieren, dass es gebummst hat.          */
/* ------------------------------------------------------------------------ */

Boolean HorizontalBounceCheck (int x, int y, int size,
							   unsigned char *bouncePartner)
{
    int i;
    
    for (i = 0; i < size; i ++)
    {
        if ((*bouncePartner = *(gMyStatusArea + (y *   H_STEPS) + (i + x))) 
            != (char) EMPTY)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}

/* ------------------------------------------------------------------------ */
/* VerticalBounceCheck testet ab der uebergebenen Position ueber die Hoehe  */
/* von size, ob der Bereich unbelegt (EMPTY) ist. Andernfalls wird TRUE 	*/
/* zurueckgegeben, um zu signalisieren, dass es gebummst hat.               */
/* ------------------------------------------------------------------------ */

Boolean VerticalBounceCheck (int x, int y, int size, 
							unsigned char *bouncePartner)
{
    int i;
    
    for (i = 0; i < size; i ++)
    {
        if ((*bouncePartner = *(gMyStatusArea + ((i + y)   * H_STEPS) + x)) 
            != (char) EMPTY)
        {
            return TRUE;
        }
    }
    
    return FALSE;
}


/* ------------------------------------------------------------------------ */
/* NewFlyerPosition berechnet die naechste Position der Flieger und zeich-	*/
/* net sie entsprechend.                                                  	*/
/* ------------------------------------------------------------------------ */
    
Boolean NewFlyerPosition (void)
{
    int i;
    unsigned char   bouncePartner;
    Boolean     returnFlag, vbFlag, hbFlag;

	GWorldEntry ();

    returnFlag = FALSE;
    vbFlag = FALSE;
    hbFlag = FALSE;

    
    for (i = 0; i < gFlyerCount; i ++)	
    {
		vbFlag = VerticalBounceCheck (gFlyer[i].x + gFlyer[i].dx, gFlyer[i].y, 
                							RATIO, &bouncePartner);
		if (vbFlag == TRUE)
		{
			gFlyer[i].dx *= -1;

            if ((bouncePartner == ((unsigned char) WAY)) ||
                (bouncePartner == ((unsigned char) RUNNER)))
                returnFlag = TRUE;

			if (HorizontalBounceCheck (gFlyer[i].x + gFlyer[i].dx,
										gFlyer[i].y + gFlyer[i].dy, 
            							1, &bouncePartner) == TRUE)
				gFlyer[i].dy *= -1;	

            if ((bouncePartner == ((unsigned char) WAY)) ||
                (bouncePartner == ((unsigned char) RUNNER)))
                returnFlag = TRUE;
		}

		hbFlag = HorizontalBounceCheck (gFlyer[i].x,
										gFlyer[i].y + gFlyer[i].dy,
										RATIO, &bouncePartner);
		if (hbFlag == TRUE)
		{
			gFlyer[i].dy *= -1;

            if ((bouncePartner == ((unsigned char) WAY)) ||
                (bouncePartner == ((unsigned char) RUNNER)))
                returnFlag = TRUE;

			if (HorizontalBounceCheck (gFlyer[i].x + gFlyer[i].dx,
										gFlyer[i].y + gFlyer[i].dy, 
            							1, &bouncePartner) == TRUE)
				gFlyer[i].dx *= -1;	

            if ((bouncePartner == ((unsigned char) WAY)) ||
                (bouncePartner == ((unsigned char) RUNNER)))
                returnFlag = TRUE;
		}
            
		if ((vbFlag == FALSE) && (hbFlag == FALSE))	/* evtl. nur diagonal ? */
		{
			if (HorizontalBounceCheck (gFlyer[i].x + gFlyer[i].dx,
										gFlyer[i].y + gFlyer[i].dy, 
            							1, &bouncePartner) == TRUE)
			{
				gFlyer[i].dx *= -1;
				gFlyer[i].dy *= -1;

	            if ((bouncePartner == ((unsigned char) WAY)) ||
   	            	(bouncePartner == ((unsigned char) RUNNER)))
                	returnFlag = TRUE;

			}
		}            
	}    
    
    /* Anstoessiges ist geklaert, jetzt geht es zur Sache */
    
    for (i = 0; (i < gFlyerCount) && (returnFlag == FALSE); i ++)
    {
    	SetOldRect (&(gFlyer[i]));
    	
        DrawEmptyToGWorld (gFlyer[i].x, gFlyer[i].y);

        UnsetPlayerToStatus (gFlyer[i].x, gFlyer[i].y, (unsigned char) FLYER);

        gFlyer[i].x += gFlyer[i].dx;
        gFlyer[i].y += gFlyer[i].dy;
    
        SetPlayerToStatus (gFlyer[i].x, gFlyer[i].y, (unsigned char) FLYER);
                
        DrawFlyerToGWorld (gFlyer[i].x, gFlyer[i].y);
        
        SetUnionRect (&(gFlyer[i]));
    }
    
    GWorldExitFlyer ();
	
    return returnFlag;
}


/* ------------------------------------------------------------------------ */
/* Animate laesst alle Figuren auf ihre neuen Positionen wechseln und auf   */
/* evtl. Kollisionen bzw. Fuellerfordernisse ueberpruefen                   */
/* ------------------------------------------------------------------------ */

void Animate (void) 
{
    switch (NewRunnerPosition ())
    {   
		case (unsigned char) FILLED:		/* Abgrenzung geschaffen		*/

	        FillNewArea ();
        
        	if (CheckPercentage () == TRUE)
			{
		    	NewLevel ();				/* Schwelle Fuellstand erreicht	*/
			}

			break;

		case (unsigned char) WAY:			/* Eigene Spur erwischt			*/

			GetNewPlayer ();

			break;

		default:

    		if (NewEaterPosition () == TRUE)/* Spielfigur von Fresser "ange-*/
    		{                               /* knabbert"                    */
        		GetNewPlayer ();
    		}
    		else if (NewFlyerPosition () == TRUE) /* Spur der Spielfigur von*/
    		{                               /* Flieger getroffen            */
        		GetNewPlayer ();
			}
			break;
	}

	if(gRun && !gPause)
	{
		gLoops ++;
	}
		 
}  


