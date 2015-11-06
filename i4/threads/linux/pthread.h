/*
 * $Id: pthread.h 80 2008-03-04 23:32:10Z sam $
 ****************************************************************************
 *    
 *  COPYRIGHT NOTICE
 *    
 *  Copyright (C) 1995, 1996 Michael T. Peterson
 *  This file is part of the PCthreads (tm) multithreading library
 *  package.
 *    
 *  The source files and libraries constituting the PCthreads (tm) package
 *  are free software; you can redistribute them and/or modify them under
 *  the terms of the GNU Library General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *    
 *  The PCthreads (tm) package is distributed in the hope that it will
 *  be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *    
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library (see the file COPYING.LIB); if not,
 *  write to the Free Software Foundation, Inc., 675 Mass Ave, Cambridge,
 *  MA 02139, USA.
 *    
 *  To report bugs, bug fixes, or provide feedback you may contact the
 *  author, Michael T. Peterson, at either of the two email addresses listed
 *  below:
 *    
 *             mtp@big.aa.net (preferred)
 *             mtp@zso.dec.com
 *    
 *    
 *  Michael T. Peterson
 *  Redmond, WA.
 *  13 January, 1996
 ****************************************************************************
 * $Log: pthread.h,v $
 * Revision 1.1  1996/08/26 18:24:53  jc
 * new and improved!
 *
 * Revision 1.1.1.1  1996/06/29 01:20:53  mtp
 * pthreads 1.0 distribution
 *
 * Revision 1.1.1.1  1996/06/28 15:45:41  mtp
 * Official V1.0 Sources
 *
 * Revision 2.0  1996/06/28 14:58:13  mtp
 * + Release 2.0
 *
 * Revision 1.6  1996/01/14 20:17:08  mtp
 * + Changed prototype for pthread_unblockasync_np() to return void.
 *
 * Revision 1.5  1996/01/14 18:32:44  mtp
 * + Replaced @Header@ with COPYRIGHT NOTICE.
 *
 * Revision 1.4  1996/01/10 19:05:23  mtp
 * + Removed prototypes for pthread_mutex_{get,set}prio_ceiling().
 *
 * Revision 1.3  1996/01/10 17:15:01  mtp
 * + Added prototypes for pthread_blockasync_np(), pthread_unblockasync_np(),
 *   and pthread_cond_signal_intr_np().
 * + Added comments for all pthread_xxxxx_np() functions.
 *
 * Revision 1.2  1995/12/31 06:21:26  mtp
 * + Changed pthread_get_errno_np() to pthread_geterrno_np().
 * + Changed pthread_set_errno_np() to pthread_seterrno_np().
 * + Changed pthread_get_expiration_np() to pthread_getexpiration_np().
 *
 * Revision 1.1.1.1  1995/12/16 21:42:00  mtp
 * Placed under CVS control
 *
 ****************************************************************************
 */
#ifndef _pthread_
#define _pthread_

#ifdef __cplusplus
extern "C" {
#endif
#include <unistd.h>
#include "typedefs.h"

/*
 * --  Forward declare these pointers to structures that will be defined
 *     internally.
 */
typedef int                                pthread_key_t;
typedef struct PTHREAD_HANDLE *            pthread_t;
typedef struct PTHREAD_MUTEX_HANDLE *      pthread_mutex_t;
typedef struct PTHREAD_CONDV_HANDLE *      pthread_cond_t;
typedef struct PTHREAD_ATTR_HANDLE *       pthread_attr_t;
typedef struct PTHREAD_MUTEXATTR_HANDLE *  pthread_mutexattr_t;
typedef struct PTHREAD_CONDATTR_HANDLE *   pthread_condattr_t;

/*
 * --   These constants inform clients of the capabilities
 *      of PCthreads
 */
#define _POSIX_THREADS
#define _POSIX_THREAD_ATTR_STACKSIZE
#define _POSIX_THREAD_PRIORITY_SCHEDULING
#define _POSIX_THREAD_PRIO_PROTECT
#define _POSIX_THREAD_PRIO_INHERIT

#ifdef _POSIX_THREADS_PROCESS_SHARED
#undef _POSIX_THREADS_PROCESS_SHARED
#endif

#define PTHREAD_KEYS_MAX PTHREAD_C_MAX_DATAKEYS
#define PTHREAD_DESTRUCTOR_ITERATIONS ((int) 100)

/*
 * --  These constants are required by POSIX.1c
 */
#define SCHED_FIFO   ((int) SCHED_FCFS_C)
#define SCHED_RR     ((int) SCHED_ROUND_ROBIN_C)
#define SCHED_OTHER  ((int) SCHED_PRIORITY_DECAY_C)

#define PRI_FIFO_MIN       PTHREAD_MIN_PRIO_C
#define PRI_FIFO_DEFAULT   PTHREAD_DEFAULT_PRIO_C
#define PRI_FIFO_MAX       PTHREAD_MAX_PRIO_C

#define PRI_RR_MIN      PRI_FIFO_MIN
#define PRI_RR_DEFAULT  PRI_FIFO_DEFAULT
#define PRI_RR_MAX      PRI_FIFO_MAX

#define PRI_OTHER_MIN      PRI_FIFO_MIN
#define PRI_OTHER_DEFAULT  PRI_FIFO_DEFAULT
#define PRI_OTHER_MAX      PRI_FIFO_MAX

#define PTHREAD_INHERIT_SCHED          PTHREAD_USE_ATTRIBUTES_C
#define PTHREAD_DEFAULT_SCHED          PTHREAD_SCHED_INHERIT_C

#define PTHREAD_CANCEL_ENABLE          THREAD_CANCEL_ENABLED_C
#define PTHREAD_CANCEL_DISABLE         THREAD_CANCEL_DISABLED_C
#define PTHREAD_CANCEL_DEFERRED        THREAD_CANCEL_DEFERRED_C
#define PTHREAD_CANCEL_ASYNCHRONOUS    THREAD_CANCEL_ASYNC_C
#define PTHREAD_CANCELED               (0xDEADBABE)

#define MUTEX_SCHED_DEFAULT            SCHED_MUTEX_NO_PRIO_INHERIT_C
#define MUTEX_SCHED_INHERIT            SCHED_MUTEX_PRIO_INHERIT_C
#define MUTEX_SCHED_PROTECT            SCHED_MUTEX_PRIO_PROTECT_C

#define PTHREAD_CREATE_JOINABLE        PTHREAD_JOINABLE_C
#define PTHREAD_CREATE_DETACHED        PTHREAD_DETACHED_C

/*-------------------------------------------------------------------------*
 * --  POSIX .1c API services.                                             *
 *-------------------------------------------------------------------------*/

/*
 * This service allows a thread to wait for a specified set of signals
 * to be delivered to the process.  Upon delivery of any one of the set
 * of signals, the thread returns from this function with the value of
 * the delivered signal.  The sigwait() service is a cancellation point.
 */
extern int
sigwait( sigset_t sigset );


/*
 * Client applications should use pthread_sigmask() instead of sigprocmask().
 * In fact, all occurrences of sigprocmask are redefined to pthread_sigmask()
 * by this header file.
 */
extern int
pthread_sigmask( int how, const sigset_t *newmask, sigset_t *prev );


/*
 * This routine sends a signal to a specified thread.  Any signal defined to
 * stop, continue, or terminate will be applied to all threads in the
 * process, i.e., the effect will be process-wide.  For example, sending 
 * SIGTERM to *any* thread terminates *all* threads in the process - Even 
 * though it may be handled by the thread to which it was sent.
 */
extern int
pthread_kill( pthread_t handle, int sig );


extern int  
pthread_create( pthread_t      *handle,
                pthread_attr_t *th_attr,
                void *         (*th_proc)(void *),
                void *         th_proc_arg );
extern int  
pthread_join( pthread_t handle, void **return_value );

extern int  
pthread_detach( pthread_t  handle );

extern int
pthread_setschedparam( pthread_t handle, 
                       int policy,
                       const struct sched_param *param );

extern int
pthread_getschedparam( pthread_t handle,
                       int *policy,
                       struct sched_param *param );
extern int 
pthread_once( pthread_once_t  *once_control, 
              void (*init_routine)(void));

extern pthread_t
pthread_self( void );

extern int
pthread_key_create( pthread_key_t *key, void (*destructor)(void *) );

extern int
pthread_getspecific( pthread_key_t key, void **value );

extern int
pthread_setspecific( pthread_key_t key, void *value );

extern int  
pthread_cancel( pthread_t th_h );

extern void  
pthread_testcancel( void );

extern int  
pthread_setcancelstate( int new_state, int *prev_state );

extern int 
pthread_setcanceltype( int new_type, int *old_type );

extern void
pthread_cleanup_push( void (*cleanup_routine)(void *), void *arg );

extern void
pthread_cleanup_pop( int execute );

extern void  
pthread_yield( void  *arg );

extern int
pthread_equal( pthread_t th1_handle, pthread_t th2_handle );

extern void  
pthread_exit( void *exit_value );

extern int  
pthread_mutex_init( pthread_mutex_t  *handle, 
                    pthread_mutexattr_t  *mu_attr_h );

extern int  
pthread_mutex_destroy( pthread_mutex_t  *handle );

extern int  
pthread_mutex_lock( pthread_mutex_t  *handle );

extern int  
pthread_mutex_unlock( pthread_mutex_t  *handle );

extern int  
pthread_mutex_trylock( pthread_mutex_t  *mu_h );

extern int  
pthread_cond_init( pthread_cond_t  *handle, pthread_condattr_t  *cv_attr_h );

extern int  
pthread_cond_destroy( pthread_cond_t  *handle );

extern int  
pthread_cond_wait( pthread_cond_t  *cv_h,  pthread_mutex_t *mu_h );

extern int  
pthread_cond_signal( pthread_cond_t  *handle );

extern int
pthread_cond_broadcast( pthread_cond_t *handle );

extern int  
pthread_cond_timedwait( pthread_cond_t  *cv_h, 
                        pthread_mutex_t  *mu_h, 
                        const struct timespec *abstime );

extern int  
pthread_attr_init( pthread_attr_t *handle );

extern int  
pthread_attr_destroy( pthread_attr_t *handle );

extern int 
pthread_attr_setdetachstate( pthread_attr_t  *handle,
                             int  detached_state );
extern int 
pthread_attr_getdetachstate( pthread_attr_t  *handle,
                             int *detached_state );

extern int  
pthread_attr_setstacksize( pthread_attr_t  *handle, size_t stack_size );

extern int  
pthread_attr_getstacksize( pthread_attr_t  *handle, size_t  *stack_size );

extern int  
pthread_attr_setinheritsched( pthread_attr_t  *handle, int inherit_sched );

extern int  
pthread_attr_getinheritsched( pthread_attr_t *handle, int *inherit_sched );

extern int  
pthread_attr_setschedpolicy( pthread_attr_t  *handle, int sched_policy );

extern int
pthread_attr_getschedpolicy( pthread_attr_t *handle, int *sched_policy );

/*
 * --  The xxxschedparam() routines replace the get/setscheduler() and
 *     get/setprio() routines.
 */
extern int  
pthread_attr_getschedparam( pthread_attr_t *handle,
                            struct sched_param *sched_param );

extern int  
pthread_attr_setschedparam( pthread_attr_t  *handle, 
                            const struct sched_param *sched_param );

extern int
pthread_attr_getschedpriority( pthread_attr_t *handle, int *sched_priority );

extern int  
pthread_mutexattr_init( pthread_mutexattr_t  *handle );

extern int  
pthread_mutexattr_destroy( pthread_mutexattr_t  *handle );

extern int  
pthread_mutexattr_setprotocol(pthread_mutexattr_t  *handle, 
                              pthread_protocol_t protocol );

extern int
pthread_mutexattr_getprotocol( pthread_mutexattr_t *handle,
                              pthread_protocol_t *protocol );

extern int
pthread_mutexattr_getprio_ceiling( pthread_mutexattr_t *handle,
                                   int *prio_ceiling );

extern int 
pthread_mutexattr_setprio_ceiling( pthread_mutexattr_t  *handle,
                                   int prio_ceiling );

extern int  
pthread_condattr_init( pthread_condattr_t  *handle );

extern int  
pthread_condattr_destroy( pthread_condattr_t  *handle );

/*
 * --  Utility functions (non-portable)
 */
extern int
pthread_delay_np( const struct timespec *interval );


/*
 * Return the thread's sequence number.  This is a unique,
 * integer valued number such that (2 <= N <= MAX_INT).
 */
extern long
pthread_getsequence_np( const pthread_t handle );

/*
 * Return the absolute time as the sum of the interval (delta)
 * and the current time.
 */
extern int
pthread_getexpiration_np( const struct timespec *delta,
                          struct timespec *abstime );

/*
 * Returns a process-wide count of the number of thread context
 * switches.
 */
extern int
pthread_get_ctxsw_counts_np( pthread_t handle,
                             unsigned long *total_count,
                             unsigned long *async_count );

/*
 * Return the number of bytes of stack that have not been
 * used.  Call this routine at the end of a thread's procedure
 * to see whether you've allocated too much stack space.
 */
extern int
pthread_checkstack_np( pthread_t handle, size_t *bytes );

/*
 * Return the thread-specific errno value, or Zero.
 */
extern int
pthread_geterrno_np( pthread_t handle, int *err );

/*
 * Set a thread-specific errno value.
 */
extern int
pthread_seterrno_np( int err );

/*
 * Return the number of threads waiting at the specified
 * condition variable.
 */
extern int
pthread_cond_getwaiters_np( pthread_cond_t handle,
                            unsigned int *waiting_threads );

/*
 * Allow a condition variable to be signaled from an interrupt
 * handler. E.g., signal a condition variable from a ctrl-C interrupt.
 */
extern int
pthread_cond_signal_intr_np( pthread_cond_t *handle );

/*
 * Return the number of threads blocked at a specified mutex.
 */
extern int
pthread_mutex_getblocked_np( pthread_mutex_t handle,
                             unsigned int *blocked_threads );

/*
 * Obtain the execution priority of the specified thread.
 */
extern int
pthread_getprio_np( pthread_t handle, int *current_prio );

/*
 * Set the execution priority of the specified thread.
 */
extern int
pthread_setprio_np( pthread_t handle, int new_priority );

/*
 * Thread-safe wrapper for UNIX gettimeofday() function.
 */
extern int
pthread_gettimeofday_np( struct timeval *tv, struct timezone *tz );

/*
 * Lock and unlock the single, process-wide mutex.
 */
extern int
pthread_lock_global_np( void );

extern int
pthread_unlock_global_np( void );

/*
 * Block and unblock asynchronous thread context switches.  Use
 * as follows:
 *
 *      flag = pthread_blockasync_np();
 *                  .
 *                  .
 *                  .
 *      (void) pthread_unblockasync_np( flag );
 *
 * Use with great caution and do not execute any functions that
 * may cause the thread to take a synchronous context switch, e.g.,
 * lock a mutex, signal a condition variable, etc.
 */
extern int
pthread_blockasync_np( void );

extern void
pthread_unblockasync_np( int flag );

/*
 * Return some process-wide statistics.
 */
extern int
system_get_state( unsigned long *intr_count,
                  unsigned long *ctxsw_count,
                  unsigned long *async_preemptions,
                  unsigned long *elapsed_time,
                  int *active_priority );

/*
 * A thread-safe wrapper for sigaction.
 */
extern int
pthread_sigaction_np( int signal,
                      const struct sigaction *new_action,
                      struct sigaction *prev_action );

#undef sigprocmask
#define sigprocmask pthread_sigmask

#undef gettimeofday
#define gettimeofday pthread_gettimeofday_np

#ifdef _DCE_COMPAT_
#include <pthread_dce.h>
#endif

#ifdef __cplusplus
}
#endif
#endif
