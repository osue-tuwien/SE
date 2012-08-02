/*******************************************************************************
*                                                                              *
*     Author:      A. Vrchoticky                                               *
*                  Technische Universitaet Wien                                *
*                  Institut fuer Technische Informatik E182/1                  *
*                  Treitlstrasse 3                                             *
*                  1040 Wien                                                   *
*                  Tel.: (0222) 58801 / 8168                                   *
*                  E-Mail: alex@vmars.tuwien.ac.at                             *
*     File:        seqev.h						       *
*     Version:     4.12							       *
*     Date:        9/7/94						       *
*                                                                              *
*******************************************************************************/

#ifndef SEQEV_H
#define SEQEV_H

#include <sys/types.h>
#include <sequencer.h>
#include <eventcounter.h>

typedef Sequencer_t    sequencer_t;
typedef Eventcounter_t eventcounter_t;


extern sequencer_t *create_sequencer (key_t key);
extern long         sticket       (sequencer_t *sequencer);
extern int    		rm_sequencer   (sequencer_t *sequencer); 

extern eventcounter_t *create_eventcounter (key_t key);
extern int  eawait   (eventcounter_t *eventcounter, long value);
extern long eread    (eventcounter_t *eventcounter);
extern int  eadvance (eventcounter_t *eventcounter);
extern int rm_eventcounter (eventcounter_t *eventcounter); 

#endif /* SEQEV_H */
