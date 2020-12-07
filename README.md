##Socket_FileSender

#How to run:

**1) First compile and run the server file from terminal**
```
	gcc server.c
	./server
```

**2) Compile and run the Client file.**
```
	gcc client.c -lreadline
	./client
```
	
	*All the files that are downloaded from the server will be saved in Client folder*
	
Assumption:
The percentage display uses line (\n) as a measure of completed task and hence the task is divided accordingly
This takes us to the assumption that each line will have no more than 1024 character in it.
However if required it can be altered as it is defined as #define SIZE

	
Note : 
	In order to display the % gradually increasing i have put a sleep(1) comman after each time percentage is printed

	
