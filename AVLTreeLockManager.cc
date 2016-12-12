/*
 * AVLTreeLockManager.cc
 *
 *  Created on: Aug 2, 2016
 *      Author: theo
 */

#include <cassert>
#include <iostream>
#include <utility>

#include "AVLTreeLockManager.h"
#include "TestEvent.h"

AVLTreeLockManager::AVLTreeLockManager() {
  int err;

  // Create lock for key to key information dictionary.
  err = pthread_rwlock_init(&_key_map_lock, nullptr);
  if (err != 0) {
    std::cerr << "AVLTreeLockManager: Failed to create pthread locks with error " << err << std::endl;
    exit(0);
  }

  // Create lock for transaction ID to transaction information dictionary.
  err = pthread_rwlock_init(&_transaction_map_lock, NULL);
  if (err != 0) {
    std::cerr << "AVLTreeLockManager: Failed to create pthread locks with error " << err << std::endl;
    exit(0);
  }
}

AVLTreeLockManager::~AVLTreeLockManager() {
  // Destroy key to key information dictionary.
  pthread_rwlock_wrlock(&_key_map_lock);
  for (std::map<uint64_t, KeyInfo *>::iterator it = _key_map.begin(); it != _key_map.end(); ++it) {
    it->second->_mutex.lock();
    it->second->_events.clear();
    delete it->second->_root;
    it->second->_mutex.unlock();
  }
  _key_map.clear();
  pthread_rwlock_unlock(&_key_map_lock);
  pthread_rwlock_destroy(&_key_map_lock);

  // Destroy transaction ID to transaction information dictionary.
  pthread_rwlock_wrlock(&_transaction_map_lock);
  for (std::map<uint64_t, TransactionInfo *>::iterator it = _transaction_map.begin(); it != _transaction_map.end();
      ++it) {
    it->second->_mutex.lock();
    it->second->_intervals.clear();
    it->second->_mutex.unlock();
  }
  _transaction_map.clear();
  pthread_rwlock_unlock(&_transaction_map_lock);
  pthread_rwlock_destroy(&_transaction_map_lock);
}

void AVLTreeLockManager::createKeyInfo(uint64_t key) {
  KeyInfo *key_info;
  AVLTreeLockNode *root;
  AVLTreeLockNode *temp_node;

  // See if key already exists.
  pthread_rwlock_wrlock(&_key_map_lock);
  if (_key_map.find(key) != _key_map.end()) {
    pthread_rwlock_unlock(&_key_map_lock);
    return;
  }

  // Construct initial AVL tree.
  root = new AVLTreeLockNode();
  root->_status = FREEZE_WRITE_LOCK;
  root->_end = 0;
  temp_node = new AVLTreeLockNode();
  temp_node->_start = 1;
  root = root->insert(temp_node);

  // Initialize key info.
  key_info = new KeyInfo();
  key_info->_root = root;

  // Set corresponding entry in map.
  _key_map[key] = key_info;
  pthread_rwlock_unlock(&_key_map_lock);
}

void AVLTreeLockManager::addEvent(uint64_t key, uint64_t ts_end, Event *event) {
  KeyInfo *key_info;

  // Find key information.
  if (_key_map.find(key) == _key_map.end()) {
    std::cerr << "AVLTreeLockManager: Key " << key << "was not found when trying to add event." << std::endl;
    return;
  }
  key_info = _key_map[key];

  // Associate event with key.
  if (key_info->_events.find(ts_end) == key_info->_events.end())
    key_info->_events[ts_end] = new std::set<Event *> ();
  key_info->_events[ts_end]->insert(event);
}

void AVLTreeLockManager::createTransactionInfo(uint64_t tid) {
  // See if transaction information already exists.
  pthread_rwlock_wrlock(&_transaction_map_lock);
  if (_transaction_map.find(tid) != _transaction_map.end()) {
    pthread_rwlock_unlock(&_transaction_map_lock);
    return;
  }

  // Add transaction info.
  _transaction_map[tid] = new TransactionInfo();
  pthread_rwlock_unlock(&_transaction_map_lock);
}

void AVLTreeLockManager::deleteTransactionInfo(uint64_t tid) {
  // See if transaction information does not exist.
  pthread_rwlock_wrlock(&_transaction_map_lock);
  if (_transaction_map.find(tid) == _transaction_map.end()) {
    pthread_rwlock_unlock(&_transaction_map_lock);
    return;
  }

  // Delete transaction info.
  _transaction_map.erase(tid);
  pthread_rwlock_unlock(&_transaction_map_lock);
}

void AVLTreeLockManager::addTransactionLock(uint64_t tid, uint64_t key, uint64_t start, uint64_t end) {
  TransactionInfo *tr_info;
  std::pair<uint64_t, uint64_t> *temp_pair;

  // Lock transaction info.
  pthread_rwlock_rdlock(&_transaction_map_lock);
  if (_transaction_map.find(tid) == _transaction_map.end()) {
    pthread_rwlock_unlock(&_transaction_map_lock);
    createTransactionInfo(tid);
    pthread_rwlock_rdlock(&_transaction_map_lock);
  }

  // Update intervals.
  tr_info = _transaction_map[tid];
  tr_info->_mutex.lock();
  if (tr_info->_intervals.find(key) == tr_info->_intervals.end()) {
    tr_info->_intervals[key] = new std::pair<uint64_t, uint64_t>(start, end);
  } else {
    temp_pair = tr_info->_intervals[key];
    if (start < temp_pair->first)
      temp_pair->first = start;
    if (end > temp_pair->second)
      temp_pair->second = end;
  }

  // Unlock transaction info.
  tr_info->_mutex.unlock();
  pthread_rwlock_unlock(&_transaction_map_lock);
}

bool AVLTreeLockManager::tryBackwardLock(uint64_t key, uint64_t tid, uint64_t ts_end, bool is_read, Event *event,
    uint64_t *ts_start) {
  std::vector<AVLTreeLockNode *> locks;
  AVLTreeLockNode *root;
  AVLTreeLockNode *new_lock = nullptr;
  AVLTreeLockNode *cur_lock;
  uint64_t cur_ts = ts_end;
  bool res = true;

  // If there is no key information, create one. Lock all the corresponding entries.
  pthread_rwlock_rdlock(&_key_map_lock);
  if ((_key_map.find(key) == _key_map.end()) && (_key_map.find(key) == _key_map.end())) {
    pthread_rwlock_unlock(&_key_map_lock);
    createKeyInfo(key);
    pthread_rwlock_rdlock(&_key_map_lock);
  }
  _key_map[key]->_mutex.lock();

  // Create a new entry if necessary.
  root = _key_map[key]->_root;
  root->find(&locks, cur_ts);
  cur_lock = locks.back();
  if (((cur_lock->_status == FREE) || ((cur_lock->_status == READ_LOCK) && is_read)) && (cur_ts < cur_lock->_end)) {
    new_lock = new AVLTreeLockNode(cur_lock);
    new_lock->_start = cur_ts + 1;
    new_lock->_end = cur_lock->_end;
    cur_lock->_end = cur_ts;
    root = cur_lock->insert(new_lock);
    locks.clear();
    root->find(&locks, cur_ts);
  }

  // Start locking from ts_end and go backwards.
  while (!locks.empty()) {
    cur_lock = locks.back();
    locks.pop_back();
    if ((cur_ts < cur_lock->_start) || (cur_ts > cur_lock->_end)) {
      std::cerr << "AVLTreeLockManager Error: Find does not work as intended. Time " << cur_ts << " is not in ["
                << cur_lock->_start << "," << cur_lock->_end << "]" << std::endl;
      _key_map[key]->_mutex.unlock();
      pthread_rwlock_unlock(&_key_map_lock);
      return false;
    }
    switch (cur_lock->_status) {
      case FREE: {
        cur_lock->_status = is_read ? READ_LOCK : WRITE_LOCK;
        cur_ts = cur_lock->_start - 1;
        cur_lock->_tids->insert(tid);
        break;
      }
      case READ_LOCK: {
        if (is_read) {
          cur_ts = cur_lock->_start - 1;
          cur_lock->_tids->insert(tid);
        } else {
          addEvent(key, cur_lock->_end, event);
          res = false;
        }
        break;
      }
      case WRITE_LOCK: {
        addEvent(key, cur_lock->_end, event);
        res = false;
        break;
      }
      case FREEZE_READ_LOCK: {
        if (is_read)
          cur_ts = cur_lock->_start - 1;
        break;
      }
      case FREEZE_WRITE_LOCK: {
        break;
      }
    }
    if (cur_ts != cur_lock->_start - 1) {
      addTransactionLock(tid, key, cur_ts+1, ts_end);
      break;
    }
    cur_lock->find(&locks, cur_ts);
  }

  // Update the result and new root.
  *ts_start = cur_ts;
  _key_map[key]->_root = root;
  _key_map[key]->_mutex.unlock();
  pthread_rwlock_unlock(&_key_map_lock);

  // Run event if result is successful.
  if (res)
    event->run();
  return res;
}

bool AVLTreeLockManager::tryLock(uint64_t key, uint64_t tid, uint64_t ts_start, uint64_t ts_end, bool is_read) {
  std::vector<AVLTreeLockNode *> pend_locks;
  std::vector<AVLTreeLockNode *> locks;
  AVLTreeLockNode *root;
  AVLTreeLockNode *cur_lock;
  bool res = true;

  // If there is no key information, create one. Lock all the corresponding entries.
  pthread_rwlock_rdlock(&_key_map_lock);
  if ((_key_map.find(key) == _key_map.end()) && (_key_map.find(key) == _key_map.end())) {
    pthread_rwlock_unlock(&_key_map_lock);
    createKeyInfo(key);
    pthread_rwlock_rdlock(&_key_map_lock);
  }
  _key_map[key]->_mutex.lock();

  // Find the interval [ts_start,ts_end]. Check if it can be locked.
  root = _key_map[key]->_root;
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
      locks.clear();
      break;
    }
    if (cur_lock->_start > ts_start)
      cur_lock->find(&locks, ts_start - 1);
    else
      locks.clear();
  }

  // Lock the  interval [ts_start,ts_end] if it is available.
  if (res) {
    for (std::vector<AVLTreeLockNode *>::iterator it = pend_locks.begin(); it != pend_locks.end(); ++it) {
      if ((*it)->_start < ts_start) {
        AVLTreeLockNode *new_lock = new AVLTreeLockNode(*it);

        new_lock->_end = ts_start - 1;
        (*it)->_start = ts_start;
        root = (*it)->insert(new_lock);
      }
      if ((*it)->_end > ts_end) {
        AVLTreeLockNode *new_lock = new AVLTreeLockNode(*it);

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
    addTransactionLock(tid, key, ts_start, ts_end);
  }
  pend_locks.clear();

  // Update lock and root for AVL Tree.
  _key_map[key]->_root = root;
  _key_map[key]->_mutex.unlock();
  pthread_rwlock_unlock(&_key_map_lock);

  return res;
}

void AVLTreeLockManager::freeze(uint64_t tid) {
  TransactionInfo *tr_info;
  KeyInfo *key_info;
  AVLTreeLockNode *root, *cur_lock;
  std::pair<uint64_t, uint64_t> *temp_pair;
  std::vector<AVLTreeLockNode *> locks;
  std::set<Event *> events;
  uint64_t ts_start, ts_end, key;

  // Lock transaction information.
  pthread_rwlock_rdlock(&_transaction_map_lock);
  if (_transaction_map.find(tid) == _transaction_map.end()) {
    pthread_rwlock_unlock(&_transaction_map_lock);
    std::cerr << "AVLTreeLockManager: Freeze was called for unknown transaction " << tid << "." << std::endl;
    return;
  }
  tr_info = _transaction_map[tid];
  tr_info->_mutex.lock();

  // Freeze all the transactions.
  for (std::map<uint64_t, std::pair<uint64_t, uint64_t> *>::iterator it = tr_info->_intervals.begin();
       it != tr_info->_intervals.end(); ++it) {
    key = it->first;
    pthread_rwlock_rdlock(&_key_map_lock);
    if (_key_map.find(key) == _key_map.end()) {
      std::cerr << "AVLTreeLockManager: Key " << key << " was not found for transaction " << tid << "." << std::endl;
      pthread_rwlock_unlock(&_key_map_lock);
      continue;
    }
    key_info = _key_map[key];
    key_info->_mutex.lock();
    temp_pair = it->second;
    ts_start = temp_pair->first;
    ts_end = temp_pair->second;
    root = key_info->_root;
    root->find(&locks, ts_end);
    while (ts_end >= ts_start) {
      cur_lock = locks.back();
      locks.pop_back();
      switch (cur_lock->_status) {
        case FREE:
          break;
        case READ_LOCK:
          if (cur_lock->_tids->find(tid) != cur_lock->_tids->end()) {
            cur_lock->_status = FREEZE_READ_LOCK;
            cur_lock->_tids->clear();
            if (key_info->_events.find(cur_lock->_end) != key_info->_events.end())
              events.insert(key_info->_events[cur_lock->_end]->begin(), key_info->_events[cur_lock->_end]->end());
          }
          break;
        case WRITE_LOCK:
          if (cur_lock->_tids->find(tid) != cur_lock->_tids->end()) {
            cur_lock->_status = FREEZE_WRITE_LOCK;
            cur_lock->_tids->clear();
            if (key_info->_events.find(cur_lock->_end) != key_info->_events.end())
              events.insert(key_info->_events[cur_lock->_end]->begin(), key_info->_events[cur_lock->_end]->end());
          }
          break;
        case FREEZE_READ_LOCK:
          break;
        case FREEZE_WRITE_LOCK:
          break;
      }
      ts_end = cur_lock->_start - 1;
      cur_lock->find(&locks, ts_end);
    }
    delete temp_pair;
    locks.clear();
    key_info->_mutex.unlock();
    pthread_rwlock_unlock(&_key_map_lock);
  }
  _transaction_map[tid]->_intervals.clear();
  _transaction_map[tid]->_mutex.unlock();
  pthread_rwlock_unlock(&_transaction_map_lock);
  deleteTransactionInfo(tid);

  // Run events.
  for (std::set<Event *>::iterator it = events.begin(); it != events.end(); ++it)
    (*it)->run();
}

void AVLTreeLockManager::unlock(uint64_t tid) {
  TransactionInfo *tr_info;
  KeyInfo *key_info;
  AVLTreeLockNode *root, *cur_lock;
  std::pair<uint64_t, uint64_t> *temp_pair;
  std::vector<AVLTreeLockNode *> locks;
  std::set<Event *> events;
  uint64_t ts_start, ts_end, key;

  // Lock transaction information.
  pthread_rwlock_rdlock(&_transaction_map_lock);
  if (_transaction_map.find(tid) == _transaction_map.end()) {
    pthread_rwlock_unlock(&_transaction_map_lock);
    std::cerr << "AVLTreeLockManager: Freeze was called for unknown transaction " << tid << "." << std::endl;
    return;
  }
  tr_info = _transaction_map[tid];
  tr_info->_mutex.lock();

  // Unlock all the transactions.
  for (std::map<uint64_t, std::pair<uint64_t, uint64_t> *>::iterator it = tr_info->_intervals.begin();
       it != tr_info->_intervals.end(); ++it) {
    key = it->first;
    pthread_rwlock_rdlock(&_key_map_lock);
    if (_key_map.find(key) == _key_map.end()) {
      pthread_rwlock_unlock(&_key_map_lock);
      std::cerr << "AVLTreeLockManager: Key " << key << " was not found for transaction " << tid << "." << std::endl;
      continue;
    }
    key_info = _key_map[key];
    key_info->_mutex.lock();
    temp_pair = it->second;
    ts_start = temp_pair->first;
    ts_end = temp_pair->second;
    root = key_info->_root;
    root->find(&locks, ts_end);
    while (ts_end >= ts_start) {
      cur_lock = locks.back();
      locks.pop_back();
      switch (cur_lock->_status) {
        case FREE:
          break;
        case READ_LOCK:
          if (cur_lock->_tids->find(tid) != cur_lock->_tids->end()) {
            cur_lock->_status = FREE;
            cur_lock->_tids->erase(tid);
            if ((cur_lock->_tids->empty()) && (key_info->_events.find(cur_lock->_end) != key_info->_events.end()))
              events.insert(key_info->_events[cur_lock->_end]->begin(), key_info->_events[cur_lock->_end]->end());
          }
          break;
        case WRITE_LOCK:
          if (cur_lock->_tids->find(tid) != cur_lock->_tids->end()) {
            cur_lock->_status = FREE;
            cur_lock->_tids->erase(tid);
            if (key_info->_events.find(cur_lock->_end) != key_info->_events.end())
              events.insert(key_info->_events[cur_lock->_end]->begin(), key_info->_events[cur_lock->_end]->end());
          }
          break;
        case FREEZE_READ_LOCK:
          break;
        case FREEZE_WRITE_LOCK:
          break;
      }
      ts_end = cur_lock->_start - 1;
      cur_lock->find(&locks, ts_end);
    }
    delete temp_pair;
    locks.clear();
    key_info->_mutex.unlock();
    pthread_rwlock_unlock(&_key_map_lock);
  }
  _transaction_map[tid]->_intervals.clear();
  _transaction_map[tid]->_mutex.unlock();
  pthread_rwlock_unlock(&_transaction_map_lock);
  deleteTransactionInfo(tid);

  // Run events.
  for (std::set<Event *>::iterator it = events.begin(); it != events.end(); ++it)
    (*it)->run();
}
