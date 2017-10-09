/*
 * TServer.h
 *
 *  Created on: Jul 12, 2016
 *      Author: theo
 */

#ifndef TSERVER_H_
#define TSERVER_H_

#include <map>
#include <mutex>
#include "KeyMapper.h"
#include "LockManager.h"
#include "RPCServer.h"
#include "SafeQueue.h"
#include "ServerEvent.h"

class TServer
{
protected:
  std::set<uint64_t> _nodes;
  RPCServer *_rpc_server;
  KeyMapper *_key_mapper;
  LockManager *_lock_manager;
  SafeQueue<void *> _queue;
  std::map<uint64_t, LockManager *> _tid_to_locks;
  std::mutex _tid_mutex;
  std::map<uint64_t,std::set<uint64_t> *> _tid_to_nodes;
  std::mutex _tid_write_mutex;
  std::map<uint64_t,std::set<uint64_t> *> _tid_to_write_nodes;

public:
  TServer(RPCServer *rpc_server, KeyMapper *key_mapper, LockManager *lock_manager, uint64_t queue_size)
      : _rpc_server(rpc_server), _key_mapper(key_mapper), _lock_manager(lock_manager), _queue(queue_size) {};
  virtual ~TServer() {};

  virtual ServerEvent *getEvent() = 0;
  virtual void addEvent(ServerEvent *event) = 0;
  virtual void sendReply(ServerEvent *event, uint64_t rid) = 0;
  virtual std::string *get(uint64_t key, uint64_t ts) = 0;
  virtual void set(uint64_t key, uint64_t ts, std::string *value) = 0;
  virtual void add(uint64_t tid, uint64_t key, std::string *value) = 0;
  virtual std::vector<uint64_t> *getKeys(uint64_t tid) = 0;
  virtual void prepare(uint64_t tid, uint64_t ts) = 0;
  virtual void finalize(uint64_t tid, bool success) = 0;

  LockManager *getLockManager() {
    return _lock_manager;
  }
};

#endif /* TSERVER_H_ */
