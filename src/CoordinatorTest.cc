/*
 * CoordinatorTest.cc
 *
 *  Created on: May 15, 2017
 *      Author: theo
 */

#include <cassert>
#include <thread>

#include "Request.h"
#include "WithdrawCoordinator.h"
#include "CoordinatorTest.h"

CoordinatorTest::CoordinatorTest(Coordinator *coordinator, std::vector<RPCServer *> *rpc_servers, KeyMapper *key_mapper)
: _coordinator(coordinator), _rpc_servers(rpc_servers), _key_mapper(key_mapper) {
}

CoordinatorTest::~CoordinatorTest() {
}

void CoordinatorTest::run() {
  std::thread server_thread(replyRequest, _rpc_servers, _key_mapper);

  _coordinator->run();
  server_thread.join();
  std::cout << "Coordinator test was successful." << std::endl;
}

void CoordinatorTest::replyRequest(std::vector<RPCServer *> *rpc_servers, KeyMapper *key_mapper) {
  std::vector<RPCServer *>::iterator it;
  bool set_start = false;
  bool set_commit = false;
  bool finished = false;
  uint64_t rid1, rid2, start_ts, commit_ts, tid, node;
  std::map<RPCServer *, std::set<uint64_t>> read_nodes, write_nodes, nodes;
  request_t request;
  void *args;
  rpc_read_args_t *read_args;
  rpc_write_args_t *write_args;
  rpc_p1c_args_t *p1c_args;
  rpc_p2c_args_t *p2c_args;

  while (!finished) {
    for (it = rpc_servers->begin(); it != rpc_servers->end(); ++it) {
      if ((*it)->asyncNextRequest(&rid1, &request, &args)) {
        switch (request) {
          case TREAD:
            read_args = (rpc_read_args_t *) args;
            tid = read_args->read_args->tid;
            node = key_mapper->getNode(read_args->read_args->key);
            read_nodes[*it].insert(node);
            nodes[*it].insert(node);
            if (!set_start) {
              start_ts = read_args->read_args->start_ts;
              set_start = true;
            } else {
              assert(read_args->read_args->start_ts == start_ts);
            }
            read_args->value = new std::string("100");
            read_args->status = true;
            break;
          case TWRITE:
            write_args = (rpc_write_args_t *) args;
            assert(write_args->write_args->tid == tid);
            node = key_mapper->getNode(write_args->write_args->key);
            write_nodes[*it].insert(node);
            nodes[*it].insert(node);
            write_args->status = true;
            break;
          case TP1C:
            p1c_args = (rpc_p1c_args_t *) args;
            assert(p1c_args->p1c_args->tid == tid);
            if (!set_start) {
              start_ts = p1c_args->p1c_args->start_ts;
              set_start = true;
            } else {
              assert(p1c_args->p1c_args->start_ts == start_ts);
            }
            if (!set_commit) {
              commit_ts = p1c_args->p1c_args->commit_ts;
              assert(commit_ts >= start_ts);
              set_commit = true;
            } else {
              assert(p1c_args->p1c_args->commit_ts == commit_ts);
            }
            assert(p1c_args->p1c_args->read_nodes->size() == read_nodes[*it].size());
            for (std::set<uint64_t>::iterator it2 = p1c_args->p1c_args->read_nodes->begin();
                it2 != p1c_args->p1c_args->read_nodes->end(); ++it2)
              assert(read_nodes[*it].find(*it2) != read_nodes[*it].end());
            assert(p1c_args->p1c_args->write_nodes->size() == write_nodes[*it].size());
            for (std::set<uint64_t>::iterator it2 = p1c_args->p1c_args->write_nodes->begin();
                it2 != p1c_args->p1c_args->write_nodes->end(); ++it2)
              assert(write_nodes[*it].find(*it2) != write_nodes[*it].end());
            p1c_args->nodes = new std::set<uint64_t> (nodes[*it]);
            p1c_args->vote = true;
            break;
          case TP2C:
            p2c_args = (rpc_p2c_args_t *) args;
            assert(p2c_args->p2c_args->tid == tid);
            assert(p2c_args->p2c_args->vote);
            p2c_args->nodes = new std::set<uint64_t> (nodes[*it]);
            nodes.erase(*it);
            if (nodes.size() == 0)
              finished = true;
            break;
        }
        (*it)->sendReply(rid1);
        while (!(*it)->asyncNextCompletedReply(&rid2));
        assert(rid1 == rid2);

        // Garbage Collection.
        (*it)->deleteRequest(rid2);
      }
    }
  }
}
