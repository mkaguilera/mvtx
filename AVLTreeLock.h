/**
 * AVLTreeLock.h
 *
 *  Created on: Jul 25, 2016
 *      Author: theo
 */

#ifndef AVLTREELOCK_H_
#define AVLTREELOCK_H_

#include <set>
#include <string>
#include <vector>
#include <stdint.h>

class AVLTreeLock
{
public:
  enum Status { FREE, READ_LOCK, WRITE_LOCK, FREEZE_READ_LOCK, FREEZE_WRITE_LOCK };

  Status _status;
  uint64_t _start;
  uint64_t _end;
  std::set<uint64_t> *_tids;
  std::set<void *> *_events;

private:
  int _left_height;
  int _right_height;
  AVLTreeLock *_parent;
  AVLTreeLock *_left;
  AVLTreeLock *_right;

public:
  AVLTreeLock();
  AVLTreeLock(AVLTreeLock *lock);
  ~AVLTreeLock();
  static AVLTreeLock *createLock();

  AVLTreeLock *getRoot();
  void find(std::vector<AVLTreeLock*> *locks, uint64_t ts);
  void insert(AVLTreeLock *new_lock);
  static void remove(AVLTreeLock *lock);
  static void swap(AVLTreeLock *parent, AVLTreeLock *child, bool is_left);
  void updateHeight();
  void balance();
  std::string toString();

  /*
  bool tryBackwardLock(uint64_t tid, uint64_t ts, bool is_read, void *event, uint64_t *last_write, AVLTreeLock **root);
  bool tryLock(uint64_t tid, uint64_t ts_start, uint64_t ts_end, bool is_read, AVLTreeLock **root);
  void freeze(uint64_t tid, uint64_t ts_start, uint64_t ts_end, TServer *tserver, AVLTreeLock **root);
  void unlock(uint64_t tid, uint64_t ts_start, uint64_t ts_end, TServer *tserver, AVLTreeLock **root);
  */
};

#endif /* AVLTREELOCK_H_ */
