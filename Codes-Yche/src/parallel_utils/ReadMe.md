#Implementation Thoughts
##Breakable Thread Pool
###Traits
- Could not let master thread add tasks when it is break
- Could not let other threads acquire tasks when it is break
- Before Resume the tasks, the master thread is required to reset the is_break flag
###Current Bug
- After Break, the master thread can also add tasks, which make all threads busy again, logical error
