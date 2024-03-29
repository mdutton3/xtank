
/*
** Xtank
**
** Copyright 1988 by Terry Donahue
**
** icounter.c
*/

/*
$Author: lidl $
$Id: icounter.c,v 1.1.1.1 1995/02/01 00:25:35 lidl Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "malloc.h"
#include "sysdep.h"
#include "icounter.h"
#include "common.h"
#include "tanktypes.h"
#include "clfkr.h"
#include "proto.h"

#ifdef UNIX
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>

#define INC_TIME 10000

int elapsed_time;

/*
** Virtual timer alarm handler.  Updates how much time has passed
** by incrementing elapsed_time by INC_TIME.
*/

#if defined(MOTOROLA) && defined(m68k)
int increment_time()
#else
void increment_time()
#endif
{
	elapsed_time += INC_TIME;
#ifdef mmax
	sigset(SIGVTALRM, increment_time);
#endif
#ifdef linux
	signal(SIGVTALRM, increment_time);
#endif
}

#if defined(MOTOROLA) && defined(m68k)
#undef SIGVTALRM
#define SIGVTALRM SIGALRM
#endif

setup_counter()
{
#if defined(MOTOROLA) && defined(m88k) || defined(SVR4)
	if ((int) sigset(SIGVTALRM, increment_time) == -1)
#else
	if ((int) signal(SIGVTALRM, increment_time) == -1)
#endif
		rorre("Could not set up interval timer signal handler.");
}

#if defined(MOTOROLA) && defined(m68k)

static unsigned int start_val = INC_TIME / 1000;
static unsigned int stop_val = 0;

#else /* !defined(MOTOROLA) !! !definded(m68k) (ie everything else) */

static struct itimerval start_val =
{
	{0, INC_TIME},
	{0, INC_TIME}};
static struct itimerval stop_val =
{
	{0, 0},
	{0, 0}};

#endif /* !defined(MOTOROLA) !! !definded(m68k) (ie everything else) */

/*
** Start up interval timer to call increment_time every INC_TIME ms.
*/
start_counter()
{
#if defined(MOTOROLA) && defined(m68k)
	(void) alarm(start_val);
#else
	(void) setitimer(ITIMER_VIRTUAL, &start_val, (struct itimerval *) NULL);
#endif
}

/*
** Stop the interval timer.
*/
stop_counter()
{
#if defined(MOTOROLA) && defined(m68k)
	(void) alarm(stop_val);
#else
	(void) setitimer(ITIMER_VIRTUAL, &stop_val, (struct itimerval *) NULL);
#endif
}

/* Has enough real time passed since the last frame? */
static Boolean real_timer_expired = TRUE;

void sigalrm_handler()
{
	real_timer_expired = TRUE;
#ifdef linux
	/* Reinstall the sigalrm_handler each time */
	signal(SIGALRM, sigalrm_handler);
#endif
}

#if defined(MOTOROLA) && defined(m88k)
#define timerclear(tvp)	(tvp)->tv_sec = (tvp)->tv_usec = 0
#endif

#if defined(MOTOROLA) && defined(m68k)

start_real_counter(time)
int time;
{
}

wait_for_real_counter()
{
}

#else


extern struct CLFkr command_options;	/* options for how xtank starts / exits */

start_real_counter(time)
int time;
{
	struct itimerval real_timer;

	if (!command_options.NoDelay) {
		/* Set up a real-time interval timer that expires every time useconds.
           Each time it expires, the variable real_timer_expired will be set to
	       true. */
		timerclear(&real_timer.it_interval);
		timerclear(&real_timer.it_value);
		real_timer.it_interval.tv_usec = real_timer.it_value.tv_usec = time;
		(void) setitimer(ITIMER_REAL, &real_timer, (struct itimerval *) NULL);

		/* Call the sigalrm_handler function every time the handler expires */
#if defined(MOTOROLA) && defined(m88k) || defined(SVR4)
		sigset(SIGALRM, sigalrm_handler);
#else
		signal(SIGALRM, sigalrm_handler);
#endif
	}
}

wait_for_real_counter()
{
	/* The variable real_timer_expires will be set to true when the ITIMER_REAL
       timer expires.  Until then, pause() is called, which gives up process
       control until an interval timer expires. This way we don't waste CPU
       time. */
	if (!command_options.NoDelay) {
		while (!real_timer_expired)
			pause();
	}
	real_timer_expired = FALSE;
}
#endif /* MOTOROLA && m68k */

#endif

#ifdef AMIGA
struct Custom *c = (struct Custom *) 0xDFF000;
extern void VB_count();
struct Interrupt intserver;
int count;

setup_counter()
{
	intserver.is_Node.ln_Type = NT_INTERRUPT;
	intserver.is_Node.ln_Pri = 120;
	intserver.is_Node.ln_Name = "VB-count";
	intserver.is_Data = (APTR) & count;
	intserver.is_Code = VB_count;
}

start_counter()
{
	AddIntServer(INTB_VERTB, &intserver);
}

stop_counter()
{
	RemIntServer(INTB_VERTB, &intserver);
}

start_real_counter(time)
int time;
{
}

wait_for_real_counter()
{
}

#endif
