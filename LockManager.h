/**
 * LockManager.h
 *
 *  Created on: Aug 2, 2016
 *      Author: theo
 */

#ifndef LOCKMANAGER_H_
#define LOCKMANAGER_H_

class TServer;

#include <set>

class LockManager
{
public:
  virtual ~LockManager() {};

  virtual bool tryBackwardLock(uint64_t key, uint64_t tid, uint64_t ts, bool is_read, void *event, uint64_t *last) = 0;
  virtual bool tryLock(uint64_t key, uint64_t tid, uint64_t ts_start, uint64_t ts_end, bool is_read) = 0;
  virtual void freeze(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end, TServer *tserver) = 0;
  virtual void unlock(std::set<uint64_t> keys, uint64_t tid, uint64_t ts_start, uint64_t ts_end, TServer *tserver) = 0;
};

#endif /* LOCKMANAGER_H_ */
