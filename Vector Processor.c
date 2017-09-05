


#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <sys/resource.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>


/* 
 * Halston Raddatz
 * 
 * 
 * Streamed Vector Processing
 * The intended usage of this project is to write a C program that perform subtraction 
 * on two streams of binary numbers. 
 * 
 */

int initial_pause = 1;
int pause_variable = 0;

//function prototypes
void adder(char *numberFromPipe, char *numberFromFile, char *total, int bitLength);
void complementer(char *binaryInput, int bitLength);
void incrementer(char *binaryInput, int bitLength);
void sigHandler(int sigNum);

//signal handler for breaking from pause in program
void sigHandler (int sigNum) {
   if (initial_pause == 1) {
      initial_pause = 0;
   }
}


void adder(char *numberFromPipe, char *numberFromFile, char *total, int bitLength){

   int carryBit = 0;

   int i;
   for (i = bitLength; i >= 0; i--) {

      //subtracting '0' from a char, due to ASCII mapping, will give us the int representation of our char
      //numberFromFile[i] accounts for the binary binaryInput currently in our pipe  
      int token1 = numberFromPipe[i] - '0';
      //numberFromFile[i] accounts for the binary binaryInput from vectorDataFile1
      int token2 = numberFromFile[i] - '0';

      int additionResult = carryBit + token1 + token2;

      //if numberFromFile[i], numberFromFile[i], and carryBit are 0, adding them will
      //result in 0 with no need for an additional carryBit bit
      if (additionResult == 0 ) {
         carryBit = 0;

      //if one of either numberFromFile[i], numberFromFile[i], or the carryBit are 1 then
      //our total is 1, still no need for a carryBit bit
      } else if (additionResult == 1) {
         numberFromPipe[i] = '1';
         carryBit = 0;

      //if two of numberFromFile[i], numberFromFile[i], or the carryBit are 1 then our total is 2,
      //we set our numberFromFile[i] to 0 and carryBit out a 1 via the carryBit variable
      } else if (additionResult == 2) {
         numberFromPipe[i] = '0';
         carryBit = 1;

      //if all 3 of numberFromFile[i], numberFromFile[i], and the carryBit are 1 then our total is 3.
      //we set our numberFromFile[i] to 1 and carryBit out a 1 via the carryBit variable
      } else if (additionResult == 3) {
         numberFromPipe[i] = '1';
         carryBit = 1;
      }
   }
}
//function that does the work of incrementing supplied binary binaryInput
//works by looping through digits of the char array,
//starting from right/last slot in array. if value is 0, just
//swap for 1 and break from the loop. if value is 1, turn the
//value 0 and keep looping until a position with 0 is found.
void incrementer(char *binaryInput, int bitLength) {

   int i;
   for (i = bitLength - 1; i >= 0; i--) {
      if (binaryInput[i] == '0') {
         binaryInput[i] = '1';
         break;
      } else {
         binaryInput[i] = '0';
      }
   }
}


//function that does the work of complementing binary numbers
//parameters: pointer to char array (binary binaryInput) & length of that binary #
//works by looping through every digit of the char array, starting from left,
//using the supplied length parameter and flipping 1s to 0s and 0s to 1s
void complementer(char *binaryInput, int bitLength) {

   int i;
   for (i = 0; i < bitLength; i++) {
      if (binaryInput[i] == '1') {
         binaryInput[i] = '0';
      } else {
         binaryInput[i] = '1';
      }
   }
}


int main(int argc, char **argv) {

   pid_t pid;

   //"installing" signal handler
   signal (SIGINT, sigHandler);

   if (argc != 4) {
      fprintf(stdout, "//// BAD PARAMETERS. SHOULD BE (((FILE1))) (((FILE2))) (((BITS)))\n");
      exit(1);
   }

    //storing arguements from user
    char *vectorDataFile1 = argv[1];
    char *vectorDataFile2 = argv[2];

    //atoi() function converts a string into an integer & returns that integer
    //converting the users arguement (bit size) from a string to an int
    int inputBitLength = atoi(argv[3]);

    fprintf(stdout, ".......\n");
    fprintf(stdout, "//// ACCEPTED PARAMETERS, STARTING PROGRAM USING ((( %s ))) & ((( %s))) AND SUPPLIED BINARY LENGTH OF ((( %d BITS ))) \n", 
		     vectorDataFile1, vectorDataFile2, inputBitLength);
    fprintf(stdout, ".......\n");

    int status;

   //setting up pipes
   //The arrays pipe1 & pipe2 is used to return two file descriptors referring to the ends of the pipe. 
   //pipe1[0] & pipe2[0] refer to the read end of their pipes, respectively. Pipe_one[1] & pipe2[1] 
   //refers to the write end of the pipe. 
   //Data written to the write end of the pipe is buffered by the kernel until it is read from the read end of the pipe.
   int pipe1[2];
   int pipe2[2];
   pipe(pipe1);
   pipe(pipe2);

   printf("//// Parent PID#:  %d\n", getpid());

   int i;
   for (i = 0; i < 2; i++) {


     //System call fork() is used to create processes. It takes no arguments and returns a process ID. 
     //The purpose of fork() is to create a new process, which becomes the child process of the caller. 
     //After a new child process is created, both processes will execute the next instruction following the fork() system call.
     pid = fork();

     //fork() returns a zero to the newly created child process.
     //if new child, do this
     if (pid == 0) {
         printf("//// SPAWNED CHILD W/ PID#:  %d\n", getpid());
       //  printf("interator = %d\n", i);


	//if first run of loop, we need to create the increment
         if (i == 0) {
	
      
            // Spawn Increment Process
  	    printf("//// PROCESS W/ PID#: %d SET AS INCREMENTOR\n", getpid());
            char pipeBuffer[inputBitLength + 1];

	    //closing the write-end of pipe1
            close(pipe1[1]);
	    //closing the read-end of pipe2
            close(pipe2[0]);

            while(1) {
	
	       //int  read(  int  handle,  void  *buffer,  int  nbyte );
	       //The read() function attempts to read nbytes from the file associated with handle, 
	       //and places the characters read into buffer.
	       //If no process has the pipe open for writing, read() shall return 0 to indicate end-of-file.
               if (read(pipe1[0], pipeBuffer, sizeof(pipeBuffer) + 1) == 0) {
                 break;
               }

               fprintf(stdout, "[INCREMENTER][PID# %d] READ %s FROM [PIPE]      ", getpid(), pipeBuffer);

               incrementer(pipeBuffer, inputBitLength);

                fprintf(stdout, "[INCREMENTER][PID# %d] WROTE %s TO [PIPE] \n", getpid(), pipeBuffer);

	       //int  write(  int  handle,  void  *buffer,  int  nbyte  )
	       //The write() function attempts to write nbytes from buffer to the file associated with handle.
               //The function returns the binaryInput of bytes written to the file. A return value of -1 indicates an error
               write(pipe2[1], pipeBuffer, sizeof(pipeBuffer) + 1);
            }

	    //closing the read-end of pipe1
            close(pipe1[0]);
	    //closing the write-end of pipe2
            close(pipe2[1]);


         } else {

            // Spawn Adder Process
            FILE *input_file, *output_file;
	    printf("//// PROCESS W/ PID#: %d SET AS ADDER\n", getpid());

            char pipeBuffer[inputBitLength + 1];
            char fileBuffer[inputBitLength + 1];

            //closing the write-end of pipe1	   
            close(pipe1[1]);
	    //closing the read-end of pipe1
            close(pipe1[0]);
 	    //closing the write-end of pipe2
            close(pipe2[1]);

            input_file = fopen(vectorDataFile1, "r");
            output_file = fopen("output.dat", "w");

            while(1) {

               read(pipe2[0], pipeBuffer, sizeof(pipeBuffer) + 1);

               if (fgets(fileBuffer, inputBitLength + 3, input_file) == NULL) {
                  break;
               }

               // clean input of newlines and CR
               fileBuffer[strcspn(fileBuffer, "\r\n")] = 0;

	       fprintf(stdout, "[ADDER][PID# %d] READ %s FROM [PIPE]     ", getpid(), pipeBuffer);
	       fprintf(stdout, "[ADDER][PID# %d] READ %s FROM [FILE_A]   ", getpid(), fileBuffer);

               char total[inputBitLength + 1];

               adder(pipeBuffer, fileBuffer, total, inputBitLength);

	       fprintf(stdout, "[ADDER][PID# %d] WROTE %s TO [OUTPUT.DAT]    \n", getpid(), pipeBuffer);
               fprintf(output_file, "%s\n", pipeBuffer);
            }

            close(pipe2[0]);
         }

         exit(0);
      }
   }

   // Parent Process - Complementer
   printf("//// PROCESS W/ PID#: %d SET AS COMPLEMENTER\n", getpid());
   FILE *input_file;

   char file_buffer[inputBitLength + 1];

   //closing the read-end of pipe1 
   close(pipe1[0]);
   //closing the read-end of pipe2
   close(pipe2[0]);
   //closing the write-end of pipe2
   close(pipe2[1]);

   input_file = fopen(vectorDataFile2, "r");


   //pausing the program before any arithmetic or piping is done
   while (initial_pause) {
            ;
   }


   //The C library function char *fgets(char *str, int n, FILE *stream) reads a line from the 
   //specified stream and stores it into the string pointed to by str. 
   //input_file is the maximum binaryInput of characters to be read (including the final null-character and new line)
   //On success, the function returns the same str parameter. If the End-of-File is encountered
   //and no characters have been read, the contents of str remain unchanged and a null pointer is returned.
   //If an error occurs, a null pointer is returned.
   while(fgets(file_buffer, inputBitLength + 3, input_file) != NULL) {

      // remove newline and any carrage returns (windows)
      file_buffer[strcspn(file_buffer, "\r\n")] = 0;

      fprintf(stdout, "[COMPLEMENTER][PID# %d] READ %s FROM [FILE_B]   ", getpid(), file_buffer);

      complementer(file_buffer, inputBitLength);

      fprintf(stdout, "[COMPLEMENTER][PID# %d] WROTE %s TO [PIPE]\n", getpid(), file_buffer);


      //int  write(  int  handle,  void  *buffer,  int  nbyte  )
      //The write() function attempts to write nbytes from buffer to the file associated with handle.
      //The function returns the binaryInput of bytes written to the file. A return value of -1 indicates an error
      write(pipe1[1], file_buffer, sizeof(file_buffer) + 1);

   }

   close(pipe1[1]);


   //pid_t waitpid(pid_t pid, int *stat_loc, int options);
   //The waitpid() function shall be equivalent to wait() if the pid argument is (pid_t)-1 and the options argument is 0. 
   //Otherwise, its behavior shall be modified by the values of the pid and options arguments.
   //equivalent to wait(&status), suspends execution until status is changed
   waitpid(-1, &status, 0);
   return 0;
}



