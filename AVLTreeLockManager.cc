/*
 * AVLTreeLockManager.cc
 *
 *  Created on: Aug 2, 2016
 *      Author: theo
 */

#ifndef AVLTREELOCKMANAGER_CC_
#define AVLTREELOCKMANAGER_CC_

#include <cassert>
#include <iostream>
#include <utility>

#include "AVLTreeLock.h"
#include "AVLTreeLockManager.h"

AVLTreeLockManager::AVLTreeLockManager() {
}

AVLTreeLockManager::~AVLTreeLockManager() {
  _key_to_locks.clear();
}

void AVLTreeLockManager::createNewLock(uint64_t key) {
  std::mutex *temp_mutex = new std::mutex();
  AVLTreeLock *temp_tree = new AVLTreeLock();
  std::pair<std::mutex*,AVLTreeLock*> *temp_pair = new std::pair<std::mutex*,AVLTreeLock*> (temp_mutex, temp_tree);
  std::set<uint64_t> keys;

  keys.insert(0);
  temp_mutex->lock();
  _map_lock.lock();
  if (_key_to_locks.count(key) == 1) {
    std::cerr << "AVLTreeLockManager Error: Key " << key << " already has an associated AVL Tree." << std::endl;
    return;
  }
  _key_to_locks[key] = temp_pair;
  _map_lock.unlock();
  tryLock(key, 0, 0, 0, false);
  freeze(keys, 0, 0, 0);
  temp_mutex->unlock();
}

bool AVLTreeLockManager::tryBackwardLock(uint64_t key, uint64_t tid, uint64_t ts, bool is_read, Event *event,
    uint64_t *last) {
  std::vector<AVLTreeLock *> locks;
  AVLTreeLock *root;
  AVLTreeLock *new_lock = nullptr;
  AVLTreeLock *cur_lock;
  uint64_t cur_ts = ts;
  bool res = true;

  // Find the corresponding lock and lock it. If it does not exist create one.
  if ((_key_to_locks.find(key) == _key_to_locks.end()) && (_key_to_locks.find(key) == _key_to_locks.end()))
      createNewLock(key);
  _key_to_locks[key]->first->lock();

  // Start locking from ts and go backwards.
  root = _key_to_locks[key]->second;
  root->find(&locks, cur_ts);
  while (!locks.empty()) {
    cur_lock = locks.back();
    locks.pop_back();

    if ((cur_ts < cur_lock->_start) || (cur_ts > cur_lock->_end)) {
      std::cerr << "AVLTreeLockManager Error: Find does not work as intended. Time " << cur_ts << " is not in ["
                << cur_lock->_start << "," << cur_lock->_end << "]" << std::endl;
      return false;
    }

    switch (cur_lock->_status) {
      case FREE: {
        if (cur_ts < cur_lock->_end) {
          new_lock = new AVLTreeLock(cur_lock);
          new_lock->_start = cur_ts + 1;
          new_lock->_end = cur_lock->_end;
          cur_lock->_end = cur_ts;
          root = cur_lock->insert(new_lock);
        }
        cur_lock->_status = is_read ? READ_LOCK : WRITE_LOCK;
        cur_lock->_tids->insert(tid);
        cur_ts = cur_lock->_start - 1;
        break;
      }
      case READ_LOCK: {
        if (is_read) {
          if (cur_ts < cur_lock->_end) {
            new_lock = new AVLTreeLock(cur_lock);
            new_lock->_start = cur_ts + 1;
            new_lock->_end = cur_lock->_end;
            cur_lock->_end = cur_ts;
            root = cur_lock->insert(new_lock);
          }
          cur_lock->_tids->insert(tid);
          cur_ts = cur_lock->_start - 1;
        } else {
          cur_lock->_events->insert(event);
          res = false;
        }
        break;
      }
      case WRITE_LOCK: {
        cur_lock->_events->insert(event);
        res = false;
        break;
      }
      case FREEZE_READ_LOCK: {
        if (is_read)
          cur_ts = cur_lock->_start - 1;
        else
          res = true;
        break;
      }
      case FREEZE_WRITE_LOCK: {
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
  _key_to_locks[key]->second = root;
  _key_to_locks[key]->first->unlock();

  return res;
}

bool AVLTreeLockManager::tryLock(uint64_t key, uint64_t tid, uint64_t ts_start, uint64_t ts_end, bool is_read) {
  std::vector<AVLTreeLock *> pend_locks;
  std::vector<AVLTreeLock *> locks;
  AVLTreeLock *root;
  AVLTreeLock *cur_lock;
  bool res = true;

  // Find the corresponding lock and lock it. If it does not exist create one.
  if ((_key_to_locks.find(key) == _key_to_locks.end()) && (_key_to_locks.find(key) == _key_to_locks.end()))
      createNewLock(key);
  _key_to_locks[key]->first->lock();

  // Find the interval [ts_start,ts_end]. Check if it can be locked.
  root = _key_to_locks[key]->second;
  root->find(&locks, ts_end);
  while (!locks.empty()) {
    cur_lock = locks.back();
    locks.pop_back();
    switch (cur_lock->_status) {
      case FREE: {
        pend_locks.push_back(cur_lock);
        break;
      }
      case READ_LOCK: {
        if (is_read)
          pend_locks.push_back(cur_lock);
        else
          res = false;
        break;
      }
      case WRITE_LOCK: {
        res = false;
        break;
      }
      case FREEZE_READ_LOCK: {
        if (!is_read)
          res = false;
        break;
      }
      case FREEZE_WRITE_LOCK: {
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
  for (std::vector<AVLTreeLock *>::iterator it = pend_locks.begin(); it != pend_locks.end(); ++it) {
    if ((*it)->_start < ts_start) {
      AVLTreeLock *new_lock = new AVLTreeLock(*it);

      new_lock->_end = ts_start - 1;
      (*it)->_start = ts_start;
      root = (*it)->insert(new_lock);
    }
    if ((*it)->_end > ts_end) {
      AVLTreeLock *new_lock = new AVLTreeLock(*it);

      new_lock->_start = ts_end + 1;
      (*it)->_end = ts_end;
      root = (*it)->insert(new_lock);
    }
    if (is_read)
      (*it)->_status = READ_LOCK;
    else
      (*it)->_status = WRITE_LOCK;
    (*it)->_tids->insert(tid);
  }

  // Update lock and root for AVL Tree.
  _key_to_locks[key]->second = root;
  _key_to_locks[key]->first->unlock();

  return res;
}

void AVLTreeLockManager::freeze(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end) {
  std::set<Event *> events;

  for (std::set<uint64_t>::iterator it = keys.begin(); it != keys.end(); ++it) {
    std::vector<AVLTreeLock *> locks;
    AVLTreeLock *root;
    AVLTreeLock *cur_lock;
    uint64_t key = (*it);
    uint64_t ts = ts_end;

    _key_to_locks[key]->first->lock();
    root = _key_to_locks[key]->second;
    cur_lock = root;
    while (ts >= ts_start) {
      cur_lock->find(&locks, ts);
      cur_lock = locks.back();
      locks.pop_back();
      switch (cur_lock->_status) {
        case FREE:
          continue;
        case READ_LOCK:
          if (cur_lock->_tids->find(tid) != cur_lock->_tids->end()) {
            cur_lock->_status = FREEZE_READ_LOCK;
            cur_lock->_tids->clear();
            events.insert(cur_lock->_events->begin(), cur_lock->_events->end());
          }
          break;
        case WRITE_LOCK:
          if (cur_lock->_tids->find(tid) != cur_lock->_tids->end()) {
            cur_lock->_status = FREEZE_WRITE_LOCK;
            cur_lock->_tids->clear();
            if (!events.empty())
              std::cerr << "AVLTreeLockManager Error: Events should not be filled when status is WRITE_LOCK."
                        << std::endl;
          }
          break;
        case FREEZE_READ_LOCK:
          continue;
        case FREEZE_WRITE_LOCK:
          continue;
      }
      ts = cur_lock->_start - 1;
    }
    _key_to_locks[key]->first->unlock();
  }
  for (std::set<Event *>::iterator it = events.begin(); it != events.end(); ++it)
    (*it)->Run();
}

void AVLTreeLockManager::unlock(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end) {
  std::set<Event *> events;

  for (std::set<uint64_t>::iterator it = keys.begin(); it != keys.end(); ++it) {
    std::vector<AVLTreeLock *> locks;
    AVLTreeLock *root;
    AVLTreeLock *cur_lock;
    uint64_t key = (*it);
    uint64_t ts = ts_end;

    _key_to_locks[key]->first->lock();
    root = _key_to_locks[key]->second;
    cur_lock = root;
    while (ts >= ts_start) {
      cur_lock->find(&locks, ts);
      cur_lock = locks.back();
      locks.pop_back();
      switch (cur_lock->_status) {
        case FREE:
          continue;
        case READ_LOCK:
          if (cur_lock->_tids->find(tid) != cur_lock->_tids->end()) {
            cur_lock->_status = FREE;
            cur_lock->_tids->clear();
            events.insert(cur_lock->_events->begin(), cur_lock->_events->end());
          }
          break;
        case WRITE_LOCK:
          if (cur_lock->_tids->find(tid) != cur_lock->_tids->end()) {
            cur_lock->_status = FREE;
            cur_lock->_tids->clear();
            if (!events.empty())
              std::cerr << "AVLTreeLockManager Error: Events should not be filled when status is WRITE_LOCK."
                        << std::endl;
          }
          break;
        case FREEZE_READ_LOCK:
          continue;
        case FREEZE_WRITE_LOCK:
          continue;
      }
      ts = cur_lock->_start - 1;
    }
    _key_to_locks[key]->first->unlock();
  }
  for (std::set<Event *>::iterator it = events.begin(); it != events.end(); ++it)
    (*it)->Run();
}

int main(int argc, char **argv) {
  AVLTreeLockManager *avl_lock_manager = new AVLTreeLockManager();
  uint64_t last;

  for (int i = 0; i < 5; i++) {
    assert(avl_lock_manager->tryBackwardLock(0, i, 4 * i, true, nullptr, &last));
    assert(last == 0);
    if (i % 3 == 1) {
      assert(avl_lock_manager->tryBackwardLock(1, i, 4 * i, true, nullptr, &last));
      assert(last == 0);
    }
    if (i % 2 == 0) {
      assert(avl_lock_manager->tryBackwardLock(2, i, 4 * i, true, nullptr, &last));
      assert(last == 0);
    }
  }
  assert(avl_lock_manager->tryLock(0, 1, 25, 25, false));
  assert(!avl_lock_manager->tryLock(0, 2, 15, 15, false));

  return 0;
}

#endif /* AVLLOCKMANAGER_CC_ */
