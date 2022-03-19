#include "../include/factory.h"

T Factory::acquire_work() {

    std::unique_lock<std::mutex> local_lock(this->mutex, std::defer_lock);
    while ( 1 ) {
        local_lock.lock();
        if ( this->queue_of_work.empty() ) {
            local_lock.unlock();
            std::this_thread::yield();
        }
        else break;
    }

    T work = this->queue_of_work.front();
    this->queue_of_work.pop();
    return work;
}

void Factory::do_work(const T& arg) const noexcept {
        std::cout << "Thread " << std::this_thread::get_id() << ": " << arg << std::endl;
        std::this_thread::sleep_for( std::chrono::milliseconds(500) );
}

void Factory::thread_target() {

    T work;
    while ( 1 ) {
        work = this->acquire_work();
        if ( work != this->sentinel ) this->do_work(work);
        else break;
    }
}


Factory::Factory(const T sentinel, const unsigned n_threads = std::thread::hardware_concurrency())
            : sentinel(sentinel), n_threads(n_threads) {
        this->threads.reserve( this->n_threads );
}



