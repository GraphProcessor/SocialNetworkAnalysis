#Implementation Thoughts
##Thread Pool
###Important Status Variables
- is_ready_finishing_, managed by master thread, set true only when all current-stage-tasks finished
- is_finished_, manged by master thread, set true only when current worker-threads are joined

##Breakable Thread Pool
###Traits
- Could not let master thread add tasks when it is break
- Could not let other threads acquire tasks when it is break
- Before Resume the tasks, the master thread is required to reset the is_break flag
###Current Bug
- After Break, the master thread can also add tasks, which make all threads busy again, logical error
- Break in advance, but the flag is_break_ is not set, then the boss is able to add another task
