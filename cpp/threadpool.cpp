#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

using namespace std;

class threadpool {
private: 
    mutex m;
    condition_variable cond;
    int max_threads = 0;
    vector<thread> mthreads;
    bool stopCommand = false;
    using Task = function<void()>;
    queue<Task> tasks;
public:
    explicit threadpool(int maxThreads): max_threads(maxThreads){
        start();
    }
    threadpool(const threadpool &) = delete;
    threadpool& operator=(const threadpool &) = delete;

    ~threadpool() {
        stop();
    }
    
    void enqueue(Task task) {
        unique_lock<mutex> locker(m);
        tasks.emplace(task);
        cond.notify_one();
    }

    void start() {
        for(int i=0; i<max_threads; i++) {
            // cout << " i = " << i << endl;
            mthreads.emplace_back([&]() {
                while(1) {
                    Task task;
                    // {
                        unique_lock<mutex> locker(m);
                        while(!stopCommand || tasks.empty()) {
                            cond.wait(locker);
                        }
                        if(stopCommand && tasks.empty()) break;
                        task = move(tasks.front());
                        tasks.pop();
                        locker.unlock();
                    // }
                    task();
                }
            });
        }
    }

    void stop() {
        unique_lock<mutex> locker(m);
        stopCommand = true;
        cond.notify_all();
        locker.unlock();

        for(thread &t : mthreads) {
            t.join();
        }
    }

};

int main() {
    threadpool mypool(10);

    // mypool.enqueue([](){
    //     return 1;
    // });
    mypool.enqueue([]() {
        cout <<  "1" << endl;
    });
    mypool.enqueue([]() {
        cout <<  "2" << endl;
    });
    return 0;
}