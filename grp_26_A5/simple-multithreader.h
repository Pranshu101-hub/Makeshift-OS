#include <iostream>
#include <list>
#include <functional>
#include <stdlib.h>
#include <cstring>
#include <chrono>
#include <pthread.h>

int user_main(int argc, char **argv);

//arguments for 1D Parallel for
typedef struct {
  int low;
  int high;
  std::function<void(int)> lambda;
} thread_args_1;

//thread function for 1D Parallel for
void *thread_func_1(void *ptr){
  thread_args_1 *t = static_cast<thread_args_1*> (ptr);
  int i = t->low;
  while (i < t->high)
  {
    t->lambda(i); i++; // lambda execution
  }
  return NULL;
}

//arguments for 2D Parallel for
typedef struct 
{
  int low_1, low_2;
  int high_1, high_2;
  std::function<void(int, int)> lambda;
} thread_args_2;

//thread function for 2D Parallel for
void *thread_func_2(void *ptr){
  thread_args_2 *t = static_cast<thread_args_2*> (ptr);
  int i = t->low_1;
  while (i < t->high_1){
    int j = t->low_2;
    while(j < t->high_2){
      t->lambda(i,j); //lambda execution
      j++;
    }
    i++;
  }
  return NULL;
}

//1D Parallel for implementation
void parallel_for(int low, int high, std::function<void(int)> && lambda, int NTHREADS){

  auto start_time = std::chrono::high_resolution_clock::now();

  pthread_t tid[NTHREADS];
  thread_args_1 args[NTHREADS];
  int chunk = (high - low) / NTHREADS; //size of each chunk

  int i = 0;
  while (i < NTHREADS){
    //ranges for each thread
    args[i].low = i * chunk;
    args[i].high = (i+1) * chunk;
    args[i].lambda = lambda;
    if (i == NTHREADS-1){
      args[i].high = high;
    }
    //create thread
    if (pthread_create(&tid[i], nullptr, thread_func_1, &args[i]) != 0){
      std::cerr<<"Error in creating thread" << i << std::endl;
      std::exit(EXIT_FAILURE);
    }
    i++;
  }

  //join threads
  i = 0;
  while (i < NTHREADS){
    if(pthread_join(tid[i], nullptr) != 0){
      std::cerr<<"Error in joing the thread" << i << std::endl;
      std::exit(EXIT_FAILURE);
    }
    i++;
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed_time = end_time - start_time;
  std::cout << "Execution time: " << elapsed_time.count() << " seconds\n";
}

//2D Parallel for implementation
void parallel_for(int low_1, int high_1, int low_2, int high_2, std::function<void(int, int)> &&lambda, int numThreads) {
    pthread_t tid[numThreads];
    thread_args_2 args[numThreads];
    int chunk_1 = (high_1 - low_1) / numThreads; //size of each chunk in first dimension
    
    auto start_time = std::chrono::high_resolution_clock::now(); 

    int i = 0;
    while (i < numThreads) {
      //ranges for each thread
        args[i].low_1 = low_1 + i * chunk_1;
        args[i].high_1 = (i == numThreads - 1) ? high_1 : args[i].low_1 + chunk_1;
        args[i].low_2 = low_2;
        args[i].high_2 = high_2;
        args[i].lambda = lambda;

        //create thread
        if (pthread_create(&tid[i], nullptr, thread_func_2, &args[i]) != 0) {
            std::cerr << "Error creating thread " << i << std::endl;
            std::exit(EXIT_FAILURE);
        }
        ++i;
    }
    //join threads
    i = 0;
    while (i < numThreads) {
        if (pthread_join(tid[i], nullptr) != 0) {
            std::cerr << "Error joining thread " << i << std::endl;
            std::exit(EXIT_FAILURE);
        }
        ++i;
    }

    auto end_time = std::chrono::high_resolution_clock::now();  
    std::chrono::duration<double> elapsed_time = end_time - start_time;

    std::cout << "Execution time: " << elapsed_time.count() << " seconds\n";
}

int main(int argc, char **argv) {
  //call user main
  int rc = user_main(argc, argv);
  return rc;
}

#define main user_main