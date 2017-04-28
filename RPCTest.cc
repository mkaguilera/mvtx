/*
 * RPCTest.cc
 *
 *  Created on: Dec 12, 2016
 *      Author: theo
 */

#include <cassert>
#include <thread>
#include <unistd.h>

#include "GRPCClient.h"
#include "GRPCServer.h"
#include "Request.h"
#include "RPCTest.h"

RPCTest::RPCTest(RPCClient *rpc_client, RPCServer *rpc_server, int port)
  : _rpc_client(rpc_client), _rpc_server(rpc_server), _port(port) {
}

void RPCTest::runClient(RPCClient *rpc_client, int port) {
  // Make four requests of different types.
  rpc_read_args_t *read_args;
  rpc_write_args_t *write_args;
  rpc_p1c_args_t *p1c_args;
  rpc_p2c_args_t *p2c_args;

  read_args = (rpc_read_args_t *) malloc(sizeof(rpc_read_args_t));
  read_args->read_args = (read_args_t *) malloc(sizeof(read_args_t));
  read_args->read_args->tid = 0;
  read_args->read_args->key = 135;
  read_args->read_args->start_ts = 254;
  rpc_client->asyncRPC("0.0.0.0:" + std::to_string(port), 1, READ, (void *) read_args);
  assert(rpc_client->waitAsyncReply(1));
  assert(read_args->status);
  assert((*(read_args->value)) == "0");
  free(read_args->read_args);
  free(read_args);

  write_args = (rpc_write_args_t *) malloc(sizeof(rpc_write_args_t));
  write_args->write_args = (write_args_t *) malloc(sizeof(write_args_t));
  write_args->write_args->tid = 0;
  write_args->write_args->key = 1;
  write_args->write_args->value = new std::string("1");
  rpc_client->asyncRPC("0.0.0.0:" + std::to_string(port), 2, WRITE, write_args);
  assert(rpc_client->waitAsyncReply(2));
  assert(write_args->status);
  free(write_args->write_args);
  free(write_args);

  p1c_args = (rpc_p1c_args_t *) malloc(sizeof(rpc_p1c_args_t));
  p1c_args->p1c_args = (p1c_args_t *) malloc(sizeof(p1c_args_t));
  p1c_args->p1c_args->tid = 0;
  p1c_args->p1c_args->start_ts = 2;
  p1c_args->p1c_args->commit_ts = 2;
  p1c_args->p1c_args->read_nodes = new std::set<uint64_t>();
  p1c_args->p1c_args->read_nodes->insert(0);
  p1c_args->p1c_args->write_nodes = new std::set<uint64_t>();
  p1c_args->p1c_args->write_nodes->insert(1);
  p1c_args->nodes = new std::set<uint64_t> ();
  rpc_client->asyncRPC("0.0.0.0:" + std::to_string(port), 3, P1C, p1c_args);
  assert(rpc_client->waitAsyncReply(3));
  assert(p1c_args->nodes->size() == 1);
  assert(*(p1c_args->nodes->begin()) == 2);
  assert(p1c_args->vote);
  free(p1c_args->p1c_args);
  free(p1c_args);

  p2c_args = (rpc_p2c_args_t *) malloc(sizeof(rpc_p2c_args_t));
  p2c_args->p2c_args = (p2c_args_t *) malloc(sizeof(p2c_args_t));
  p2c_args->p2c_args->tid = 0;
  p2c_args->p2c_args->vote = true;
  p2c_args->nodes = new std::set<uint64_t> ();
  rpc_client->asyncRPC("0.0.0.0:" + std::to_string(port), 4, P2C, p2c_args);
  assert(rpc_client->waitAsyncReply(4));
  assert(p2c_args->nodes->size() == 1);
  assert(*(p2c_args->nodes->begin()) == 3);
  free(p2c_args->p2c_args);
  free(p2c_args);
}

void RPCTest::runServer(RPCServer *rpc_server) {
  // Prepare for four requests of different types.
  rpc_read_args_t *read_args;
  rpc_write_args_t *write_args;
  rpc_p1c_args_t *p1c_args;
  rpc_p2c_args_t *p2c_args;
  uint64_t rid1, rid2;
  request_t request;
  void *args;
  std::set<uint64_t> *nodes = new std::set<uint64_t> ();

  while (!(rpc_server->asyncNextRequest(&rid1, &request, &args) || rpc_server->asyncNextCompletedReply(&rid1)));
  assert(request == READ);
  read_args = (rpc_read_args_t *) args;
  assert(read_args->read_args->tid == 0);
  assert(read_args->read_args->key == 135);
  assert(read_args->read_args->start_ts == 254);
  read_args->status = true;
  read_args->value = new std::string("0");
  rpc_server->sendReply(rid1);
  rpc_server->nextCompletedReply(&rid2);
  assert(rid1 == rid2);
  rpc_server->deleteRequest(rid2);

  while (!rpc_server->asyncNextRequest(&rid1, &request, &args));
  assert(request == WRITE);
  write_args = (rpc_write_args_t *) args;
  assert(write_args->write_args->tid == 0);
  assert(write_args->write_args->key == 1);
  assert(*(write_args->write_args->value) == "1");
  write_args->status = true;
  rpc_server->sendReply(rid1);
  while (!rpc_server->asyncNextCompletedReply(&rid2));
  assert(rid1 == rid2);
  rpc_server->deleteRequest(rid2);

  while (!rpc_server->asyncNextRequest(&rid1, &request, &args));
  assert(request == P1C);
  p1c_args = (rpc_p1c_args_t *) args;
  assert(p1c_args->p1c_args->tid == 0);
  assert(p1c_args->p1c_args->start_ts == 2);
  assert(p1c_args->p1c_args->commit_ts == 2);
  assert(p1c_args->p1c_args->read_nodes->size() == 1);
  assert(*(p1c_args->p1c_args->read_nodes->begin()) == 0);
  assert(p1c_args->p1c_args->write_nodes->size() == 1);
  assert(*(p1c_args->p1c_args->write_nodes->begin()) == 1);
  nodes->insert(2);
  p1c_args->nodes = nodes;
  p1c_args->vote = true;
  rpc_server->sendReply(rid1);
  while (!rpc_server->asyncNextCompletedReply(&rid2));
  assert(rid1 == rid2);
  p1c_args->nodes->clear();
  rpc_server->deleteRequest(rid2);
  nodes->clear();

  while (!rpc_server->asyncNextRequest(&rid1, &request, &args));
  assert(request == P2C);
  p2c_args = (rpc_p2c_args_t *) args;
  assert(p2c_args->p2c_args->tid == 0);
  assert(p2c_args->p2c_args->vote);
  nodes->insert(3);
  p2c_args->nodes = nodes;
  rpc_server->sendReply(rid1);
  while (!rpc_server->asyncNextCompletedReply(&rid2));
  assert(rid1 == rid2);
  p2c_args->nodes->clear();
  rpc_server->deleteRequest(rid2);
  nodes->clear();
}

void RPCTest::run() {
  std::thread server_thread(runServer, _rpc_server);
  std::thread client_thread(runClient, _rpc_client, _port);

  server_thread.join();
  client_thread.join();
  delete _rpc_client;
  delete _rpc_server;

  std::cout << "RPC test was successful." << std::endl;
}

int main(int argc, char **argv) {
  // Take arguments.
  if (argc != 2) {
    std::cerr << "Usage: ./RPCTest <server port>" << std::endl;
    return 0;
  }

  int port = atoi(argv[1]);
  GRPCServer *grpc_server = new GRPCServer(port);
  GRPCClient *grpc_client = new GRPCClient();
  RPCTest rpcTest(grpc_client, grpc_server, port);

  rpcTest.run();
  return 0;
}
