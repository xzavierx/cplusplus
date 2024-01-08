#include <condition_variable>
#include <memory>
#include <mutex>
#include <iostream>
#include <ostream>
#include <queue>

class Solution{
public:
  void print() {
    std::thread t1([this]{
      for (;;) {
        std::unique_lock<std::mutex> lock(_mtx);
        _cv1.wait(lock, [this]{
          return _num == 1;
        });
        // std::cout << "thread print 1" << std::endl; 
        std::cout << "1" << std::endl;
        _num = 2;
        _cv2.notify_one();
      }
    });
    std::thread t2([this]{
      for (;;) {
        std::unique_lock<std::mutex> lock(_mtx);
        _cv2.wait(lock, [this]{
          return _num == 2;
        });
        // std::cout << "thread print 2" << std::endl; 
        std::cout << "2" << std::endl;
        _num = 1;
        _cv1.notify_one();
      }
    });
    t1.join();
    t2.join();
  }
private:
  std::condition_variable _cv1;
  std::condition_variable _cv2;
  std::mutex _mtx;
  int _num = 1;
};
// 一个条件变量也可以
class Solution1{
public:
  void print() {
    std::thread t1([this]{
      for (;;) {
        std::unique_lock<std::mutex> lock(_mtx);
        _cv1.wait(lock, [this]{
          return _num == 1;
        });
        // std::cout << "thread print 1" << std::endl; 
        std::cout << "1" << std::endl;
        _num = 2;
        _cv1.notify_one();
      }
    });
    std::thread t2([this]{
      for (;;) {
        std::unique_lock<std::mutex> lock(_mtx);
        _cv1.wait(lock, [this]{
          return _num == 2;
        });
        // std::cout << "thread print 2" << std::endl; 
        std::cout << "2" << std::endl;
        std::cout.flush();
        _num = 1;
        _cv1.notify_one();
      }
    });
    t1.join();
    t2.join();
  }
private:
  std::condition_variable _cv1;
  std::mutex _mtx;
  int _num = 1;
};

int main() {
  Solution1 s;
  std::queue<int> queue;
  s.print();
}