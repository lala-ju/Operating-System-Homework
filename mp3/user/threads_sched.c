#include "kernel/types.h"
#include "user/user.h"
#include "user/list.h"
#include "user/threads.h"
#include "user/threads_sched.h"
#include <stdbool.h>

#define NULL 0

/* default scheduling algorithm */
struct threads_sched_result schedule_default(struct threads_sched_args args)
{
    struct thread *thread_with_smallest_id = NULL;
    struct thread *th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_with_smallest_id == NULL || th->ID < thread_with_smallest_id->ID) {
            thread_with_smallest_id = th;
        }
    }

    struct threads_sched_result r;
    if (thread_with_smallest_id != NULL) {
        r.scheduled_thread_list_member = &thread_with_smallest_id->thread_list;
        r.allocated_time = thread_with_smallest_id->remaining_time;
    } else {
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = 1;
    }

    return r;
}

/* Earliest-Deadline-First scheduling */
struct threads_sched_result schedule_edf(struct threads_sched_args args)
{
    struct thread *tar = NULL;
    struct thread *th = NULL;
    bool miss = false;
    struct threads_sched_result r;
    //check miss deadline thread
    list_for_each_entry(th, args.run_queue, thread_list) {
        if(th->current_deadline <= args.current_time )
        {
            if(! miss)
            {
                miss = true;
                tar = th;
            }
            else if(th->ID < tar->ID)
                tar = th;
        }
    }
    if(miss)
    {
        r.scheduled_thread_list_member = &tar->thread_list;
        r.allocated_time = 0;
        return r;
    }
    
    //find high priority thread to execute
    tar = NULL;
    th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if(tar == NULL)
            tar = th;
        else
        {
            if(th->current_deadline < tar->current_deadline)
                tar = th;
            else if(th->current_deadline == tar->current_deadline && th->ID < tar->ID)
                tar = th;
        }
    }

    //assign to return value
    if(tar != NULL)
    {
        r.scheduled_thread_list_member = &tar->thread_list;
        if(args.current_time + tar->remaining_time > tar->current_deadline)
            r.allocated_time = tar->current_deadline - args.current_time;
        else
            r.allocated_time = tar->remaining_time;
        //check preempted higher priority
        int finishtime = args.current_time + r.allocated_time;
        struct release_queue_entry *cur = NULL;
        list_for_each_entry(cur, args.release_queue, thread_list)
        {
            int nxtdl = (cur->release_time + cur->thrd->period);
            if(nxtdl < tar->current_deadline && finishtime > cur->release_time)
                r.allocated_time = cur->release_time - args.current_time;
            else if(nxtdl == tar->current_deadline && cur->thrd->ID < tar->ID && finishtime > cur->release_time)
                r.allocated_time = cur->release_time - args.current_time;
        }
    }
    else //run queue is empty
    {
        r.scheduled_thread_list_member = args.run_queue;
        struct release_queue_entry *cur = NULL;
        int min = -1;
        //get the next closest release time
        list_for_each_entry(cur, args.release_queue, thread_list)
        {
            if(min == -1)
                min = cur->release_time - args.current_time;
            else if((cur->release_time - args.current_time) < min)
                min = cur->release_time - args.current_time;
        }
        r.allocated_time = min;
    }
    return r;
}

/* Rate-Monotonic Scheduling */
struct threads_sched_result schedule_rm(struct threads_sched_args args)
{
    struct thread *tar = NULL;
    struct thread *th = NULL;
    bool miss = false;
    struct threads_sched_result r;
    //check miss deadline thread
    list_for_each_entry(th, args.run_queue, thread_list) {
        if(th->current_deadline <= args.current_time )
        {
            if(! miss)
            {
                miss = true;
                tar = th;
            }
            else if(th->ID < tar->ID)
                tar = th;
        }
    }
    if(miss)
    {
        r.scheduled_thread_list_member = &tar->thread_list;
        r.allocated_time = 0;
        return r;
    }
    
    //find high priority thread to execute
    tar = NULL;
    th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if(tar == NULL)
            tar = th;
        else
        {
            if(th->period < tar->period)
                tar = th;
            else if(th->period == tar->period && th->ID < tar->ID)
                tar = th;
        }
    }

    //assign to return value
    if(tar != NULL)
    {
        r.scheduled_thread_list_member = &tar->thread_list;
        if(args.current_time + tar->remaining_time > tar->current_deadline)
            r.allocated_time = tar->current_deadline - args.current_time;
        else
            r.allocated_time = tar->remaining_time;

        int finishtime = args.current_time + r.allocated_time;
        //check preempted higher priority
        struct release_queue_entry *cur = NULL;
        list_for_each_entry(cur, args.release_queue, thread_list)
        {
            if(cur->thrd->period < tar->period && cur->release_time < finishtime){
                r.allocated_time = cur->release_time - args.current_time;
                finishtime = args.current_time + r.allocated_time;
            }
        }
    }
    else //run queue is empty
    {
        r.scheduled_thread_list_member = args.run_queue;

        //get the next closest release time
        struct release_queue_entry *cur = NULL;
        int min = -1;
        list_for_each_entry(cur, args.release_queue, thread_list)
        {
            if(min == -1)
                min = cur->release_time - args.current_time;
            else if((cur->release_time - args.current_time) < min)
                min = (cur->release_time - args.current_time);
        }
        r.allocated_time = min;
    }
    return r;
}
