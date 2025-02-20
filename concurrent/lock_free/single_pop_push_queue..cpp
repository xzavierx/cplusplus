#include <memory>
#include <atomic>
#include <thread>
#include <cassert>

// 使用单生产者单消费者场景, 一个线程负责生产，一个线程负责消费
template <typename T>
class SinglePopPushQueue{
  struct node {
    std::shared_ptr<T> data;
    node* next = nullptr;
  };
public:
  SinglePopPushQueue():
    _head(new node), _tail(_head.load()) { }
  SinglePopPushQueue(const SinglePopPushQueue& other) = delete;
  SinglePopPushQueue& operator=(const SinglePopPushQueue& other) = delete;
  ~SinglePopPushQueue() {
    while (node* const oldHead = _head.load()) {
      _head.store(oldHead->next);
      delete oldHead;
    }
  }

  std::shared_ptr<T> pop() {
    node* oldHead = popHead();
    if (!oldHead) {
      return std::shared_ptr<T>();
    }

    std::shared_ptr<T> const v(oldHead->data);
    delete oldHead;
    return v;
  }


  void push(T value) {
    std::shared_ptr<T> data(std::make_shared<T>(value));
    node* p = new node;
    node* const oldTail = _tail.load();
    // 将数据存储到尾节点，并更新尾节点的next指针
    oldTail->data.swap(data);
    oldTail->next = p;
    // 更新尾指针
    _tail.store(p);
  }

private:
  node* popHead() {
    node* const oldHead = _head.load();
    // 如果队列为空，返回空指针
    if (oldHead == _tail.load()) {
      return nullptr;
    }
    // 更新头指针
    _head.store(oldHead->next);
    return oldHead;
  }

  std::atomic<node*> _head;
  std::atomic<node*> _tail;
};

int main() {
  SinglePopPushQueue<int> queue;
  std::thread tpush([&]{
    auto i = 0;
    while (true) {
      queue.push(i++);
    }
  });

  std::thread tpop([&]{
    auto i = 0;
    while (true) {
      auto v = queue.pop();
      if (v) {
        assert(*v == i);
        ++i;
      }
    }
  });
  tpush.join(), tpop.join();
}