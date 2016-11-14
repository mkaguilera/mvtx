/*
 * AVLTreeLockManager.h
 *
 *  Created on: Aug 2, 2016
 *      Author: theo
 */

#ifndef AVLTREELOCKMANAGER_H_
#define AVLTREELOCKMANAGER_H_

#include <map>
#include <mutex>

#include "AVLTreeLock.h"
#include "LockManager.h"

/**
 * Instantiation of LockManager using AVL Trees.
 */
class AVLTreeLockManager: public LockManager
{
  private:
    std::map<uint64_t, std::pair<std::mutex *, AVLTreeLock *> *> _key_to_locks;
    std::mutex _map_lock;

  public:
    AVLTreeLockManager();
    ~AVLTreeLockManager();

  private:
    void createNewLock(uint64_t key);

  public:
    bool tryBackwardLock(uint64_t key, uint64_t tid, uint64_t ts, bool is_read, Event *event, uint64_t *last) override;
    bool tryLock(uint64_t key, uint64_t tid, uint64_t ts_start, uint64_t ts_end, bool is_read) override;
    void freeze(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end) override;
    void unlock(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end) override;
};

#endif /* AVLTREELOCKMANAGER_H_ */
