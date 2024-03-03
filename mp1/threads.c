#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0


static struct thread* current_thread = NULL;
static int id = 1;
static jmp_buf env_st;
//static jmp_buf env_tmp;

struct thread *thread_create(void (*f)(void *), void *arg){
    struct thread *t = (struct thread*) malloc(sizeof(struct thread));
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x100);
    new_stack_p = new_stack +0x100*8-0x2*8;
    t->fp = f;
    t->arg = arg;
    t->ID  = id;
    t->buf_set = 0;
    t->stack = (void*) new_stack;
    t->stack_p = (void*) new_stack_p;
    id++;

    // part 2
    t->sig_handler[0] = NULL_FUNC;
    t->sig_handler[1] = NULL_FUNC;
    t->signo = -1;
    t->handler_buf_set = 0;
    
    return t;
}
void thread_add_runqueue(struct thread *t){
    if(current_thread == NULL){
        // TODO
        current_thread = t;
        current_thread->previous = t;
        current_thread->next = t;
    }
    else{
        // TODO
        current_thread->previous->next = t;
        t->previous = current_thread->previous;
        t->next = current_thread;
        current_thread->previous = t;

        t->sig_handler[0] = current_thread->sig_handler[0];
        t->sig_handler[1] = current_thread->sig_handler[1];
    }
    return;
}
void thread_yield(void){
    // TODO
    if(current_thread->signo == -1) //call in function
    {
        int jump = setjmp(current_thread->env);
        if(jump == 0)
        {
            schedule();
            dispatch();
        }
        else
        {
        }
    }
    else//call by signal handler
    {
        int jmp = setjmp(current_thread->handler_env);
        if(jmp == 0)
        {
            schedule();
            dispatch();
        }
        else
        {
        }
    }
}
void dispatch(void){
    // TODO
    if(current_thread->signo == -1)
    {
        if(current_thread->buf_set == 1)
        {
            longjmp(current_thread->env, 1);
        }
        else
        {
            current_thread->buf_set = 1;
            int s = setjmp(current_thread->env);
            if(s == 0)
            {
                current_thread->env->sp = (unsigned long) current_thread->stack_p;
                longjmp(current_thread->env, 1);
            }
            else
            { 
                (*(current_thread->fp))(current_thread->arg);
            }
        }
        thread_exit();
    }
    else
    {
        if(current_thread->sig_handler[current_thread->signo] == NULL_FUNC)
            thread_exit();

        if(current_thread->handler_buf_set == 1)
        {
            longjmp(current_thread->handler_env, 1);
        }
        else
        {
            current_thread->handler_buf_set = 1;
            int s = setjmp(current_thread->handler_env);
            if(s == 0)
            {
                if(current_thread->buf_set == 1)
                    current_thread->handler_env->sp = (unsigned long) current_thread->env->sp;
                else
                    current_thread->handler_env->sp = (unsigned long) current_thread->stack_p;
                longjmp(current_thread->handler_env, 1);
            }
            else
            { 
                (*current_thread->sig_handler[current_thread->signo])(current_thread->signo);
            }
        }
        current_thread->signo = -1;
        if(current_thread->buf_set == 0)
        {
            current_thread->buf_set = 1;
            int s = setjmp(current_thread->env);
            if(s == 0)
            {
                current_thread->env->sp = (unsigned long) current_thread->stack_p;
                longjmp(current_thread->env, 1);
            }
            else
            { 
                (*(current_thread->fp))(current_thread->arg);
            }
        }
        else
            longjmp(current_thread->env, 1);
    }
    thread_exit();
}
void schedule(void){
    // TODO
    current_thread = current_thread->next;
    return;
}
void thread_exit(void){
    setjmp(current_thread->env);
    if(current_thread->next != current_thread){
        // TODO
        //printf("%d exit\n", current_thread->ID);
        struct thread *tmp = current_thread;
        tmp->previous->next = tmp->next;
        tmp->next->previous = tmp->previous;
        current_thread = current_thread->next;
        free(tmp->stack_p);
        free(tmp);
        dispatch();
    }
    else{
        // TODO
        // Hint: No more thread to execute
        free(current_thread->stack_p);
        free(current_thread);
        current_thread = NULL;
        longjmp(env_st, 1);
    }
}
void thread_start_threading(void){
    // TODO
    int s = setjmp(env_st);
    if(s == 0)
        dispatch();
    else
        return;
}
// part 2
void thread_register_handler(int signo, void (*handler)(int)){
    // TODO
    current_thread->sig_handler[signo] = handler;
}
void thread_kill(struct thread *t, int signo){
    // TODO
    t->signo = signo;
}