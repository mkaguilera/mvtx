/*
 * SafeQueue.h
 *
 *  Created on: Jun 16, 2016
 *      Author: theo
 */

#ifndef SAFEQUEUE_H_
#define SAFEQUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>

template <class T>
class SafeQueue
{
private:
  uint64_t _max_size;
  std::queue<T> _queue;
  std::mutex _mutex;
  std::condition_variable _cond1, _cond2;

public:
  SafeQueue(uint64_t max_size);
  ~SafeQueue();

  void enqueue(T t);
  T dequeue ();
  bool tryDequeue(T *elem);
};

#endif /* SAFEQUEUE_H_ */
