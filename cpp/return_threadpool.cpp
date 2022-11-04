#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <memory>

using namespace std;

class threadpool {
private: 
    mutex m;
    condition_variable cond;
    int max_threads = 0;
    vector<thread> mthreads;
    bool stopCommand = false;
    // using Task = function<void()>;
    // queue<Task> tasks;
    using Task = packaged_task<void()>;
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
    
    template<typename T>
    auto enqueue(T&& task) -> future<decltype(task())> {
        // auto up = make_shared<packaged_task<decltype(task())()>>(move(task));
        packaged_task<decltype(task())()> pt(move(task));
        future<decltype(task())> r = pt.get_future();
        unique_lock<mutex> locker(m);
        
        // tasks.emplace([=](){
        //     cout << "hello" << endl;
        //     // (*up)();
        // });
        tasks.emplace(move(pt));
        cond.notify_one();
        locker.unlock();
        //return up->get_future();
        return r;
    }

    // add new work item to the pool
template<class F, class... Args>
auto enqueue1(F&& f, Args&&... args) -> 
future<typename result_of<F(Args...)>::type>
{
    using return_type = typename result_of<F(Args...)>::type;
    auto task = std::make_shared<packaged_task<return_type()> >(
            bind( forward<F>(f), forward<Args>(args)...)
        );
        
    future<return_type> res = task->get_future();
    {
        unique_lock<mutex> lock(m);

        // don't allow enqueueing after stopping the pool
        if(stopCommand)
            throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task](){ (*task)(); });
    }
    cond.notify_one();
    return res;
}
    void start() {
        for(int i=0; i<max_threads; i++) {
            // cout << " i = " << i << endl;
            mthreads.emplace_back([&]() {
                while(1) {
                    Task task;
                    // {
                        unique_lock<mutex> locker(m);
                        while(tasks.empty() && !stopCommand) {
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

int fact(int n) {
    if(n == 1) return 1;
    return n*fact(n-1);
}

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
    auto f1 = mypool.enqueue([]() {
        return 3;
    });
    auto f2 = mypool.enqueue([]() {
        char b[6];
        b[0] = 'H';
        b[1] = 'e';
        b[2] = 'l';
        b[3] = 'l';
        b[4] = 'o';
        b[5] = '\n';
        return b;
    });
    auto f3 = mypool.enqueue([]() {
        return "abcd";
    });
    auto f4 = mypool.enqueue1([](){
        return fact(4);
    });
    char *b = f2.get();
    cout << "myf = " << (char)b[0] << endl;
    cout << "f1 = " << f1.get() << endl;
    // cout << "f2 = " << f2.get() << endl;
    cout << "f3 = " << f3.get() << endl;
    cout << "f4 = " << f4.get() << endl;
    return 0;
}