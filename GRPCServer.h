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
      protected:
        /**
         * Request status for the server side. CREATE means that it is waiting for a request. PROCESS means that it has
         * processed a request but has not yet sent the reply, and FINISH means that the reply has been sent and
         * handler is available for garbage collection.
         */
        enum RequestStatus
        {
          CREATE, PROCESS, FINISH
        };

        ///> Service from GRPC server.
        Mvtkvs::AsyncService *_service;
        ///> Queue that stores pending requests.
        ServerCompletionQueue *_request_queue;
        ///> Queue that stores pending replies.
        ServerCompletionQueue *_reply_queue;
        ///> Server context for GRPC server.
        ServerContext _ctx;
        ///> Status of the request.
        RequestStatus _status;

      public:
        /**
         * Constructor of RequestHandler.
         * @param service       - Service (RPCs) that is going to be provided to the clients.
         * @param request_queue - Queue that holds all incoming requests that have not been processed yet.
         * @param reply_queue   - Queue that holds all outgoing replies that have not been processed yet.
         */
        RequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
            ServerCompletionQueue *reply_queue);

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
    };

    /**
     * Implementation of read request handlers.
     */
    class ReadRequestHandler: private RequestHandler
    {
      private:
        ReadRequest _request;
        ServerAsyncResponseWriter<ReadReply> _responder;
        rpc_read_args_t _rpc_read_args;
      public:
        /**
         * Constructor of ReadRequestHandler.
         * @param service       - Service (RPCs) that is going to be provided to the clients.
         * @param request_queue - Queue that holds all incoming requests that have not been processed yet.
         * @param reply_queue   - Queue that holds all outgoing replies that have not been processed yet.
         */
        ReadRequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
            ServerCompletionQueue *reply_queue);
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
        WriteRequest _request;
        ServerAsyncResponseWriter<WriteReply> _responder;
        rpc_write_args_t _rpc_write_args;
      public:
        /**
         * Constructor of WriteRequestHandler.
         * @param service       - Service (RPCs) that is going to be provided to the clients.
         * @param request_queue - Queue that holds all incoming requests that have not been processed yet.
         * @param reply_queue   - Queue that holds all outgoing replies that have not been processed yet.
         */
        WriteRequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
            ServerCompletionQueue *reply_queue);
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
        PhaseOneCommitRequest _request;
        ServerAsyncResponseWriter<PhaseOneCommitReply> _responder;
        rpc_p1c_args_t _rpc_p1c_args;
      public:
        /**
         * Constructor of PhaseOneCommitRequestHandler.
         * @param service       - Service (RPCs) that is going to be provided to the clients.
         * @param request_queue - Queue that holds all incoming requests that have not been processed yet.
         * @param reply_queue   - Queue that holds all outgoing replies that have not been processed yet.
         */
        PhaseOneCommitRequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
            ServerCompletionQueue *reply_queue);
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
        PhaseTwoCommitRequest _request;                             ///< Phase two commit request for GRPC.
        ServerAsyncResponseWriter<PhaseTwoCommitReply> _responder;  ///< Responder for phase one commit reply.
        rpc_p2c_args_t _rpc_p2c_args;                               ///< Phase two commit RPC arguments.
      public:
        /**
         * Constructor of PhaseTwoCommitRequestHandler.
         * @param service       - Service (RPCs) that is going to be provided to the clients.
         * @param request_queue - Queue that holds all incoming requests that have not been processed yet.
         * @param reply_queue   - Queue that holds all outgoing replies that have not been processed yet.
         */
        PhaseTwoCommitRequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
            ServerCompletionQueue *reply_queue);
        /**
         * Destructor of PhaseTwoCommitRequestHandler.
         */
        ~PhaseTwoCommitRequestHandler();

        void setReply() override;
        request_t getRequest() override;
        void *getArgs() override;
    };

  private:
    ///> Queue that maintains all the incoming requests.
    std::unique_ptr<ServerCompletionQueue> _request_queue;
    ///> Queue that maintains all the pending replies.
    std::unique_ptr<ServerCompletionQueue> _reply_queue;
    ///> GRPC server object.
    std::unique_ptr<Server> _server;
    ///< GRPC service object needed for receiving requests and sending replies.
    Mvtkvs::AsyncService _service;

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

    void nextRequest(uint64_t *rid, request_t *request, void **args) override;
    bool asyncNextRequest(uint64_t *rid, request_t *request, void **args) override;
    void sendReply(uint64_t rid) override;
    void nextCompletedReply(uint64_t *rid) override;
    bool asyncNextCompletedReply(uint64_t *rid) override;
    void deleteRequest(uint64_t rid) override;

  private:
    /**
     * Prepares next handler for incoming requests.
     * @param request - Type of the request.
     */
    void prepareNextRequest(request_t request);
};

#endif /* GRPCSERVER_H_ */
