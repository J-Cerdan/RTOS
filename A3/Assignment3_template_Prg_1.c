/**********************************************************************************

           *************NOTE**************
This is a template for the subject of RTOS in University of Technology Sydney(UTS)
Please complete the code based on the assignment requirement.

Assignment 3 Program_1

Author: Joseph Cerdan (12544312)

**********************************************************************************/
/*
  To compile prog_1 ensure that gcc is installed and run the following command:
  gcc -Wall -O2 program_1.c -o prog_1 -lpthread -lrt
	gcc -Wall -O2 Assignment3_template_Prg_1.c -o prog_1 -lpthread -lrt


*/

#include <pthread.h> 	/* pthread functions and data structures for pipe */
#include <unistd.h> 	/* for POSIX API */
#include <stdlib.h> 	/* for exit() function */
#include <stdio.h>	/* standard I/O routines */
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>

typedef struct SRTF_Params {
  //add your variables here
	int pid;//process id
	int arrive_t, wait_t, burst_t, turnaround_t, remain_t;//process time
} Process_Params;

//Max number of processes
#define PROCESSNUM 7

//Index variable
int i;
//Averages calculated
float avg_wait_t = 0.0, avg_turnaround_t = 0.0;
//Semaphore
sem_t sem_SRTF;
//Pthreads
pthread_t thread1, thread2;
//File Name
char file_name[100];

/*---------------------------------- Functions -------------------------------*/
//Create process arrive times and burst times, taken from assignment details
void input_processes(Process_Params *params);
//Schedule processes according to SRTF rule
void process_SRTF(Process_Params *params);
//Simple calculate average wait time and turnaround time function
void calculate_average();
//Read average wait time and turnaround time from fifo then write to output.txt
void read_FIFO();
//Print results, taken from sample
void print_results(Process_Params *params);
//Send and write average wait time and turnaround time to fifo
void send_FIFO(); 

//Thread 1 of assignment
void *worker1(void *params);
//Thread 2 of assignment
void *worker2();

/* this function calculates CPU SRTF scheduling, writes waiting time and turn-around time to th FIFO */
void *worker1(void *params)
{
   // add your code here
   	input_processes(params);
	process_SRTF(params);
	calculate_average();
	print_results(params);
	send_FIFO();

	return 0;
}

/* reads the waiting time and turn-around time through the FIFO and writes to text file */
void *worker2()
{
   // add your code here
   	sem_wait(&sem_SRTF);
	read_FIFO();

	return 0;
}

/* this main function creates named pipe and threads */
int main(int argc, char const *argv[])
{

	/* Verify the correct number of arguments were passed in */
	if (argc != 2) {
		fprintf(stderr, "USAGE:./prog_1.out data.txt\n");
	}

	strcpy(file_name, argv[1]); //copy a string from the command line

	printf("Output File Name: %s\n", file_name);


	/* initialize the parameters */

	//Array of processes with 1 extra for placeholder remain_t
	Process_Params processes[8];

	if(sem_init(&sem_SRTF, 0, 0)!=0)
	{
	    printf("semaphore initialize error\n");
	    return(-10);
	}
	
	/* create threads */
	
	if(pthread_create(&thread1, NULL, worker1, (void*)(&processes))!=0)
 	{
	    printf("Thread 1 created error\n");
	    return -1;
	}
	if(pthread_create(&thread2, NULL, worker2, NULL)!=0)
	{
	    printf("Thread 2 created error\n");
	    return -2;
	}
	
	if(pthread_join(thread1, NULL)!=0)
	{
	    printf("join thread 1 error\n");
	    return -3;
	}
	if(pthread_join(thread2, NULL)!=0)
	{
	    printf("join thread 2 error\n");
	    return -4;
	}
	
	if(sem_destroy(&sem_SRTF)!=0)
	{
	    printf("Semaphore destroy error\n");
	    return -5;
	}

	/* wait for the thread to exit */
	
	return 0;
}

void input_processes(Process_Params *processes) {
	processes[0].pid = 1; processes[0].arrive_t = 8; processes[0].burst_t = 10;
	processes[1].pid = 2; processes[1].arrive_t = 10; processes[1].burst_t = 3;
	processes[2].pid = 3; processes[2].arrive_t = 14; processes[2].burst_t = 7;
	processes[3].pid = 4; processes[3].arrive_t = 9; processes[3].burst_t = 5;
	processes[4].pid = 5; processes[4].arrive_t = 16; processes[4].burst_t = 4;
	processes[5].pid = 6; processes[5].arrive_t = 21; processes[5].burst_t = 6;
	processes[6].pid = 7; processes[6].arrive_t = 26; processes[6].burst_t = 2;
	
	//Initialise remaining time to be same as burst time
	for (i = 0; i < PROCESSNUM; i++) {
		processes[i].remain_t = processes[i].burst_t;
	}
}

//Schedule processes according to SRTF rule
void process_SRTF(Process_Params *processes) {
	
    int endTime, smallest, time, remain = 0;
	
    //Placeholder remaining time to be replaced
    processes[7].remain_t=9999;
	
    //Run function until remain is equal to number of processes
    for (time = 0; remain != PROCESSNUM; time++) {
		
	//Assign placeholder remaining time as smallest
        smallest = 7;
		
	//Check all processes that have arrived for lowest remain time then set the lowest to be smallest
        for (i=0;i<PROCESSNUM;i++) {
            if (processes[i].arrive_t <= time && processes[i].remain_t < processes[smallest].remain_t && processes[i].remain_t > 0) {
                smallest = i;
            }
        }
		
	//Decrease remaining time as time increases
        processes[smallest].remain_t--;
		
	//If process is finished, save time information, add to average totals and increase remain
        if (processes[smallest].remain_t == 0) {
			
            remain++;
			
            endTime=time+1;
			
	    processes[smallest].turnaround_t = endTime-processes[smallest].arrive_t;
			
	    processes[smallest].wait_t = endTime-processes[smallest].burst_t-processes[smallest].arrive_t;
			
	    avg_wait_t += processes[smallest].wait_t;
			
	    avg_turnaround_t += processes[smallest].turnaround_t;
        }
    }
	
}

//Simple calculate average wait time and turnaround time function
void calculate_average() {
	avg_wait_t /= PROCESSNUM;
	avg_turnaround_t /= PROCESSNUM;
}

//Print results, taken from sample
void print_results(Process_Params *processes) {
	
	printf("Process Schedule Table: \n");
	
	printf("\tProcess ID\tArrival Time\tBurst Time\tWait Time\tTurnaround Time\n");
	
	for (i = 0; i<PROCESSNUM; i++) {
	  	printf("\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", processes[i].pid, processes[i].arrive_t, processes[i].burst_t, processes[i].wait_t, processes[i].turnaround_t);
	}
	
}

//Send and write average wait time and turnaround time to fifo
void send_FIFO() {
	int res, fifofd;
	int result = 0;

	char * myfifo = "/tmp/myfifo1";

	/* creating a named pipe(FIFO) with read/write permission */
	res = mkfifo(myfifo, 0777);
	
	if (res < 0) {
		printf("mkfifo error\n");
		exit(0);
	}
	
	sem_post(&sem_SRTF);
	
	//Open file for writing
	fifofd = open(myfifo, O_WRONLY);
	
	if (fifofd < 0) {
		printf("fifo open send error\n");
		exit(0);
	}
	
	//Write to FIFO
	result = write(fifofd, &avg_wait_t, sizeof(avg_wait_t));
	if (result == -1){
		perror ("Error: writing data to FIFO");
	}
	result = write(fifofd, &avg_turnaround_t, sizeof(avg_turnaround_t));
	if (result == -1){
		perror ("Error: writing data to FIFO");
	}

	printf("\nWriting into FIFO(%s): Average wait time: %fs\n", myfifo, avg_wait_t);
	
	printf("\nWriting into FIFO(%s): Average turnaround time: %fs\n", myfifo, avg_turnaround_t);

	close(fifofd);
	
	unlink(myfifo);
}

//Read average wait time and turnaround time from fifo then write to output.txt
void read_FIFO() {
	int fifofd;
	int result = 0;

	FILE *writeFptr;

	char * myfifo = "/tmp/myfifo1";
	
	float fifo_avg_turnaround_t, fifo_avg_wait_t;
	
	
	//Open file for reading
	fifofd = open(myfifo, O_RDONLY);
	
	if (fifofd < 0) {
		printf("fifo open read error\n");
		exit(0);
	}
	
	//Read from FIFO
	result = read(fifofd, &fifo_avg_wait_t, sizeof(int));
	if (result == -1){
		perror ("Error: reading data from FIFO");
	}
	result= read(fifofd, &fifo_avg_turnaround_t, sizeof(int));
	if (result == -1){
		perror ("Error: reading data from FIFO");
	}
	
	printf("\nRead from FIFO(%s): %fs Average wait time\n", myfifo, fifo_avg_wait_t);
	printf("\nRead from FIFO(%s): %fs Average turnaround time\n", myfifo, fifo_avg_turnaround_t);

		//File open for writing
	if ((writeFptr = fopen("output.txt", "w")) == NULL) {
		printf("Error! opening file");
		// Program exits if file pointer returns NULL.
		exit(1);
	}

	fprintf(writeFptr, "%fs Average wait time\n", fifo_avg_wait_t);
	fprintf(writeFptr, "%fs Average turnaround time\n", fifo_avg_turnaround_t);
		
	close(fifofd);
	
	remove(myfifo);

	fclose(writeFptr);
}
