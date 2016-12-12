/*
 *  mvtkvsAsyncServer.cc
 *
 *  Created on: June 1, 2016
 *      Author: theo
 */

#include <memory>
#include <iostream>
#include <string>
#include <thread>
#include <grpc++/grpc++.h>
#include <unistd.h>
#include "KeyMapper.h"
#include "MvtkvsService.grpc.pb.h"

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

class MvtkvsAsyncServer final
{
private:
  std::unique_ptr<ServerCompletionQueue> _cq;
  std::unique_ptr<Server> _server;
  Mvtkvs::AsyncService _service;
  uint32_t _nr_threads;

  struct thread_args_t {
    ServerCompletionQueue *cq;
    int id;
  };

  class Handler
  {
  protected:
    Mvtkvs::AsyncService *_service;
    ServerCompletionQueue *_cq;
    ServerContext _ctx;

    enum handler_status_t
    {
      CREATE, PROCESS, FINISH
    };
    handler_status_t _status;
  public:
    Handler(Mvtkvs::AsyncService *service, ServerCompletionQueue *cq)
        : _service(service), _cq(cq), _status(CREATE) { }

    virtual void proceed() = 0;
  };

  class ReadHandler: private Handler
  {
  private:
    ReadRequest _request;
    ServerAsyncResponseWriter<ReadReply> _responder;
  public:
    ReadHandler(Mvtkvs::AsyncService* service, ServerCompletionQueue* cq)
        : _responder(&_ctx), Handler(service, cq) {
      proceed();
    }

    void proceed() override {
      switch (_status) {
        case CREATE: {
          _status = PROCESS;
          _service->RequestRead(&_ctx, &_request, &_responder, _cq, _cq, this);
          break;
        }
        case PROCESS: {
          ReadReply reply;
          uint64_t tid;
          uint64_t start_ts;
          uint64_t key;

          new ReadHandler(_service, _cq);
          tid = _request.tid();
          start_ts = _request.start_ts();
          key = _request.key();

          // TODO: Pass the parameters to user.
          // std::cout << this << " Read(" << tid << "," << start_ts << "," << key << ")" << std::endl;
          reply.set_value("10");
          reply.set_status(true);
          _status = FINISH;
          _responder.Finish(reply, Status::OK, this);
          break;
        }
        case FINISH: {
          delete this;
          break;
        }
      }
    }
  };

  class WriteHandler: private Handler
  {
  private:
    WriteRequest _request;
    ServerAsyncResponseWriter<WriteReply> _responder;
    public:
      WriteHandler(Mvtkvs::AsyncService* service, ServerCompletionQueue* cq)
          : _responder(&_ctx), Handler(service, cq) {
        proceed();
      }

    void proceed() override {
      switch (_status) {
        case CREATE:
        {
          _status = PROCESS;
          _service->RequestWrite(&_ctx, &_request, &_responder, _cq, _cq, this);
          break;
        }
        case PROCESS:
        {
          WriteReply reply;
          uint64_t tid;
          uint64_t key;
          std::string value;

          new WriteHandler(_service, _cq);
          tid = _request.tid();
          key = _request.key();
          value = _request.value();
          // TODO: Pass the parameters to user.
          // std::cout << this << " Write(" << tid << "," << key << "," << value << ")" << std::endl;
          _status = FINISH;
          reply.set_status(true);
          _responder.Finish(reply, Status::OK, this);
          break;
        }
        case FINISH:
        {
          delete this;
          break;
        }
      }
    }
  };

  class PhaseOneCommitHandler: private Handler
  {
  private:
    PhaseOneCommitRequest _request;
    ServerAsyncResponseWriter<PhaseOneCommitReply> _responder;
  public:
      PhaseOneCommitHandler(Mvtkvs::AsyncService* service, ServerCompletionQueue* cq)
          : _responder(&_ctx), Handler(service, cq) {
        proceed();
      }

    void proceed() override {
      switch (_status) {
        case CREATE:
        {
          _status = PROCESS;
          _service->RequestP1C(&_ctx, &_request, &_responder, _cq, _cq, this);
          break;
        }
        case PROCESS:
        {
          PhaseOneCommitReply reply;
          uint64_t tid;
          uint64_t start_ts;
          uint64_t commit_ts;
          std::vector<uint64_t> read_nodes;
          std::vector<uint64_t> write_nodes;

          new PhaseOneCommitHandler(_service, _cq);
          tid = _request.tid();
          start_ts = _request.start_ts();
          commit_ts = _request.commit_ts();
          for (int i = 0; i < _request.read_node_size(); i++)
            read_nodes.push_back(_request.read_node(i));
          for (int i = 0; i < _request.write_node_size(); i++)
            write_nodes.push_back(_request.write_node(i));
          // TODO: Pass the parameters to user.
          // std::cout << this << " P1C(" << tid << "," << start_ts << "," << commit_ts << ")" << std::endl;
          reply.set_vote(true);
          for (int i = 0; i < write_nodes.size(); i++)
            reply.add_node(write_nodes[i]);
          _status = FINISH;
          _responder.Finish(reply, Status::OK, this);
          break;
        }
        case FINISH:
        {
          delete this;
          break;
        }
      }
    }
  };

  class PhaseTwoCommitHandler: private Handler
  {
  private:
    PhaseTwoCommitRequest _request;
    ServerAsyncResponseWriter<PhaseTwoCommitReply> _responder;
  public:
      PhaseTwoCommitHandler(Mvtkvs::AsyncService* service, ServerCompletionQueue* cq)
          : _responder(&_ctx), Handler(service, cq) {
        proceed();
      }

    void proceed() override {
      switch (_status) {
        case CREATE:
        {
          _status = PROCESS;
          _service->RequestP2C(&_ctx, &_request, &_responder, _cq, _cq, this);
          break;
        }
        case PROCESS:
        {
          PhaseTwoCommitReply reply;
          uint64_t node;
          uint64_t tid;
          bool vote;

          new PhaseTwoCommitHandler(_service, _cq);
          tid = _request.tid();
          vote = _request.vote();
          reply.add_node(0);
          // TODO: Pass the parameters to user.
          // std::cout << this << " P2C(" << tid << "," << vote << ")" << std::endl;
          _status = FINISH;

          _responder.Finish(reply, Status::OK, this);
          break;
        }
        case FINISH:
        {
          delete this;
          break;
        }
      }
    }
  };

  public:
  MvtkvsAsyncServer()
      : _nr_threads(std::thread::hardware_concurrency()){
  }

  ~MvtkvsAsyncServer() {
    _server->Shutdown();
    _cq->Shutdown();
  }

  void run(int port, int nr_threads) {
    std::string server_address("0.0.0.0:" + std::to_string(port));
    ServerBuilder builder;

    _nr_threads = nr_threads;

    pthread_t threads[_nr_threads];

    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&_service);
    _cq = builder.AddCompletionQueue();
    _server = builder.BuildAndStart();
    std::cout << "Number of Threads: " << _nr_threads << std::endl;
    std::cout << "Server listening on " << server_address << std::endl;
    new ReadHandler(&_service, _cq.get());
    new WriteHandler(&_service, _cq.get());
    new PhaseOneCommitHandler(&_service, _cq.get());
    new PhaseTwoCommitHandler(&_service, _cq.get());
    for (int i = 0; i < _nr_threads; i++) {
      thread_args_t *thread_args = (thread_args_t *) malloc(sizeof(thread_args_t));

      thread_args->cq = _cq.get();
      thread_args->id = i;
      if (pthread_create(&threads[i], NULL, HandleRpcs, thread_args)) {
        std::cout << "Error:unable to create thread" << std::endl;
        exit(1);
      }
    }
    for (int i = 0; i < _nr_threads; i++)
      pthread_join(threads[i], NULL);
  }

  static void *HandleRpcs(void *args) {
    void *tag;
    bool ok;
    thread_args_t *thread_args = (thread_args_t *) args;
    ServerCompletionQueue *cq = thread_args->cq;
    int thread_id = thread_args->id;

    while (true) {
      cq->Next(&tag, &ok);
      assert(ok);
      static_cast<Handler*>(tag)->proceed();
    }

    pthread_exit(0);
  }
};

int main(int argc, char** argv) {
  MvtkvsAsyncServer server;

  if (argc != 3) {
    std::cerr << "Usage: ./MvtkvsAsyncServer <port no> <nr threads>" << std::endl;
    exit(1);
  }
  server.run(atoi(argv[1]), atoi(argv[2]));
  return 0;
}
