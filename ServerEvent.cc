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
    : _tserver(tserver), _rid(rid), _phase(0) {}

ServerEvent::~ServerEvent() {}

ReadEvent::ReadEvent(TServer *tserver, uint64_t rid, rpc_read_args_t *rpc_read_args)
    : ServerEvent(tserver, rid), _rpc_read_args(rpc_read_args) {}

void ReadEvent::run() {
  switch (_phase) {
    case 0:
    {
      // TODO: Replace rpc_read_args with read args.
      uint64_t tid = _rpc_read_args->read_args->tid;
      uint64_t ts = _rpc_read_args->read_args->start_ts;

      _phase++;
      // TODO: Invoke the Lock Manager. If you do not acquire a lock stay in this phase.
      std::cout << "BackwardsLock(" << tid << "," << ts << ",READ)" << std::endl;
      _tserver->addEvent(this);
      break;
    }
    case 1:
    {
      _phase++;
      _rpc_read_args->value = new std::string("100");
      std::cout << "Value read " << _rpc_read_args->value << std::endl;
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
  switch(_phase) {
    case 0:
    {
      uint64_t tid = _rpc_write_args->write_args->tid;
      uint64_t key = _rpc_write_args->write_args->key;
      std::string *value = _rpc_write_args->write_args->value;

      _phase++;
      std::cout << "Write (" << tid << "," << key << "," << *value << ")" << std::endl;
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
  switch(_phase) {
    case 0:
    {
      uint64_t tid = _rpc_p1c_args->p1c_args->tid;
      uint64_t start_ts = _rpc_p1c_args->p1c_args->start_ts;
      uint64_t commit_ts = _rpc_p1c_args->p1c_args->commit_ts;
      std::set<uint64_t> *read_nodes = _rpc_p1c_args->p1c_args->read_nodes;
      std::set<uint64_t> *write_nodes = _rpc_p1c_args->p1c_args->write_nodes;

      _phase++;
      std::cout << "Lock(" << tid << "," << start_ts << "," << commit_ts << ",P1C)" << std::endl;
      for (std::set<uint64_t>::iterator it = read_nodes->begin(); it != read_nodes->end(); ++it)
        _rpc_p1c_args->nodes->insert(*it);
      for (std::set<uint64_t>::iterator it = write_nodes->begin(); it != write_nodes->end(); ++it)
        _rpc_p1c_args->nodes->insert(*it);
      _rpc_p1c_args->vote = true;
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

      _phase++;
      if (vote)
        std::cout << "Freeze Locks(" << tid << ")" << std::endl;
      else
        std::cout << "Unlocks(" << tid << ")" << std::endl;
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
