/*
 * SimpleTServer.cc
 *
 *  Created on: Jul 13, 2016
 *      Author: theo
 */

#include <iostream>
#include "SimpleTServer.h"

SimpleTServer::SimpleTServer(RPCServer *rpc_server, KeyMapper *key_mapper, LockManager *lock_manager,
                             uint64_t queue_size)
    : TServer(rpc_server, key_mapper, lock_manager, queue_size) {
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
        case TREAD: {
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
        case TWRITE: {
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
        case TP1C: {
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
        case TP2C: {
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
  return nullptr;
}

void SimpleTServer::addEvent(ServerEvent *event) {
  _queue.enqueue(event);
}

void SimpleTServer::sendReply(ServerEvent *event, uint64_t rid) {
  _rpc_server->sendReply(rid);
}

std::string *SimpleTServer::get(uint64_t key, uint64_t ts) {
  if (ts == 0)
    return new std::string("0");
  if (_store.find(key) == _store.end())
    return nullptr;
  if (_store[key]->find(ts) == _store[key]->end()) {
    std::cout << "Asked for key " << key << " and ts " << ts << " that cannot be found." << std::endl;
    return nullptr;
  }
  std::cout << (*_store[key])[ts] << std::endl;
  return &((*_store[key])[ts]);
}

void SimpleTServer::set(uint64_t key, uint64_t ts, std::string *value) {
  std::cout << "Set " << key << "," << ts << "," << *value << " called" << std::endl;
  if (_store.find(key) == _store.end())
    _store[key] = new std::map<uint64_t, std::string> ();
  (*_store[key])[ts] = std::string(*value);
}

void SimpleTServer::add(uint64_t tid, uint64_t key, std::string *value) {
  if (_pend_writes.find(tid) == _pend_writes.end())
    _pend_writes[tid] = new std::vector<std::pair<uint64_t, std::string *>> ();
  _pend_writes[tid]->push_back(std::pair<uint64_t, std::string *>(key, new std::string(*value)));
}

std::vector<uint64_t> *SimpleTServer::getKeys(uint64_t tid) {
  std::vector<uint64_t> *res = new std::vector<uint64_t>();

  if (_pend_writes.find(tid) == _pend_writes.end()) {
    std::cout << "Writes for transaction ID " << tid << " do not exist." << std::endl;
    return res;
  }
  for (uint64_t i = 0; i < _pend_writes[tid]->size(); i++)
    res->push_back((*_pend_writes[tid])[i].first);
  return res;
}

void SimpleTServer::prepare(uint64_t tid, uint64_t ts) {
  _rdy_to_commit[tid] = ts;
}

void SimpleTServer::finalize(uint64_t tid, bool success) {
  if (_rdy_to_commit.find(tid) == _rdy_to_commit.end()) {
    std::cout << "Timestamp for transaction ID " << tid << " do not exist." << std::endl;
    return;
  }
  if (_pend_writes.find(tid) == _pend_writes.end()) {
    std::cout << "Writes for transaction ID " << tid << " do not exist." << std::endl;
    _rdy_to_commit.erase(tid);
    return;
  }
  if (success) {
    std::cout << "Finalize pending writes." << std::endl;
    for (uint64_t i = 0; i < _pend_writes[tid]->size(); i++)
      set((*_pend_writes[tid])[i].first, _rdy_to_commit[tid], new std::string(*(*_pend_writes[tid])[i].second));
  }
  delete _pend_writes[tid];
  _pend_writes.erase(tid);
  _rdy_to_commit.erase(tid);
}
