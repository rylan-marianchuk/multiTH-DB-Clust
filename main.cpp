#include <iostream>
#include <pthread.h>
#include <sqlite3.h>
#include <string>
#include "include/forThread.h"
#include <sys/sysinfo.h>

void * dbscan(void * params){

    pthread_exit(NULL);
}

int main(int argc, char * argv[]){
    /*
     * Invoke with parameters
     *
     */
    std::string db_path = argv[1];
    int batch_size = std::stoi(argv[2]);
    float eps = std::stof(argv[2]);
    int minPts = std::stoi(argv[3]);

    // Read in the database

    // If the column DBSCAN is not present, create it all NULL


    int n_threads = get_nprocs();
    std::cout << "Using " << n_threads << " threads." << std::endl;


}
