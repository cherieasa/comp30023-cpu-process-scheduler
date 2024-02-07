# comp30023-cpu-process-scheduler
Process Scheduling for a simulator that allocates CPU to the running processes. 

## Process Schedulers

### Single processor scheduler
- Processes will be allocated CPU time according to shortest-time-remaining algorithm, a preemptive scheduling algorithm.
- Process ids are used to break ties (favoring smaller ids)

### Two processor scheduler
- Extend previous single processor scheduler to work with 2 processors in parallel (for parallelisable processes)
- For non parallelisable processes - assigned to queue with the smallest amount of remaining execution time for all processes and subprocesses

### N processor scheduler
- Generalise the 2 processor setting to N >= 3 processors

## Performance
- Turnaround time: average time (in seconds, rounded up to an integer) between the time when theprocess completed and when it arrived
- Time overhead: maximum and average time overhead when running a process, both rounded to thefirst two decimal points, where overhead is defined as the turnaround time of the process divided by its total execution time (i.e., the one specified in the process description file)
- Makespan: the time in seconds when your simulation ended
