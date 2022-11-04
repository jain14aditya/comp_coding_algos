#include <iostream>
using namespace std;

class A {
};

class B : public A{
};

int main() {
    A* a = new B();
    cout << "Hello world " << endl;
}
