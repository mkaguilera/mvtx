/**
 * SafeQueue.cc
 *
 *  Created on: Jul 18, 2016
 *      Author: theo
 */

#include <iostream>
#include "SafeQueue.h"

template <class T>
SafeQueue<T>::SafeQueue(uint64_t max_size)
    : _max_size(max_size) {}

template <class T>
SafeQueue<T>::~SafeQueue() {}

template <class T>
void SafeQueue<T>::enqueue(T t) {
  std::unique_lock<std::mutex> lock(_mutex);

  while (_queue.size() == _max_size)
    _cond1.wait(lock);
  _queue.push(t);
  _cond2.notify_one();
}

template <class T>
T SafeQueue<T>::dequeue() {
  T res;
  std::unique_lock<std::mutex> lock(_mutex);

  while (_queue.size() == 0)
    _cond2.wait(lock);
  res = _queue.front();
  _queue.pop();
  _cond1.notify_one();
  return res;
}

template <class T>
bool SafeQueue<T>::tryDequeue(T *elem) {
  std::unique_lock<std::mutex> lock(_mutex);

  if (_queue.size() == 0)
    return false;
  *elem = _queue.front();
  _queue.pop();
  _cond1.notify_one();
  return true;
}

template class SafeQueue<void *>;