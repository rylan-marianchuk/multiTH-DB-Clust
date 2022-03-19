#ifndef MULTITH_DB_CLUST_FACTORY_H
#define MULTITH_DB_CLUST_FACTORY_H

#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
#include <bits/stdc++.h>

template<typename T>
class Factory {

private:
    const T sentinel;
    const unsigned n_threads;
    bool started = false;
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::queue<T> queue_of_work;


    T acquire_work();
    void do_work(const T& arg) const noexcept;
    void thread_target();

public:
    Factory(const T sentinel, const unsigned n_threads = std::thread::hardware_concurrency());
};


#endif //MULTITH_DB_CLUST_FACTORY_H
