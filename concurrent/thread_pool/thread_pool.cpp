#include <algorithm>
#include <atomic>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>
#include <vector>
#include <list>
#include <iterator>
class ThreadPool{
public:
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;


  static ThreadPool& instance() {
    static ThreadPool s;
    return s;
  }


  using Task = std::packaged_task<void()>;
  ~ThreadPool() {
    stop();
  }

  template<class Fn, class... Args>
  auto commit(Fn&& fn, Args&&... args) -> std::future<decltype(fn(args...))> {
    using RetType = decltype(fn(args...));
    if (_stop.load()) {
      return std::future<RetType>{};
    }
    auto task = std::make_shared<std::packaged_task<RetType()>>(
      std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
    std::future<RetType> ret = task->get_future();
    {
      std::lock_guard<std::mutex> lock(_mtx);
      _tasks.emplace([=]{(*task)();});
    }
    _cv.notify_one();
    return ret;
  }

private:
  ThreadPool(unsigned int num = 1) : _stop(false) {
    _threadNum = num > 1 ? num : 1;
    start();
  }
  
  void start() {
    for (auto i= 0; i < _threadNum; ++i) {
      _pool.emplace_back([this]{
        while (!this->_stop.load()) {
          Task task;
          {
            std::unique_lock<std::mutex> lock(_mtx);
            this->_cv.wait(lock, [this]{
              return this->_stop.load() || !this->_tasks.empty();
            });
            if (this->_tasks.empty()) {
              return;
            }
            task = std::move(this->_tasks.front());
            this->_tasks.pop();
          }
          this->_threadNum--;
          task();
          this->_threadNum++;
        }
      });
    }
  }

  void stop() {
    _stop.store(true);
    _cv.notify_all();
    for (auto& td : _pool) {
      if (td.joinable()) {
        std::cout << "join thread: " << td.get_id() << std::endl;
        td.join();
      }
    }
  }

private:
  std::mutex _mtx;
  std::condition_variable _cv;
  std::atomic_bool _stop;
  std::atomic_int _threadNum;
  std::queue<Task> _tasks;
  std::vector<std::thread> _pool;
};

template<class T>
void quickSort(
  std::list<T>& input, 
  const typename std::list<T>::iterator& start,
  const typename std::list<T>::iterator& end) {
  if (start == end) {
    return ;
  } 
  // 这里不能用引用，否则会导致在partition的过程中发生修改
  const auto pivot = *start;
  auto iter = std::partition(start,end, [&](T const& t){return t < pivot;});
  // auto futureLower = ThreadPool::instance().commit(&quickSort<T>, input, start, iter);
  std::future<void> futureLower = std::async(&quickSort<T>, std::ref(input), start, iter);
  quickSort(input, ++iter, end);
  futureLower.get();
}

//并行版本
template<typename T>
std::list<T> parallel_quick_sort(std::list<T> input)
{
    if (input.empty())
    {
        return input;
    }
    std::list<T> result;
    //list::splice实现list拼接功能，将源list的内容部分或全部删除，插入到目的list
    result.splice(result.begin(), input, input.begin());
    // 取result的第一个元素
    T const& pivot = *result.begin();
    auto divide_point = std::partition(input.begin(), input.end(),
        [&](T const& t) {return t < pivot; });
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(),
        divide_point);
    // ①因为lower_part是副本，所以并行操作不会引发逻辑错误，这里可以启动future做排序
    // 线程数量随着调用层数增加而翻倍
    /*        input(main)           1个线程
            /           \
          l1(t1)         h1(main)   2个线程
          /  \          /  \
      l2(t2) h2(t1) l2(t3) h2(main) 4个线程
    */  
    std::future<std::list<T>> new_lower(
        std::async(&parallel_quick_sort<T>, std::move(lower_part)));
    auto new_higher(
        parallel_quick_sort(std::move(input)));    
    // result中存在一个中间元素，然后将大的部分往后插入，小的部分往前插入
    result.splice(result.end(), new_higher);    
    result.splice(result.begin(), new_lower.get());    
    return result;
}

template<typename T>
std::list<T> thread_pool_quick_sort(std::list<T> input)
{
    if (input.empty())
    {
        return input;
    }
    std::list<T> result;
    //list::splice实现list拼接功能，将源list的内容部分或全部删除，插入到目的list
    result.splice(result.begin(), input, input.begin());
    T const& pivot = *result.begin();
    auto divide_point = std::partition(input.begin(), input.end(),
        [&](T const& t) {return t < pivot; });
    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(),
        divide_point);
    // ①因为lower_part是副本，所以并行操作不会引发逻辑错误，这里投递给线程池处理
    auto new_lower = ThreadPool::instance().commit(&parallel_quick_sort<T>, std::move(lower_part));
    // ②
    auto new_higher(
        parallel_quick_sort(std::move(input)));
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get());
    return result;
}

void test_quickSort() {
  std::list<int> a = {5, 1, 8, 2, 3, 4, 7, 9, 0};
  // std::partition(a.begin(), a.end(), [&](int const& t) { return t < 5; });
  quickSort(a, a.begin(), a.end());
  std::copy(a.begin(), a.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout << std::endl;
}

void test_parallel_quick_sort() {
  std::list<int> a = {5, 1, 8, 2, 3, 4, 7, 9, 0};
  auto sort_result = parallel_quick_sort(a);
  std::copy(sort_result.begin(), sort_result.end(), 
    std::ostream_iterator<int>(std::cout, " "));
  std::cout << std::endl;
}

void test_thread_pool_quick_sort() {
  std::list<int> numlists = { 6,1,0,7,5,2,9,-1 };
    auto sort_result = thread_pool_quick_sort(numlists);
    std::cout << "sorted result is ";
    for (auto iter = sort_result.begin(); iter != sort_result.end(); iter++) {
        std::cout << " " << (*iter);
    }
  std::cout << std::endl;
}

int main() {
  // int m = 0;
  // auto future = ThreadPool::instance().commit([](int& m){
  //   m = 1024;
  //   std::cout << "in thread m:" << m << std::endl;
  // }, std::ref(m));
  // future.get();
  // std::cout << "in main thread, m:" << m << std::endl;

  test_quickSort();
  // test_parallel_quick_sort();
  // test_thread_pool_quick_sort();

  return 0;
}