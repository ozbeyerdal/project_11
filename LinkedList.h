#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdexcept>

// Pointer-based singly linked list.
//
// This project forbids array-based custom data structures. To keep the rest of
// the code simple (especially JSON parsing and controller loops), this list
// exposes a minimal "array-like" interface:
//   - size()
//   - push_back()
//   - operator[]  (O(n) traversal)
//
// No STL containers are used.

template <typename T> class LinkedList {
private:
  struct Node {
    T value;
    Node *next;
    Node(const T &v) : value(v), next(nullptr) {}
  };

  Node *head_;
  Node *tail_;
  int sz_;

  Node *nodeAt(int idx) const {
    if (idx < 0 || idx >= sz_) throw std::out_of_range("LinkedList index");
    Node *cur = head_;
    for (int i = 0; i < idx; ++i) cur = cur->next;
    return cur;
  }

  void clearInternal() {
    Node *cur = head_;
    while (cur) {
      Node *n = cur->next;
      delete cur;
      cur = n;
    }
    head_ = tail_ = nullptr;
    sz_ = 0;
  }

public:
  LinkedList() : head_(nullptr), tail_(nullptr), sz_(0) {}

  ~LinkedList() { clearInternal(); }

  LinkedList(const LinkedList &other) : head_(nullptr), tail_(nullptr), sz_(0) {
    for (int i = 0; i < other.size(); ++i) push_back(other[i]);
  }

  LinkedList &operator=(const LinkedList &other) {
    if (this == &other) return *this;
    clearInternal();
    for (int i = 0; i < other.size(); ++i) push_back(other[i]);
    return *this;
  }

  int size() const { return sz_; }

  bool empty() const { return sz_ == 0; }

  void push_back(const T &v) {
    Node *n = new Node(v);
    if (!head_) {
      head_ = tail_ = n;
    } else {
      tail_->next = n;
      tail_ = n;
    }
    ++sz_;
  }

  // Convenience for tests/edge cases
  void pop_front() {
    if (!head_) return;
    Node *old = head_;
    head_ = head_->next;
    if (!head_) tail_ = nullptr;
    delete old;
    --sz_;
  }

  T &operator[](int idx) { return nodeAt(idx)->value; }
  const T &operator[](int idx) const { return nodeAt(idx)->value; }
};

#endif // LINKED_LIST_H
