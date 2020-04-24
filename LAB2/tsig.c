/*
tsig.c

Program to examine behavour of processes created by fork() function,
 and understand mechanism of sending information between them by signals

Michal Skarzynski

24.04.2020
  
*/

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h> // implicit declaration fork / getppid ect...
#include <sys/wait.h>
#include <stdbool.h>

#define WITH_SIGNALS // part 3 WITH 4

bool KEYBOARD_INTERUPT = false;

void		handlerTerminateChild();
void		handlerKeybaordInterupt();
void		sendSignalTermination(pid_t * childs, const unsigned int id);
void		performChildAction();
void		ignoreAllSignals();
void		restoreAllSignals();
unsigned int	waitForChildSynchronization();
void		createChildProcesses(pid_t *child_process_array, unsigned int NUM_CHILD);

/*
Function: handlerTerminateChild
------------------------------- 
Description:
Function will handle SIGTERM signal send to a child
if WITH_SIGNALS is defined function will only prints information about recived such signal
when WITH_SIGNALS is not defined received SIGTERM will result in exiting from function with code 1
*/
void handlerTerminateChild()
{
	printf("\tCHILD[%d]: Received SIGTERM signal.\n", getpid());
	#ifndef WITH_SIGNALS	 // if with part 4 we sholud "only print a message", if not return code - for me its very strange requirement
	int exit_code = 1;	
	printf("\tCHILD[%d]: Exiting with code: %d\n", getpid(), exit_code);	
	exit(exit_code);
	#endif		
}

/*
Function: handlerKeybaordInterupt
------------------------------- 
Description: 
Function will handle SIGINT signal send to a parent after keyboard interrupt occurs
information about this interrupt is stored in global variable KEYBOARD_INTERUPT
*/
void handlerKeybaordInterupt()
{
	printf("\n");
	printf("PARENT[%d]: Received keyboard interrupt\n", getpid());
	KEYBOARD_INTERUPT = true;
}

/*
Function: sendSignalTermination
------------------------------- 
Description: 
Function used by parent process to terminate set of childreen, 
it will send SIGTERM to childs [0 to id-1]
used when afterr creating process ID there occurs error
it means that all signals 0 to ID-1 must be terminated
Used also when keyboard interupt occured -> keyboard interupt is checked BEFORE creation of ID signal,
so all signals between 0 and ID-1 must be terminated
-------------------------------
Arguments:
child_process_array : array of child processes
id : number of process up to which SIGTERM will be send. EXCLUDING ID
*/
void sendSignalTermination(pid_t * child_process_array, const unsigned int id)
{
 	if(id > 0)printf("PARENT[%d]: Sending SIGTERM signal to childs [%d ... %d]\n", getpid(), child_process_array[0], child_process_array[id - 1]);
	sleep(1);	// I don't like it, but there might be a situation in which signal does not have enough time to assign SIGTERM signal to handler and we send it without any effect
	for(unsigned int i = 0; i<id; ++i)
	{
	kill(child_process_array[i], SIGTERM);
	}
}

/*
Function: performChildAction
------------------------------- 
Description: 
Function opened by child process as its first action
it's basically, main() of child process,
*/
void performChildAction()
{
	signal(SIGTERM, handlerTerminateChild); // set callback for child
	printf("\tCHILD[%d]: Started, parent process ID %d\n", getpid(), getppid());


	#ifdef WITH_SIGNALS
	printf("\tCHILD[%d]: Ignoring signal SIGINT\n", getpid());
	signal(SIGINT,SIG_IGN);
	#endif
	
	sleep(10);

	printf("\tCHILD[%d]: Exiting with code 0\n", getpid());
	exit(0);
}

/*
Function: ignoreAllSignals
------------------------------- 
Description: 
Function used by parent process to ignore all signals
it uses _NUM constant to determine number of signals defined in system
*/
void ignoreAllSignals()
{
	printf("PARENT[%d]: Ignoring signals throught 0 to %d\n", getpid(), _NSIG);
	for(int i=0; i <=_NSIG; ++i)signal(i,SIG_IGN);	
}

/*
Function: restoreAllSignals
------------------------------- 
Description: 
Function used by parent process to restore all signals
it uses _NUM constant to determine number of signals defined in system
*/
void restoreAllSignals()
{
	printf("PARENT[%d]: Restoring signals throught 0 to %d\n", getpid(), _NSIG);
	for(int i=0; i <=_NSIG; ++i)signal(i,SIG_DFL);	
}

/*
Function: waitForChildSynchronization
------------------------------- 
Description: 
Function used by parent process to synchronize all child processes
it uses wait() function which stores information about exit code of child process
if wait function returns PID equals to 0 it means that no other processes are needed to synchronize
Function will return number of synchronized childreen processes
-------------------------------
Return value:
Number of terminated child processes
*/
unsigned int waitForChildSynchronization()
{
	unsigned int NUM_CHILD_TERMINATED = 0;
	int terminated_pid = 0;
	do
	{
		int status;
		terminated_pid = wait(&status);
		if(terminated_pid > 0)
		{
			NUM_CHILD_TERMINATED++;
			printf("PARENT[%d]: Process %d terminated with code %d\n",getpid(), terminated_pid, status);
		}

	}while(terminated_pid > 0);

	return NUM_CHILD_TERMINATED;
}

/*
Function: createChildProcesses
------------------------------- 
Description: 
Function used by parent create NUM_CHILD processes
PID of those processes are stored in child_process_array
if WITH_SIGNALS is defined then before creating process we check whether KEYBOARD_INTERUPT is set
if so, we want to send SIGTERM to all signals from 0 to i-1 

function uses fork() function to create new process
if return value of fork() is equal to 0 then it means that we are in child proces, and we can start child action function
if return value of fork() function is greater than zero it means that we are in parent process and we can store this number in child_process_array
if forks returns value -1, less then zero, it means that there is an error so we need to send SIGTERM to all signals between 0 and i-1 
-------------------------------
Arguments:
child_process_array : array to which PID of child processes will be saved
NUM_CHILD : number processes to create, also size of a child_process_array 
*/
void createChildProcesses(pid_t *child_process_array, unsigned int NUM_CHILD)
{
	pid_t process_id = 0;

	for(unsigned int i=0; i<NUM_CHILD; ++i)
	{

		#ifdef WITH_SIGNALS
		if(KEYBOARD_INTERUPT)
		{	
			printf("PARENT[%d]: Creation of processes interupted\n", getpid());

			sendSignalTermination(child_process_array, i);
			break;
		}
		#endif

		if(i>0)sleep(1);
		process_id = fork();

		if(process_id == 0)
		{
			performChildAction();
			break;
		}
		else
		{
			if(process_id > 0)
			{
				//We received child process id from fork function
				child_process_array[i] = process_id;
				printf("PARENT[%d]: Child %d created\n", getpid(), process_id);
			}
			else
			{
				printf("PARENT[%d]: Error on creating child number %d\n", getpid(), i);
				//Error occured after creating process 'i' -> so kill from <0 ... i-1>
				sendSignalTermination(child_process_array, i);
			}
		}
	}

}

int main(int argc, char *argv[]) {
	printf("\nProcess %d started\n", getpid());

	#ifdef WITH_SIGNALS
	printf(" --- WITH_SIGNALS defined (part 3 WITH 4)  ---\n");
	ignoreAllSignals();
	printf("PARENT[%d]: Restoring SIG_DFL handler to SIGCHLD signal\n", getpid());
	signal(SIGCHLD, SIG_DFL); // restoring default for SIGCHLD
	printf("PARENT[%d]: Assigning custom handler to SIGINT signal\n", getpid());
	signal(SIGINT, handlerKeybaordInterupt);
	#endif


	unsigned int NUM_CHILD = 10;
	pid_t  * child_process_array = (pid_t*)malloc(NUM_CHILD * sizeof(pid_t));
	
	createChildProcesses(child_process_array, NUM_CHILD);

	printf("PARENT[%d]: Wait() loop started\n", getpid());
	//Process synchronization with parent
	unsigned int NUM_CHILD_TERMINATED = waitForChildSynchronization();

	printf("PARENT[%d]: There are no more child processes\n", getpid());
	printf("PARENT[%d]: Received %d exit codes\n", getpid(), NUM_CHILD_TERMINATED);
	
	#ifdef WITH_SIGNALS
	restoreAllSignals();
	#endif

	free(child_process_array);
	printf("Process %d ends\n", getpid());
	return 0;
}
