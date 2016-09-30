/**
 * Node.h
 *
 *  Created on: Jul 21, 2016
 *      Author: theo
 */

#ifndef NODE_H_
#define NODE_H_

#include <map>

#include "ALock.h"

class Node {
  std::map<uint64_t, LockManager> *_locks;
  std::map<uint64_t, std::string> *_cache;

  Node();
  ~Node();

  virtual BackwardsLock(uint64_t tid, uint64_t key, uint64_t ts) = 0;
  virtual Lock(uint64_t tid, uint64_t key, uint64_t ts_start, uint64_t ts_end) = 0;
  virtual Freeze(uint64_t tid) = 0;
  virtual Unlocks(uint64_t tid) = 0;
};

#endif /* NODE_H_ */