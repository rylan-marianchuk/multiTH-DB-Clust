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

    T acquire_work() {

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

    void do_work(const T& arg) const noexcept {

        std::cout << "Thread " << std::this_thread::get_id() << ": " << arg << std::endl;
        std::this_thread::sleep_for( std::chrono::milliseconds(500) );
    }

    void thread_target() {

        T work;
        while ( 1 ) {
            work = this->acquire_work();
            if ( work != this->sentinel ) this->do_work(work);
            else break;
        }
    }

public:

    Factory(const T sentinel, const unsigned n_threads = std::thread::hardware_concurrency())
            : sentinel(sentinel), n_threads(n_threads) {
        this->threads.reserve( this->n_threads );
    }

    ~Factory() { if ( this->started ) this->halt(); }

    Factory(const Factory<T>&) = delete;
    Factory<T>& operator=(const Factory<T>&) = delete;

    void start() {

        this->started = true;
        for (size_t i = 0; i < this->n_threads; i++) {
            threads.emplace_back( &Factory::thread_target, this );
        }
    }

    void load(const std::vector<T>& work_to_deposit) {

        std::scoped_lock<std::mutex>(this->mutex);
        for ( const auto& w : work_to_deposit ) this->queue_of_work.push(w);
    }

    void load(std::queue<T>&& work_to_deposit) {

        std::scoped_lock<std::mutex>(this->mutex);
        if ( this->queue_of_work.empty() ) this->queue_of_work = std::move( work_to_deposit );
        else {
            while ( ! work_to_deposit.empty() ) {
                this->queue_of_work.push( work_to_deposit.front() );
                work_to_deposit.pop();
            }
        }
    }

    void halt() {

        this->started = false;
        {
            std::scoped_lock<std::mutex>(this->mutex);
            for (size_t i = 0; i < this->n_threads; i++) this->queue_of_work.push(this->sentinel);
        }
        for (auto& thread : this->threads) thread.join();
    }

    bool empty() const {
        std::scoped_lock<std::mutex>(this->mutex);
        return this->queue_of_work.empty();
    }

};



int main() {

    std::vector<int> some_work = {1, 2, 3, 4};
    std::queue<int> more_work;
    for (int i = 1; i < 5; i++) more_work.push(i);

    Factory<int> factory(INT_MIN);

    factory.start();
    factory.load( std::move(more_work) );
    factory.load(some_work);
    std::this_thread::sleep_for( std::chrono::seconds(1) );
    factory.load(some_work);

}