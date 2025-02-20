#include <atomic>
#include <iostream>
#include <thread>

std::atomic<int> x{0};
std::atomic<int> y{0};

void thread1() {
    x.store(1, std::memory_order_seq_cst); // seq_cst 写操作
    int r = y.load(std::memory_order_seq_cst); // seq_cst 读操作
    std::cout << "Thread1: x = " << x << ", y = " << r << std::endl;
}

void thread2() {
    y.store(1, std::memory_order_seq_cst); // seq_cst 写操作
    int r = x.load(std::memory_order_seq_cst); // seq_cst 读操作
    std::cout << "Thread2: y = " << y << ", x = " << r << std::endl;
}

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);

    t1.join();
    t2.join();

    return 0;
}