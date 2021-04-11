// Parallel Merge Sort
// Last Edited by Xander Winans 04-05-2021
/*
Purpose: The program demonstrates merge sort using parallel threads. The thread library used is openmp.
         The program creates an array of a given size and fills it with random values.
         Then it uses a given number of threads to each sort their allocated section of the array before merging
         The program then does a parallel tree merge to merge the sorted arrays back together.


How to Run:
    Navigate to where this file is in your teminal
    Compile with    $> g++ -fopenmp pmSort.cpp
    Run with        $> ./a.out N P D
    ..WHERE
       N = size of the array
       P = Number of Processors
       D = (optional) Display [D == 1 turns on display]
    NOTE: For simplicity this program assumes that N % P == 0. It could be changed to accept any combination of N,P
*/


#include <iostream>
#include <omp.h>
#include <iomanip>
using namespace std;

//Globals
int *globalArray;   //Array of size N that will be sorted by threads
int *AUX;           //Array of size N used as storage in the merge step as storage
//User input
int SIZE;           //size of global array
int threadCount;
int workLoad;       //global variable since this calculation is used often
bool requestDisplay;

//Function Headers
void quickSort();                   //used to sort the base case section for each thread
void merge(int, int);               //merges two sections together
void printArray();
bool isSorted();
int Compare(const void *a_p, const void *b_p);  //Provided by Dr. Park, used in qsort


//main will initialize the globalArray (size taken from cmd) with random values < size.
// uses omp to perform merge sort across P threads (P taken from cmd)
// outputs the sorted array (optional), if the array is sorted, and the execution time.
int main(int argc, char *argv[]) {
    if (argc < 3) {
        cout << "Error, missing line arguments.\n";
    } else {
        //Get input from cmd arg
        SIZE = stoi(argv[1]);
        threadCount = stoi(argv[2]);
        requestDisplay=false;

        cout << "N = " << SIZE << endl;
        cout << "Thread Count = " << threadCount << endl;

        globalArray = new int[SIZE];
        AUX = new int[SIZE];
        workLoad = SIZE / threadCount;

        //Get display option.
       if(argc>3)
       {
           if(stoi(argv[3])==1)     // if 1 turn all displays on
               requestDisplay = true;
                                        //else do nothing
       }

        auto startTime = omp_get_wtime(); //start timer
#       pragma omp parallel num_threads (threadCount)
        {
            int myRank = omp_get_thread_num();

            //initialize array section that this thread will work on to random numbers
#         pragma omp parallel for num_threads (threadCount)
            for (int i = myRank * workLoad; i < (myRank + 1) * workLoad; i++) {
                globalArray[i] = (int) random() % SIZE;
            }


            //display the global array
            if(requestDisplay)
            {
                #pragma omp barrier     //ensures the other threads finish initializing their sections
                #pragma omp single      //only one thread needs to display
                printArray();
            }
            //sort the section before merging
            quickSort();

            //Prepare for merging
            int divisor = 2; int coreDiff = 1;
            int myFirstIndex = myRank * workLoad;
            //perform merge
            while (divisor <= threadCount) {
            #pragma omp barrier //barrier to ensure both sender and receiver and ready
                //receiver calls merge
                if (myRank % divisor == 0) {
                    merge(myFirstIndex, myFirstIndex + (coreDiff * workLoad));
                } else {} //sender does nothing
                divisor *= 2;
                coreDiff *= 2;
            }
        }//end pragma omp parallel

        //Display Results
        auto endTime = omp_get_wtime();     //end timer
            if(requestDisplay)
        {
            printArray();
        }
        if(isSorted()) {
            cout << "The list is sorted :) " << endl;
        }
        else cout<<"Uh oh! The list is NOT sorted :(\n";
        cout << "Time Elapsed: " << endTime - startTime <<" seconds"<< endl;
    }
    return 0;
}//end main

// This function is executed by each thread. It copies a section of the global array into an aux array
//     ( The section depends on the thread's rank and is equally distributed )
//  It then performs quicksort array section copy before copying the sorted values back into the global array
// NOTE: quickSort doesn't copy the array since we can call qsort with the starting index, size of where we want to sort.
void quickSort() {
    int myRank = omp_get_thread_num();
    int myFirstIndex = myRank * workLoad;
    if(requestDisplay){  //display sorted
        #pragma omp critical
        {
            cout << "Unsorted Thread #" << myRank << "\t[";
            int i;
            for (i = myFirstIndex; i < myFirstIndex + workLoad - 1; i++) {
                cout <<setw(2)<< globalArray[i] << ", ";
            }
            cout << setw(2)<<globalArray[i] << "]\n";
        }//critical display not sorted
}
    qsort(&globalArray[myFirstIndex], workLoad, sizeof(int), Compare); //quicksort the section

    if(requestDisplay) {
        #pragma omp critical  //display the sorted sub array
        {
            cout << "Sorted BY Thread #" << myRank << "\t[";
            int i;
            for (i = myFirstIndex; i < myFirstIndex + workLoad - 1; i++) {
                cout <<setw(2)<< globalArray[i] << ", ";
            }
            cout << setw(2)<<globalArray[i] << "]\n";
        }//critical display sorted
    }

    //Below is the implementation for copying the array using an openmp parallel for loop
    //this isn't necessary because we pass the starting index and size to qsort to sort the proper section
    //this code is left here to demonstrate that I understand how to copy the array using omp parallel for
    //    int *myQuickAux = new int[workLoad];
    //copy for quick sort
    //#pragma  omp parallel for num_threads(threadCount)
    //    for (int i = 0; i < workLoad; i++) {
    //        myQuickAux[i] = globalArray[myFirstIndex + i];
    //    }
        //copy back
    //    qsort(&myQuickAux,workLoad,sizeof(int),Compare);
    //#pragma  omp parallel for num_threads(threadCount)
    //    for (int i = 0; i < workLoad; i++) {
    //        globalArray[myFirstIndex + i] = myQuickAux[i];
    //    }
}//quickSort()


// The merge function takes as input the starting index of 2 contiguous sections of the global array and
// merges the two sorted sections of the global array back together.
// In order to do this in parallel each threads will not share sections of either the global or auxiliary array.
// The legal section of the array for each thread is calculated with the given indexes
void merge(int myFirstIndex, int mySecondIndex) {
    int size = mySecondIndex - myFirstIndex;
    int myLastIndex = mySecondIndex + size;
    int i = myFirstIndex;
    int j = mySecondIndex;
    int myGlobalIndex = myFirstIndex;
#   pragma omp parallel for num_threads(threadCount)
    //copy array section into corresponding aux
        for (int k = myFirstIndex; k < myLastIndex; k++) {
            AUX[k] = globalArray[k];
        }

    //Select the largest of the two until only one half has contents
    while (i < mySecondIndex && j < myLastIndex) {
        if (AUX[i] <= AUX[j]) {
            globalArray[myGlobalIndex++] = AUX[i++];
        } else if (AUX[i] > AUX[j]) {
            globalArray[myGlobalIndex++] = AUX[j++];
        }
    }
    //copy the remaining values from either array
    //uses parallel for loop
    int remainder = myLastIndex-myGlobalIndex;
    if(i<mySecondIndex) {
#      pragma omp parallel for num_threads(threadCount)
        for (int k = 0; k < remainder; k++) {
            globalArray[myGlobalIndex + k] = AUX[i + k];
        }
    }
    else if(j<myLastIndex) {
#      pragma omp parallel for num_threads(threadCount)
        for (int k = 0; k < remainder; k++) {
            globalArray[myGlobalIndex + k] = AUX[j + k];
        }
    }
}//end merge()

//checks whether the global array is sorted
bool isSorted() {
bool ret = true;
#pragma omp parallel for num_threads(threadCount)
    for (int i = 0; i < SIZE - 1; i++) {
        if (globalArray[i] > globalArray[i + 1]) ret = false;
    }
    return ret;
}

//displays the global array
void printArray() {

    cout << "\n-- The Array-- \n[ ";
    for (int i = 0; i < SIZE - 1; i++) {
        cout << globalArray[i] << ", ";
    }
    cout << globalArray[SIZE - 1] << " ]\n----------------\n";
}

//Comparison function provided by DR.Park
// Function: Compare -- needed for GNU library qsort()'s last parameter
// Purpose:  Compare 2 ints, return -1, 0, or 1, respectively, when the
//           first int is less than, equal, or greater than the 2nd

int Compare(const void *a_p, const void *b_p) {
    int a = *((int *) a_p);
    int b = *((int *) b_p);
    if (a < b) return -1;
    else if (a == b) return 0;
    else return 1; //(a > b)
}