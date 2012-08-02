/*******************************************************************************
*                                                                              *
*     Author:      G. Leber                                                    *
*                  Technische Universitaet Wien                                *
*                  Institut fuer Technische Informatik E182/1                  *
*                  Treitlstrasse 3                                             *
*                  1040 Wien                                                   *
*                  Tel.: (0222) 58801 / 8176                                   *
*                  E-Mail: guenther@vmars.tuwien.ac.at                         *
*     File:        sequencer.h						       *
*     Version:     4.12							       *
*     Date:        9/7/94						       *
*                                                                              *
*******************************************************************************/

#ifndef SEQUENCER_H
#define SEQUENCER_H

/* Structure to describe a Sequencer */

typedef struct {
	int	semid,
		shmid;
	long	*shmaddr,
		checksum;
} Sequencer_t;


#if defined(__STDC__) || defined(__cplusplus)
#	define P_(s) s
#else
#	define P_(s) ()
#endif

/* sequencer.c */
int initSequencer P_((key_t key, Sequencer_t *Sequencer));
int rmSequencer P_((Sequencer_t Sequencer));
long Sticket P_((Sequencer_t Sequencer));

#undef P_

#endif /* SEQUENCER_H */
