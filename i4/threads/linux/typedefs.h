/*
 * $Id: typedefs.h 80 2008-03-04 23:32:10Z sam $
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
 * $Log: typedefs.h,v $
 * Revision 1.1  1996/08/28 20:14:35  jc
 * new
 *
 * Revision 1.1.1.1  1996/06/29 01:20:54  mtp
 * pthreads 1.0 distribution
 *
 * Revision 1.1.1.1  1996/06/28 15:45:41  mtp
 * Official V1.0 Sources
 *
 * Revision 2.0  1996/06/28 14:58:16  mtp
 * + Release 2.0
 *
 * Revision 1.5  1996/06/28 14:38:12  mtp
 * + Removed conditional compilation for struct timespec and added a comment
 *   instructing developer how to build for versions of Linux prior to
 *   V2.0.
 *
 * Revision 1.3  1996/01/14 20:22:56  mtp
 * + Modified copyright and permission notice to conform with GNU requirements.
 *
 * Revision 1.2  1996/01/14 18:32:47  mtp
 * + Replaced @Header@ with COPYRIGHT NOTICE.
 *
 * Revision 1.1.1.1  1995/12/16 21:42:00  mtp
 * Placed under CVS control
 *
 ****************************************************************************
 */
#ifndef _typedefs_
#define _typedefs_

#ifdef __cplusplus
extern "C" {
#endif

#define _POSIX_SOURCE 1    /* Disallow all non-POSIX vendor extensions */
#include <stdlib.h>        /* Get size_t declaration */
#include <signal.h>
#include <sys/time.h>


extern const char * const sys_signame[];

typedef enum SCHEDULING_POLICY
{
    SCHED_POLICY_MIN_C,                /* Must be first */
    SCHED_FCFS_C,
    SCHED_ROUND_ROBIN_C,               /* Default sched policy */
    SCHED_PRIORITY_DECAY_C,
    SCHED_POLICY_MAX_C                 /* Must be last */

}                              sched_policy_t;

struct sched_param
{
   sched_policy_t  sched_policy;
   int             sched_priority;
   int             sched_quantum;
};

typedef enum MUTEX_SCHED_PROTOCOL
{
    SCHED_MUTEX_PRIO_MIN_C,        /* Must be first */
    SCHED_MUTEX_NO_PRIO_INHERIT_C, /* Default upon mutex creation */
    SCHED_MUTEX_PRIO_INHERIT_C,    /* Not implemented, V1.0 */
    SCHED_MUTEX_PRIO_PROTECT_C,    /* Not implemented, V1.0 */
    SCHED_MUTEX_PRIO_MAX_C         /* Must be last */

} pthread_protocol_t;

typedef enum DETACHED_STATE
{
   PTHREAD_DETACHED_MIN_C,         /* Must be first */
   PTHREAD_JOINABLE_C,
   PTHREAD_DETACHED_C,              /* Default upon thread creation */
   PTHREAD_DETACHED_MAX_C

} detached_state_t;

/*
 * --  Versions of Linux previous to 2.0 may not have a struct timespec
 *     data structure.  To compile PCthreads for a version of Linux
 *     that does not support "struct timespec", add the structure here.
 */

#if 0
struct timespec
{
   long tv_sec;
   long tv_nsec;                    /* nanoseconds */
};
#endif

#define PTHREAD_MAX_PRIO_C     ((int) 32)
#define PTHREAD_DEFAULT_PRIO_C ((int) 16)
#define PTHREAD_MIN_PRIO_C     ((int) 1)

#define PTHREAD_SCHED_USE_ATTRIBUTES_C ((int) 0)
#define PTHREAD_SCHED_INHERIT_C        ((int) 1)

#undef FALSE
#define FALSE ((int) 0)
#undef TRUE
#define TRUE  ((int) 1)

typedef int                pthread_once_t;
#define PTHREAD_ONCE_INIT  FALSE
#define pthread_once_init  PTHREAD_ONCE_INIT

typedef void * any_t;
typedef void * thread_arg_t;
typedef void * arg_t;
typedef void * (*thread_proc_t)(void *);
typedef void   (*exit_proc_t)(void);
typedef void   (*destructor_t)(void *);
typedef void   (*cleanup_t)(void *);
typedef void   (*initroutine_t)(void);

#define THREAD_CANCEL_ENABLED_C    ((int) 1) /* Default upon creation */
#define THREAD_CANCEL_DISABLED_C   ((int) 0)
#define THREAD_CANCEL_DEFERRED_C   ((int) 1) /* Default upon creation */
#define THREAD_CANCEL_ASYNC_C      ((int) 0)

#define PTHREAD_C_MAX_DATAKEYS     ((int) 100)

#undef SUCCESS
#define SUCCESS ((int) 0)

#undef FAILURE
#define FAILURE ((int) -1)

#ifdef __cplusplus
}
#endif
#endif
