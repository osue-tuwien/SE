/*******************************************************************************
*                                                                              *
*     Author:      G. Leber                                                    *
*                  Technische Universitaet Wien                                *
*                  Institut fuer Technische Informatik E182/1                  *
*                  Treitlstrasse 3                                             *
*                  1040 Wien                                                   *
*                  Tel.: (0222) 58801 / 8176                                   *
*                  E-Mail: guenther@vmars.tuwien.ac.at                         *
*     File:        sequencer.c						       *
*     Version:     4.12							       *
*     Date:        9/7/94						       *
*                                                                              *
*******************************************************************************/

#ifndef lint
static char	sequencer_c_sccsID[] = "@(#)sequencer.c	4.12	9/7/94";
#endif /* lint */


#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <assert.h>


#include <errno.h>

#include "sequencer.h"


#define Fprintf		(void) fprintf
#define DEBUGSEQUENCER	"DEBUGSEQUENCER"
#define DEBUGDEFAULTLVL	1

#define SEQUENCERMAGIC	0x56789abcL

#define check(S) assert(SEQUENCERMAGIC + S.semid + S.shmid + \
			(long) S.shmaddr == S.checksum)


#if __STDC__

#if _mips
char	*shmat(int, char*, int);
int	shmget(key_t, u_int, u_int);
#endif

#endif


/* this variable is used for debugging */
static	int	Debug_sequencer	= 0;	/* debugginglevel zero initially */


/*******************************************************************************
*				initSequencer				       *
*******************************************************************************/

#if __STDC__

int	initSequencer(key_t key, Sequencer_t *Sequencer)

#else

int	initSequencer(key, Sequencer)

key_t	key;
Sequencer_t	*Sequencer;

#endif

{
 /* initialize the Sequencer */

 int	initshm,
	semid,
	shmid;
 long	*p_value,
	dval;
 char	*evarptr,	/* pointer to the value of a environment variable */
	*eptr;


 /* some initializations for debugging */
 if ( (evarptr = getenv(DEBUGSEQUENCER)) != NULL) {
	/* analyse val of ENVVAR */
	if (*evarptr != '\0') {
		/* try to convert the value */
		dval = strtol(evarptr, &eptr, 0);
		if ( (eptr != evarptr) && (*eptr == '\0') )
			Debug_sequencer = dval;		/* set debug level */
		else {
			Debug_sequencer = DEBUGDEFAULTLVL;
			Fprintf(stderr, DEBUGSEQUENCER " garbled: %s\n"
					"Using default level\n", evarptr);
		}
	} else	Debug_sequencer = DEBUGDEFAULTLVL;	/* VAR= */
 }

 /* initialize resources for Sequencer */

 /* get semaphore */
 semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0600);
 if (semid < 0) {
	if (errno != EEXIST) {
		if (Debug_sequencer & 1) {
			perror("semget(...,IPC_CREAT | IPC_EXCL...)");
			Fprintf(stderr, "Cannot create semaphore for Sequencer\n");
		}
		if (Debug_sequencer & 2) {
			abort();
		}
		return -1;
	}
	/* semaphor already exist; get a descriptor for it */
	semid = semget(key, 1, 0600);
	if (semid < 0) {
		if (Debug_sequencer & 1) {
			perror("semget");
			Fprintf(stderr, "Cannot create semaphore for Sequencer\n");
		}
		if (Debug_sequencer & 2) {
			abort();
		}
		return -1;
	}
 } else {
	if (semctl(semid, 0, SETVAL, 0) < 0) { /* block sequencer ! */
		if (Debug_sequencer & 1) {
			perror("semctl(SETVAL)");
			Fprintf(stderr, "Cannot initialize semaphore for Sequencer\n");
		}
		if (Debug_sequencer & 2) {
			abort();
		}
		if (semctl(semid, 1, IPC_RMID, 0) < 0) {
			if (Debug_sequencer & 1) {
				perror("semctl(IPC_RMID)");
				Fprintf(stderr, "Panic: Cannot remove Semaphore for Sequencer\n");
			}
			if (Debug_sequencer & 2) {
				abort();
			}
		}
		return -1;
	}
 }
 /* get shared memory */
 shmid = shmget(key, (int) sizeof(long), IPC_CREAT | IPC_EXCL | 0600);
 if (shmid < 0) {
	if (errno != EEXIST) {
		if (Debug_sequencer & 1) {
			perror("shmget(...,IPC_CREAT | IPC_EXCL...)");
			Fprintf(stderr, "Cannot create shared memory for Sequencer\n");
		}
		if (semctl(semid, 1, IPC_RMID, 0) < 0) {
			if (Debug_sequencer & 1) {
				perror("semctl(IPC_RMID)");
				Fprintf(stderr, "Panic: Cannot remove Semaphore for Sequencer\n");
			}
		}
		if (Debug_sequencer & 2) {
			abort();
		}
		return -1;
	}
	/* shm already exist; get a descriptor */
	initshm = 0;
	shmid = shmget(key, (int) sizeof(long), 0600);
	if (shmid < 0) {
		if (Debug_sequencer & 1) {
			perror("shmget");
			Fprintf(stderr, "Cannot create shared memory for Sequencer\n");
		}
		if (semctl(semid, 1, IPC_RMID, 0) < 0) {
			if (Debug_sequencer & 1) {
				perror("semctl(IPC_RMID)");
				Fprintf(stderr, "Panic: Cannot remove Semaphore for Sequencer\n");
			}
		}
		if (Debug_sequencer & 2) {
			abort();
		}
		return -1;
	}
 } else {
	initshm = 1;
 }
 /* attach shared memory */
 p_value = (long *) shmat(shmid, (char *) 0, 0);
 if (p_value == (long *) -1) {
	if (Debug_sequencer & 1) {
		perror("shmat");
	}
	if (semctl(semid, 1, IPC_RMID, 0) < 0) {
		if (Debug_sequencer & 1) {
			perror("semctl(IPC_RMID)");
			Fprintf(stderr, "Panic: Cannot remove Semaphore for Sequencer\n");
		}
	}
	if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
		if (Debug_sequencer & 1) {
			perror("shmctl(IPC_RMID)");
			Fprintf(stderr, "Panic: Cannot remove shared memory for Sequencer\n");
		}
	}
	if (Debug_sequencer & 2) {
		abort();
	}
	return -1;
 }
 /* initialize shared memory */
 if (initshm == 1) {
	struct sembuf	semp[1];


	*p_value = 0;

	/* sequencer is now ready!! release it!!!! */
	semp[0].sem_num = 0;
	semp[0].sem_op = 1;
	semp[0].sem_flg = 0;
	if (semop(semid, &semp[0], 1) < 0) {
		if (Debug_sequencer & 1) {
			perror("semop");
			Fprintf(stderr, "Cannot do V() on semaphore for Sequencer\n");
		}
		if (Debug_sequencer & 2) {
			abort();
		}
		return -1;
	}
 }

 /* return Sequencer */
 Sequencer->semid = semid;
 Sequencer->shmid = shmid;
 Sequencer->shmaddr = p_value;
 Sequencer->checksum = SEQUENCERMAGIC + semid + shmid + (long) p_value;
 return 0;
}


/*******************************************************************************
*				rmSequencer				       *
*******************************************************************************/

#if __STDC__

int	rmSequencer(Sequencer_t Sequencer)

#else

int	rmSequencer(Sequencer)

Sequencer_t	Sequencer;

#endif

{
 /* remove the Sequencer */

 int error = 0;


 check(Sequencer);

 if (semctl(Sequencer.semid, 1, IPC_RMID, 0) < 0) {
	error = -1;
 }
 if (shmdt((char *) Sequencer.shmaddr) < 0) {
	error = -1;
 }
 if (shmctl(Sequencer.shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
	error = -1;
 }

 if ( (error < 0) && (Debug_sequencer & 2) ) {
	abort();
 }

 return error;
}


/*******************************************************************************
*				Sticket					       *
*******************************************************************************/

#if __STDC__

long	Sticket(Sequencer_t Sequencer)

#else

long	Sticket(Sequencer)

Sequencer_t	Sequencer;

#endif

{
 /* return a ticket */

 struct sembuf	semp[1];
 long	svalue;
 int	error = 0;


 check(Sequencer);

 /* wait for critical section */
 semp[0].sem_num = 0;
 semp[0].sem_op = -1;
 semp[0].sem_flg = SEM_UNDO;
 if (semop(Sequencer.semid, &semp[0], 1) < 0) {
	if (Debug_sequencer & 1) {
		perror("semop");
		Fprintf(stderr, "Cannot do P() on semaphore for Sequencer\n");
	}
	if (Debug_sequencer & 2) {
		abort();
	}
	return -1;
 }

 /* modify value of Sequencer */
 svalue = (Sequencer.shmaddr[0])++;

 /* check for overflow */
 if (Sequencer.shmaddr[0] < 0) {
	errno = ERANGE;
	error = -1;
 }

 /* signal critical section */
 semp[0].sem_num = 0;
 semp[0].sem_op = 1;
 semp[0].sem_flg = SEM_UNDO;
 if (semop(Sequencer.semid, &semp[0], 1) < 0) {
	if (Debug_sequencer & 1) {
		perror("semop");
		Fprintf(stderr, "Cannot do V() on semaphore for Sequencer\n");
	}
	if (Debug_sequencer & 2) {
		abort();
	}
	return -1;
 }

 if ( (error < 0) && (Debug_sequencer & 2) ) {
	abort();
 }

 /* return error in case of error, else return value of sequencer */
 return (error != 0) ? error : svalue;
}
