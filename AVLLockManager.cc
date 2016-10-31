/**
 * AVLLockManager.cc
 *
 *  Created on: Aug 2, 2016
 *      Author: theo
 */

#ifndef AVLLOCKMANAGER_CC_
#define AVLLOCKMANAGER_CC_

#include "AVLLockManager.h"

#include <cassert>
#include <iostream>
#include <utility>

AVLLockManager::AVLLockManager() {
}

AVLLockManager::~AVLLockManager() {
  _key_to_locks.clear();
}

void AVLLockManager::createEntry(uint64_t key) {
  std::mutex *temp_mutex = new std::mutex();
  AVLLockNode *temp_lock = AVLLockNode::createLockTree();
  std::pair<std::mutex *, AVLLockNode *> *value = new std::pair<std::mutex *, AVLLockNode *>(temp_mutex, temp_lock);
  std::pair<uint64_t, std::pair<std::mutex *, AVLLockNode *> *> key_value(key, value);

  _key_to_locks.insert(key_value);
}

bool AVLLockManager::tryBackwardLock(uint64_t key, uint64_t tid, uint64_t ts, bool is_read, void *event,
    uint64_t *last) {
  std::vector<AVLLockNode *> locks;
  AVLLockNode *root;
  AVLLockNode *new_lock = nullptr;
  AVLLockNode *cur_lock;
  uint64_t cur_ts = ts;
  bool res = true;

  // Find the corresponding lock. If it does not exist create one.
  if (_key_to_locks.find(key) == _key_to_locks.end()) {
    _map_lock.lock();
    if (_key_to_locks.find(key) == _key_to_locks.end())
      createEntry(key);
    _map_lock.unlock();
  }
  _key_to_locks[key]->first->lock();

  // Start locking from ts and go backwards.
  root = _key_to_locks[key]->second;
  root->find(&locks, cur_ts);
  while (!locks.empty()) {
    cur_lock = locks.back();
    locks.pop_back();

    if ((cur_ts < cur_lock->_start) || (cur_ts > cur_lock->_end)) {
      std::cerr << "Find does not work as intended. Time " << cur_ts << " is not in [" << cur_lock->_start << ","
          << cur_lock->_end << "]" << std::endl;
      exit(-1);
    }

    switch (cur_lock->_status) {
      case AVLLockNode::FREE: {
        if (cur_ts < cur_lock->_end) {
          new_lock = new AVLLockNode(cur_lock);
          new_lock->_start = cur_ts + 1;
          new_lock->_end = cur_lock->_end;
          cur_lock->_end = cur_ts;
          cur_lock->insert(new_lock);
        }
        cur_lock->_status = is_read ? AVLLockNode::READ_LOCK : AVLLockNode::WRITE_LOCK;
        cur_lock->_tids->insert(tid);
        cur_ts = cur_lock->_start - 1;
        break;
      }
      case AVLLockNode::READ_LOCK: {
        if (is_read) {
          if (cur_ts < cur_lock->_end) {
            new_lock = new AVLLockNode(cur_lock);
            new_lock->_start = cur_ts + 1;
            new_lock->_end = cur_lock->_end;
            cur_lock->_end = cur_ts;
            cur_lock->insert(new_lock);
          }
          cur_lock->_tids->insert(tid);
          cur_ts = cur_lock->_start - 1;
        } else {
          cur_lock->_events->insert(event);
          res = false;
        }
        break;
      }
      case AVLLockNode::WRITE_LOCK: {
        cur_lock->_events->insert(event);
        res = false;
        break;
      }
      case AVLLockNode::FREEZE_READ_LOCK: {
        if (is_read)
          cur_ts = cur_lock->_start - 1;
        else
          res = true;
        break;
      }
      case AVLLockNode::FREEZE_WRITE_LOCK: {
        res = true;
        break;
      }
    }
    if (cur_ts != cur_lock->_start - 1)
      break;
    cur_lock->find(&locks, cur_ts);
  }

  // Update the result and new root.
  *last = cur_ts;
  if (new_lock != nullptr)
    new_lock->balance();
  // _key_to_locks[key]->second = _key_to_locks[key]->second->getRoot();
  _key_to_locks[key]->first->unlock();

  return res;
}

bool AVLLockManager::tryLock(uint64_t key, uint64_t tid, uint64_t ts_start, uint64_t ts_end, bool is_read) {
  std::vector<AVLLockNode *> pend_locks;
  std::vector<AVLLockNode *> locks;
  AVLLockNode *root;
  AVLLockNode *cur_lock;
  bool res = true;

  // Find the corresponding lock. If it does not exist create one.
  if (_key_to_locks.find(key) == _key_to_locks.end()) {
    _map_lock.lock();
    if (_key_to_locks.find(key) == _key_to_locks.end())
      createEntry(key);
    _map_lock.unlock();
  }
  _key_to_locks[key]->first->lock();

  // Find the interval [ts_start,ts_end]. Check if it can be locked.
  root = _key_to_locks[key]->second;
  root->find(&locks, ts_end);
  while (!locks.empty()) {
    cur_lock = locks.back();
    locks.pop_back();
    switch (cur_lock->_status) {
      case AVLLockNode::FREE: {
        pend_locks.push_back(cur_lock);
        break;
      }
      case AVLLockNode::READ_LOCK: {
        if (is_read)
          pend_locks.push_back(cur_lock);
        else
          res = false;
        break;
      }
      case AVLLockNode::WRITE_LOCK: {
        res = false;
        break;
      }
      case AVLLockNode::FREEZE_READ_LOCK: {
        if (!is_read)
          res = false;
        break;
      }
      case AVLLockNode::FREEZE_WRITE_LOCK: {
        res = false;
        break;
      }
    }
    if (!res) {
      pend_locks.clear();
      locks.clear();
      break;
    }
    if (cur_lock->_start > ts_start)
      cur_lock->find(&locks, ts_start - 1);
    else
      locks.clear();
  }

  // Lock the  interval [ts_start,ts_end] if it is available.
  for (std::vector<AVLLockNode *>::iterator it = pend_locks.begin(); it != pend_locks.end(); ++it) {
    if ((*it)->_start < ts_start) {
      AVLLockNode *new_lock = new AVLLockNode(*it);

      new_lock->_end = ts_start - 1;
      (*it)->_start = ts_start;
      (*it)->insert(new_lock);
      new_lock->balance();
    }
    if ((*it)->_end > ts_end) {
      AVLLockNode *new_lock = new AVLLockNode(*it);

      new_lock->_start = ts_end + 1;
      (*it)->_end = ts_end;
      (*it)->insert(new_lock);
      new_lock->balance();
    }
    if (is_read)
      (*it)->_status = AVLLockNode::READ_LOCK;
    else
      (*it)->_status = AVLLockNode::WRITE_LOCK;
    (*it)->_tids->insert(tid);
  }

  // Update lock and root for AVL Tree.
  // _key_to_locks[key]->second = _key_to_locks[key]->second->getRoot();
  _key_to_locks[key]->first->unlock();
  return res;
}

void AVLLockManager::freeze(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end,
    TServer *tserver) {
}

void AVLLockManager::unlock(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end,
    TServer *tserver) {
}

std::string AVLLockManager::lockToString(uint64_t key) {
  std::string res;

  if (_key_to_locks.find(key) == _key_to_locks.end()) {
    _map_lock.lock();
    if (_key_to_locks.find(key) == _key_to_locks.end())
      createEntry(key);
    _map_lock.unlock();
  }

  _key_to_locks[key]->first->lock();
  res = _key_to_locks[key]->second->toString();
  _key_to_locks[key]->first->unlock();
  return res;
}

int main(int argc, char **argv) {
  AVLLockManager *simple_lock_manager = new AVLLockManager();
  uint64_t last;

  for (int i = 0; i < 5; i++) {
    assert(simple_lock_manager->tryBackwardLock(0, i, 4 * i, true, simple_lock_manager, &last));
    assert(last == 0);
    if (i % 3 == 1) {
      assert(simple_lock_manager->tryBackwardLock(1, i, 4 * i, true, simple_lock_manager, &last));
      assert(last == 0);
    }
    if (i % 2 == 0) {
      assert(simple_lock_manager->tryBackwardLock(2, i, 4 * i, true, simple_lock_manager, &last));
      assert(last == 0);
    }
  }

  assert(simple_lock_manager->tryLock(0, 1, 25, 25, false));
  assert(!simple_lock_manager->tryLock(0, 2, 15, 15, false));

  for (int i = 0; i < 3; i++)
    std::cout << "Key " << i << std::endl << simple_lock_manager->lockToString(i) << std::endl;
  return 0;
}

#endif /* AVLLOCKMANAGER_CC_ */
