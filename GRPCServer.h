/**
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
 * Implementation of the RPC layer in the server side using google RPC.
 */
class GRPCServer: public RPCServer
{
public:

  /**
   * Handler for one specific request.
   */
  class RequestHandler
  {
  protected:

    /**
     * Enumeration for different status the requests can be in the server side.
     */
    enum request_status_t
    {
      CREATE, PROCESS, FINISH
    };

    Mvtkvs::AsyncService *_service;         ///< Service from GRPC server.
    ServerCompletionQueue *_request_queue;  ///< Queue that stores pending requests.
    ServerCompletionQueue *_reply_queue;    ///< Queue that stores pending replies.
    ServerContext _ctx;                     ///< Server context for GRPC server.
    request_status_t _status;               ///< Status of the request.

  public:
    RequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
                   ServerCompletionQueue *reply_queue);
    virtual ~RequestHandler();

    /**
     * Prepare reply and send it. The status of the request should be PROCESS.
     * @param args  - Arguments for filling the reply.
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
   * Handler for one specific READ request.
   */
  class ReadHandler : private RequestHandler
  {
  private:
    ReadRequest _request;                             ///< Read request for GRPC.
    ServerAsyncResponseWriter<ReadReply> _responder;  ///< Responder for read reply.
    rpc_read_args_t _rpc_read_args;                   ///< Read RPC arguments.
  public:
    ReadHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
                ServerCompletionQueue *reply_queue);
    ~ReadHandler();

    void setReply() override;
    request_t getRequest() override;
    void *getArgs() override;
  };

  /**
   * Handler for one specific WRITE request.
   */
  class WriteHandler: private RequestHandler
  {
  private:
    WriteRequest _request;                            ///< Write request for GRPC.
    ServerAsyncResponseWriter<WriteReply> _responder; ///< Responder for write reply.
    rpc_write_args_t _rpc_write_args;                 ///< Write RPC arguments.
  public:
    WriteHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
                 ServerCompletionQueue *reply_queue);
    ~WriteHandler();

    void setReply() override;
    request_t getRequest() override;
    void *getArgs() override;
  };

  /**
   * Handler for one specific READ request.
   */
  class PhaseOneCommitHandler: private RequestHandler
  {
  private:
    PhaseOneCommitRequest _request;                             ///< Phase one commit request for GRPC.
    ServerAsyncResponseWriter<PhaseOneCommitReply> _responder;  ///< Responder for phase one commit reply.
    rpc_p1c_args_t _rpc_p1c_args;                               ///< Phase one commit RPC arguments.
  public:
    PhaseOneCommitHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
                          ServerCompletionQueue *reply_queue);
    ~PhaseOneCommitHandler();

    void setReply() override;
    request_t getRequest() override;
    void *getArgs() override;
  };

  /**
   * Handler for one specific READ request.
   */
  class PhaseTwoCommitHandler: private RequestHandler
  {
  private:
    PhaseTwoCommitRequest _request;                             ///< Phase two commit request for GRPC.
    ServerAsyncResponseWriter<PhaseTwoCommitReply> _responder;  ///< Responder for phase one commit reply.
    rpc_p2c_args_t _rpc_p2c_args;                               ///< Phase two commit RPC arguments.
  public:
    PhaseTwoCommitHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
                          ServerCompletionQueue *reply_queue);
    ~PhaseTwoCommitHandler();

    void setReply() override;
    request_t getRequest() override;
    void *getArgs() override;
  };

private:
  std::unique_ptr<ServerCompletionQueue> _request_queue;  ///< Queue that maintains all the incoming requests.
  std::unique_ptr<ServerCompletionQueue> _reply_queue;    ///< Queue that maintains all the pending replies.
  std::unique_ptr<Server> _server;                        ///< GRPC server object.
  Mvtkvs::AsyncService _service;                          ///< GRPC service object needed for receiving requests and
                                                          ///  sending replies.

public:
  GRPCServer(int port);
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

/**
 * Structure necessary for passing arguments to different threads.
 */
/*
struct thread_args_t {
  int id;                   ///< Thread ID.
  GRPCServer *grpc_server;  ///< GRPCServer object.
};
*/

#endif /* GRPCSERVER_H_ */