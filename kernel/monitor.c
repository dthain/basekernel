/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "monitor.h"
#include "process.h"
#include "interrupt.h"

void monitor_lock(struct monitor *m)
{
	if(m->type==MONITOR_TYPE_INTERRUPT_SAFE) {
		interrupt_block();
	} else {
		mutex_lock(&m->mutex);
	}
}

void monitor_unlock(struct monitor *m)
{
	if(m->type==MONITOR_TYPE_INTERRUPT_SAFE) {
		interrupt_unblock();
	} else {
		mutex_unlock(&m->mutex);
	}
}

void monitor_wait(struct monitor *m)
{
	if(m->type==MONITOR_TYPE_INTERRUPT_SAFE) {
		process_wait(&m->queue);
	} else {
		mutex_unlock_and_wait(&m->mutex,&m->queue);
	}
	monitor_lock(m);
}

void monitor_notify(struct monitor *m)
{
	process_wakeup(&m->queue);
}

void monitor_notify_all(struct monitor *m)
{
	while(m->queue.head) {
		process_wakeup(&m->queue);
	}
}
