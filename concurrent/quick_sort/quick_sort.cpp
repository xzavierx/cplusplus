
#include "../thread_pool/thread_pool.h"
#include <algorithm>
#include <list>

template<class ForwardIt>
void quicksort(ForwardIt first, ForwardIt last) {
  if (first == last) 
    return;

  auto pivot = *first;
  // std::partition将元素分为两组 条件为true的在第一组，条件为false的在第二组, 返回值是第二组的第一个位置
  auto middle1 = std::partition(first, last, [pivot](const auto& em) {
    return em < pivot;
  });
  // 将小于等于pivot的元素排在前面, 返回的middle2就是第一个大于pivot的元素
  auto middle2 = std::partition(middle1, last, [pivot](const auto& em){
    return !(pivot < em);
  });
  quicksort(first, middle1);
  quicksort(middle2, last);
}

template<class T>
void parallel_quicksort(
  std::list<T>& input, 
  const typename std::list<T>::iterator start,
  const typename std::list<T>::iterator end) {
  if (start == end) {
    return ;
  } 
  // 这里不能用引用，否则会导致在partition的过程中发生修改
  const auto pivot = *start;
  auto middle1 = std::partition(start,end, [&](T const& t){return t < pivot;});
  auto middle2 = std::partition(start, end, [&](T const& t) {return !(pivot < t);});
  // auto futureLower = ThreadPool::instance().commit(&quickSort<T>, input, start, iter);
  std::future<void> futureLower = std::async(&parallel_quicksort<T>, std::ref(input), start, middle1);
  parallel_quicksort(input, middle2, end);
  futureLower.get();
}

//并行版本
template<typename T>
std::list<T> parallel_quicksort_splice(std::list<T> input)
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
        std::async(&parallel_quicksort_splice<T>, std::move(lower_part)));
    auto new_higher(
        parallel_quicksort_splice(std::move(input)));    
    // result中存在一个中间元素，然后将大的部分往后插入，小的部分往前插入
    result.splice(result.end(), new_higher);    
    result.splice(result.begin(), new_lower.get());    
    return result;
}

template<typename T>
std::list<T> thread_pool_quicksort(std::list<T> input)
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
    auto new_lower = ThreadPool::instance().commit(&thread_pool_quicksort<T>, std::move(lower_part));
    // ②
    auto new_higher(
        thread_pool_quicksort<T>(std::move(input)));
    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get());
    return result;
}

void test_quicksort() {
  std::list<int> a = {5, 1, 8, 2, 3, 4, 7, 9, 0};
  quicksort(a.begin(), a.end());
  std::copy(a.begin(), a.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout << std::endl;
}

void test_parallel_quicksort() {
  std::list<int> a = {5, 1, 8, 2, 3, 4, 7, 9, 0};
  // std::partition(a.begin(), a.end(), [&](int const& t) { return t < 5; });
  parallel_quicksort(a, a.begin(), a.end());
  std::copy(a.begin(), a.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout << std::endl;
}

void test_parallel_quick_sort_splice() {
  std::list<int> a = {5, 1, 8, 2, 3, 4, 0, 7, 9};
  auto sort_result = parallel_quicksort_splice(a);
  std::copy(sort_result.begin(), sort_result.end(), 
    std::ostream_iterator<int>(std::cout, " "));
  std::cout << std::endl;
}

void test_thread_pool_quick_sort() {
  std::list<int> numlists = { 6,1,0,7,5,2,9,-1 };
    auto sort_result = thread_pool_quicksort(numlists);
    std::cout << "sorted result is ";
    for (auto iter = sort_result.begin(); iter != sort_result.end(); iter++) {
        std::cout << " " << (*iter);
    }
  std::cout << std::endl;
}

int main() {
  // test_quicksort();
  test_parallel_quicksort();
  // test_parallel_quick_sort();
  // test_thread_pool_quick_sort();
  return 0;
}