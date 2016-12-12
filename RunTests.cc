/*
 * RunTests.cc
 *
 *  Created on: Dec 8, 2016
 *      Author: theo
 */

#include "AVLTreeLockManager.h"
#include "GRPCClient.h"
#include "GRPCServer.h"

#include "LockManagerTest.h"
#include "RPCTest.h"

int main(int argc, char **argv) {
  AVLTreeLockManager lock_manager;
  GRPCClient grpcClient;
  GRPCServer grpcServer(10000);

  LockManagerTest lock_manager_test(&lock_manager);
  RPCTest rpc_test(&grpcClient, &grpcServer, 10000);

  lock_manager_test.run();
  // rpc_test.run();
}
