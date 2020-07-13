/***********************************************************************************/
//***********************************************************************************
//            *************NOTE**************
// This is a template for the subject of RTOS in University of Technology Sydney(UTS)
// Please complete the code based on the assignment requirement.

//AUTHORS: Joseph Cerdan (12544312) & Jacob Elali (13372937)

//***********************************************************************************
/***********************************************************************************/

/*
  To compile prog_1 ensure that gcc is installed and run the following command:
  gcc -Wall -O2 prog_1.c -o prog_1 -lpthread -lrt
*/
#include  <pthread.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <stdio.h>
#include  <sys/types.h>
#include  <fcntl.h>
#include  <string.h>
#include  <sys/stat.h>
#include  <semaphore.h>
#include  <sys/time.h>

/* --- Structs --- */

typedef struct ThreadParams {
  int pipeFile[2];
  sem_t sem_A_to_B, sem_B_to_C, sem_C_to_A;
  char message[255];
  FILE *readFptr, *writeFptr;
  
} ThreadParams;

typedef char buffer_item;
#define BUFFER_SIZE 255

/* --- Prototypes --- */

/* Initializes data and utilities used in thread params */
void initializeData(ThreadParams *params);

/* This thread reads data from data.txt and writes each line to a pipe */
void *ThreadA(void *params);

/* This thread reads data from pipe used in ThreadA and writes it to a shared variable */
void *ThreadB(void *params);

/* This thread reads from shared variable and outputs non-header text to src.txt */
void *ThreadC(void *params);

/* --- Main Code --- */
int main(int argc, char const *argv[]) {
  
  int result;
  pthread_t tid1, tid2, tid3; //Thread ID
  pthread_attr_t attr;
  char file_name[100];


  ThreadParams params;
 

  //OPEN FILE TO BE READ
	/* Verify the correct number of arguments were passed in */
	if (argc != 2) {
		fprintf(stderr, "USAGE:./main.out data.txt\n");
	}

	strcpy(file_name, argv[1]); //copy a string from the command line

  //File open for reading
	if ((params.readFptr = fopen(file_name, "r")) == NULL) {
		printf("Error! opening file");
		// Program exits if file pointer returns NULL.
		exit(1);
	}

  //File open for writing
  if ((params.writeFptr = fopen("output.txt", "w")) == NULL) {
		printf("Error! opening file");
		// Program exits if file pointer returns NULL.
		exit(1);
	}

	// reads text until newline is encountered
	printf("\nReading data file...:\n");

    
  // Initialization
  initializeData(&params);
  pthread_attr_init(&attr);

  // Create pipe
  result = pipe (params.pipeFile);
   if (result < 0){ 
    perror("Error creating pipe\n");
    exit(1); 
    } else {
      printf("Pipe created\n");
      }

  // Create Threads
  if(pthread_create(&tid1, &attr, ThreadA, (void*)(&params))!=0)
  {
	  perror("Error creating thread A \n");
      exit(-1);
  }
  printf("Thread A Created\n");

 if(pthread_create(&tid2, &attr, ThreadB, (void*)(&params))!=0)
  {
	  perror("Error creating thread B \n");
      exit(-1);
  }
  printf("Thread B Created\n");
 
  if(pthread_create(&tid3, &attr, ThreadC, (void*)(&params))!=0)
  {
	  perror("Error creating thread C \n");
      exit(-1);
  }
  printf("Thread C Created\n");
 

  // Wait on threads to finish
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  pthread_join(tid3, NULL);

  
  //CLOSE FILE BEING READ
  fclose(params.readFptr);

  //CLOSE FILE BEING WRITTEN
  fclose(params.writeFptr);

  return 0;

}

void initializeData(ThreadParams *params) {
  // Initialize Sempahores
  if (sem_init(&(params->sem_A_to_B), 0, 1)){
    perror("Error initialising semAtoB semaphore. \n");
  }
  
  //TODO: add your code

  if (sem_init(&(params->sem_B_to_C), 0, 0)){
    perror("Error initialising semBtoC semaphore. \n");
  }
  if (sem_init(&(params->sem_C_to_A), 0, 0)){
    perror("Error initialising semCtoA semaphore. \n");
  }


  return;
}

void *ThreadA(void *params) {
  //TODO: add your code
  
  /* note: Since the data_stract is declared as pointer. the A_threa],&d_params->message */
  ThreadParams *A_thread_params = (ThreadParams *)(params);
  buffer_item line[BUFFER_SIZE];
  
  //Wait for semaphore and line to be read
  while (!sem_wait(&(A_thread_params->sem_A_to_B)) && fgets(line, BUFFER_SIZE, A_thread_params->readFptr) != NULL) {
    
    int result=0;
  
    //display 
    printf("\nData: %s", line);
    
    //write into pipe
    result = write(A_thread_params->pipeFile[1], line, strlen(line));
    if (result!=1){ 
        perror ("A> Writing data to pipe"); 
    }


    //memset(line, 0, BUFFER_SIZE);
    sem_post(&(A_thread_params->sem_B_to_C));
    
	}

  //Close writing pipe
  close(A_thread_params->pipeFile[1]);

  exit(1);

  return 0;

}

void *ThreadB(void *params) {
  //TODO: add your code

  ThreadParams *B_thread_params = (ThreadParams *)(params);

  while(!sem_wait(&(B_thread_params->sem_B_to_C))){
    int result=0;

    //Read from pipe
    result = read(B_thread_params->pipeFile[0], B_thread_params->message, BUFFER_SIZE);

    if (result == -1){
      printf("pipe error");
    }
    if (result != 1) {
      perror("B> Reading from pipe and placing in shared variable");
    }

    sem_post(&(B_thread_params->sem_C_to_A));
  
  }

  //Close writing pipe
  close(B_thread_params->pipeFile[0]);

  return 0;

}

void *ThreadC(void *params) {
  //TODO: add your code
  ThreadParams *C_thread_params = (ThreadParams *)(params);
  int signalend = 0;
  char check[100] = "end_header\n";

  while(!sem_wait(&(C_thread_params->sem_C_to_A))){
    
    //write into output.txt if after end header
    if(signalend == 1){
      fprintf(C_thread_params->writeFptr, "%s", C_thread_params->message);
      printf("C> Writing line to output from shared variable: %s", C_thread_params->message);
      fflush(C_thread_params->writeFptr);
    } else if (strcmp(C_thread_params->message, check)==0){ //check end header line
      signalend = 1;
    }
    // reset char array to empty
    memset(C_thread_params->message, 0, BUFFER_SIZE);
    sem_post(&(C_thread_params->sem_A_to_B));
    printf("Signal end: %d\n", signalend);
  }

  return 0;

}
