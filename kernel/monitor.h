/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef MONITOR_H
#define MONITOR_H

#include "mutex.h"
#include "list.h"

struct monitor {
	int type;
	struct mutex mutex;
	struct list queue;
};

#define MONITOR_TYPE_INTERRUPT_SAFE 1
#define MONITOR_TYPE_PROCESS_SAFE 2

#define MONITOR_INIT_INTERRUPT_SAFE {MONITOR_TYPE_INTERRUPT_SAFE,MUTEX_INIT,LIST_INIT}
#define MONITOR_INIT_PROCESS_SAFE   {MONITOR_TYPE_PROCESS_SAFE,MUTEX_INIT,LIST_INIT}

void monitor_lock( struct monitor *m );
void monitor_wait( struct monitor *m );
void monitor_notify( struct monitor *m );
void monitor_notify_all( struct monitor *m );
void monitor_unlock( struct monitor *m );

#endif
