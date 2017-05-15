/*
 * RunTests.cc
 *
 *  Created on: Dec 8, 2016
 *      Author: theo
 */

#include "AVLTreeLockManager.h"
#include "GRPCClient.h"
#include "GRPCServer.h"
#include "SimpleKeyMapper.h"
#include "SimpleResolutionClient.h"
#include "SimpleTransactionIDGenerator.h"
#include "SimpleTimestampGenerator.h"
#include "WithdrawCoordinator.h"

#include "LockManagerTest.h"
#include "RPCTest.h"
#include "ResolutionClientTest.h"
#include "CoordinatorTest.h"

int main(int argc, char **argv) {
  AVLTreeLockManager lock_manager;
  GRPCClient grpc_client;
  GRPCServer grpc_server(10000);
  SimpleResolutionClient simple_resolution_client(&grpc_client);
  SimpleKeyMapper simple_key_mapper;
  SimpleTimestampGenerator simple_timestamp_generator;
  SimpleTransactionIDGenerator simple_transaction_id_generator;
  WithdrawCoordinator withdraw_coordinator(&simple_resolution_client, &simple_key_mapper,
      &simple_transaction_id_generator, &simple_timestamp_generator, 100);
  std::vector<RPCServer *> rpc_servers;
  rpc_servers.push_back(&grpc_server);

  LockManagerTest lock_manager_test(&lock_manager);
  RPCTest rpc_test(&grpc_client, &grpc_server, 10000);
  ResolutionClientTest resolution_client_test(&simple_resolution_client, &rpc_servers);
  CoordinatorTest coordinator_test(&withdraw_coordinator, &rpc_servers, &simple_key_mapper);

  lock_manager_test.run();
  rpc_test.run();
  resolution_client_test.run();
  coordinator_test.run();
}
