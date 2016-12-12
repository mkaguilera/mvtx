/*
 * RPCTest.cc
 *
 *  Created on: Dec 12, 2016
 *      Author: theo
 */

#include <cassert>

#include "Request.h"
#include "RPCTest.h"

RPCTest::RPCTest(RPCClient *rpc_client, RPCServer *rpc_server, int port)
  : _rpc_client(rpc_client), _rpc_server(rpc_server), _port(port) {
}

void RPCTest::run() {
  // Make four requests of different types.
  rpc_read_args_t *client_read_args = (rpc_read_args_t *) malloc(sizeof(rpc_read_args_t));
  rpc_write_args_t *client_write_args = (rpc_write_args_t *) malloc(sizeof(rpc_write_args_t));
  rpc_p1c_args_t *client_p1c_args = (rpc_p1c_args_t *) malloc(sizeof(rpc_p1c_args_t));
  rpc_p2c_args_t *client_p2c_args = (rpc_p2c_args_t *) malloc(sizeof(rpc_p2c_args_t));
  rpc_read_args_t *server_read_args;
  rpc_write_args_t *server_write_args;
  rpc_p1c_args_t *server_p1c_args;
  rpc_p2c_args_t *server_p2c_args;
  uint64_t rid;
  request_t request;
  void *args;

  client_read_args->read_args = (read_args_t *) malloc(sizeof(read_args_t));
  client_read_args->read_args->tid = 0;
  client_read_args->read_args->key = 0;
  client_read_args->read_args->start_ts = 0;
  client_write_args->write_args = (write_args_t *) malloc(sizeof(write_args_t));
  client_write_args->write_args->tid = 1;
  client_write_args->write_args->key = 1;
  client_write_args->write_args->value = new std::string("1");
  client_p1c_args->p1c_args = (p1c_args_t *) malloc(sizeof(p1c_args_t));
  client_p1c_args->p1c_args->tid = 2;
  client_p1c_args->p1c_args->start_ts = 2;
  client_p1c_args->p1c_args->commit_ts = 2;
  client_p1c_args->p1c_args->read_nodes = new std::set<uint64_t>();
  client_p1c_args->p1c_args->read_nodes->insert(0);
  client_p1c_args->p1c_args->write_nodes = new std::set<uint64_t>();
  client_p1c_args->p1c_args->write_nodes->insert(1);
  client_p2c_args->p2c_args = (p2c_args_t *) malloc(sizeof(p2c_args_t));
  client_p2c_args->p2c_args->tid = 3;
  client_p2c_args->p2c_args->vote = true;

  // Check synchronous-synchronous communication.
  _rpc_server->nextRequest(&rid, &request, &args);
  _rpc_client->syncRPC("localhost::" + std::to_string(_port), READ, client_read_args);
  assert(request == READ);
  server_read_args = (rpc_read_args_t *) args;
  assert(server_read_args->read_args->tid == 0);
  assert(server_read_args->read_args->key == 0);
  assert(server_read_args->read_args->start_ts == 0);
  server_read_args->status = true;
  server_read_args->value = "0";
  _rpc_server->sendReply(rid);

  std::cout << "RPC Test Successful." << std::endl;
}
