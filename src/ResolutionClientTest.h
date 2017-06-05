/*
 * ResolutionClientTest.h
 *
 *  Created on: May 1, 2017
 *      Author: theo
 */
#ifndef RESOLUTIONCLIENTTEST_H_
#define RESOLUTIONCLIENTTEST_H_

#include <vector>

#include "ResolutionClient.h"
#include "RPCServer.h"
#include "Test.h"

/**
 *  Test class for ResolutionClient.
 */
class ResolutionClientTest: public Test
{
  private:
    ResolutionClient *_resolution_client;
    std::vector<RPCServer *> *_rpc_servers;

  public:
    /**
     * Initiates resolution client test.
     * @param resolution_client - ResolutionClient class that needs to be tested.
     * @param rpc_server        - Servers that need to be utilized.
     */
    ResolutionClientTest(ResolutionClient *resolution_client, std::vector<RPCServer *> *rpc_servers);

    virtual ~ResolutionClientTest();
    void run() override;

  private:
    static void replyRequest(std::vector<RPCServer *> *rpc_servers);
};

#endif /* RESOLUTIONCLIENTTEST_H_ */
