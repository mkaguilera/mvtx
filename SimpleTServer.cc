/**
 * SimpleTServer.cc
 *
 *  Created on: Jul 13, 2016
 *      Author: theo
 */

#include <iostream>
#include "SimpleTServer.h"

SimpleTServer::SimpleTServer(RPCServer *rpc_server, KeyMapper *key_mapper, uint64_t queue_size)
    : TServer(rpc_server, key_mapper, queue_size) {
  _nodes.insert(0);
}

SimpleTServer::~SimpleTServer() {
  std::map<uint64_t,std::set<uint64_t> *>::iterator it;

  for (it = _tid_to_nodes.begin(); it != _tid_to_nodes.end(); ++it)
    _tid_to_nodes[it->first]->clear();
  _tid_to_nodes.clear();
  for (it = _tid_to_write_nodes.begin(); it != _tid_to_write_nodes.end(); ++it)
    _tid_to_write_nodes[it->first]->clear();
  _tid_to_write_nodes.clear();
  _nodes.clear();
}

ServerEvent *SimpleTServer::getEvent() {
  void *res;
  uint64_t rid1, rid2;
  request_t request;
  void *request_args;

  while (true) {
    res = NULL;
    rid1 = 0;
    rid2 = 0;
    while ((!_queue.tryDequeue(&res)) && (!_rpc_server->asyncNextRequest(&rid1, &request, &request_args)) &&
           (!_rpc_server->asyncNextCompletedReply(&rid2)));
    if (res != NULL)
      return reinterpret_cast<ServerEvent *> (res);
    if (rid1 != 0) {
      switch (request) {
        case READ: {
          rpc_read_args_t *rpc_read_args = (rpc_read_args_t *) request_args;
          uint64_t node = _key_mapper->getNode(rpc_read_args->read_args->key);
          uint64_t tid = rpc_read_args->read_args->tid;

          if (_nodes.find(node) == _nodes.end()) {
            rpc_read_args->status = false;
            _rpc_server->sendReply(rid1);
            continue;
          }
          rpc_read_args->status = true;
          _tid_mutex.lock();
          if (_tid_to_nodes.find(tid) == _tid_to_nodes.end())
            _tid_to_nodes[tid] = new std::set<uint64_t> ();
          _tid_to_nodes[tid]->insert(node);
          _tid_mutex.unlock();
          res = new ReadEvent(this, rid1, rpc_read_args);
          break;
        }
        case WRITE: {
          rpc_write_args_t *rpc_write_args = (rpc_write_args_t *) request_args;
          uint64_t node = _key_mapper->getNode(rpc_write_args->write_args->key);
          uint64_t tid = rpc_write_args->write_args->tid;

          if (_nodes.find(node) == _nodes.end()) {
            rpc_write_args->status = false;
            _rpc_server->sendReply(rid1);
            continue;
          }
          rpc_write_args->status = true;
          _tid_mutex.lock();
          if (_tid_to_nodes.find(tid) == _tid_to_nodes.end())
            _tid_to_nodes[tid] = new std::set<uint64_t> ();
          _tid_to_nodes[tid]->insert(node);
          _tid_mutex.unlock();
          _tid_write_mutex.lock();
          if (_tid_to_write_nodes.find(tid) == _tid_to_write_nodes.end())
            _tid_to_write_nodes[tid] = new std::set<uint64_t> ();
          _tid_to_write_nodes[tid]->insert(node);
          _tid_write_mutex.unlock();
          res = new WriteEvent(this, rid1, rpc_write_args);
          break;
        }
        case P1C: {
          rpc_p1c_args_t *rpc_p1c_args = (rpc_p1c_args_t *) request_args;
          uint64_t tid = rpc_p1c_args->p1c_args->tid;

          _tid_write_mutex.lock();
          if (_tid_to_write_nodes.find(tid) == _tid_to_write_nodes.end()) {
            _rpc_server->sendReply(rid1);
            continue;
          }
          rpc_p1c_args->nodes = _tid_to_write_nodes[tid];
          _tid_to_write_nodes.erase(tid);
          _tid_write_mutex.unlock();
          res = new P1CEvent(this, rid1, rpc_p1c_args);
          break;
        }
        case P2C: {
          rpc_p2c_args_t *rpc_p2c_args = (rpc_p2c_args_t *) request_args;
          uint64_t tid = rpc_p2c_args->p2c_args->tid;

          _tid_mutex.lock();
          if (_tid_to_nodes.find(tid) == _tid_to_nodes.end()) {
            _rpc_server->sendReply(rid1);
            continue;
          }
          rpc_p2c_args->nodes = _tid_to_nodes[tid];
          _tid_to_nodes.erase(tid);
          _tid_mutex.unlock();
          res = new P2CEvent(this, rid1, rpc_p2c_args);
          break;
        }
      }
      return reinterpret_cast<ServerEvent *> (res);
    }
    if (rid2 != 0)
      _rpc_server->deleteRequest(rid2);
  }
  return NULL;
}

void SimpleTServer::addEvent(ServerEvent *event) {
  _queue.enqueue(event);
}

void SimpleTServer::sendReply(ServerEvent *event, uint64_t rid) {
  _rpc_server->sendReply(rid);
}