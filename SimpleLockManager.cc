/**
 * SimpleLockManager.cc
 *
 *  Created on: Aug 2, 2016
 *      Author: theo
 */

#ifndef SIMPLELOCKMANAGER_CC_
#define SIMPLELOCKMANAGER_CC_

#include <iostream>
#include <utility>
#include <assert.h>
#include "SimpleLockManager.h"

SimpleLockManager::SimpleLockManager() {}

SimpleLockManager::~SimpleLockManager() {
  _key_to_locks.clear();
}

void SimpleLockManager::createEntry(uint64_t key) {
  std::mutex *temp_mutex = new std::mutex();
  AVLTreeLock *temp_lock = AVLTreeLock::createLock();
  std::pair<std::mutex *, AVLTreeLock *> *value = new std::pair<std::mutex *, AVLTreeLock *>(temp_mutex, temp_lock);
  std::pair<uint64_t, std::pair<std::mutex *, AVLTreeLock *> *> key_value(key, value);

  _key_to_locks.insert(key_value);
}

bool SimpleLockManager::tryBackwardLock(uint64_t key, uint64_t tid, uint64_t ts, bool is_read, void *event,
                                        uint64_t *last) {
  std::vector<AVLTreeLock *> locks;
  AVLTreeLock *root;
  AVLTreeLock *new_lock = nullptr;
  AVLTreeLock *cur_lock;
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
      case AVLTreeLock::FREE:
      {
        if (cur_ts < cur_lock->_end) {
          new_lock = new AVLTreeLock(cur_lock);
          new_lock->_start = cur_ts + 1;
          new_lock->_end = cur_lock->_end;
          cur_lock->_end = cur_ts;
          cur_lock->insert(new_lock);
        }
        cur_lock->_status = is_read ? AVLTreeLock::READ_LOCK : AVLTreeLock::WRITE_LOCK;
        cur_lock->_tids->insert(tid);
        cur_ts = cur_lock->_start-1;
        break;
      }
      case AVLTreeLock::READ_LOCK:
      {
        if (is_read) {
          if (cur_ts < cur_lock->_end) {
            new_lock = new AVLTreeLock(cur_lock);
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
      case AVLTreeLock::WRITE_LOCK:
      {
        cur_lock->_events->insert(event);
        res = false;
        break;
      }
      case AVLTreeLock::FREEZE_READ_LOCK:
      {
        if (is_read)
          cur_ts = cur_lock->_start - 1;
        else
          res = true;
        break;
      }
      case AVLTreeLock::FREEZE_WRITE_LOCK:
      {
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
  _key_to_locks[key]->second = _key_to_locks[key]->second->getRoot();
  _key_to_locks[key]->first->unlock();

  return res;
}

bool SimpleLockManager::tryLock(uint64_t key, uint64_t tid, uint64_t ts_start, uint64_t ts_end, bool is_read) {
  std::vector<AVLTreeLock *> pend_locks;
  std::vector<AVLTreeLock *> locks;
  AVLTreeLock *root;
  AVLTreeLock *cur_lock;
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
      case AVLTreeLock::FREE:
      {
        pend_locks.push_back(cur_lock);
        break;
      }
      case AVLTreeLock::READ_LOCK:
      {
        if (is_read)
          pend_locks.push_back(cur_lock);
        else
          res = false;
        break;
      }
      case AVLTreeLock::WRITE_LOCK:
      {
        res = false;
        break;
      }
      case AVLTreeLock::FREEZE_READ_LOCK:
      {
        if (!is_read)
          res = false;
        break;
      }
      case AVLTreeLock::FREEZE_WRITE_LOCK:
      {
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
      cur_lock->find(&locks, ts_start-1);
    else
      locks.clear();
  }

  // Lock the  interval [ts_start,ts_end] if it is available.
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
    if (is_read)
      (*it)->_status = AVLTreeLock::READ_LOCK;
    else
      (*it)->_status = AVLTreeLock::WRITE_LOCK;
    (*it)->_tids->insert(tid);
  }

  // Update lock and root for AVL Tree.
  _key_to_locks[key]->second = _key_to_locks[key]->second->getRoot();
  _key_to_locks[key]->first->unlock();
  return res;
}

void SimpleLockManager::freeze(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end,
                               TServer *tserver) {
}

void SimpleLockManager::unlock(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end,
                               TServer *tserver) {
}

std::string SimpleLockManager::lockToString(uint64_t key) {
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
  SimpleLockManager *simple_lock_manager = new SimpleLockManager();
  uint64_t last;

  for (int i = 0; i < 5; i++) {
    assert(simple_lock_manager->tryBackwardLock(0, i, 4*i, true, simple_lock_manager, &last));
    assert(last == 0);
    if (i % 3 == 1) {
      assert(simple_lock_manager->tryBackwardLock(1, i, 4*i, true, simple_lock_manager, &last));
      assert(last == 0);
    }
    if (i % 2 == 0) {
      assert(simple_lock_manager->tryBackwardLock(2, i, 4*i, true, simple_lock_manager, &last));
      assert(last == 0);
    }
  }

  assert(simple_lock_manager->tryLock(0, 1, 25, 25, false));
  assert(!simple_lock_manager->tryLock(0, 2, 15, 15, false));


  for (int i = 0; i < 3; i++)
    std::cout << "Key " << i << std::endl << simple_lock_manager->lockToString(i) << std::endl;
  return 0;
}

#endif /* SIMPLELOCKMANAGER_CC_ */
