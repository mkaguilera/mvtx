/*
 * GRPCClient.h
 *
 *  Created on: Jun 7, 2016
 *      Author: theo
 */

#ifndef GRPCCLIENT_H_
#define GRPCCLIENT_H_

#include <unordered_map>
#include <grpc++/grpc++.h>
#include "MvtkvsService.grpc.pb.h"
#include "RPCClient.h"

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
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
 * Implementation of Remote Procedure Call (RPC) client layer with Google RPC (gRPC).
 */
class GRPCClient: public RPCClient
{
  public:
    /**
     * Implementation of request handlers, responsible for initializing, sending and deleting a gRPC request.
     * Applicable only for asynchronous requests.
     */
    class RequestHandler
    {
      protected:
        /**
         * Request status. INIT means that it is initialized but not yet send. WAIT means that it has been sent but the
         * reply is still pending.
         */
        enum RequestStatus
        {
          INIT, WAIT
        };

        ///> Client making the request.
        GRPCClient *_grpc_client;
        ///> Server address to send the request.
        std::string _addr;
        ///> Client context for gRPC calls.
        ClientContext _ctx;
        ///> Queue for getting the reply when it is completed.
        CompletionQueue _cq;
        ///> Whether or not gRPC is successful.
        Status _status;
        ///> Status of the request.
        RequestStatus _rstatus;

      public:
        /**
         * Constructor of RequestHandler.
         * @param grpc_client - GRPCClient that utilizes these handlers.
         * @param addr        - Address to send the request.
         */
        RequestHandler(GRPCClient *grpc_client, const std::string &addr);

        /**
         * Destructor of RequestHandler.
         */
        virtual ~RequestHandler();

        /**
         * Execute part of the request (INIT or WAIT).
         */
        virtual void proceed() = 0;

        /**
         * Get the status of the request.
         * @return  - OK if it is successful, CANCELLED otherwise.
         */
        virtual Status getStatus();
    };

    /**
     * Implementation of read request handlers.
     */
    class ReadRequestHandler: public RequestHandler
    {
      private:
        ///< Read gRPC arguments.
        rpc_read_args_t *_rpc_read_args;
        ///< Reader of gRPC reply for the specific request.
        std::unique_ptr<ClientAsyncResponseReader<ReadReply>> _reply_reader;
        ///< Structure to hold the reply.
        ReadReply _reply;

      public:
        /**
         * ReadRequestHandler constructor.
         * @param grpc_client - GRPCClient that utilizes this handler.
         * @param addr        - Address to send the read request.
         * @param read_args   - Arguments needed for the read request.
         */
        ReadRequestHandler(GRPCClient *grpc_client, const std::string &addr, rpc_read_args_t *read_args);
        ~ReadRequestHandler();
        void proceed();
    };

    /**
     * Implementation of write request handlers.
     */
    class WriteRequestHandler: public RequestHandler
    {
      private:
        ///> Write gRPC arguments.
        rpc_write_args_t *_rpc_write_args;
        ///> Reader of gRPC reply for the specific request.
        std::unique_ptr<ClientAsyncResponseReader<WriteReply>> _reply_reader;
        ///> Structure to hold the reply.
        WriteReply _reply;

      public:
        /**
         * WriteRequestHandler constructor.
         * @param grpc_client - GRPCClient that utilizes this handler.
         * @param addr        - Address to send the write request.
         * @param write_args  - Arguments needed for the write request.
         */
        WriteRequestHandler(GRPCClient *grpc_client, const std::string &addr, rpc_write_args_t *write_args);
        ~WriteRequestHandler();
        void proceed();
    };

    /**
     * Implementation of phase one commit request handlers.
     */
    class PhaseOneCommitRequestHandler: public RequestHandler
    {
      private:
        ///> Phase one commit gRPC arguments.
        rpc_p1c_args_t *_rpc_p1c_args;
        ///> Reader of gRPC reply for the specific request.
        std::unique_ptr<ClientAsyncResponseReader<PhaseOneCommitReply>> _reply_reader;
        ///> Structure to hold the reply.
        PhaseOneCommitReply _reply;

      public:
        /**
         * PhaseOneCommitRequestHandler constructor.
         * @param grpc_client - GRPCClient that utilizes this handler.
         * @param addr        - Address to send the phase one commit request.
         * @param p1c_args    - Arguments needed for the phase one commit request.
         */
        PhaseOneCommitRequestHandler(GRPCClient *grpc_client, const std::string &addr, rpc_p1c_args_t *p1c_args);
        ~PhaseOneCommitRequestHandler();
        void proceed();
    };

    /**
     * Implementation of phase two commit request handlers.
     */
    class PhaseTwoCommitRequestHandler: public RequestHandler
    {
      private:
        ///> Phase two commit gRPC arguments.
        rpc_p2c_args_t *_rpc_p2c_args;
        ///> Reader of gRPC reply for the specific request.
        std::unique_ptr<ClientAsyncResponseReader<PhaseTwoCommitReply>> _reply_reader;
        ///> Structure to hold the reply.
        PhaseTwoCommitReply _reply;

      public:
        /**
         * PhaseTwoCommitRequestHandler constructor.
         * @param grpc_client - GRPCClient that utilizes this handler.
         * @param addr        - Address to send the phase two commit request.
         * @param p2c_args    - Arguments needed for the phase two commit request.
         */
        PhaseTwoCommitRequestHandler(GRPCClient *grpc_client, const std::string &addr, rpc_p2c_args_t *p2c_args);
        ~PhaseTwoCommitRequestHandler();
        void proceed();
    };

  private:
    std::unordered_map<std::string, std::unique_ptr<Mvtkvs::Stub>> _address_to_stub;
    std::unordered_map<uint64_t, RequestHandler *> _tag_to_handler;
    std::mutex _mutex1, _mutex2;

  public:
    /**
     * Destructor of GRPCClient.
     */
    ~GRPCClient();

  private:
    void makeStub(const std::string &addr);

  public:
    /**
     * Get stub for gRPCs.
     * @param addr  - Address for which stub is needed.
     * @return      - Pointer to the appropriate stub.
     */
    Mvtkvs::Stub *getStub(std::string addr);

    /**
     * Prepare a read request (marshalling).
     * @param read_args - Arguments needed for a gRPC read request.
     * @return          - Marshalled gRPC read request.
     */
    static ReadRequest makeReadRequest(const read_args_t *read_args);

    /**
     * Prepare a write request (marshalling).
     * @param write_args  - Arguments needed for a gRPC write request.
     * @return            - Marshalled gRPC write request.
     */
    static WriteRequest makeWriteRequest(const write_args_t *write_args);

    /**
     * Prepare a phase one commit request (marshalling).
     * @param p1c_args  - Arguments needed for a gRPC phase one commit request.
     * @return          - Marshalled gRPC phase one commit request.
     */
    static PhaseOneCommitRequest makePhaseOneCommitRequest(const p1c_args_t *p1c_args);

    /**
     * Prepare a phase two commit request (marshalling).
     * @param p2c_args  - Arguments needed for a gRPC phase two commit request.
     * @return          - Marshalled gRPC phase two commit request.
     */
    static PhaseTwoCommitRequest makePhaseTwoCommitRequest(const p2c_args_t *p2c_args);

    bool syncRPC(const std::string &addr, request_t request, void *args) override;
    void asyncRPC(const std::string &addr, uint64_t tag, request_t request, void *args) override;
    bool waitAsyncReply(uint64_t tag) override;
};

#endif /* GRPCCLIENT_H_ */
