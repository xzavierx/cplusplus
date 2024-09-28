#include <memory>
#include <atomic>
#include <thread>
#include <cassert>

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
    oldTail->data.swap(data);
    oldTail->next = p;
    _tail.store(p);
  }

private:
  node* popHead() {
    node* const oldHead = _head.load();
    if (oldHead == _tail.load()) {
      return nullptr;
    }
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