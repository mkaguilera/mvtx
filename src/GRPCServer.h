/*
 * GRPCServer.h
 *
 *  Created on: Jun 21, 2016
 *      Author: theo
 */

#ifndef GRPCSERVER_H_
#define GRPCSERVER_H_

#include <grpc++/grpc++.h>
#include "MvtkvsService.grpc.pb.h"
#include "RPCServer.h"

using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using mvtkvs::ReadRequest;
using mvtkvs::ReadReply;
using mvtkvs::WriteRequest;
using mvtkvs::WriteReply;
using mvtkvs::PhaseOneCommitRequest;
using mvtkvs::PhaseOneCommitReply;
using mvtkvs::PhaseTwoCommitRequest;
using mvtkvs::PhaseTwoCommitReply;
using mvtkvs::Mvtkvs;

/**
 * Implementation of Remote Procedure Call (RPC) server layer with Google RPC (gRPC).
 */
class GRPCServer: public RPCServer
{
  public:

    /**
     * Generic class for request handlers, that are responsible for processing gRPC requests and replying to them.
     */
    class RequestHandler
    {
      public:
        /**
         * Request status for the server side. CREATE means that it is waiting for a request. PROCESS means that it has
         * processed a request but has not yet sent the reply, and FINISH means that the reply has been sent and
         * handler is available for garbage collection.
         */
        enum RequestStatus
        {
          CREATE, PROCESS, FINISH
        };

      protected:
        ///> Status of the request.
        RequestStatus _status;

        ///> Server context for GRPC server.
        ServerContext _ctx;

      public:
        /**
         * Constructor of RequestHandler.
         */
        RequestHandler();

        /**
         * Destructor of RequestHandler. It erases all the queues and their content.
         */
        virtual ~RequestHandler();

        /**
         * Prepare reply. The status of the request should be PROCESS.
         */
        virtual void setReply() = 0;

        /**
         * Get the type of the request. The status of the request should be PROCESS.
         * @return  - The type of the request.
         */
        virtual request_t getRequest() = 0;

        /**
         * Get the arguments for this request. The status of the request should be PROCESS.
         * @return  - The arguments of the request.
         */
        virtual void *getArgs() = 0;

        /**
         * Get the status of this request.
         * @return  - The status of the request.
         */
        RequestStatus getStatus();
    };

    /**
     * Implementation of read request handlers.
     */
    class ReadRequestHandler: private RequestHandler
    {
      private:
        ReadRequest _request;                             ///> Read request for GRPC.
        ServerAsyncResponseWriter<ReadReply> _responder;  ///> Responder for read reply.
        rpc_read_args_t _rpc_read_args;                   ///> Read RPC arguments.
      public:
        /**
         * Constructor of ReadRequestHandler.
         * @param service - Service (RPCs) that is going to be provided to the clients.
         * @param queue   - Queue that holds all incoming requests and all outgoing replies that have not been
         *                  processed yet.
         */
        ReadRequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *queue);
        /**
         * Destructor of ReadRequestHandler.
         */
        ~ReadRequestHandler();

        void setReply() override;
        request_t getRequest() override;
        void *getArgs() override;
    };

    /**
     * Handler for one specific WRITE request.
     */
    class WriteRequestHandler: private RequestHandler
    {
      private:
        WriteRequest _request;                            ///> Write request for GRPC.
        ServerAsyncResponseWriter<WriteReply> _responder; ///> Responder for write reply.
        rpc_write_args_t _rpc_write_args;                 ///> Write RPC arguments.
      public:
        /**
         * Constructor of WriteRequestHandler.
         * @param service - Service (RPCs) that is going to be provided to the clients.
         * @param queue   - Queue that holds all incoming requests and all outgoing replies that have not been
         *                  processed yet.
         */
        WriteRequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *queue);
        /**
         * Destructor of WriteRequestHandler.
         */
        ~WriteRequestHandler();

        void setReply() override;
        request_t getRequest() override;
        void *getArgs() override;
    };

    /**
     * Handler for one specific READ request.
     */
    class PhaseOneCommitRequestHandler: private RequestHandler
    {
      private:
        PhaseOneCommitRequest _request;                             ///> Phase one commit request for GRPC.
        ServerAsyncResponseWriter<PhaseOneCommitReply> _responder;  ///> Responder for phase one commit reply.
        rpc_p1c_args_t _rpc_p1c_args;                               ///> Phase one commit RPC arguments.
      public:
        /**
         * Constructor of PhaseOneCommitRequestHandler.
         * @param service - Service (RPCs) that is going to be provided to the clients.
         * @param queue   - Queue that holds all incoming requests and all outgoing replies that have not been
         *                  processed yet.
         */
        PhaseOneCommitRequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *queue);
        /**
         * Destructor of PhaseOneCommitRequestHandler.
         */
        ~PhaseOneCommitRequestHandler();

        void setReply() override;
        request_t getRequest() override;
        void *getArgs() override;
    };

    /**
     * Handler for one specific READ request.
     */
    class PhaseTwoCommitRequestHandler: private RequestHandler
    {
      private:
        PhaseTwoCommitRequest _request;                             ///> Phase two commit request for GRPC.
        ServerAsyncResponseWriter<PhaseTwoCommitReply> _responder;  ///> Responder for phase two commit reply.
        rpc_p2c_args_t _rpc_p2c_args;                               ///> Phase two commit RPC arguments.
      public:
        /**
         * Constructor of PhaseTwoCommitRequestHandler.
         * @param service - Service (RPCs) that is going to be provided to the clients.
         * @param queue   - Queue that holds all incoming requests and all outgoing replies that have not been
         *                  processed yet.
         */
        PhaseTwoCommitRequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *queue);
        /**
         * Destructor of PhaseTwoCommitRequestHandler.
         */
        ~PhaseTwoCommitRequestHandler();

        void setReply() override;
        request_t getRequest() override;
        void *getArgs() override;
    };

  private:
    ///> GRPC server object.
    std::unique_ptr<Server> _server;
    ///> Service from GRPC server.
    Mvtkvs::AsyncService _service;
    ///> Queue that stores pending requests and replies.
    std::unique_ptr<ServerCompletionQueue> _queue;
    ///> Request queue that stores requests handlers in the PROCESS state.
    std::vector<RequestHandler *> *_request_queue;
    ///> Reply queue that stores requests handlers in the FINISH state.
    std::vector<RequestHandler *> *_reply_queue;

  public:
    /**
     * Constructor of GRPCServer. The server listens for RPC requests at a specific port.
     * @param port  - Network port in which server receives requests.
     */
    GRPCServer(int port);

    /**
     * Destructor of GRPCServer. The server stops listening for requests.
     */
    ~GRPCServer();

  private:
    /**
     * Process gRPC queue and put messages to request or reply queue as handlers (synchronous).
     * @param request - Type of the request.
     */
    void processSyncQueue();

    /**
     * Process gRPC queue and put messages to request or reply queue as handlers (asynchronous).
     * @param request - Type of the request.
     */
    void processAsyncQueue();

    /**
     * Prepares next handler for incoming requests.
     * @param request - Type of the request.
     */
    void prepareNextRequest(request_t request);

  public:
    void nextRequest(uint64_t *rid, request_t *request, void **args) override;
    bool asyncNextRequest(uint64_t *rid, request_t *request, void **args) override;
    void sendReply(uint64_t rid) override;
    void nextCompletedReply(uint64_t *rid) override;
    bool asyncNextCompletedReply(uint64_t *rid) override;
    void deleteRequest(uint64_t rid) override;
};

#endif /* GRPCSERVER_H_ */
