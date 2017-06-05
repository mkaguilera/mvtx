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
#include <set>

#include <pthread.h>

#include "AVLTreeLockNode.h"
#include "LockManager.h"

/**
 * Instantiation of LockManager using AVL trees. AVL trees are capable of providing every operation in
 * O(log(total_nr_intervals) +  involved_nr_intervals). As a result, AVL trees are very efficient and natural choice
 * for supporting the LockManager API.
 */
class AVLTreeLockManager: public LockManager
{
  public:
    ///> Information needed per key.
    struct KeyInfo {
        ///> Locks access for this key.
        std::mutex _mutex;
        ///> Events wait to be executed for this key.
        std::map<uint64_t, std::set<Event *> *> _events;
        ///> Root AVLTreeLockNode for this key.
        AVLTreeLockNode *_root;
    };

    ///> Information needed per transaction.
    struct TransactionInfo {
        ///> Locks access for this transaction.
        std::mutex _mutex;
        ///> Keys along with the minimum and maximum time that was locked in this transaction.
        std::map<uint64_t, std::pair<uint64_t, uint64_t> *> _intervals;
    };

  private:
    std::map<uint64_t, KeyInfo*> _key_map;
    pthread_rwlock_t _key_map_lock;
    std::map<uint64_t, TransactionInfo *> _transaction_map;
    pthread_rwlock_t _transaction_map_lock;

  public:
    /**
     * Constructor of AVLTreeLockManager.
     */
    AVLTreeLockManager();
    /**
     * Destructor of AVLTreeLockManager.
     */
    ~AVLTreeLockManager();

  private:
    void createKeyInfo(uint64_t key);
    void addEvent(uint64_t key, uint64_t ts_end, Event *event);
    void createTransactionInfo(uint64_t tid);
    void deleteTransactionInfo(uint64_t tid);
    void addTransactionLock(uint64_t tid, uint64_t key, uint64_t start, uint64_t end);

  public:
    bool tryBackwardLock(uint64_t key, uint64_t tid, uint64_t ts_end, bool is_read, Event *event,
        uint64_t *ts_start) override;
    bool tryLock(uint64_t key, uint64_t tid, uint64_t ts_start, uint64_t ts_end, bool is_read) override;
    void freeze(uint64_t tid) override;
    void unlock(uint64_t tid) override;
};

#endif /* AVLTREELOCKMANAGER_H_ */
