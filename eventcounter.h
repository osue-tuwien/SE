/*******************************************************************************
*                                                                              *
*     Author:      G. Leber                                                    *
*                  Technische Universitaet Wien                                *
*                  Institut fuer Technische Informatik E182/1                  *
*                  Treitlstrasse 3                                             *
*                  1040 Wien                                                   *
*                  Tel.: (0222) 58801 / 8176                                   *
*                  E-Mail: guenther@vmars.tuwien.ac.at                         *
*     File:        eventcounter.h					       *
*     Version:     4.12							       *
*     Date:        9/7/94						       *
*                                                                              *
*******************************************************************************/

#ifndef EVENTCOUNTER_H
#define EVENTCOUNTER_H

/* Structure describing an eventcounter */

typedef struct {
	int	semid,
		shmid;
	long	*shmaddr,
		checksum;
} Eventcounter_t;


#if defined(__STDC__) || defined(__cplusplus)
#	define P_(s) s
#else
#	define P_(s) ()
#endif

/* eventcounter.c */
int initEventcounter P_((key_t key, Eventcounter_t *Eventcounter));
int rmEventcounter P_((Eventcounter_t Eventcounter));
long Eread P_((Eventcounter_t Eventcounter));
int Eawait P_((Eventcounter_t Eventcounter, long v));
int Eadvance P_((Eventcounter_t Eventcounter));

#undef P_

#endif /* EVENTCOUNTER_H */
