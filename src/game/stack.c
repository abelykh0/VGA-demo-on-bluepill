/* Hey emacs, this is -*- Mode: C++; tab-width: 4 -*-  */
/*
 * simple Point stack
 *
 * stack.c,v 1.6 1995/09/08 09:51:28 j Exp
 */

/*
 * Copyright (c) 1995
 *   Alfredo Herrera Hernandez <alf@narcisa.sax.de>
 *   Torsten Schoenitz <torsten_schoenitz@bonnie.heep.sax.de>
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

#include <stdlib.h>
#include "xonix.h"
#include "stack.h"

#define INIT_SIZE (32)

/* ------------------------------------------------------------------------ */
/* InitMyPoints legt einen Speicherbereich mit der gewuenschten Groesse an	*/
/* ------------------------------------------------------------------------ */

myPoint *InitMyPoints (int size)
{
	return ((myPoint *) malloc (size * sizeof (myPoint)));
}


/* ------------------------------------------------------------------------ */
/* InitMySegments legt einen Speicherbereich mit der gewuenschten Groesse   */
/* an																		*/
/* ------------------------------------------------------------------------ */

mySegment *InitMySegments (int size)
{
	return ((mySegment *) malloc (size * sizeof (mySegment)));
}


/* ------------------------------------------------------------------------ */
/* InitStack initialisiert die Stackverwaltung fuer myPoints			 	*/
/* ------------------------------------------------------------------------ */

stack *InitStack (void)
{
	stack *s;

	if ((s = (stack *) malloc (sizeof (stack))) == NULL) return NULL;

	if ((s -> bottom = InitMyPoints (INIT_SIZE)) == NULL) return NULL;

	s -> size = INIT_SIZE;
	s -> counter = 0;

	return s;
}


/* ------------------------------------------------------------------------ */
/* InitSegmentStack initialisiert die Stackverwaltung fuer mySegments	 	*/
/* ------------------------------------------------------------------------ */

segStack *InitSegmentStack (void)
{
	segStack *s;

	if ((s = (segStack *) malloc (sizeof (segStack))) == NULL) return NULL;

	if ((s -> bottom = InitMySegments (INIT_SIZE)) == NULL) return NULL;

	s -> size = INIT_SIZE;
	s -> counter = 0;

	return s;
}


/* ------------------------------------------------------------------------ */
/* DeinitMyPoints gibt die Stackelemente wieder frei						*/
/* ------------------------------------------------------------------------ */

int DeinitMyPoints (myPoint *p)
{
	free (p);

	return 1;
}


/* ------------------------------------------------------------------------ */
/* DeinitMySegments gibt die Stackelemente wieder frei						*/
/* ------------------------------------------------------------------------ */

int DeinitMySegments (mySegment *p)
{
	free (p);

	return 1;
}


/* ------------------------------------------------------------------------ */
/* DeinitStack gibt den Stack wieder frei									*/
/* ------------------------------------------------------------------------ */

int DeinitStack (stack *s)
{
	
	if (s == NULL) return 0;

	DeinitMyPoints (s -> bottom);
	
	free (s);

	return 1;
}


/* ------------------------------------------------------------------------ */
/* DeinitSegmentStack gibt den Stack wieder frei                            */
/* ------------------------------------------------------------------------ */
    
int DeinitSegmentStack (segStack *s)          
{
    
    if (s == NULL) return 0;

    DeinitMySegments (s -> bottom);
   
    free (s);

    return 1;
}


/* ------------------------------------------------------------------------ */
/* ReallocMyPoints fordert Speicher fuer size Stackelemente an				*/
/* ------------------------------------------------------------------------ */

myPoint *ReallocMyPoints (myPoint *p, int size)
{
	return ((myPoint *) realloc (p, size * sizeof (myPoint)));
}


/* ------------------------------------------------------------------------ */
/* ReallocMySegments fordert Speicher fuer size Stackelemente an              */
/* ------------------------------------------------------------------------ */

mySegment *ReallocMySegments (mySegment *p, int size)
{
    return ((mySegment *) realloc (p, size * sizeof (mySegment)));
}


/* ------------------------------------------------------------------------ */
/* Push packt ein Element auf den Stack										*/
/* ------------------------------------------------------------------------ */

int Push (stack *s, int x, int y)
{
	if (s -> size == s -> counter)
	{
		if ((s -> bottom = ReallocMyPoints (s -> bottom, 
											s -> size + 0x1000)) == NULL)
			return 0;
			
		s -> size += 0x1000;
	}
	
	(s -> bottom + s -> counter) -> h = x;
	(s -> bottom + s -> counter) -> v = y;
	s -> counter ++;
	
	return 1;
}


/* ------------------------------------------------------------------------ */
/* PushSeg packt ein Segment auf den Stack                                  */
/* ------------------------------------------------------------------------ */

int PushSeg (segStack *s, int y, int xl, int xr, int dy)
{
    if (s -> size == s -> counter)
    {
        if ((s -> bottom = ReallocMySegments (s -> bottom,
                                            s -> size + 0x1000)) == NULL)
            return 0;

        s -> size += 0x1000;
    }
    
	if ((y + dy >= RATIO) && (y + dy <= V_STEPS - RATIO))
	{
    	(s -> bottom + s -> counter) -> y = y;
    	(s -> bottom + s -> counter) -> xl = xl;
    	(s -> bottom + s -> counter) -> xr = xr;
    	(s -> bottom + s -> counter) -> dy = dy;
    	s -> counter ++;
	}
   
    return 1;
}


/* ------------------------------------------------------------------------ */
/* Pop liefert das oberste Element aus dem Stack							*/
/* ------------------------------------------------------------------------ */

int Pop (stack *s, myPoint *p)
{
	if (s -> counter) s -> counter --;
	else return 0;
	
	if (s -> size > (s -> counter + 0x1000))
	{
		if ((s -> bottom = ReallocMyPoints (s -> bottom, 
											s -> size - 0x1000)) == NULL)
		return 0;
		
		s -> size -= 0x1000;
	}

	*p = *(s -> bottom + s -> counter);

	return 1;
}


/* ------------------------------------------------------------------------ */
/* PopSeg liefert das oberste Segment aus dem Stack                         */
/* ------------------------------------------------------------------------ */

int PopSeg (segStack *s, mySegment *p)
{
    if (s -> counter) s -> counter --;
    else return 0;

    if (s -> size > (s -> counter + 0x1000))
    {
        if ((s -> bottom = ReallocMySegments (s -> bottom,
                                            s -> size - 0x1000)) == NULL)
        return 0;

        s -> size -= 0x1000;
    }
    
	(s -> bottom + s -> counter) -> y += (s -> bottom + s -> counter) -> dy;

    *p = *(s -> bottom + s -> counter);

    return 1;
}

