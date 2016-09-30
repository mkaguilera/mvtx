/**
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
 * Implementation of Remote Procedutre Call (RPC) layer with Google RPC (gRPC).
 */
class GRPCClient : public RPCClient
{
public:
  /**
   * Implementation of request handlers, responsible for initializing, sending and deleting a request. Applicable only
   * to asynchronous requests.
   */
  class RequestHandler
  {
  public:
    /**
     * Request status. INIT means that it is initialized but not yet send. WAIT means that it has been sent but the
     * reply is still pending. Finally, FINISH means that the reply has been received (structures associated
     * with the request can be deleted).
     */
    enum RequestStatus {INIT, WAIT};
  protected:
    GRPCClient *_grpc_client; ///< Client making the request.
    std::string _addr;        ///< Server address to send the request.
    ClientContext _ctx;       ///< Client context for gRPC calls.
    CompletionQueue _cq;      ///< Queue for getting the reply when it is completed.
    Status _status;           ///< Whether or not gRPC is successful.
    RequestStatus _rstatus;   ///< Status of the request.

  public:
    RequestHandler (GRPCClient *grpc_client, const std::string &addr);
    virtual ~RequestHandler();
    virtual void proceed() = 0;       ///< Move request to the next phase.
    virtual Status getStatus();       ///< Whether or not gRPC is successful.
  };

  /**
   * Implementation of read request handlers.
   */
  class ReadRequestHandler : public RequestHandler
  {
  private:
    rpc_read_args_t *_rpc_read_args;                                      ///< Read gRPC arguments.
    std::unique_ptr<ClientAsyncResponseReader<ReadReply>> _reply_reader;  ///< Reader of gRPC reply for the specific
                                                                          ///  request.
    ReadReply _reply;                                                     ///< Structure to hold the reply.

  public:
    ReadRequestHandler (GRPCClient *grpc_client, const std::string &addr, rpc_read_args_t *read_args);
    ~ReadRequestHandler() {};
    void proceed();                   ///< Move read request to the next phase.
  };

  /**
   * Implementation of read request handlers.
   */
  class WriteRequestHandler : public RequestHandler
  {
  private:
    rpc_write_args_t *_rpc_write_args;                                    ///< Write gRPC arguments.
    std::unique_ptr<ClientAsyncResponseReader<WriteReply>> _reply_reader; ///< Reader of gRPC reply for the specific
                                                                          ///  request.
    WriteReply _reply;                                                    ///< Structure to hold the reply.

  public:
    WriteRequestHandler(GRPCClient *grpc_client, const std::string &addr, rpc_write_args_t *write_args);
    ~WriteRequestHandler() {};
    void proceed();                   ///< Move write request to the next phase.
  };

  /**
   * Implementation of read request handlers.
   */
  class PhaseOneCommitRequestHandler : public RequestHandler
  {
  private:
    rpc_p1c_args_t *_rpc_p1c_args;                                                  ///< P1C gRPC arguments.
    std::unique_ptr<ClientAsyncResponseReader<PhaseOneCommitReply>> _reply_reader;  ///< Reader of gRPC reply for the
                                                                                    ///  specific request.
    PhaseOneCommitReply _reply;                                                     ///< Structure to hold the reply.

  public:
    PhaseOneCommitRequestHandler(GRPCClient *grpc_client, const std::string &addr, rpc_p1c_args_t *p1c_args);
    ~PhaseOneCommitRequestHandler() {};
    void proceed();                   ///< Move phase one commit request to the next phase.
  };

  /**
   * Implementation of read request handlers.
   */
  class PhaseTwoCommitRequestHandler : public RequestHandler
  {
  private:
    rpc_p2c_args_t *_rpc_p2c_args;                                                  ///< P2C gRPC arguments.
    std::unique_ptr<ClientAsyncResponseReader<PhaseTwoCommitReply>> _reply_reader;  ///< Reader of gRPC reply for the
                                                                                    ///  specific request.
    PhaseTwoCommitReply _reply;                                                     ///< Structure to hold the reply.

  public:
    PhaseTwoCommitRequestHandler(GRPCClient *grpc_client, const std::string &addr, rpc_p2c_args_t *p2c_args);
    ~PhaseTwoCommitRequestHandler() {};
    void proceed();                   ///< Move phase two commit request to the next phase.
  };

private:
  std::unordered_map<std::string, std::unique_ptr<Mvtkvs::Stub>> _address_to_stub;  ///< Map from address to gRPC stub.
  std::unordered_map<uint64_t, RequestHandler *> _tag_to_handler;                   ///< Map from tag to request.
  std::mutex _mutex1, _mutex2;                                                      ///< Locks for thread safe
                                                                                    ///  implementation.

public:
  ~GRPCClient();

private:
  /**
   * Create stub for gRPCs if it does not already exist.
   * @param addr  - Address of the server.
   */
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
   * @param read_args - Arguments needed for a read request.
   * @return          - Marshalled read request.
   */
  static ReadRequest makeReadRequest(const read_args_t *read_args);

  /**
   * Prepare a write request (marshalling).
   * @param write_args  - Arguments needed for a write request.
   * @return            - Marshalled write request.
   */
  static WriteRequest makeWriteRequest(const write_args_t *write_args);

  /**
   * Prepare a phase one commit request (marshalling).
   * @param p1c_args  - Arguments needed for a phase one commit request.
   * @return          - Marshalled phase one commit request.
   */
  static PhaseOneCommitRequest makePhaseOneCommitRequest(const p1c_args_t *p1c_args);

  /**
   * Prepare a phase two commit request (marshalling).
   * @param p2c_args  - Arguments needed for a phase two commit request.
   * @return          - Marshalled phase two commit request.
   */
  static PhaseTwoCommitRequest makePhaseTwoCommitRequest(const p2c_args_t *p2c_args);

  bool syncRPC(const std::string &addr, request_t request, void *args) override;
  void asyncRPC(const std::string &addr, uint64_t tag, request_t request, void *args) override;
  bool waitAsyncReply(uint64_t tag) override;
};

#endif /* GRPCCLIENT_H_ */