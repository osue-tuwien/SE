/*******************************************************************************
*                                                                              *
*     Author:      G. Leber                                                    *
*                  Technische Universitaet Wien                                *
*                  Institut fuer Technische Informatik E182/1                  *
*                  Treitlstrasse 3                                             *
*                  1040 Wien                                                   *
*                  Tel.: (0222) 58801 / 8176                                   *
*                  E-Mail: guenther@vmars.tuwien.ac.at                         *
*     File:        eventcounter.c					       *
*     Version:     4.12							       *
*     Date:        9/7/94						       *
*                                                                              *
*******************************************************************************/

#ifndef lint
static char	eventcounter_c_sccsID[] = "@(#)eventcounter.c	4.12	9/7/94";
#endif /* lint */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>


#include "eventcounter.h"

#if !defined(SEMMSL)
#	define SEMMSL	25
#endif

#if !defined(SEMOPM)
#	define SEMOPM	10
#endif


#define	await	(Eventcounter.shmaddr)
#define value	(Eventcounter.shmaddr[0])

#define Fprintf	(void) fprintf
#define DEBUGEVENTCOUNTER	"DEBUGEVENTCOUNTER"
#define DEBUGDEFAULTLVL	1

#define EVENTCOUNTERMAGIC	0xfedc4321L

#define check(E) assert(EVENTCOUNTERMAGIC + E.semid + E.shmid + \
				(long) E.shmaddr == E.checksum)


/* this variable is used for debugging */
static int	Debug_eventcounter = 0;	/* debugginglevel zero initially */



#if __STDC__
#if _mips
char	*shmat(int, char *, int);
int	shmget(key_t, u_int, u_int);
#endif
#endif

/*******************************************************************************
*				initEventcounter			       *
*******************************************************************************/

#if __STDC__
int	initEventcounter(key_t key, Eventcounter_t *Eventcounter)
#else
int	initEventcounter(key, Eventcounter)

key_t	key;
Eventcounter_t	*Eventcounter;
#endif
{
 /* This function initializes an Eventcounter */

 int	initshm,
	semid,
	shmid,
	i;
 ushort	semval[SEMMSL];
 long	*p_awaits,
	dval;
 char	*evarptr,	/* pointer to the value of a environment variable */
	*eptr;

 /* some initializations for debugging */
 if ( (evarptr = getenv(DEBUGEVENTCOUNTER)) != NULL) {
	/* analyse val of ENVVAR */
	if (*evarptr != '\0') {
		/* try to convert the value */
		dval = strtol(evarptr, &eptr, 0);
		if ( (eptr != evarptr) && (*eptr == '\0') )
			Debug_eventcounter = dval;	/* set debug level */
		else {
			Debug_eventcounter = DEBUGDEFAULTLVL;
			Fprintf(stderr, DEBUGEVENTCOUNTER " garbled: %s\n"
					"Using default level\n", evarptr);
		}
	} else	Debug_eventcounter = DEBUGDEFAULTLVL;	/* VAR= */
 }

 /* initialize values for semaphores */
 for (i = 0; i < SEMMSL; i++)	semval[i] = 0;

 /* initialize resources for Eventcounter */

 /* get semaphore */
 semid = semget(key, SEMMSL, IPC_CREAT | IPC_EXCL | 0600);
 if (semid < 0) {
	if (errno != EEXIST) {
		if (Debug_eventcounter & 1) {
			perror("semget(..., IPC_CREAT | IPC_EXCL ...)");
			Fprintf(stderr, "Cannot create semaphore for Eventcounter\n");
		}
		if (Debug_eventcounter & 2) {
			abort();
		}
		return -1;
	}
	/* semaphore already exists; get a descriptor */
	semid = semget(key, SEMMSL, 0600);
	if (semid < 0) {
		if (Debug_eventcounter & 1) {
			perror("semget");
			Fprintf(stderr, "Cannot create semaphore for Eventcounter\n");
		}
		if (Debug_eventcounter & 2) {
			abort();
		}
		return -1;
	}
 } else {
	if (semctl(semid, SEMMSL, SETALL, semval) < 0) {
		if (Debug_eventcounter & 1) {
			perror("semctl(SETALL)");
			Fprintf(stderr, "Cannot initialize semaphore for Eventcounter\n");
		}
		if (semctl(semid, SEMMSL, IPC_RMID, 0) < 0) {
			if (Debug_eventcounter & 1) {
				perror("semctl(IPC_RMID)");
				Fprintf(stderr, "Panic: Cannot remove Semaphore for Eventcounter\n");
			}
		}
		if (Debug_eventcounter & 2) {
			abort();
		}
		return -1;
	}
 }
 /* get shared memory */
 shmid = shmget(key, (int) sizeof(long) * SEMMSL, IPC_CREAT | IPC_EXCL | 0600);
 if (shmid < 0) {
	if (errno != EEXIST) {
		if (Debug_eventcounter & 1) {
			perror("shmget(..., IPC_CREAT | IPC_EXCL...)");
			Fprintf(stderr, "Cannot create shared memory for Eventcounter\n");
		}
		if (semctl(semid, SEMMSL, IPC_RMID, 0) < 0) {
			if (Debug_eventcounter & 1) {
				perror("semctl(IPC_RMID)");
				Fprintf(stderr, "Panic: Cannot remove Semaphore for Eventcounter\n");
			}
		}
		if (Debug_eventcounter & 2) {
			abort();
		}
		return -1;
	}
	/* shm already exists; get a descriptor */
	initshm = 0;
	shmid = shmget(key, (int) sizeof(long) * SEMMSL, 0600);
	if (shmid < 0) {
		if (Debug_eventcounter & 1) {
			perror("shmget");
			Fprintf(stderr, "Cannot create shared memory for Eventcounter\n");
		}
		if (semctl(semid, SEMMSL, IPC_RMID, 0) < 0) {
			if (Debug_eventcounter & 1) {
				perror("semctl(IPC_RMID)");
				Fprintf(stderr, "Panic: Cannot remove Semaphore for Eventcounter\n");
			}
		}
		if (Debug_eventcounter & 2) {
			abort();
		}
		return -1;
	}
 } else {
	initshm = 1;	/* I created the shared memory, so initialize it */
 }
 /* attach shared memory */
 p_awaits = (long *) shmat(shmid, (char *) 0, 0);
 if (p_awaits == (long *) -1) {
	if (Debug_eventcounter & 1) {
		perror("shmat");
	}
	if (semctl(semid, SEMMSL, IPC_RMID, 0) < 0) {
		if (Debug_eventcounter & 1) {
			perror("semctl(IPC_RMID)");
			Fprintf(stderr, "Panic: Cannot remove Semaphore for Eventcounter\n");
		}
	}
	if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
		if (Debug_eventcounter & 1) {
			perror("shmctl(IPC_RMID)");
			Fprintf(stderr, "Panic: Cannot remove shared memory for Eventcounter\n");
		}
	}
	if (Debug_eventcounter & 2) {
		abort();
	}
	return -1;
 }
 /* initialize shared memory */
 if (initshm == 1) {
	struct sembuf	semp[1];


	for (i = 0; i < SEMMSL; i++)	p_awaits[i] = 0l;

	/* now release eventcounter for further use */
	semp[0].sem_num = 0;
	semp[0].sem_op = 1;
	semp[0].sem_flg = 0;
	if (semop(semid, &semp[0], 1) < 0) {
		if (Debug_eventcounter & 1) {
			perror("semop");
			Fprintf(stderr, "Cannot do P() on semaphore for Eventcounter\n");
		}
		if (Debug_eventcounter & 2) {
			abort();
		}
		return -1;
	}
 }

 /* return Eventcounter */
 Eventcounter->semid = semid;
 Eventcounter->shmid = shmid;
 Eventcounter->shmaddr = p_awaits;
 Eventcounter->checksum = EVENTCOUNTERMAGIC + semid + shmid + (long) p_awaits;
 return 0;
}


/*******************************************************************************
*				rmEventcounter				       *
*******************************************************************************/

#if __STDC__
int	rmEventcounter(Eventcounter_t Eventcounter)
#else
int	rmEventcounter(Eventcounter)

Eventcounter_t	Eventcounter;
#endif
{
 /* remove an Eventcounter */

 int error = 0;


 check(Eventcounter);

 if (semctl(Eventcounter.semid, SEMMSL, IPC_RMID, 0) < 0) {
	error = -1;
 }
 if (shmdt((char *) Eventcounter.shmaddr) < 0) {
	error = -1;
 }
 if (shmctl(Eventcounter.shmid, IPC_RMID, (struct shmid_ds *) 0) < 0) {
	error = -1;
 }

 if ( (error < 0) && (Debug_eventcounter & 2) ) {
	abort();
 }

 return error;
}


/*******************************************************************************
*				Eread					       *
*******************************************************************************/

#if __STDC__
long	Eread(Eventcounter_t Eventcounter)
#else
long	Eread(Eventcounter)

Eventcounter_t	Eventcounter;
#endif
{
 /* read the value of the Eventcounter and return it to the caller */

 struct sembuf	semp[1];
 long	evalue;


 check(Eventcounter);

 /* wait for critical section */
 semp[0].sem_num = 0;
 semp[0].sem_op = -1;
 semp[0].sem_flg = SEM_UNDO;
 if (semop(Eventcounter.semid, &semp[0], 1) < 0) {
	if (Debug_eventcounter & 1) {
		perror("semop");
		Fprintf(stderr, "Cannot do P() on semaphore for Eventcounter\n");
	}
	if (Debug_eventcounter & 2) {
		abort();
	}
	return -1;
 }

 /* read the value of the Eventcounter */
 evalue = value;

 /* signal critical section */
 semp[0].sem_num = 0;
 semp[0].sem_op = 1;
 semp[0].sem_flg = SEM_UNDO;
 if (semop(Eventcounter.semid, &semp[0], 1) < 0) {
	if (Debug_eventcounter & 1) {
		perror("semop");
		Fprintf(stderr, "Cannot do V() on semaphore for Eventcounter\n");
	}
	if (Debug_eventcounter & 2) {
		abort();
	}
	return -1;
 }

 return evalue;
}


/*******************************************************************************
*				Eawait					       *
*******************************************************************************/

#if __STDC__
int	Eawait(Eventcounter_t Eventcounter, long v)
#else
int	Eawait(Eventcounter, v)

Eventcounter_t	Eventcounter;
long	v;
#endif
{
 /* await the specified value of the Eventcounter */

 struct sembuf	semp[1];
 int	slot = 0,
	error = 0;
 long	waitval;


 check(Eventcounter);

 /* wait for critical section */
 semp[0].sem_num = 0;
 semp[0].sem_op = -1;
 semp[0].sem_flg = SEM_UNDO;
 if (semop(Eventcounter.semid, &semp[0], 1) < 0) {
	if (Debug_eventcounter & 1) {
		perror("semop");
		Fprintf(stderr, "Cannot do P() on semaphore for Eventcounter\n");
	}
	if (Debug_eventcounter & 2) {
		abort();
	}
	return -1;
 }

 /* get number of waits */
 waitval = v - value;

 if (waitval > 0l) {
	/* look for free slot in wait Q */
	for (slot = 1; slot < SEMMSL; slot++) {
		if (await[slot] == 0L)	break;
	}
	/* 'ave a slot? */
	if (slot == SEMMSL) {
		int	collected = 0,
			collslot = 0;

		/* do garbage collection */
		for (slot = 1; slot < SEMMSL; slot++) {
			if ((kill(await[slot], 0) < 0) && (errno == ESRCH)) {
				/* this slot containes garbage */
				if (collected == 0) {
					/* claim that slot */
					await[slot] = getpid();
					collslot = slot;
					collected = 1;
				} else {
					await[slot] = 0L;
				}
				/* clear corresponding semaphore */
				if (semctl(Eventcounter.semid, slot,
							SETVAL, 0) < 0) {
					if (Debug_eventcounter & 1) {
						perror("semctl(SETVAL)");
						Fprintf(stderr, "Panic: Cannot reinitialize semaphore\n");
					}
					if (Debug_eventcounter & 2) {
						abort();
					}
					error = -1;
				}
			}
		}
		/* remind that slot */
		slot = collslot;
		/* 'ave a slot? */
		if (collected == 0) {
			if (Debug_eventcounter & 1)
				Fprintf(stderr, "Too few slots available for Eawait\n");
			errno = EAGAIN;
			error = -1;
		}
	} else {
		/* claim that slot */
		await[slot] = getpid();
		/* clear corresponding semaphore */
		if (semctl(Eventcounter.semid, slot, SETVAL, 0) < 0) {
			if (Debug_eventcounter & 1) {
				perror("semctl(SETVAL)");
				Fprintf(stderr, "Panic: Cannot reinitialize semaphore\n");
			}
			if (Debug_eventcounter & 2) {
				abort();
			}
			error = -1;
		}
	}
 }

 /* leave critical section */
 semp[0].sem_num = 0;
 semp[0].sem_op = 1;
 semp[0].sem_flg = SEM_UNDO;
 if (semop(Eventcounter.semid, &semp[0], 1) < 0) {
	if (Debug_eventcounter & 1) {
		perror("semop");
		Fprintf(stderr, "Cannot do V() on semaphore for Eventcounter\n");
	}
	if (Debug_eventcounter & 2) {
		abort();
	}
	return -1;
 }

 if ((error != -1) && (waitval > 0)) {
	/* do wait on aquired semaphore */
	semp[0].sem_num = slot;
	semp[0].sem_op = (- waitval);
	semp[0].sem_flg = 0;
	if (semop(Eventcounter.semid, &semp[0], 1) < 0) {
		if (Debug_eventcounter & 1) {
			perror("semop");
			Fprintf(stderr, "Cannot do P() on semaphore for Eawait\n");
		}
		error = -1;
	}
	/* free slot */
	await[slot] = 0;
 }

 if ( (error < 0) && (Debug_eventcounter & 2) ) {
	abort();
 }

 return error;
}


/*******************************************************************************
*				Eadvance				       *
*******************************************************************************/

#if __STDC__
int	Eadvance(Eventcounter_t Eventcounter)
#else
int	Eadvance(Eventcounter)

Eventcounter_t	Eventcounter;
#endif
{
 /* advance the eventcounter by one */

 struct sembuf	semp[SEMMSL];
 int	i,
	slot,
	startslot,
	error = 0;
 long	v;


 check(Eventcounter);

 /* wait for critical section */
 semp[0].sem_num = 0;
 semp[0].sem_op = -1;
 semp[0].sem_flg = SEM_UNDO;
 if (semop(Eventcounter.semid, &semp[0], 1) < 0) {
	if (Debug_eventcounter & 1) {
		perror("semop");
		Fprintf(stderr, "Cannot do P() on semaphore for Eventcounter\n");
	}
	if (Debug_eventcounter & 2) {
		abort();
	}
	return -1;
 }

 /* advance eventcount */
 value++;

 v = value;

 /* check for overflow */
 if (v < 0) {
	errno = ERANGE;
	error = -1;
 } else {

	/* prepare signal on waiting processes */
	for (i = 1, slot = 1; i < SEMMSL; i++) {
		if (await[i] != 0l) {
			semp[slot].sem_num = i;
			semp[slot].sem_op = 1;
			semp[slot].sem_flg = 0;
			slot++;
		}
	}
 }

 /* leave critical section */
 semp[0].sem_num = 0;
 semp[0].sem_op = 1;
 semp[0].sem_flg = SEM_UNDO;
 if (semop(Eventcounter.semid, &semp[0], 1) < 0) {
	if (Debug_eventcounter & 1) {
		perror("semop");
		Fprintf(stderr, "Cannot do V() on semaphore for Eventcounter\n");
	}
	if (Debug_eventcounter & 2) {
		abort();
	}
	return -1;
 }

 if (v >= 0) {
	/* do signal on waiting processes */
	startslot = 1;
	slot --;
	while (slot > 0) {
		if (semop(Eventcounter.semid, &semp[startslot],
					(slot < SEMOPM) ? slot : SEMOPM) < 0) {
			if (Debug_eventcounter & 1) {
				perror("semop");
				Fprintf(stderr,
				"Cannot do V() on semaphores to Eadvance\n");
			}
			error = -1;
		}
		slot -= SEMOPM;
		startslot += SEMOPM;
	}
 }

 if ( (error < 0) && (Debug_eventcounter & 2) ) {
	abort();
 }

 return error;
}
