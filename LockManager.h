/*
 * LockManager.h
 *
 *  Created on: Aug 2, 2016
 *      Author: theo
 */

#ifndef LOCKMANAGER_H_
#define LOCKMANAGER_H_

class TServer;

#include <cstdint>
#include <set>
#include "Event.h"

/**
 * LockManager is responsible for maintaining lock-purpose data structures, which provide a quite flexible locking API.
 * The API is based on the premise that instead of locking objects, the users can lock time/version intervals for
 * specific keys. This flexible API is capable for supporting most of Multi-Version Concurrency Control (MVCC)
 * protocols like Multi-Version Transaction Ordering (MVTO) or Multi-Version Snapshot Isolation (MVSI), as seen in
 * Percolator. Although, this API seems more natural for MVCC, it can be easily used for Single-Version Control
 * Protocols.
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
     * @param key       - Key for which lock is performed.
     * @param tid       - Transaction ID for the transaction that performs this lock.
     * @param ts_end    - Time for which lock procedure is initiated (inclusive).
     * @param is_read   - Whether or not this is a read lock.
     * @param event     - Event to run when backwards lock finishes.
     * @param ts_start  - The earliest time that was actually locked (inclusive).
     * @return          - Whether or not locking stopped at a frozen lock or the beginning of time.
     */
    virtual bool tryBackwardLock(uint64_t key, uint64_t tid, uint64_t ts_end, bool is_read, Event *event,
        uint64_t *ts_start) = 0;

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
     * @param tid       - Transaction ID of transaction for which freeze is performed.
     */
    virtual void freeze(uint64_t tid) = 0;

    /**
     * Unlock all the locks associated with a specific transaction.
     * @param tid       - Transaction ID of transaction for which unlock is performed.
     */
    virtual void unlock(uint64_t tid) = 0;
};

#endif /* LOCKMANAGER_H_ */
