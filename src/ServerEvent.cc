/*
 * ServerEvent.cc
 *
 *  Created on: Jul 15, 2016
 *      Author: theo
 */

#include <iostream>
#include "ServerEvent.h"
#include "TServer.h"

ServerEvent::ServerEvent(TServer *tserver, uint64_t rid)
    : Event(), _tserver(tserver), _rid(rid), _phase(0) {}

ServerEvent::~ServerEvent() {}

ReadEvent::ReadEvent(TServer *tserver, uint64_t rid, rpc_read_args_t *rpc_read_args)
    : ServerEvent(tserver, rid), _rpc_read_args(rpc_read_args) {}

void ReadEvent::run() {
  uint64_t tid = _rpc_read_args->read_args->tid;
  uint64_t ts = _rpc_read_args->read_args->start_ts;
  uint64_t key = _rpc_read_args->read_args->key;
  LockManager *lockManager = _tserver->getLockManager();

  switch (_phase) {
    case 0:
    {
      std::cout << "BackwardsLock(" << tid << "," << ts-1 << ",READ)" << std::endl;
      if (lockManager->tryBackwardLock(key, tid, ts-1, true, this, &(_rpc_read_args->read_args->start_ts))) {
        _phase++;
        run();
      }
      break;
    }
    case 1:
    {
      _phase++;
      _rpc_read_args->value = _tserver->get(key, ts-1);
      std::cout << "Value read " << *(_rpc_read_args->value) << std::endl;
      _tserver->sendReply(this, _rid);
      break;
    }
    default:
    {
      std::cerr << "Reads do not have phase " << _phase << "." << std::endl;
      exit(1);
    }
  }
}

WriteEvent::WriteEvent(TServer *tserver, uint64_t rid, rpc_write_args_t *rpc_write_args)
    : ServerEvent(tserver, rid), _rpc_write_args(rpc_write_args) {}

void WriteEvent::run() {
  uint64_t tid = _rpc_write_args->write_args->tid;
  uint64_t key = _rpc_write_args->write_args->key;
  std::string *value = _rpc_write_args->write_args->value;

  switch(_phase) {
    case 0:
    {
      std::cout << "Phase 1 of write" << std::endl;

      _phase++;
      std::cout << "Write (" << tid << "," << key << "," << *value << ")" << std::endl;
      _tserver->add(tid, key, value);
      _tserver->sendReply(this, _rid);
      break;
    }
    default:
    {
      std::cerr << "Writes do not have phase " << _phase << "." << std::endl;
      exit(1);
    }
  }
}

P1CEvent::P1CEvent(TServer *tserver, uint64_t rid, rpc_p1c_args_t *rpc_p1c_args)
    : ServerEvent(tserver, rid), _rpc_p1c_args(rpc_p1c_args) {}

void P1CEvent::run () {
  uint64_t tid = _rpc_p1c_args->p1c_args->tid;
  uint64_t start_ts = _rpc_p1c_args->p1c_args->start_ts;
  uint64_t commit_ts = _rpc_p1c_args->p1c_args->commit_ts;
  std::set<uint64_t> *read_nodes = _rpc_p1c_args->p1c_args->read_nodes;
  std::set<uint64_t> *write_nodes = _rpc_p1c_args->p1c_args->write_nodes;

  switch(_phase) {
    case 0:
    {
      std::vector<uint64_t> *keys = _tserver->getKeys(tid);
      bool res = true;

      std::cout << "Phase 1 of P1C" << std::endl;

      _phase++;
      std::cout << "Lock(" << tid << "," << start_ts << "," << start_ts << ",P1C)" << std::endl;
      for (uint64_t i = 0; i < keys->size(); i++) {
        if (!_tserver->getLockManager()->tryLock((*keys)[i], tid, start_ts, commit_ts, false)) {
          res = false;
          break;
        }
      }
      for (std::set<uint64_t>::iterator it = read_nodes->begin(); it != read_nodes->end(); ++it)
        _rpc_p1c_args->nodes->insert(*it);
      for (std::set<uint64_t>::iterator it = write_nodes->begin(); it != write_nodes->end(); ++it)
        _rpc_p1c_args->nodes->insert(*it);
      _rpc_p1c_args->vote = res;
      _tserver->prepare(tid, commit_ts);
      _tserver->sendReply(this, _rid);
      break;
    }
    default:
    {
      std::cerr << "P1C do not have phase " << _phase << "." << std::endl;
      exit(1);
    }
  }
}

P2CEvent::P2CEvent(TServer *tserver, uint64_t rid, rpc_p2c_args_t *rpc_p2c_args)
    : ServerEvent(tserver, rid), _rpc_p2c_args(rpc_p2c_args) {}

void P2CEvent::run () {
  switch(_phase) {
    case 0:
    {
      uint64_t tid = _rpc_p2c_args->p2c_args->tid;
      uint64_t vote = _rpc_p2c_args->p2c_args->vote;

      std::cout << "Phase 1 of P2C" << std::endl;

      _phase++;
      if (vote) {
        std::cout << "Freeze Locks(" << tid << ")" << std::endl;
        _tserver->getLockManager()->freeze(tid);
      } else {
        std::cout << "Unlocks(" << tid << ")" << std::endl;
        _tserver->getLockManager()->unlock(tid);
      }
      std::cout << "Call Finalize" << std::endl;
      _tserver->finalize(tid, vote);
      std::cout << "Send Reply" << std::endl;
      _tserver->sendReply(this, _rid);
      break;
    }
    case 1:
    {
      std::cerr << "P2C do not have phase " << _phase << "." << std::endl;
      exit(1);
    }
  }
}
