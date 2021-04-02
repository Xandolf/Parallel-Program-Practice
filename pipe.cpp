/* Last Updated 02/07/2021 By Xander Winans
 * -- Program Summary --
 * The Purpose of this program is to demonstrate message passing using UNIX pipe
 * The program will time the execution of two implementations of Fibonacci function in parallel
 **** Recursive vs Iterative
 * The program is based on the SPMD (Single Program, Multiple Data) task parallel model
 **** (1 parent and 3 children)
 ****** Parent takes N from command line arg and creates children
 * **** Child 1 send N to children [2,3]. Receives and outputs [2,3]'s exe time
 ****** Child 2 receives N, times fiboRec(N), sends time to child 1.
 ****** Child 3 receives N, times fiboIt(N), sends time to child 1.

 * -- How to Run the program --
 *  This program was created on Ubuntu and requires a c++ compiler.
 *  If you are using a different operating system this process may be different.
 *
 *  Using a terminal navigate to directory where this file is located.
 *  Compile this program with the command   $> g++ pipe.cpp
 *  Run the program with the command        $> ./a.out  N
 ****** IMPORTANT : N here is an argument passed to a.out. Replace it with an integer
 ********** For example: $> ./a.out 5
 *
 * */
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include "cstdlib"
using namespace std;

long fiboIter(int N);
long fiboRec(int N);

//Get_Time Macro provided by Dr. Park
//macro (in-line expansion) for GET_TIME(double)
#define GET_TIME(now)\
{ struct timeval t; gettimeofday(&t, NULL);\
now = t.tv_sec + t.tv_usec/1000000.0; }
//end macro

int main(int argc, char *argv[]) {

    int pid, status, i, N, nForChild2, nForChild3;
    int p1[2], p2[2], p3[2], p4[2];         //pipe descriptors
    pipe(p1);    pipe(p2);    pipe(p3);    pipe(p4);
    double time1, time2, childOfTime1, childOfTime2, startTimeRec,startTimeIt, stopTimeRec,stopTimeIt;

    if(argc > 1)
    {
        N = atoi(argv[1]);
       // cout<< "N="<<N;
    }
    else{
        cout<<"No valid argument for N. Set to 5 by default."<<endl;
        N=5;
    };
    // One loop for each new process
    for (i = 1; i < 4; i++) {
        pid = fork();
        int myI = i;
        if (pid == 0 && myI == 1) //The first child needs to use a pipe to tell the other children the value of n
        {
            write(p1[1], &N, sizeof(int));     //send N to second child   (Recursion)
            write(p2[1], &N, sizeof(int));     //send N to third child  (Iterative)

            read(p4[0], &time2, sizeof(double));    //read time from Child 3
            read(p3[0], &time1, sizeof(double));    //read time from Child 2
            cout << "Time for Recursive version: " << time1 <<" Seconds"<< endl;
            cout << "Time for Iterative version: " << time2 <<" Seconds"<<endl;
            exit(0);
        }
        else if (pid == 0 && myI == 2) //The second child reads the number from the pipe and calculates Fibb Rec
        {
            read(p1[0], &nForChild2, sizeof(int));         //read N from first child

            GET_TIME(startTimeRec);
            long resultRec = fiboRec(nForChild2);
            GET_TIME(stopTimeRec);
            childOfTime1 = stopTimeRec - startTimeRec;
            cout<<"Fibonacci Recursive: ("<<nForChild2<<"): "<< resultRec<<endl;

            write(p3[1], &childOfTime1, sizeof(double));   //write time to first child
            exit(0);

        }
        else if (pid == 0 && myI == 3) //The third child reads the number from the pipe and calculates Fibb  Iter
        {
            read(p2[0], &nForChild3, sizeof(int)); //read N from first child

            GET_TIME(startTimeIt);
            long resultIt = fiboIter(nForChild3);
            GET_TIME(stopTimeIt);

            childOfTime2 = stopTimeIt - startTimeIt;
            write(p4[1], &childOfTime2, sizeof(double)); // Child 3 writes time to Child 1
            cout<<"Fibonacci Iterative: ("<<nForChild3<<"): "<< resultIt<<endl;
            exit(0);
        }
    }//for  --------
    //wait for the 3 child process to finish
    for (i = 0; i < 3; i++) {
        pid = wait(&status);
        cout << "Child (pid:=" << pid << ") has exited! status = " << status << endl;
    }
    return 0;
}//end main

//Code given in class by Dr. Park adapted to c++
// Computes Fibonacci Iteratively
long fiboIter(int N) {
    long f1 = 1;
    long f2 = 1;
    if (N == 1 or N == 2)
        return 1;
    else {
        long temp;
        for (int i = 3; i <= N; i++) {
            temp = f1 + f2;
            f1 = f2;
            f2 = temp;
        }
        return temp;
    }
}

//Code given in class by Dr. Park adapted to c++
// Computes Fibonacci Recursively
long fiboRec(int N) {
    if (N == 1 or N == 2) return 1;
    else return (fiboRec(N - 1) + fiboRec(N - 2));
}