#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <fcntl.h>

int CUSTOMERS = 6;

int N0, N1, N2, M;
#define SEM_BARBER_0_FREE 0 
#define SEM_BARBER_1_FREE 1
#define SEM_BARBER_2_FREE 2

#define SEM_BARBER_0_NOTIFY 3
#define SEM_BARBER_1_NOTIFY 4
#define SEM_SEATS 5
#define ASSIGNMENT 6 

unsigned int sem_num = 7;

struct SharedMemory{
	int N0_free;
	int N1_free;
	int N2_free;
	int M_free;
};

struct SharedMemory * shared_memory = NULL;


int sem_id, shm_id;
key_t sem_key, shm_key;

void input(int args, char ** argsv);

void fillSharedMemory()
{
	shared_memory = (struct SharedMemory *)shmat(shm_id, NULL, 0);
	if (shared_memory == NULL) {
    		perror("shmat");
	   	exit(1);
	}
}

void connectSharedMemory()
{

	if ((sem_key = ftok("barber", 58)) == (key_t) -1)
    	perror("IPC error: ftok");
	
	if ((sem_id = semget( sem_key , sem_num, 0666 )) == -1)
	perror("semget");

	if ((shm_key = ftok("barber", 1)) == (key_t) -1)
    	perror("IPC error: ftok");

	if ((shm_id = shmget(shm_key, sizeof(struct SharedMemory), 0666 )) == -1)
        printf("main: shmget() failed\n");

}

void input(int args, char ** argsv)
{
	if(args != 5)
	{
		printf("Entrer N0, N1, N2 and M\n");		
		scanf("%d %d %d %d", &N0 , &N1, &N2, &M);
	}
	else
	{
		N0 = atoi(argsv[1]);
		N1 = atoi(argsv[2]);
		N2 = atoi(argsv[3]);
		M = atoi(argsv[4]);
	}

	(shared_memory->N0_free) = N0;
	(shared_memory->N1_free) = N1;
	(shared_memory->N2_free) = N2;
	(shared_memory->M_free) = M;
}

void generateSharedMemory()
{

//---------------- CREATE SHARED SEMAPHORS
	if ((sem_key = ftok("barber", 58)) == (key_t) -1)
    	perror("IPC error: ftok");
	
	if ((sem_id = semget (IPC_PRIVATE , sem_num,  IPC_CREAT | 0666 )) == -1)
	perror("semget");
	

//---------------- CREATE SHARED MEMORY
	if ((shm_key = ftok("barber", 1)) == (key_t) -1)
    	perror("IPC error: ftok");

	if ((shm_id = shmget(shm_key , sizeof(struct SharedMemory),  IPC_CREAT | 0666 )) == -1)
        printf("main: shmget() failed\n");

	printf("sem_id = %d, allocated (%lu)\t shm_id=%d\n", sem_id,sizeof(struct SharedMemory), shm_id);

}

void semOp(int sem_num, int op)
{
	struct sembuf sb;
	sb.sem_num = sem_num;
	sb.sem_op = op;
	sb.sem_flg = 0;


	if (semop (sem_id, &sb, 1) == -1)
    	perror("semop; op");

}

void semOp2(int sem_num0, int sem_num1, int op)
{
	struct sembuf sb[2];

	sb[0].sem_num = sem_num0;
	sb[0].sem_op = op;
	sb[0].sem_flg = 0;

	sb[1].sem_num = sem_num1;
	sb[1].sem_op = op;
	sb[1].sem_flg = 0;


	if (semop (sem_id, sb, 2) == -1)
    	perror("semop; op");

}
//-------------------------------

void color(int c)
{
	int code = c % 6;

	switch(code)	
	{
	case 0: printf("\033[1;31m"); break;
	case 1: printf("\033[1;32m"); break;
	case 2: printf("\033[1;33m"); break;
	case 3: printf("\033[1;34m"); break;
	case 4: printf("\033[1;35m"); break;
	case 5: printf("\033[1;36m"); break;
	}
}

void colorReset()
{
	printf("\033[0m");
}

void terminateProcess(int code)
{
	shmdt(shared_memory);
	exit(code);
}


void haircut(int type, int sex)
{
	color(getpid());
	printf("Client: [%d]{%d} [%d, %d, %d | %d] getting haircut from type %d\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free), type);
	colorReset();	
	if(type == -1)terminateProcess(0);

	sleep(random()%5);

	switch(type)
	{
	case 0:
	{
	// MALE BARBER
	semOp(SEM_BARBER_0_FREE, -1);
	(shared_memory->N0_free) = (shared_memory->N0_free) + 1;
	semOp(SEM_BARBER_0_FREE, 1);

	semOp(SEM_BARBER_0_NOTIFY, 1); // notify about posibility to haircut for male
	color(getpid());	
	printf("Client: [%d]{%d} [%d, %d, %d | %d] Notified male\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free));
	colorReset();	
	break;
	}

	case 1:
	{
	//FEMALE BARBER
	semOp(SEM_BARBER_1_FREE, -1);
	(shared_memory->N1_free) = (shared_memory->N1_free) + 1;
	semOp(SEM_BARBER_1_FREE, 1);

	semOp(SEM_BARBER_1_NOTIFY, 1); // notify about posibility to haircut for female
	color(getpid());		
	printf("Client: [%d]{%d} [%d, %d, %d | %d] Notified female\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free));
	colorReset();	
	break;
	}

	case 2:
	{
	//UNISEX BARBER
	semOp(SEM_BARBER_2_FREE, -1);
	(shared_memory->N2_free) = (shared_memory->N2_free) + 1;
	semOp(SEM_BARBER_2_FREE, 1);

	semOp2(SEM_BARBER_0_NOTIFY, SEM_BARBER_1_NOTIFY, 1); // notify for posibility for both sexes
	color(getpid());
	printf("Client: [%d]{%d} [%d, %d, %d | %d] Notified both\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free));
	colorReset();
	break;
	}
	default: terminateProcess(0); break;
	}
	color(getpid());
	printf("Client: [%d]{%d} [%d, %d, %d | %d] goes away with cool new haircut, from barber type %d\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free), type);
	colorReset();	
	terminateProcess(0);
}

int assignToBarber(int sex)
{
	if(sex == 0)
	{
	//MALE
		semOp(SEM_BARBER_0_FREE, -1); // mutex for N0_free checking taken
		if((shared_memory->N0_free) > 0)
		{
			(shared_memory->N0_free) = (shared_memory->N0_free) - 1;
			semOp(SEM_BARBER_0_NOTIFY, -1); // notify about taken posibility
			semOp(SEM_BARBER_0_FREE, 1); // mutex for N0_free checking released -> haircut
			color(getpid());
			printf("Client: [%d]{%d} [%d, %d, %d | %d] taken barber type 0\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free));
			colorReset();
			return 0;
		}
		else
		{
			semOp(SEM_BARBER_0_FREE, 1); // mutex for N0_free checking released -> haircut
		
			semOp(SEM_BARBER_2_FREE, -1); // mutex for N2_free checking taken
			if((shared_memory->N2_free) > 0)
			{
				(shared_memory->N2_free) = (shared_memory->N2_free) - 1;
				semOp2(SEM_BARBER_0_NOTIFY, SEM_BARBER_1_NOTIFY, -1); // notify both sexes about taken posibility
				semOp(SEM_BARBER_2_FREE, 1); // mutex for N2_free checking released -> haircut
				color(getpid());
				printf("Client: [%d]{%d} [%d, %d, %d | %d] taken barber type 2\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free));
				colorReset();
				return 2;
			}
			semOp(SEM_BARBER_2_FREE, 1); // mutex for N2_free checking released -> goes to waiting room
		}

	}
	else
	{
	//FEMALE
		semOp(SEM_BARBER_1_FREE, -1); // mutex for N1_free checking taken
		if((shared_memory->N1_free) > 0)
		{
			(shared_memory->N1_free) = (shared_memory->N1_free) - 1;
			semOp(SEM_BARBER_1_NOTIFY, -1); // notify about taken posibility
			semOp(SEM_BARBER_1_FREE, 1); // mutex for N1_free checking released
			color(getpid());			
			printf("Client: [%d]{%d} [%d, %d, %d | %d] taken barber type 1\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free));
			colorReset();
			return 1;
		}
		else
		{
			semOp(SEM_BARBER_1_FREE, 1); // mutex for N1_free checking released -> goes to waiting room

			semOp(SEM_BARBER_2_FREE, -1); // mutex for N2_free checking taken
			if((shared_memory->N2_free) > 0)
			{
				(shared_memory->N2_free) = (shared_memory->N2_free) - 1;
				semOp2(SEM_BARBER_0_NOTIFY, SEM_BARBER_1_NOTIFY, -1); // notify both sexes about taken posibility
				
				semOp(SEM_BARBER_2_FREE, 1); // mutex for N2_free checking released
				color(getpid());
				printf("Client: [%d]{%d} [%d, %d, %d | %d] taken barber type 2\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free));
				colorReset();
				return 2;
			}
			semOp(SEM_BARBER_2_FREE, 1); // mutex for N2_free checking released -> goes to waiting room
		}


	}
	return -1;
}

void customer()
{

	fillSharedMemory();

	int sex = random()%2;

	if(sex == 0)
	{
		if(N0 + N2 == 0)
		{
		color(getpid());
		printf("Client: [%d]{%d} [%d, %d, %d | %d] Goes away - there are no suitable barbers at all\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free) );	
		colorReset();
		terminateProcess(0);		
		}
	}
	else
	{
		if(N1 + N2 == 0)
		{
		color(getpid());
		printf("Client: [%d]{%d} [%d, %d, %d | %d] Goes away - there are no suitable barbers at all\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free) );	
		colorReset();
		terminateProcess(0);		
		}
	}

	color(getpid());
	printf("Client: [%d]{%d} [%d, %d, %d | %d] Goes to barber\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free) );
	colorReset();

	semOp(ASSIGNMENT, -1);
	int type = assignToBarber(sex);
	semOp(ASSIGNMENT, 1);

	if(type != -1)haircut(type, sex); 

	semOp(SEM_SEATS, -1); // mutex for seats taken
	// WAITING ROOM
	if((shared_memory->M_free) > 0)
	{

	(shared_memory->M_free) = (shared_memory->M_free) - 1;
	color(getpid());
	printf("Client: [%d]{%d} [%d, %d, %d | %d] Waits in waiting room\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free) );
	colorReset();	
	semOp(SEM_SEATS, 1);// mutex for seats realeased -> keeps waiting PRZECIEZ TA OPERACJA NIE MOZE ZATRZYMYWAC PROCESU W ZADNYM PRZEYPADKU!!!!!
	}
	else
	{
	semOp(SEM_SEATS, 1);// mutex for seats realeased -> goes away
	color(getpid());
	printf("Client: [%d]{%d} [%d, %d, %d | %d] Cannot wait - no chairs in waiting room\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free) );
	colorReset();	
	terminateProcess(0);
	}


	if(sex == 0)
	{
		semOp2(SEM_BARBER_0_NOTIFY, ASSIGNMENT, -1); // wait for notification

		color(getpid());
		printf("Client: [%d]{%d} ASSIGNMENT NOTIFIED\n", getpid(), sex);
 		colorReset();

		semOp(SEM_BARBER_0_NOTIFY, 1);
		int type = assignToBarber(sex);
		color(getpid());
		printf("Client: [%d]{%d} [%d, %d, %d | %d] Notified, barber type %d assigned\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free) , type);
 		colorReset();
		semOp(ASSIGNMENT, 1);	
		

		semOp(SEM_SEATS, -1); // mutex for seats taken	
		(shared_memory->M_free) = (shared_memory->M_free) + 1;
		semOp(SEM_SEATS, 1); // mutex for seats release	

		haircut(type, 0);
	
		
	}
	else
	{
		semOp2(SEM_BARBER_1_NOTIFY, ASSIGNMENT, -1); // wait for notification

		color(getpid());
		printf("Client: [%d]{%d} ASSIGNMENT NOTIFIED\n", getpid(), sex);
 		colorReset();

		semOp(SEM_BARBER_1_NOTIFY, 1);
		int type = assignToBarber(sex);

		color(getpid());
		printf("Client: [%d]{%d} [%d, %d, %d | %d] Notified, barber type %d assigned\n", getpid(), sex, (shared_memory->N0_free),(shared_memory->N1_free), (shared_memory->N2_free), (shared_memory->M_free) , type);
		colorReset();
		semOp(ASSIGNMENT, 1);

		semOp(SEM_SEATS, -1); // mutex for seats taken	
		(shared_memory->M_free) = (shared_memory->M_free) + 1;
		semOp(SEM_SEATS, 1); // mutex for seats release	

		haircut(type, 1);
	}

	
	terminateProcess(0);
}

int main(int args, char ** argsv)
{

	printf("PID[%d]\n", getpid());

	srandom(time(NULL));
	generateSharedMemory();// num of semafores
	fillSharedMemory();

	input(args, argsv);

	//initial conditions
	semOp(SEM_BARBER_0_FREE, 1);
	semOp(SEM_BARBER_1_FREE, 1);
	semOp(SEM_BARBER_2_FREE, 1);

	semOp(SEM_BARBER_0_NOTIFY, N0 + N2);
	semOp(SEM_BARBER_1_NOTIFY, N1 + N2);
	semOp(SEM_SEATS, 1);
	semOp(ASSIGNMENT, 1);


	for(unsigned int i=0; i<CUSTOMERS; ++i)
	{
		//sleep(random()%3);
		pid_t process_id = 0;
		process_id = fork();

		if(process_id == 0)
		{
			customer();
			break;
		}
		else
		{
			colorReset();	
			if(process_id > 0)
			printf("PARENT[%d]: Child %d created\n", getpid(), process_id);
			else
			printf("PARENT[%d]: Error on creating child number %d\n", getpid(), i);
		}
	}
	

	int terminated_pid = 0;
	do
	{
		int status;
		terminated_pid = wait(&status);
	}while(terminated_pid > 0);


	shmctl(shm_id,IPC_RMID, NULL); // delete shared memory

	printf("PARENT[%d]: END \n", getpid());

	return 0;
}
