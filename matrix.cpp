// Last updated by Xander Winans on 2021-03-22
// The purpose of this program is to perform matrix multiplication in parallel using pthreads
//      it takes cmd line input for threadCount, L, M, N where
//          A is a L x M matrix
//          B is a M x N matrix

// To run the program:
//      navigate to where this file is located in your terminal
//      compile with    $> g++ matrix.cpp -pthread
//      run with        $> ./a.out P L M N
//            *NOTE: Replace P L M N with numbers chosen for threadCount and matrix dimensions*
// **This program was created using Ubuntu 20.04.2, if you are using a different OS the process may vary**

#include <iostream>
#include <iomanip>
#include <chrono>       //for time checking
#include <pthread.h>
using namespace std;

/*--Global Variables--*/
int threadCount;                    //how many threads will perform the computation
int L, M, N;                        //used for dimensions of matrices
//The matrices
unsigned long int **A;               //LxM matrix
unsigned long int **B;               //MxN matrix
unsigned long int **BT;              //transposed matrix for B with dimensions NxM
unsigned long int **C;               //resulting LxN matrix
//Time keeping
chrono::high_resolution_clock::time_point startTime;
chrono::high_resolution_clock::time_point endTime;
//Mutex to lock output
pthread_mutex_t mutex_display;

//function headers
void *slaveFunction(void *r);   //each thread computes corresponding values for C
void printResults();            //outputs the results including time.

//main function takes input from the cmd line indicating the number of threads and matrix dimensions for A, B
//      it initializes the matrixs before calling the parallel slave functions to compute C
int main(int argc, char *argv[]) {

    if (argc < 5) {
        cout << "Missing command line arguments" << endl;
    } else {
        threadCount = atoi(argv[1]);                      //get values from cmd line arg
        L = atoi(argv[2]);
        M = atoi(argv[3]);
        N = atoi(argv[4]);
        startTime = chrono::high_resolution_clock::now();

        long thread_id;                                         // long allows typecasting to void*
        pthread_t myThreads[threadCount];
        pthread_mutex_init(&mutex_display, nullptr);

        //initialize matrix A
        A = new unsigned long int *[L];
        C = new unsigned long int *[L];
        for (int i = 0; i < L; i++) {
            A[i] = new unsigned long int[M];
            for (int j = 0; j < M; j++) {
                A[i][j] = i + j + 1;
            }
        }
        //Initialize B matrix
        B = new unsigned long int *[M];
        for (int i = 0; i < M; i++) {
            B[i] = new unsigned long int[N];
            for (int j = 0; j < N; j++) {
                B[i][j] = i + j;
            }
        }

        //Transpose B into BT
        //  This is done because C++ is a row major language, this leads to less cache misses during matrix multiplication
        //  B is transposed (rotated and flipped) so that its columns (used in AxB) are stored contiguously rather than the rows.
        BT = new unsigned long int *[N];
        for (int i = 0; i < N; i++) {
            BT[i] = new unsigned long int[M];
            for (int j = 0; j < M; j++) {
                BT[i][j] = B[j][i];
            }
        }

        //create pthreads
        for (thread_id = 0; thread_id < threadCount; thread_id++) {
            pthread_create(&myThreads[thread_id], nullptr, slaveFunction, (void *) thread_id);
        }

        //join threads after they finish their function
        for (thread_id = 0; thread_id < threadCount; thread_id++) {
            pthread_join(myThreads[thread_id], nullptr);
        }
        endTime = chrono::high_resolution_clock::now();
        printResults();
    }
}


//slaveFunction computes the C matrix using matrix multiplication
//  The values of C that it computes depends on the number of threads
//  the load balancing is done cyclically
void *slaveFunction(void *r) {
    int myRank = (long) r;
    pthread_mutex_lock(&mutex_display);
    cout << "Thread ID: " << myRank << "\t Starting index:" << myRank << "\t Step" << threadCount << endl;
    pthread_mutex_unlock(&mutex_display);


    for (int i = myRank; i < L; i += threadCount) {
        C[i] = new unsigned long int[N];
        for (int j = 0; j < N; j++) {
            int value = 0;
            for (int k = 0; k < M; k++) {
                value += A[i][k] * BT[j][k];
            }
            C[i][j] = value;
        }
    }
    return nullptr;
}

//printResults outputs the information based on the current run
// It outputs the cmd line input values (threadCount, L, M, N)
//      and the Upper-Left 10x10 values of the C matrix
//      and the Bottom right 10x10 values of the C matrix
//      and the total time needed to compute C
void printResults() {

    cout << "----------------\nInput\n"
         << setw(9) << "#Threads: " << threadCount << endl
         << "L:" << L << "\tM:" << M << "\tN:" << N << endl;

    cout << "--------------------------------\nResults\n";


    cout << "\tUpper-Left most 10 X 10 Matrix:\n-------------------------------\n";

    for (int i = 0; i < 10 && i < L; i++) {
        for (int j = 0; j < 10 && j < N; j++) {
            if (j != 0)cout << ", ";
            cout << setprecision(5) << (double) C[i][j];
        }
        cout << endl;
    }
    cout << "-------------------------------\n";

    cout << "\tBottom-Right most 10 X 10 Matrix\n-------------------------------\n";
    for (int i = L - 10; i > 0 && i < L; i++) {
        for (int j = N - 10; j > 0 && j < N; j++) {
            if (j != N - 10)cout << ", ";
            cout << setprecision(5) << (double) C[i][j];
        }
        cout << endl;
    }
    cout << "-------------------------------\n";
    auto timeElapsed = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    cout << "----------------\nTotal Time Elapsed: " << timeElapsed.count() / (float) 1000 << " seconds\n";
}