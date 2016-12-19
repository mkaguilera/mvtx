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
  SafeQueue<void *> _queue;
  std::map<uint64_t, LockManager *> _tid_to_locks;
  std::mutex _tid_mutex;
  std::map<uint64_t,std::set<uint64_t> *> _tid_to_nodes;
  std::mutex _tid_write_mutex;
  std::map<uint64_t,std::set<uint64_t> *> _tid_to_write_nodes;

public:
  TServer(RPCServer *rpc_server, KeyMapper *key_mapper, uint64_t queue_size)
      : _rpc_server(rpc_server), _key_mapper(key_mapper), _queue(queue_size) {};
  virtual ~TServer() {};

  virtual ServerEvent *getEvent() = 0;
  virtual void addEvent(ServerEvent *event) = 0;
  virtual void sendReply(ServerEvent *event, uint64_t rid) = 0;
};

#endif /* TSERVER_H_ */
