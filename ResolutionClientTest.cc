/*
 * ResolutionClientTest.cc
 *
 *  Created on: May 1, 2017
 *      Author: theo
 */
#include <cassert>
#include <thread>

#include "Request.h"
#include "ResolutionClientTest.h"

ResolutionClientTest::ResolutionClientTest(ResolutionClient *resolution_client, std::vector<RPCServer *> *rpc_servers)
    : _resolution_client(resolution_client), _rpc_servers(rpc_servers) {
}

ResolutionClientTest::~ResolutionClientTest() {}

void ResolutionClientTest::replyRequest(std::vector<RPCServer *> *rpc_servers) {
  std::vector<RPCServer *>::iterator it;
  uint64_t rid1, rid2;
  request_t request;
  void *args;
  int nr_requests = 0;
  rpc_read_args_t *read_args;
  rpc_write_args_t *write_args;
  rpc_p1c_args_t *p1c_args;
  rpc_p2c_args_t *p2c_args;

  while (nr_requests < 4) {
    for (it = rpc_servers->begin(); it != rpc_servers->end(); ++it) {
      if ((*it)->asyncNextRequest(&rid1, &request, &args)) {
        switch (request) {
          case READ:
            read_args = (rpc_read_args_t *) args;
            assert(read_args->read_args->tid == 0);
            assert(read_args->read_args->key == 4);
            assert(read_args->read_args->start_ts == 30);
            read_args->value = new std::string("0");
            read_args->status = true;
            break;
          case WRITE:
            write_args = (rpc_write_args_t *) args;
            assert(write_args->write_args->tid == 0);
            assert(write_args->write_args->key == 13);
            assert(*(write_args->write_args->value) == "100");
            write_args->status = true;
            break;
          case P1C:
            p1c_args = (rpc_p1c_args_t *) args;
            assert(p1c_args->p1c_args->tid == 0);
            assert(p1c_args->p1c_args->start_ts == 4);
            assert(p1c_args->p1c_args->commit_ts == 4);
            assert(p1c_args->p1c_args->read_nodes->size() == 1);
            assert(*(p1c_args->p1c_args->read_nodes->begin()) == 0);
            assert(p1c_args->p1c_args->write_nodes->size() == 1);
            assert(*(p1c_args->p1c_args->write_nodes->begin()) == 1);
            p1c_args->nodes = new std::set<uint64_t>();
            p1c_args->nodes->insert(1);
            p1c_args->vote = true;
            break;
          case P2C:
            p2c_args = (rpc_p2c_args_t *) args;
            assert(p2c_args->p2c_args->tid == 0);
            assert(p2c_args->p2c_args->vote);
            p2c_args->nodes = new std::set<uint64_t>();
            p2c_args->nodes->insert(0);
            break;
        }
        (*it)->sendReply(rid1);
        while (!(*it)->asyncNextCompletedReply(&rid2));
        assert(rid1 == rid2);

        // Garbage Collection.
        (*it)->deleteRequest(rid2);
        nr_requests += 1;
      }
    }
  }
}

void ResolutionClientTest::run() {
  std::set<uint64_t> read_nodes, write_nodes;
  rsl_read_args_t read_args;
  rsl_write_args_t write_args;
  rsl_p1c_args_t p1c_args;
  rsl_p2c_args_t p2c_args;
  std::thread server_thread(replyRequest, _rpc_servers);

  read_nodes.insert(0);
  write_nodes.insert(1);

  read_args.read_args = (read_args_t *) malloc(sizeof(read_args_t));
  read_args.read_args->tid = 0;
  read_args.read_args->key = 4;
  read_args.read_args->start_ts = 30;
  read_args.value = new std::string();
  _resolution_client->request(read_nodes, READ, (void *) &read_args);
  assert(*(read_args.value) == "0");
  free(read_args.read_args);
  delete read_args.value;

  write_args.write_args = (write_args_t *) malloc(sizeof(write_args_t));
  write_args.write_args->tid = 0;
  write_args.write_args->key = 13;
  write_args.write_args->value = new std::string("100");
  _resolution_client->request(write_nodes, WRITE, &write_args);
  free(write_args.write_args);

  p1c_args.p1c_args = (p1c_args_t *) malloc(sizeof(p1c_args_t));
  p1c_args.p1c_args->tid = 0;
  p1c_args.p1c_args->start_ts = 4;
  p1c_args.p1c_args->commit_ts = 4;
  p1c_args.p1c_args->read_nodes = new std::set<uint64_t> (read_nodes);
  p1c_args.p1c_args->write_nodes = new std::set<uint64_t> (write_nodes);
  _resolution_client->request(write_nodes, P1C, &p1c_args);
  assert(p1c_args.vote);
  delete p1c_args.p1c_args->read_nodes;
  delete p1c_args.p1c_args->write_nodes;
  free(p1c_args.p1c_args);

  p2c_args.p2c_args = (p2c_args_t *) malloc(sizeof(p2c_args_t));
  p2c_args.p2c_args->tid = 0;
  p2c_args.p2c_args->vote = true;
  _resolution_client->request(read_nodes, P2C, &p2c_args);
  free(p2c_args.p2c_args);

  server_thread.join();

  std::cout << "ResolutionClient test was successful." << std::endl;
}
