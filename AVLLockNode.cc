/**
 * AVLTreeLock.cc
 *
 *  Created on: Jul 25, 2016
 *      Author: theo
 */

#include <cassert>
#include <iostream>
#include <limits>
#include <vector>
#include "AVLLockNode.h"
#include "TServer.h"
#include "GRPCServer.h"
#include "SimpleKeyMapper.h"
#include "SimpleTServer.h"

AVLLockNode::AVLLockNode() {
  _status = FREEZE_WRITE_LOCK;
  _start = 0;
  _end = 0;
  _tids = new std::set<uint64_t>();
  _events = new std::set<void *>();
  _parent = nullptr;
  _left = nullptr;
  _right = nullptr;
  _left_height = 0;
  _right_height = 0;
}

AVLLockNode::AVLLockNode(AVLLockNode *lock) {
  _status = lock->_status;
  _start = lock->_start;
  _end = lock->_end;
  _tids = new std::set<uint64_t>(*(lock->_tids));
  _events = new std::set<void *>(*(lock->_events));
  _parent = nullptr;
  _left = nullptr;
  _right = nullptr;
  _left_height = 0;
  _right_height = 0;
}

AVLLockNode::~AVLLockNode() {
  _tids->clear();
  delete _tids;
  _events->clear();
  delete _events;
}

AVLLockNode *AVLLockNode::getRoot() {
  if (_parent != nullptr)
    return _parent->getRoot();
  return this;
}

void AVLLockNode::updateHeight() {
  int old_left_height = _left_height;
  int old_right_height = _right_height;

  _left_height =
      _left == nullptr ?
          0 : (_left->_left_height >= _left->_right_height ? _left->_left_height + 1 : _left->_right_height + 1);
  _right_height =
      _right == nullptr ?
          0 : (_right->_left_height >= _right->_right_height ? _right->_left_height + 1 : _right->_right_height + 1);
  if ((_parent != nullptr) && ((_left_height != old_left_height) || (_right_height != old_right_height)))
    _parent->updateHeight();
}

void AVLLockNode::find(std::vector<AVLLockNode*> *locks, uint64_t ts) {
  if (ts < _start) {
    if (_left != nullptr)
      _left->find(locks,ts);
  } else if (ts > _end) {
    if (_right != nullptr) {
      locks->push_back(this);
      _right->find(locks,ts);
    }
  } else {
    locks->push_back(this);
  }
}

AVLLockNode *AVLLockNode::createLockTree() {
  AVLLockNode *root = new AVLLockNode();
  AVLLockNode *new_lock = new AVLLockNode(root);

  new_lock->_status = FREE;
  new_lock->_start = 1;
  new_lock->_end = std::numeric_limits < uint64_t > ::max();
  root->insert(new_lock);
  return root;
}

void AVLLockNode::insert(AVLLockNode *new_lock) {
  if (new_lock->_start < _start) {
    if (new_lock->_end > _start) {
      std::cerr << "New lock (" << new_lock->_start << "," << new_lock->_end << ") intersects with another lock ("
          << _start << "," << _end << ")." << std::endl;
      exit(-1);
    }
    if (_left == nullptr) {
      _left = new_lock;
      new_lock->_parent = this;
      updateHeight();
    } else {
      _left->insert(new_lock);
    }
  } else {
    if (new_lock->_start < _end) {
      std::cerr << "New lock (" << new_lock->_start << "," << new_lock->_end << ") intersects with another lock ("
          << _start << "," << _end << ")." << std::endl;
      exit(-1);
    }
    if (_right == nullptr) {
      _right = new_lock;
      new_lock->_parent = this;
      updateHeight();
    } else {
      _right->insert(new_lock);
    }
  }
}

void AVLLockNode::remove() {
  if (_left != nullptr) {
    AVLLockNode *new_lock = _left;
    AVLLockNode *parent;

    while (new_lock->_right != nullptr)
      new_lock = new_lock->_right;
    if (new_lock->_parent == this) {
      if (_parent->_left == this)
        _parent->_left = new_lock;
      else
        _parent->_right = new_lock;
      new_lock->_parent = _parent;
      if (_right != nullptr)
        _right->_parent = new_lock;
      new_lock->_right = _right;
      new_lock->updateHeight();
      new_lock->balance();
    } else {
      parent = new_lock->_parent;
      if (new_lock->_left != nullptr)
        new_lock->_left->_parent = parent;
      parent->_right = new_lock->_left;
      parent->updateHeight();
      parent->balance();
      new_lock->_parent = _parent;
      new_lock->_left = _left;
      new_lock->_right = _right;
      new_lock->_left_height = _left_height;
      new_lock->_right_height = _right_height;
      if (_parent != nullptr) {
        if (_parent->_left == this)
          _parent->_left = new_lock;
        else
          _parent->_right = new_lock;
      }
      if (_left != nullptr)
        _left->_parent = new_lock;
      if (_right != nullptr)
        _right->_parent = new_lock;
    }
  } else if (_right != nullptr) {
    if (_parent != nullptr) {
      if (_parent->_left == this)
        _parent->_left = _right;
      else
        _parent->_right = _right;
      _right->_parent = _parent;
    }
    _parent->updateHeight();
    _parent->balance();
  } else {
    if (_parent != nullptr) {
      if (_parent->_left == this)
        _parent->_left = nullptr;
      else
        _parent->_right = nullptr;
    }
    _parent->updateHeight();
    _parent->balance();
  }
}

void AVLLockNode::remove(AVLLockNode *lock) {
  if (lock->_left != nullptr) {
    AVLLockNode *new_lock = lock->_left;
    AVLLockNode *parent;

    while (new_lock->_right != nullptr)
      new_lock = new_lock->_right;
    if (new_lock->_parent == lock) {
      if (lock->_parent->_left == lock)
        lock->_parent->_left = new_lock;
      else
        lock->_parent->_right = new_lock;
      new_lock->_parent = lock->_parent;
      if (lock->_right != nullptr)
        lock->_right->_parent = new_lock;
      new_lock->_right = lock->_right;
      new_lock->updateHeight();
      new_lock->balance();
    } else {
      parent = new_lock->_parent;
      if (new_lock->_left != nullptr)
        new_lock->_left->_parent = parent;
      parent->_right = new_lock->_left;
      parent->updateHeight();
      parent->balance();
      new_lock->_parent = lock->_parent;
      new_lock->_left = lock->_left;
      new_lock->_right = lock->_right;
      new_lock->_left_height = lock->_left_height;
      new_lock->_right_height = lock->_right_height;
      if (lock->_parent != nullptr) {
        if (lock->_parent->_left == lock)
          lock->_parent->_left = new_lock;
        else
          lock->_parent->_right = new_lock;
      }
      if (lock->_left != nullptr)
        lock->_left->_parent = new_lock;
      if (lock->_right != nullptr)
        lock->_right->_parent = new_lock;
    }
  } else if (lock->_right != nullptr) {
    if (lock->_parent != nullptr) {
      if (lock->_parent->_left == lock)
        lock->_parent->_left = lock->_right;
      else
        lock->_parent->_right = lock->_right;
      lock->_right->_parent = lock->_parent;
    }
    lock->_parent->updateHeight();
    lock->_parent->balance();
  } else {
    if (lock->_parent != nullptr) {
      if (lock->_parent->_left == lock)
        lock->_parent->_left = nullptr;
      else
        lock->_parent->_right = nullptr;
    }
    lock->_parent->updateHeight();
    lock->_parent->balance();
  }
  delete lock;
}

void AVLLockNode::swap(AVLLockNode *parent, AVLLockNode *child, bool is_left) {
  child->_parent = parent->_parent;
  if (parent->_parent != nullptr) {
    if (parent->_parent->_left == parent)
      parent->_parent->_left = child;
    else
      parent->_parent->_right = child;
  }
  parent->_parent = child;
  if (is_left) {
    parent->_left = child->_right;
    if (parent->_left != nullptr)
      parent->_left->_parent = parent;
    child->_right = parent;
  } else {
    parent->_right = child->_left;
    if (parent->_right != nullptr)
      parent->_right->_parent = parent;
    child->_left = parent;
  }
  parent->updateHeight();
}

void AVLLockNode::balance() {
  int diff = _right_height - _left_height;

  if (diff < -1) {
    if (_left->_right_height - _left->_left_height == 1) {
      swap(_left, _left->_right, false);
    } else if (_left->_right_height - _left->_left_height < -1) {
      std::cerr << "AVLTree: Balance factor is " << _left->_right_height - _left->_left_height << " instead of -1."
          << std::endl;
      exit(-1);
    }
    swap(this, _left, true);
    return;
  } else if (diff > 1) {
    if (_right->_right_height - _right->_left_height == -1) {
      swap(_right, _right->_left, true);
    } else if (_right->_right_height - _right->_left_height > 1) {
      std::cerr << "AVLTree: Balance factor is " << _right->_right_height - _right->_left_height << " instead of 1."
          << std::endl;
      exit(-1);
    }
    swap(this, _right, false);
    return;
  }

  if (_parent != nullptr)
    _parent->balance();
}

/*
 bool AVLTreeLock::tryBackwardLock(uint64_t tid, uint64_t ts, bool is_write, void *event, uint64_t *last,
 AVLTreeLock **root) {
 std::vector<AVLTreeLock *> locks;
 AVLTreeLock *cur_lock = this;
 AVLTreeLock *new_lock = nullptr;
 uint64_t cur_ts = ts;

 find(&locks, cur_ts);
 while (true) {
 if (locks.empty()) {
 *last = cur_ts;
 if (new_lock != nullptr)
 new_lock->balance();
 return true;
 }
 cur_lock = locks.back();
 locks.pop_back();

 if ((cur_ts < cur_lock->_start) || (cur_ts > cur_lock->_end)) {
 std::cerr << "Find does not work as intended. Time " << cur_ts << " is not between " << cur_lock->_start
 << " and " << cur_lock->_end << "." << std::endl;
 exit(-1);
 }
 switch (cur_lock->_status) {
 case FREE:
 {
 if (cur_ts < cur_lock->_end) {
 new_lock = new AVLTreeLock(cur_lock);
 new_lock->_start = cur_ts + 1;
 new_lock->_end = cur_lock->_end;
 cur_lock->_end = cur_ts;
 cur_lock->insert(new_lock);
 }
 cur_lock->_status = is_write ? WRITE_LOCK : READ_LOCK;
 cur_lock->_tids->insert(tid);
 cur_ts = cur_lock->_start-1;
 break;
 }
 case READ_LOCK:
 {
 if (is_write) {
 *last = cur_ts;
 cur_lock->_events->insert(event);
 locks.clear();
 if (new_lock != nullptr)
 new_lock->balance();
 return false;
 } else {
 if (cur_ts < cur_lock->_end) {
 new_lock = new AVLTreeLock(cur_lock);
 new_lock->_start = cur_ts + 1;
 new_lock->_end = cur_lock->_end;
 cur_lock->_end = cur_ts;
 cur_lock->insert(new_lock);
 }
 cur_lock->_tids->insert(tid);
 cur_ts = cur_lock->_start - 1;
 }
 break;
 }
 case WRITE_LOCK:
 {
 *last = cur_ts;
 cur_lock->_events->insert(event);
 locks.clear();
 if (new_lock != nullptr)
 new_lock->balance();
 return false;
 }
 case FREEZE_READ_LOCK:
 {
 if (is_write) {
 *last = cur_ts;
 locks.clear();
 if (new_lock != nullptr)
 new_lock->balance();
 return true;
 } else {
 cur_lock->_tids->insert(tid);
 cur_ts = cur_lock->_start - 1;
 }
 break;
 }
 case FREEZE_WRITE_LOCK:
 {
 *last = cur_ts;
 locks.clear();
 if (new_lock != nullptr)
 new_lock->balance();
 return true;
 }
 }
 if (cur_lock->_left != nullptr)
 cur_lock->_left->find(&locks, cur_ts);
 }
 }

 bool AVLTreeLock::tryLock(uint64_t tid, uint64_t ts_start, uint64_t ts_end, bool is_write) {
 std::vector<AVLTreeLock *> pend_locks;
 std::vector<AVLTreeLock *> locks;
 AVLTreeLock *cur_lock;
 bool res = true;

 find(&locks, ts_end);
 while (!locks.empty()) {
 cur_lock = locks.back();
 locks.pop_back();
 if (cur_lock->_end < ts_start) {
 locks.clear();
 break;
 }
 switch (cur_lock->_status) {
 case FREE: {
 pend_locks.push_back(cur_lock);
 break;
 }
 case READ_LOCK: {
 if (is_write) {
 pend_locks.clear();
 locks.clear();
 res = false;
 } else {
 pend_locks.push_back(cur_lock);
 }
 break;
 }
 case WRITE_LOCK:
 {
 pend_locks.clear();
 locks.clear();
 res = false;
 break;
 }
 case FREEZE_READ_LOCK:
 {
 if (is_write) {
 pend_locks.clear();
 locks.clear();
 res = false;
 }
 break;
 }
 case FREEZE_WRITE_LOCK:
 {
 pend_locks.clear();
 locks.clear();
 res = false;
 break;
 }
 }
 if (cur_lock->_left != nullptr)
 cur_lock->_left->find(&locks, cur_lock->_start-1);
 }
 for (std::vector<AVLTreeLock *>::iterator it = pend_locks.begin(); it != pend_locks.end(); ++it) {
 if ((*it)->_start < ts_start) {
 AVLTreeLock *new_lock = new AVLTreeLock(*it);

 new_lock->_end = ts_start-1;
 (*it)->_start = ts_start;
 (*it)->insert(new_lock);
 new_lock->balance();
 }
 if ((*it)->_end > ts_end) {
 AVLTreeLock *new_lock = new AVLTreeLock(*it);

 new_lock->_start = ts_end+1;
 (*it)->_end = ts_end;
 (*it)->insert(new_lock);
 new_lock->balance();
 }
 if (is_write)
 (*it)->_status = WRITE_LOCK;
 else
 (*it)->_status = READ_LOCK;
 (*it)->_tids->insert(tid);
 }
 return res;
 }

 void AVLTreeLock::freeze(uint64_t tid, uint64_t ts_start, uint64_t ts_end, TServer *tserver) {
 std::vector<AVLTreeLock *> pend_locks;
 std::vector<AVLTreeLock *> locks;
 AVLTreeLock *cur_lock;
 AVLTreeLock *next = nullptr;

 find(&locks, ts_end+1);
 while (!locks.empty()) {
 cur_lock = locks.back();
 locks.pop_back();
 if ((ts_start != 0) && (cur_lock->_end < ts_start-1)) {
 locks.clear();
 break;
 }
 switch (cur_lock->_status) {
 case FREE:
 {
 break;
 }
 case READ_LOCK:
 {
 if (cur_lock->_tids->find(tid) != cur_lock->_tids->end())
 pend_locks.push_back(cur_lock);
 break;
 }
 case WRITE_LOCK:
 {
 if (cur_lock->_tids->find(tid) != cur_lock->_tids->end())
 pend_locks.push_back(cur_lock);
 break;
 }
 case FREEZE_READ_LOCK:
 {
 pend_locks.push_back(cur_lock);
 break;
 }
 case FREEZE_WRITE_LOCK:
 {
 pend_locks.push_back(cur_lock);
 break;
 }
 }
 if (cur_lock->_left != nullptr)
 cur_lock->_left->find(&locks, cur_lock->_start - 1);
 }
 for (std::vector<AVLTreeLock *>::iterator it = pend_locks.begin(); it != pend_locks.end(); ++it) {
 cur_lock = *it;
 if ((cur_lock->_end >= ts_start) && (cur_lock->_start <= ts_end)) {
 switch (cur_lock->_status) {
 case FREE:
 {
 std::cerr << "Lock [" << cur_lock->_start << "," << cur_lock->_end << "] should be locked" << std::endl;
 exit(-1);
 }
 case READ_LOCK:
 {
 cur_lock->_status = FREEZE_READ_LOCK;
 cur_lock->_tids->clear();
 for (std::set<void *>::iterator it = cur_lock->_events->begin(); it != cur_lock->_events->end(); ++it)
 tserver->addEvent(reinterpret_cast<ServerEvent *>(*it));
 if ((next != nullptr) && (next->_status == FREEZE_READ_LOCK) && (next->_start == cur_lock->_end+1)) {
 cur_lock->_end = next->_end;
 remove(next);
 }
 break;
 }
 case WRITE_LOCK:
 {
 cur_lock->_status = FREEZE_WRITE_LOCK;
 cur_lock->_tids->clear();
 for (std::set<void *>::iterator it = cur_lock->_events->begin(); it != cur_lock->_events->end(); ++it)
 tserver->addEvent(reinterpret_cast<ServerEvent *>(*it));
 if ((next != nullptr) && (next->_status == FREEZE_WRITE_LOCK) && (next->_start == cur_lock->_end+1)) {
 cur_lock->_end = next->_end;
 remove(next);
 }
 break;
 }
 case FREEZE_READ_LOCK:
 {
 if ((next != nullptr) && (next->_status == FREEZE_READ_LOCK) && (next->_start == cur_lock->_end+1)) {
 cur_lock->_end = next->_end;
 remove(next);
 }
 break;
 }
 case FREEZE_WRITE_LOCK:
 {
 if ((next != nullptr) && (next->_status == FREEZE_WRITE_LOCK) && (next->_start == cur_lock->_end + 1)) {
 cur_lock->_end = next->_end;
 remove(next);
 }
 break;
 }
 }
 }
 next = cur_lock;
 }
 }

 void AVLTreeLock::unlock(uint64_t tid, uint64_t ts_start, uint64_t ts_end, TServer *tserver) {
 std::vector<AVLTreeLock *> pend_locks;
 std::vector<AVLTreeLock *> locks;
 AVLTreeLock *cur_lock;
 AVLTreeLock *next = nullptr;

 find(&locks, ts_end+1);
 while (!locks.empty()) {
 cur_lock = locks.back();
 locks.pop_back();
 if ((ts_start != 0) && (cur_lock->_end < ts_start-1)) {
 locks.clear();
 break;
 }
 switch (cur_lock->_status) {
 case FREE:
 {
 pend_locks.push_back(cur_lock);
 break;
 }
 case READ_LOCK:
 {
 if (cur_lock->_tids->find(tid) != cur_lock->_tids->end())
 pend_locks.push_back(cur_lock);
 break;
 }
 case WRITE_LOCK:
 {
 if (cur_lock->_tids->find(tid) != cur_lock->_tids->end())
 pend_locks.push_back(cur_lock);
 break;
 }
 case FREEZE_READ_LOCK:
 {
 break;
 }
 case FREEZE_WRITE_LOCK:
 {
 break;
 }
 }
 if (cur_lock->_left != nullptr)
 cur_lock->_left->find(&locks, cur_lock->_start - 1);
 }
 for (std::vector<AVLTreeLock *>::iterator it = pend_locks.begin(); it != pend_locks.end(); ++it) {
 cur_lock = *it;
 if ((cur_lock->_end >= ts_start) && (cur_lock->_start <= ts_end)) {
 switch (cur_lock->_status) {
 case FREE:
 {
 if ((next != nullptr) && (next->_status == FREE) && (next->_start == cur_lock->_end+1)) {
 cur_lock->_end = next->_end;
 remove(next);
 }
 break;
 }
 case READ_LOCK:
 {
 cur_lock->_tids->erase(tid);
 if (cur_lock->_tids->empty()) {
 cur_lock->_status = FREE;
 for (std::set<void *>::iterator it = cur_lock->_events->begin(); it != cur_lock->_events->end(); ++it)
 tserver->addEvent(reinterpret_cast<ServerEvent *>(*it));
 if ((next != nullptr) && (next->_status == FREE) && (next->_start == cur_lock->_end+1)) {
 cur_lock->_end = next->_end;
 remove(next);
 }
 }
 break;
 }
 case WRITE_LOCK:
 {
 cur_lock->_tids->erase(tid);
 if (cur_lock->_tids->empty()) {
 cur_lock->_status = FREE;
 for (std::set<void *>::iterator it = cur_lock->_events->begin(); it != cur_lock->_events->end(); ++it)
 tserver->addEvent(reinterpret_cast<ServerEvent *>(*it));
 if ((next != nullptr) && (next->_status == FREE) && (next->_start == cur_lock->_end+1)) {
 cur_lock->_end = next->_end;
 remove(next);
 }
 }
 break;
 }
 case FREEZE_READ_LOCK:
 {
 std::cerr << "Lock [" << cur_lock->_start << "," << cur_lock->_end << "] should not be frozen" << std::endl;
 exit(-1);
 }
 case FREEZE_WRITE_LOCK:
 {
 std::cerr << "Lock [" << cur_lock->_start << "," << cur_lock->_end << "] should not be frozen" << std::endl;
 exit(-1);
 }
 }
 }
 next = cur_lock;
 }
 }*/

std::string AVLLockNode::toString() {
  std::string res = "";

  res += "Interval: [" + std::to_string(_start) + "," + std::to_string(_end) + "]\n";
  res += "Status: ";
  switch (_status) {
    case FREE:
      res += "FREE";
      break;
    case READ_LOCK:
      res += "READ_LOCK\n";
      res += "Transaction IDs:";
      for (std::set<uint64_t>::iterator it = _tids->begin(); it != _tids->end(); ++it)
        res += " " + std::to_string(*it);
      res += "\nEvents:";
      for (std::set<void *>::iterator it = _events->begin(); it != _events->end(); ++it)
        res += " " + std::to_string(reinterpret_cast<uint64_t>(*it));
      break;
    case WRITE_LOCK:
      res += "WRITE_LOCK\n";
      res += "Transaction IDs:";
      for (std::set<uint64_t>::iterator it = _tids->begin(); it != _tids->end(); ++it)
        res += " " + std::to_string(*it);
      res += "\nEvents:";
      for (std::set<void *>::iterator it = _events->begin(); it != _events->end(); ++it)
        res += " " + std::to_string(reinterpret_cast<uint64_t>(*it));
      break;
    case FREEZE_READ_LOCK:
      res += "FREEZE_READ_LOCK";
      break;
    case FREEZE_WRITE_LOCK:
      res += "FREEZE_WRITE_LOCK";
      break;
    default:
      std::cerr << "Status is necessary." << std::endl;
      exit(-1);
  }
  res += "\n";
  res += "Height: (" + std::to_string(_left_height) + "," + std::to_string(_right_height) + ")\n";

  if (_left != nullptr)
    res += "\n" + _left->toString();

  if (_right != nullptr)
    res += "\n" + _right->toString();
  return res;
}
