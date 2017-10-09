/*
 * CoordinatorTest.h
 *
 *  Created on: May 15, 2017
 *      Author: theo
 */
#ifndef COORDINATORTEST_H_
#define COORDINATORTEST_H_

#include <vector>

#include "Coordinator.h"
#include "KeyMapper.h"
#include "RPCServer.h"
#include "Test.h"

/**
 *  Test class for Coordinator.
 */
class CoordinatorTest: public Test
{
  private:
    Coordinator *_coordinator;
    std::vector<RPCServer *> *_rpc_servers;
    KeyMapper *_key_mapper;

  public:
    /**
     * Initiates coordinator test.
     * @param coordinator - Coordinator class that needs to be tested.
     * @param rpc_servers - Servers that need to be utilized.
     * @param key_mapper  - Static mapping from keys to nodes.
     */
    CoordinatorTest(Coordinator *coordinator, std::vector<RPCServer *> *rpc_servers, KeyMapper *key_mapper);
    virtual ~CoordinatorTest();
    void run() override;

  private:
    static void replyRequest(std::vector<RPCServer *> *rpc_servers, KeyMapper *key_mapper);
};

#endif /* COORDINATORTEST_H_ */
