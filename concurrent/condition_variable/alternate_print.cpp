#include <condition_variable>
#include <memory>
#include <mutex>
#include <iostream>
#include <ostream>
#include <queue>
#include <thread>


class Solution{
public:
  void print() {
    std::thread t1([this]{
      for (;;) {
        std::unique_lock<std::mutex> lock(_mtx);
        // 当条件不满足时，_cv1.wait就会挂起，等待线程t2通知唤醒，线程t2采用的是_cv1.notify_one
        _cv1.wait(lock, [this]{
          return _num == 1;
        });
        std::cout << "1" << std::endl;
        _num = 2;
        _cv2.notify_one();
      }
    });
    std::thread t2([this]{
      for (;;) {
        std::unique_lock<std::mutex> lock(_mtx);
        // 当条件不满足时，_cv2.wait就会挂起，等待线程t1通知唤醒，线程t1采用的是_cv2.notify_one
        _cv2.wait(lock, [this]{
          return _num == 2;
        });
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
//优化：
// 一个条件变量也可以
// 增加退出
class Solution1{
public:
  void print() {
    std::thread t1([this]{
      while (!_stop) {
        std::unique_lock<std::mutex> lock(_mtx);
        _cv.wait(lock, [this]{
          return _t1_turn || _stop;
        });
        if (_stop) break;
        std::cout << "1" << std::endl;
        _t1_turn = false; // 切换到t2
        _cv.notify_one();
      }
    });
    std::thread t2([this]{
      while (!_stop) {
        std::unique_lock<std::mutex> lock(_mtx);
        _cv.wait(lock, [this]{
          return !_t1_turn || _stop;
        });
        // 这里加条件条件判断是为了notify_all的立即响应退出
        if (_stop) break;
        std::cout << "2" << std::endl;
        std::cout.flush();
        _t1_turn = true; // 切换到t1
        _cv.notify_one();
      }
    });
    t1.join();
    t2.join();
  }
  void stop() {
    std::unique_lock<std::mutex> lock(_mtx);
    _stop = true;
    _cv.notify_all();
  }
private:
  std::condition_variable _cv;
  std::mutex _mtx;
  bool _t1_turn = true; // 标志：是否轮到t1执行
  bool _stop = false;
};

int main() {
  Solution1 s;
  std::queue<int> queue;
  s.print();
}