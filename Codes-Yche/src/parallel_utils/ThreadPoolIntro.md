#Yche-ThreadPool-Intro
##Basic Design
###Master-Thread-Procedures
- Stage Based Procedures
    - AddTask(use a loop of AddTasks)
    - WaitForBreakOrTerminate
- JoinAll(executed after exiting the scope)

###Worker-Thread-Procedures
- DoThreadFunction(Loop Until is_read_finishing flag)
    - Wait when the break flag is set, on task_queue_cond_var
    - Notify the master thread after each execution
- NextTask()
    - Wait when the task_queue is empty, on task_queue_cond_var
    
###Basic Syn Structures
- Task Queue
    - task_queue_lock
    - task_queue_cond_var

- CallBack
    - call_back_lock

- Boss Wait
    - boss_wait_lock
    - boss_wait_cond_var
    
###Scheduling
- Basic Impl
    - the scheduling policy could be implemented with different data structures of task queue.