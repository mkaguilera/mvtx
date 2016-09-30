/**
 * RPCClient.h
 *
 *  Created on: Jun 6, 2016
 *      Author: theo
 */

#ifndef RPCCLIENT_H_
#define RPCCLIENT_H_

#include <iostream>
#include <string>
#include <sys/types.h>
#include "Request.h"

// TODO: Revisit interface when replication or other advanced features are introduced.

/**
 * Interface for Remote Procedutre Call (RPC) clients. RPC implementations have to provide this API.
 */
class RPCClient {
public:
  virtual ~RPCClient() {};

  /**
   * Synchronous RPC. It should return after reply is received.
   * @param addr    - Address of the destination (RPC server).
   * @param request - Request type.
   * @param args    - Corresponding RPC arguments for the specific request type.
   * @return status - Whether or not the RPC was successful.
   */
  virtual bool syncRPC(const std::string &addr, request_t request, void *args) = 0;

  /**
   * Asynchronous RPC. It should return after request is sent.
   * @param addr    - Address of the destination (RPC server).
   * @param tag     - Tag which is going to be used as an identifier for the request.
   * @param request - Request type.
   * @param args    - Corresponding RPC arguments for the specific request type.
   */
  virtual void asyncRPC(const std::string &addr, uint64_t tag, request_t request, void *args) = 0;

  /**
   * Get a reply of an asynchronous request. It should block if there is no reply yet.
   * @param tag     - Tag of the corresponding request.
   * @return status - Whether or not the RPC was successful.
   */
  virtual bool waitAsyncReply(uint64_t tag) = 0;
};

#endif /* RPCCLIENT_H_ */