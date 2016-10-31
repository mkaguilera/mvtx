/**
 * AVLLockManager.h
 *
 *  Created on: Aug 2, 2016
 *      Author: theo
 */

#ifndef AVLLOCKMANAGER_H_
#define AVLLOCKMANAGER_H_

#include <map>
#include <mutex>
#include "AVLLockNode.h"
#include "LockManager.h"

class AVLLockManager: public LockManager
{
  private:
    std::map<uint64_t, std::pair<std::mutex *, AVLLockNode *> *> _key_to_locks;
    std::mutex _map_lock;

  public:
    AVLLockManager();
    ~AVLLockManager();

  private:
    void createEntry(uint64_t key);

  public:
    bool tryBackwardLock(uint64_t key, uint64_t tid, uint64_t ts, bool is_read, void *event, uint64_t *last) override;
    bool tryLock(uint64_t key, uint64_t tid, uint64_t ts_start, uint64_t ts_end, bool is_read) override;
    void freeze(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end, TServer *tserver) override;
    void unlock(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end, TServer *tserver) override;
    std::string lockToString(uint64_t key);
};

#endif /* AVLLOCKMANAGER_H_ */
