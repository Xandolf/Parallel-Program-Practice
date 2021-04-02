// Create By Xander Winans
// Last updated 2021-04-01
// The purpose of this program is to compute a global sum using parallel processing
// This program takes in thread_count as a command line argument
// Using thread_count # of threads it initializes an array of size 500,000,000 then sums their values
// 2 Mutex are used to ensure the program validity

/* How to run this program
 ***** Note: This program was created on Ubuntu 20.04.2, if you are using a different OS the steps may vary
 *  Navigate to the location of this file in the terminal
 *  Compile the program with  $> g++ -o xxx global_sum.cpp -pthread
 ******* You may replace xxx with a name that you see fit
 *  Run the program with      $> ./xxx  P
 ******* You must replace P with the number of threads wanted. Choose P to be a factor of 500,000,000
 * ***** Suggested P values are {1,2,4,8,16}
 *
*/

#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <sys/time.h>
using namespace std;

//Get_Time Macro provided by Dr. Park
//macro (in-line expansion) for GET_TIME(double)
#define GET_TIME(now)\
{ struct timeval t; gettimeofday(&t, NULL);\
now = t.tv_sec + t.tv_usec/1000000.0; }
//end macro


//global variables accessible by all threads
int global_array[500000000];        //values to sum up, a[i] = i+1
long int global_sum;
int thread_count;                   //taken from command line arg
pthread_mutex_t mutex_display;      //mutual exclusion for cout statements
pthread_mutex_t mutex_sum;          //mutual exclusion for global_sum

//prototype for thread function that computes partial sum and adds it to global_sum
void *sum_slave(void *rank);

int main(int argc, char *argv[]) {
    double startTime, endTime, timeElapsed;
    long thread_id;                 //long allows typecasting to void*
    if (argc > 1) {                 //ensure thread_count was given

        thread_count = atoi(argv[1]);
        pthread_t myThreads [thread_count];                         //defines needed threads
        pthread_mutex_init (&mutex_display, nullptr);       //initializes mutex
        pthread_mutex_init (&mutex_sum, nullptr);

        //Start timer
        GET_TIME(startTime);

        //create threads by passing sum_slave function and parameter thread_id
        for (thread_id = 0; thread_id < thread_count; thread_id++) {
            pthread_create(&myThreads[thread_id], nullptr, sum_slave, (void *) thread_id);
        }

        //join threads after they finish their function
        for (thread_id = 0; thread_id < thread_count; thread_id++) {
            pthread_join (myThreads[thread_id], nullptr);
        }

        //End Timer
        GET_TIME(endTime);
        timeElapsed = endTime - startTime;

        //Output the results. Main thread doesn't need mutex since all child threads have been joined/finished
        cout<<"Global Sum: "<<global_sum<<endl;
        cout<<"Number of Threads: "<<thread_count<<endl;
        cout<<"Time Elapsed: "<<timeElapsed<<" seconds"<<endl;


    } else cout << "\nMissing commandline argument\n";
    return 0;
}

// The slave function uses its rank determine the scope of its first and last index
// It then computes the partial sum of the global_array using those indexes
// It then adds its partial sum to the global sum
void* sum_slave(void* rank)
{
    int myRank = (long) rank;
    long int partialSum = 0;
    //double adouble = 0;

    int myWorkload = 500000000/thread_count;    //assume global_array.size % thread_count == 0
    int myFirstI = myRank * myWorkload;
    int myLastI = myFirstI + myWorkload-1;

   // Initialize the array in parallel
    for(int i = myFirstI; i<=myLastI; i++)
    {
        global_array[i]=i+1;
    }

    //computes partial sum
    for(int i = myFirstI; i<=myLastI; i++)
    {
        partialSum+=global_array[i];
    }

    pthread_mutex_lock(&mutex_display);         //reserve mutex before displaying thread info
    cout<<"----------------\n";
    cout<<"Thread ID: " <<myRank<<endl;
    cout<<"Start Index: " <<myFirstI<<endl;
    cout<<"Last Index: " << myLastI-1<<endl;
    cout<<"Partial Sum: "<<partialSum<<endl;


    cout<<"________________\n";
    pthread_mutex_unlock(&mutex_display);       //free mutex for display (cout)

    pthread_mutex_lock(&mutex_sum);             //reserve mutex for updating global_sum
    global_sum+=partialSum;
    pthread_mutex_unlock(&mutex_sum);           //free mutex for global_sum
    return nullptr;
}
