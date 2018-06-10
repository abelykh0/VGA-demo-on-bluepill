/* Hey emacs, this is -*- Mode: C++; tab-width: 4 -*-  */
/*
 * simple Point stack
 *
 * stack.h,v 1.6 1995/08/28 10:57:59 j Exp
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

#ifndef STACK_H
#define STACK_H

typedef struct {
	myPoint *bottom;
	int size;
	int counter;
	} stack;

typedef struct {
	mySegment *bottom;
	int size;
	int counter;
	} segStack;

/* ------------------------------------------------------------------------ */
/* Funktionen								     							*/
/* ------------------------------------------------------------------------ */


extern stack *InitStack ();
extern int DeinitStack (stack *s);
extern int Push (stack *s, int x, int y);
extern int Pop (stack *s, myPoint *p);

extern segStack *InitSegmentStack ();
extern int DeinitSegmentStack (segStack *s);
extern int PushSeg (segStack *s, int y, int xl, int xr, int dy);
extern int PopSeg (segStack *s, mySegment *p);

#endif /* STACK_H */
