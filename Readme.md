Project dispatcher

General

This program simulate a real call 911 dispatcher. Every call that been received is assign to 
The proper department. If this department has no resources the dispatcher will look for 
Resources in other department. If it fail the call waits on the list waiting for free resources. 

The departments

- Police 
- Fire
- Corona
- Ambulance
Each department has limited resources (Cars).

The dispatcher sends the calls to the department's queue according the call type.
The department's task read from queue that dedicated accordingly to her. 
The department check if there are free car she assign the call to the specific car.
The task assign to the car handle time between 5 to 10 sec and mark the car as busy.
For every car there is a timer, when time passed the callback function set the car 
Status to available car.

Log file
 Every event are written to log_file.txt with time stamp.





