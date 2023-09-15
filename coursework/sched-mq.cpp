/*
*	The Priority Task Scheduler
*	SKELETON IMPLEMENTATION TO BE FILLED IN FOR TASK 1
*/

#include <infos/kernel/sched.h> #include <infos/kernel/thread.h> #include <infos/kernel/log.h> #include <infos/util/list.h> #include <infos/util/lock.h> #include <infos/util/map.h>

using namespace infos::kernel; using namespace infos::util;
//UniqueIRQLock 1 probably

/**
*	A Multiple Queue priority scheduling algorithm
*/
class MultipleQueuePriorityScheduler : public SchedulingAlgorithm
{
public:
/**
* Returns the friendly name of the algorithm, for debugging and selection purposes.
*/
const char* name() const override { return "mq"; }

/**
* Called during scheduler initialisation.
*/
void init()
{
//TODO: Implement me!
}

/**
*	Called when a scheduling entity becomes eligible for running.
*	@param entity
*/
void add_to_runqueue(SchedulingEntity& entity) override
{
UniqueIRQLock l; if(entity.priority() == 0)
Realtime.append(&entity); else if(entity.priority() == 1)
Interactive.append(&entity); else if(entity.priority() == 2)
Normal.append(&entity); else Daemon.append(&entity);

}

/**
*	Called when a scheduling entity is no longer eligible for running.
*	@param entity
*/
void remove_from_runqueue(SchedulingEntity& entity) override
{
if(entity.priority() == 0) Realtime.remove(&entity);
else if(entity.priority() == 1) Interactive.remove(&entity);
else if(entity.priority() == 2) Normal.remove(&entity);
else Daemon.remove(&entity);
}

SchedulingEntity *RR(List <SchedulingEntity *>& list)
{
list.append(list.dequeue()); return list.last();
}

/**
*	Called every time a scheduling event occurs, to cause the next eligible entity
*	to be chosen. The next eligible entity might actually be the same entity, if
*	e.g. its timeslice has not expired.
*/
SchedulingEntity *pick_next_entity() override
{
if(!Realtime.empty()) return RR(Realtime);
else if(!Interactive.empty()) return RR(Interactive);
else if(!Normal.empty()) return RR(Normal);
 
else if(!Daemon.empty()) return RR(Daemon);
else return NULL;

}

private:
List<SchedulingEntity *> Realtime; List<SchedulingEntity *> Interactive; List<SchedulingEntity *> Normal; List<SchedulingEntity *> Daemon;
};

/* --- DO NOT CHANGE ANYTHING BELOW THIS LINE --- */

RegisterScheduler(MultipleQueuePriorityScheduler);
