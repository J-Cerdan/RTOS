/***********************************************************************************/
//***********************************************************************************
//            *************NOTE**************
// This is a template for the subject of RTOS in University of Technology Sydney(UTS)
// Please complete the code based on the assignment requirement.

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
  
} ThreadParams;

typedef char buffer_item;
#define BUFFER_SIZE 100

/* global vairable declare */
buffer_item buffer[BUFFER_SIZE];/* the buffer */

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
  char c[100];
	FILE* fptr;
	char file_name[100];
	int sig;
	char check[100] = "end_header\n";


  ThreadParams params;

  //OPEN FILE TO BE READ
	/* Verify the correct number of arguments were passed in */
	if (argc != 2) {
		fprintf(stderr, "USAGE:./main.out data.txt\n");
	}

	strcpy(file_name, argv[1]); //copy a string from the commond line


	if ((fptr = fopen(file_name, "r")) == NULL) {
		printf("Error! opening file");
		// Program exits if file pointer returns NULL.
		exit(1);
	}

	// reads text until newline is encountered
	printf("reading from the file:\n");

  sig = 0;

	while (fgets(params.message, sizeof(params.message), fptr) != NULL) {
		//if the program read to the end of header
		//then, read the data region and print it to the console (sig==1)
		if (sig == 1)
			fputs(params.message, stdout);

		/* check whether this line is the end of header,
		the new line in array c contains "end_header\n"*/
		if (!sig && strstr(params.message, check)!=NULL)
		{
		//Yes. The end of header
			sig = 1;
		}			
	}


    
  // Initialization
  initializeData(&params);
  pthread_attr_init(&attr);
  

  // Create pipe
  result = pipe (params.pipeFile);
   if (result < 0){ perror("pipe error");exit(1); }

  // Create Threads
  if(pthread_create(&tid1, &attr, ThreadA, (void*)(&params))!=0)
  {
	  perror("Error creating threads A: ");
      exit(-1);
  }

 if(pthread_create(&tid2, &attr, ThreadB, (void*)(&params))!=0)
  {
	  perror("Error creating threads B: ");
      exit(-1);
  }
  if(pthread_create(&tid3, &attr, ThreadC, (void*)(&params))!=0)
  {
	  perror("Error creating threads C: ");
      exit(-1);
  }
  //TODO: add your code



  sem_post(&params.sem_A_to_B); break; //unlock semaphore_one
 

  // Wait on threads to finish
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  pthread_join(tid3, NULL);
  //TODO: add your code

  //CLOSE FILE BEING READ
  fclose(fptr);

  return 0;
}

void initializeData(ThreadParams *params) {
  // Initialize Sempahores
  sem_init(&(params->sem_A_to_B), 0, 1);
  
  //TODO: add your code

  sem_init(&(params->sem_B_to_C), 0, 1);
  sem_init(&(params->sem_C_to_A), 0, 1);


  /* Get the default attributes */
  //pthread_attr_init(&attr);

  return;
}

void *ThreadA(void *params) {
  //TODO: add your code
  
  /* note: Since the data_stract is declared as pointer. the A_thread_params->message */
  ThreadParams *A_thread_params = (ThreadParams *)(params);
  sem_wait(&(A_thread_params->sem_A_to_B));
  int i,upper=atoi(A_thread_params->message);

  printf("ThreadA\n");

  //Write to pipe
  int j=0, k=0;
  int result;
  buffer_item item[BUFFER_SIZE];

  printf ("In writing thread\n");

  /* copy the input string into local variables*/
  strcpy(item, A_thread_params->message);

  // THIS CODE SENDS CHAR BY CHAR
  
  while(item[j]!='\0')
  {
    result=write (A_thread_params->pipeFile[1], &item[j], 1);
    if (result!=1){ 
      perror ("write"); 
	    exit (2);
	  }

	printf("%c", item[j]);
	j++;
  }
  
  /* add the '\0' in the end of the pipe */
  result=write (A_thread_params->pipeFile[1], &item[j], 1);
  if (result!=1){ 
    perror ("write"); 
	  exit (3);
    }
	
   printf("\nwriting pipe has finished\n"); 

  sem_post(&(A_thread_params->sem_B_to_C));

}

void *ThreadB(void *params) {
  //TODO: add your code

  ThreadParams *B_thread_params = (ThreadParams *)(params);
  sem_wait(&(B_thread_params->sem_B_to_C));
  int i,upper=atoi(B_thread_params->message);
  printf("ThreadB\n");

  //Read from pipe and send to C
  while(1){
      char    ch;
      int     result;

      result = read (B_thread_params->pipeFile[0],&ch,1);
      if (result != 1) {
        perror("read");
        exit(4);
      }
      
      if(ch !='\0')	{
      printf ("Reader: %c\n", ch);}
      else {
      printf("reading pipe has completed\n");
      exit (5);}
  }
  // put into chararray and send to c?????

  sem_post(&(B_thread_params->sem_C_to_A));

}

void *ThreadC(void *params) {
  //TODO: add your code
  ThreadParams *C_thread_params = (ThreadParams *)(params);
  sem_wait(&(C_thread_params->sem_C_to_A));
  int i,upper=atoi(C_thread_params->message);

  printf("ThreadC\n");

  //Analyse and put into output.txt



  sem_post(&(C_thread_params->sem_A_to_B));

}
