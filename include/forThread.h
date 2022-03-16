#ifndef MULTITHDBSCAN_FORTHREAD_H
#define MULTITHDBSCAN_FORTHREAD_H

struct forThread{
    size_t threadId;
    size_t startId;
    size_t endId;
    float * dim1;
    float * dim2;
    int minPts;
    float eps;
};

#endif //MULTITHDBSCAN_FORTHREAD_H
