## SimpleMultithreader



This project implements a lightweight multithreading framework using POSIX threads (pthreads). It provides two versions of parallel\_for to parallelize 1D and 2D loops using C++ lambda functions. The code divides work among threads, executes the lambda on each chunk, and measures total execution time.



###### Features:



* parallel\_for(int low, int high, lambda, NTHREADS) for 1D loops



* parallel\_for(int low1, int high1, int low2, int high2, lambda, NTHREADS) for 2D nested loops



* Uses std::function, pthreads, and chrono



* Automatic chunk distribution and thread synchronization



###### How It Works:



* The program calls user\_main, which uses parallel\_for.



* Each thread receives a range through a struct.



* Threads execute the lambda on their assigned iterations.



* The main thread joins all worker threads.



* Total execution time is printed.



##### Contributions



Dewang



Implemented the core logic for the 1D parallel\_for



Wrote and debugged thread functions



Added timing and error handling



Ensured correct lambda capture and execution



Pranshu



Designed and implemented argument structures



Implemented the 2D parallel\_for version



Handled workload distribution for threads



Assisted with integration and testing






GitHub Link - https://github.com/Pranshu101-hub/OperatingSystems-Assignment/tree/main

