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

/**
 * Class for implementing safe (synchronized) FIFO queue for any eligible type.
 */
template <class T>
class SafeQueue
{
private:
  ///> Maximum size the queue mau have.
  uint64_t _max_size;
  ///> Unsafe queue structure.
  std::queue<T> _queue;
  ///> Lock that controls queue accesses.
  std::mutex _mutex;
  ///> Condition variables used for waiting when queue is full.
  std::condition_variable _cond1;
  ///> Condition variables used for waiting when queue is empty.
  std::condition_variable _cond2;

public:
  /**
   * Constructor of SafeQueue.
   * @param max_size  - Maximum size the queue might have.
   */
  SafeQueue(uint64_t max_size);
  /**
   * Destructor of SafeQueue.
   */
  ~SafeQueue();
  /**
   * Add element to the back of the queue.
   * @param t - Element to add.
   */
  void enqueue(T t);
  /**
   * Remove element from the front of the queue.
   * @return  Removed element.
   */
  T dequeue ();
  /**
   * Try to remove specific element from the queue.
   * @param elem  - Element to remove.
   * @return  Whether or not operation is successful.
   */
  bool tryDequeue(T *elem);
};

#endif /* SAFEQUEUE_H_ */
