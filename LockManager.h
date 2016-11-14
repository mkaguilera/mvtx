/*
 * LockManager.h
 *
 *  Created on: Aug 2, 2016
 *      Author: theo
 */

#ifndef LOCKMANAGER_H_
#define LOCKMANAGER_H_

class TServer;

#include <set>
#include "Event.h"

/**
 * Class that handles locking for transactions.
 */
class LockManager
{
  public:
    virtual ~LockManager() {
    }
    ;

    /**
     * Roll backwards from a specific time and lock everything until you encounter a frozen lock or the beginning of
     * time. In case, it is impossible to proceed because of another regular (not frozen) lock, register to be resolved
     * when this lock is frozen or released.
     * @param key     - Key for which lock is performed.
     * @param tid     - Transaction ID for the transaction that performs this lock.
     * @param ts      - Time for which lock procedure is initiated.
     * @param is_read - Whether or not this is a read lock.
     * @param event   - Event to register for running in case another regular lock (that cannot be bypassed) is
     *                  encountered.
     * @param last    - Address for storing the earliest time that was actually locked.
     * @return        - Whether or not it completely stopped (encountered a frozen lock or the beginning of time).
     */
    virtual bool tryBackwardLock(uint64_t key, uint64_t tid, uint64_t ts, bool is_read, Event *event,
        uint64_t *last) = 0;

    /**
     * Try to get a lock for a specific interval.
     * @param key       - Key fow which lock is performed.
     * @param tid       - Transaction ID for the transaction that performs this lock.
     * @param ts_start  - Start of the interval to lock (inclusive).
     * @param ts_end    - End of the interval to lock (inclusive).
     * @param is_read   - Whether or not this is a read lock.
     * @return          - If lock succeeds or not.
     */
    virtual bool tryLock(uint64_t key, uint64_t tid, uint64_t ts_start, uint64_t ts_end, bool is_read) = 0;

    /**
     * Freeze all the locks associated with a specific transaction.
     * @param keys      - Keys that are involved in the transaction.
     * @param tid       - Transaction ID of transaction for which freeze is performed.
     * @param ts_start  - Minimum time point for which this transaction can hold keys (inclusive).
     * @param ts_end    - Maximum time point for which this transaction can hold keys (inclusive).
     */
    virtual void freeze(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end) = 0;

    /**
     * Unlock all the locks associated with a specific transaction.
     * @param keys      - Keys that are involved in the transaction.
     * @param tid       - Transaction ID of transaction for which unlock is performed.
     * @param ts_start  - Minimum time point for which this transaction can hold keys (inclusive).
     * @param ts_end    - Maximum time point for which this transaction can hold keys (inclusive).
     */
    virtual void unlock(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end) = 0;
};

#endif /* LOCKMANAGER_H_ */
