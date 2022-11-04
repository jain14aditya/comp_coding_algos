#include <iostream>
using namespace std;

// Reference:
// https://medium.com/analytics-vidhya/c-shared-ptr-and-how-to-write-your-own-d0d385c118ad

template <typename T>
class MyUniquePtr{
public:
    // Default constructor.
    MyUniquePtr(): ptr_(nullptr){}

    // Constructor.
    MyUniquePtr(T* ptr): ptr_(ptr){}

    // Copy constructor.
    MyUniquePtr(const MyUniquePtr<T>& other) = delete;

    // Copy assignment.
    MyUniquePtr& operator=(const MyUniquePtr<T>& other) = delete;

    // Move constructor.
    MyUniquePtr(MyUniquePtr<T>&& dying_obj){
        this->ptr_ = dying_obj.ptr_;
        dying_obj.ptr_ = nullptr;
    }

    // Move assignment.
    MyUniquePtr& operator=(MyUniquePtr&& dying_obj){
        if(ptr_ == dying_obj.ptr_) {
            return *this;
        }
        __cleanup__();

        ptr_ = dying_obj.ptr_;
        dying_obj.ptr_ = nullptr;
        return *this;
    }

    // destructor.
    ~MyUniquePtr(){
        __cleanup__();
    }

    // Overload -> operator.
    T* operator->(){
        return ptr_;
    }

    // Overload * operator.
    T& operator*(){
        return *ptr_;
    }

    // Get ptr.
    T* get_ptr(){
        return ptr_;
    }
    
private:
    void __cleanup__(){
        if(ptr_){
            delete ptr_;
        }
    }

    T *ptr_;
};

int main(){
    // Test default constructor.
    MySharedPtr<int> p;
    cout << p.get_ref_count() << endl; /* 0 */
    cout << p.get_ptr() << endl; /* 0 */

    // Test constructor.
    MySharedPtr<int> q(new int(3));
    cout << q.get_ref_count() << endl; /* 1 */
    cout << q.get_ptr() << endl; /* non-zero */

    // Test copy constructor.
    MySharedPtr<int> r = q;
    cout << q.get_ref_count() << endl; /* 2 */
    cout << r.get_ref_count() << endl; /* 2 */

    // Test copy constructor.
    MySharedPtr<int> z(new int(11));

    // z = std::move(z);
    z = z;
    cout << "testing = " << z.get_ref_count() << endl; /* 1 */

    // Test assign operator.
    r = p;
    cout << r.get_ref_count() << endl; /* 0 */
    cout << r.get_ptr() << endl; /* 0 */
    cout << q.get_ref_count() << endl; /* 1 */

    // Test move contructor.
    MySharedPtr<int> s = move(q);
    cout << s.get_ref_count() << endl; /* 1 */
    cout << s.get_ptr() << endl; /* should match with previous q ptr */
    cout << q.get_ptr() << endl; /* 0 */
    cout << q.get_ref_count() << endl;

    // Test move assignment operator.
    MySharedPtr<int> t(new int(3));
    cout << t.get_ptr() << endl; /* non-zero */
    t = move(s);
    cout << t.get_ptr() << endl; /* should match with previous q ptr */
    cout << s.get_ptr() << endl; /* 0 */
}
