/*
 * RPCTest.h
 *
 *  Created on: Dec 12, 2016
 *      Author: theo
 */

#ifndef RPCTEST_H_
#define RPCTEST_H_

#include "RPCClient.h"
#include "RPCServer.h"
#include "Test.h"

/**
 * Test class for testing the implementation of the RPC layer.
 */
class RPCTest : public Test {
  private:
    RPCClient *_rpc_client;
    RPCServer *_rpc_server;
    int _port;

  public:
    /**
     * Initiates test for RPC layer.
     * @param rpc_client  - RPC client, responsible for making requests.
     * @param rpc_server  - RPC server, responsible for receiving requests.
     * @param port        - Port at which server is operating.
     */
    RPCTest(RPCClient *rpc_client, RPCServer *rpc_server, int port);

    void run() override;
};

#endif /* RPCTEST_H_ */
