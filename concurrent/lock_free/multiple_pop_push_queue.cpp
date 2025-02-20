#include <atomic>
#include <memory>

template<typename T>
class MPSLockFreeQueue {
private:
  struct Node {
    std::shared_ptr<T> data;
    std::atomic<Node*> next;
  };

  std::atomic<Node*> _head; // 队列头部指针
  std::atomic<Node*> _tail; // 队尾尾部指针

public:
  // 增加了虚拟节点，简化了边界条件的处理
  MPSLockFreeQueue() : _head(new Node), _tail(_head.load()) {
    _head.load()->next.store(nullptr);
  }


  MPSLockFreeQueue(const MPSLockFreeQueue& other) = delete;
  MPSLockFreeQueue& operator=(const MPSLockFreeQueue& other) = delete;

  ~MPSLockFreeQueue() {
    while (Node* oldHead = _head.load()) {
      _head.store(oldHead->next.load());
      delete oldHead;
    }
  }

  void push(T value){
    std::shared_ptr<T> newData = std::make_shared<T>(value);
    Node* newNode = new Node;
    newNode->data = newData;
    newNode->next.store(nullptr);

    Node* oldTail = _tail.load();
    Node* expected = nullptr;

    // CAS循环：确保原子地更新尾节点的next指针
    // 先尝试CAS，失败则重新加载尾，然后继续CAS，成功了oldTail为新的节点
    while (!oldTail->next.compare_exchange_weak(expected, newNode)) {
      oldTail = _tail.load();
      expected = nullptr;
    }

    // 如果成功，说明当前线程成功更新了_tail
    // 如果失败，说明其他线程已经更新了_tail，当前线程无需重试，因为oldTail->next已经正确指向newNode。
    _tail.compare_exchange_weak(oldTail, newNode);
  }

  // 从队列中弹出数据
    std::shared_ptr<T> pop() {
        Node* oldHead = _head.load(); // 获取当前头节点
        Node* nextNode = oldHead->next.load(); // 获取头节点的下一个节点

        // 如果队列为空，返回空指针
        if (nextNode == nullptr) {
            return nullptr;
        }

        // CAS 循环：确保原子地更新头指针
        while (!_head.compare_exchange_weak(oldHead, nextNode)) {
            nextNode = oldHead->next.load(); // 如果失败，重新加载下一个节点
            if (nextNode == nullptr) {
                return nullptr; // 如果队列为空，返回空指针
            }
        }

        // 获取数据并删除旧的头节点
        std::shared_ptr<T> result(nextNode->data);
        delete oldHead;
        return result;
    }


};