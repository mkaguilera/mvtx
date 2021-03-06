/*
 * ResolutionClient.h
 *
 *  Created on: Jun 13, 2016
 *      Author: theo
 */

#ifndef RESOLUTIONCLIENT_H_
#define RESOLUTIONCLIENT_H_

#include "Request.h"
#include "RPCClient.h"

/**
 * Class for deciding in which physical addresses the requests should be forwarded.
 */
class ResolutionClient
{
  protected:
    ///> RPC Client to use for making RPCs.
    RPCClient *_rpc_client;

  public:
    /**
     * Constructor of ResolutionClient. It initializes the network layer beneath the ResolutionClient.
     * @param rpc_client  - RPC Client to be used for sending requests and receiving replies.
     */
    ResolutionClient(RPCClient *rpc_client)
        : _rpc_client(rpc_client) {
    }
    ;

    virtual ~ResolutionClient() {
    }
    ;

    /**
     * Make a request while being agnostic about the physical location of nodes, keys, etc. It should block until it
     * succeeds, which means that if a node is unavailable, the call blocks until the node becomes available again.
     * @param nodes   - Nodes which should receive the request.
     * @param request - Request type.
     * @param args    - Arguments used for this specific request type.
     */
    virtual void request(std::set<uint64_t> nodes, request_t request, void *args) = 0;
};

#endif /* RESOLUTIONCLIENT_H_ */
