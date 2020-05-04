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
  gcc -Wall -O2 test.c -o prog_1 -lpthread -lrt

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
#define BUFFER_SIZE 255

/* global vairable declare */
buffer_item buffer[BUFFER_SIZE];/* the buffer */
char file_name[100];

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
  ThreadParams params;

  // Initialization
  initializeData(&params);
  pthread_attr_init(&attr);

   /* Verify the correct number of arguments were passed in */
   if (argc != 2) { fprintf(stderr, "USAGE:./main.out data.txt\n"); exit(-1);}

   strcpy(file_name, argv[1]); //copy a string from the command line data.txt


  // Create pipe
  result = pipe (params.pipeFile);
   if (result < 0){ perror("pipe error"); exit(1); } 
   else { printf("Pipe created...\n");}

  // Create Threads
  if(pthread_create(&tid1, &attr, ThreadA, (void*)(&params))!=0)
  {
	  perror("Error creating threads A: ");
      exit(-1);
  }
  printf("Thread A Created\n");

 if(pthread_create(&tid2, &attr, ThreadB, (void*)(&params))!=0)
  {
	  perror("Error creating threads B: ");
      exit(-1);
  }
  printf("Thread B Created\n");
  if(pthread_create(&tid3, &attr, ThreadC, (void*)(&params))!=0)
  {
	  perror("Error creating threads C: ");
      exit(-1);
  }
  printf("Thread C Created\n");
  //TODO: add your code

 
  // Wait on threads to finish
  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  pthread_join(tid3, NULL);
  //TODO: add your code

  //release sephamores
  sem_destroy(&params.sem_A_to_B);
  sem_destroy(&params.sem_B_to_C);
  sem_destroy(&params.sem_C_to_A);

  return 0;
}

void initializeData(ThreadParams *params) {
  // Initialize Sempahores with error handlers, unlocking sephamore A_to_B first
  if(sem_init(&(params->sem_A_to_B), 0, 1)) { perror("Error initialising sephamore A_to_B\n");}
  if(sem_init(&(params->sem_B_to_C), 0, 0)) { perror("Error initialising sephamore B_to_C\n");}
  if(sem_init(&(params->sem_C_to_A), 0, 0)) { perror("Error initialising sephamore C_to_A\n");}

  return;
}

void *ThreadA(void *params) 
{
  
  /* note: Since the data_struct is declared as pointer.The A_thread_params->message */
  ThreadParams *A_thread_params = (ThreadParams *)(params);
  
  //locking sephamore 
  sem_wait(&(A_thread_params->sem_A_to_B));
  //File reading for data.txt with error handler
  FILE* data;
  data = fopen(file_name, "r");
  if(data == NULL){ printf("Error recieving data\n"); exit(1); }

  printf("\nThread A initialised\n");
  printf("Writing to pipe...\n");


  while (fgets(A_thread_params->message, sizeof(A_thread_params->message), data) != NULL) {
    //display received data
    printf("> %s", A_thread_params->message);

   //Writing data
    int result;
    result = write(A_thread_params->pipeFile[1], &(A_thread_params->message), sizeof(A_thread_params->message));
    if(result != sizeof(A_thread_params->message)){
      perror("Writing error - A\n");
      exit(1);
    }  
	}
//write indicator for end of pipe
  strcpy(A_thread_params->message, "\0");
  int result = write(A_thread_params->pipeFile[1], &(A_thread_params->message), sizeof(A_thread_params->message));
  if(result != sizeof(A_thread_params->message)){
    perror("Writing error - A");
    exit(1);
  }
 printf("\nWriting complete\n"); 
 fclose(data);
  //release sephamore lock for next thread
  sem_post(&(A_thread_params->sem_B_to_C));
    
return 0;
}

void *ThreadB(void *params) {
 //TODO: add your code



  ThreadParams *B_thread_params = (ThreadParams *)(params);



  //wait to aquire full lock

  sem_wait(&(B_thread_params->sem_B_to_C));

  printf("\nThread B initialised\n");

  printf("Data read\nWriting to pipe...\n");

  //continiously read until end of pipe

  while(1){

    int result;



    //read from pipe and error checking

    result = read(B_thread_params->pipeFile[0], &(B_thread_params->message), sizeof(B_thread_params->message));

    if(result != sizeof(B_thread_params->message)){

      perror("Reading error - B");

      exit(1);

    }



    //using written indicator \0 to assess pipe end

    if(strcmp(B_thread_params->message, "\0") != 0){

      //display received data

      printf("> %s", B_thread_params->message);



      //write to pipe and error checking

      result = write(B_thread_params->pipeFile[1], &(B_thread_params->message), sizeof(B_thread_params->message));

      if(result != sizeof(B_thread_params->message)){

        perror("writing error - B");

        exit(1);

      }

    }

    else {

      //end

      printf("\nWriting complete\n\n");



      //write "\0" to the end of the pipe

      strcpy(B_thread_params->message, "\0");

      int result = write(B_thread_params->pipeFile[1], &(B_thread_params->message), sizeof(B_thread_params->message));

      if(result != sizeof(B_thread_params->message)){

        perror("writing error - B");

        exit(1);

      }



      //release the semaphore lock for next thread

      sem_post(&(B_thread_params->sem_C_to_A));



      return 0; 

    }

  }
}

void *ThreadC(void *params) {


  ThreadParams *C_thread_params = (ThreadParams *)(params);
  //flag for checking end_header

  int sig = 0; 

  //comparator variable to signal end_header

  char comparator[12] = "end_header\n";

  //locking sephamore 

  sem_wait(&(C_thread_params->sem_C_to_A));

  printf("Thread C initialised\n");

 
  //creating output file and error handler
  FILE* output; 
  output = fopen("output.txt", "w");
  if(output == NULL) {printf("Error creating file\n");exit(1);}

  



  //executes until pipe finish

  while(1){

    int result;



    //read from pipe and error checking

    result = read(C_thread_params->pipeFile[0], &(C_thread_params->message), sizeof(C_thread_params->message));

    if(result != sizeof(C_thread_params->message)){

      perror("Reading error - C");

      exit(1);

    }



    //Checking if end of pipe signalled by \0

    if(strcmp(C_thread_params->message, "\0") != 0){
      //display recieved data

      printf(">%s", C_thread_params->message);

      

      //put required into output file if sig is flagged

      if(sig == 1) {

        fputs(C_thread_params->message, output);

      }



      //check if end of header, if so sig will flag

      if(strcmp(C_thread_params->message, comparator) == 0 && (sig == 0)){

        sig = 1;
       printf("\nEnd_header detected, writing to output...\n");


      }

    }


    else{

      printf("\nWriting to output complete, success!\n");



      //release the sephamore lock for reuse

      sem_post(&(C_thread_params->sem_A_to_B));



      //closing output file

      fclose(output);

      return 0;

    }

  }




  

}