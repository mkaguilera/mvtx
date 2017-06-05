/*
 * RPCServer.h
 *
 *  Created on: Jun 20, 2016
 *      Author: theo
 */

#ifndef RPCSERVER_H_
#define RPCSERVER_H_

#include "Request.h"

/**
 * Interface for the RPC layer in the server side.
 */
class RPCServer
{
  public:
    virtual ~RPCServer() {
    }
    ;

    /**
     * Get next incoming request (blocking).
     * @param rid     - Request ID.
     * @param request - Request type.
     * @param args    - Request arguments.
     */
    virtual void nextRequest(uint64_t *rid, request_t *request, void **args) = 0;

    /**
     * Try to get next incoming request.
     * @param rid     - Request ID.
     * @param request - Request type.
     * @param args    - Request arguments.
     * @return        - Whether or not operation was successful.
     */
    virtual bool asyncNextRequest(uint64_t *rid, request_t *request, void **args) = 0;

    /**
     * Send Reply back to coordinator (blocking).
     * @param rid - Request ID.
     */
    virtual void sendReply(uint64_t rid) = 0;

    /**
     * Get the next completed reply (blocking).
     * @param rid - Request ID.
     */
    virtual void nextCompletedReply(uint64_t *rid) = 0;

    /**
     * See if any reply has been completed.
     * @param rid - Request ID.
     * @return    - Whether or not operation was successful.
     */
    virtual bool asyncNextCompletedReply(uint64_t *rid) = 0;

    /**
     * Delete request structures.
     * @param rid - Request ID.
     */
    virtual void deleteRequest(uint64_t rid) = 0;
};

#endif /* RPCSERVER_H_ */
