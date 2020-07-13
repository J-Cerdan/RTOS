/**********************************************************************************

           *************NOTE**************
This is a template for the subject of RTOS in University of Technology Sydney(UTS)
Please complete the code based on the assignment requirement.

Assignment 3 Program_2

Author: Joseph Cerdan (12544312)

**********************************************************************************/
/*
  To compile prog_1 ensure that gcc is installed and run the following command:
  gcc -Wall -O2 program_2.c -o prog_2 -lpthread -lrt
  gcc -Wall -O2 Assignment3_template_Prg_2.c -o prog_2 -lpthread -lrt

*/
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

//Number of pagefaults in the program
int pageFaults = 0;

//Function declaration
void SignalHandler(int signal);

/**
 Main routine for the program. In charge of setting up threads and the FIFO.

 @param argc Number of arguments passed to the program.
 @param argv array of values passed to the program.
 @return returns 0 upon completion.
 */
int main(int argc, char* argv[])
{
	//Register Ctrl+c(SIGINT) signal and call the signal handler for the function.
	signal(SIGINT, SignalHandler);
	//add your code here
	
	/* Check for input parameters */
	if (argc != 2){
		fprintf(stderr,"usage: ./prog_2 number  \n<note: The frame number for execution>\n");
		return -1;
  	}

	/* Ensure frame number is not less than or equal to 0 */
  	if(atoi(argv[1]) <= 0){
		fprintf(stderr,"frame number %d must be > 0\n",atoi(argv[1]));
		return -1;
  	}
	
    int i;
	// reference number
	int REFERENCESTRINGLENGTH=24;
	//Argument from the user on the frame size, such as 4 frames in the document
	int frameSize = atoi(argv[1]);
	//Frame where we will be storing the references. -1 is equivalent to an empty value
	uint frame[frameSize];
	//Reference string from the assignment outline
	int referenceString[24] = {7,0,1,2,0,3,0,4,2,3,0,3,0,3,2,1,2,0,1,7,0,1,7,5};
	//Next position to write a new value to.
	int nextWritePosition = 0;
	//Boolean value for whether there is a match or not.
	bool match = false;


	//Initialise the empty frame with -1 to simulate empty values.
	for(i = 0; i < frameSize; i++)
	{
		frame[i] = -1;
	}

	//Loop through the reference string values.
	for(i = 0; i < REFERENCESTRINGLENGTH; i++)
	{

		printf("VALUE: %d ", referenceString[i]);

		//Loop to check if there is a match
		for (int j=0; j<frameSize; j++){

			if (referenceString[i] == frame[j]){
				match = true;
				break;
			}
    	}

		//No match triggers First in First out algortithm 
		if (!match){
			frame[nextWritePosition] = referenceString[i];
			nextWritePosition++;
			nextWritePosition %= frameSize; //Modulus to keep track of writeposition within 4 values
			pageFaults++;

			printf("Page Fault Number: %d ", pageFaults);
		} 
		else {
			printf("No fault.            ");
		}
		
		//Print frame to user
		printf("Current Frame: [ ");
		for (int i=0; i<frameSize; i++){
			printf("%d ", frame[i]);
		}
		printf("]\n");

		//Match condition reset
		match = false;
		
	}

	//Sit here until the ctrl+c signal is given by the user.
	while(1)
	{
		sleep(1);
	}

	return 0;
}

/**
 Performs the final print when the signal is received by the program.

 @param signal An integer values for the signal passed to the function.
 */
void SignalHandler(int signal)
{
	printf("\nTotal page faults: %d\n", pageFaults);
	exit(0);
}


