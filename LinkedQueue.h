#ifndef LINKED_QUEUE_H
#define LINKED_QUEUE_H

#include <stdexcept>

// Pointer-based FIFO queue implemented with a linked list of nodes.
// This is intentionally small and beginner-friendly.

template <typename T> class LinkedQueue {
private:
  struct Node {
    T value;
    Node *next;
    Node(const T &v) : value(v), next(nullptr) {}
  };

  Node *front_;
  Node *back_;
  int sz_;

  void clearInternal() {
    while (front_) {
      Node *n = front_->next;
      delete front_;
      front_ = n;
    }
    back_ = nullptr;
    sz_ = 0;
  }

public:
  LinkedQueue() : front_(nullptr), back_(nullptr), sz_(0) {}
  ~LinkedQueue() { clearInternal(); }

  LinkedQueue(const LinkedQueue &other) : front_(nullptr), back_(nullptr), sz_(0) {
    // Copy by iterating
    Node *cur = other.front_;
    while (cur) {
      push(cur->value);
      cur = cur->next;
    }
  }

  LinkedQueue &operator=(const LinkedQueue &other) {
    if (this == &other) return *this;
    clearInternal();
    Node *cur = other.front_;
    while (cur) {
      push(cur->value);
      cur = cur->next;
    }
    return *this;
  }

  int size() const { return sz_; }
  bool empty() const { return sz_ == 0; }

  void push(const T &v) {
    Node *n = new Node(v);
    if (!back_) {
      front_ = back_ = n;
    } else {
      back_->next = n;
      back_ = n;
    }
    ++sz_;
  }

  void pop() {
    if (!front_) throw std::runtime_error("LinkedQueue pop on empty");
    Node *old = front_;
    front_ = front_->next;
    if (!front_) back_ = nullptr;
    delete old;
    --sz_;
  }

  T &front() {
    if (!front_) throw std::runtime_error("LinkedQueue front on empty");
    return front_->value;
  }
  const T &front() const {
    if (!front_) throw std::runtime_error("LinkedQueue front on empty");
    return front_->value;
  }
};

#endif // LINKED_QUEUE_H
