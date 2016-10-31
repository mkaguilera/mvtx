/**
 * GRPCServer.cc
 *
 *  Created on: Jun 21, 2016
 *      Author: theo
 */

#include <chrono>
#include <thread>
#include "GRPCServer.h"

GRPCServer::RequestHandler::RequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
                                           ServerCompletionQueue *reply_queue)
    : _service(service), _request_queue(request_queue), _reply_queue(reply_queue), _status(CREATE) {}

GRPCServer::RequestHandler::~RequestHandler() {
  if (_status != FINISH) {
    std::cerr << "Status should have been 2 instead of " << _status << "." << std::endl;
    exit(1);
  }
}

GRPCServer::ReadHandler::ReadHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
                                     ServerCompletionQueue *reply_queue)
    : RequestHandler(service, request_queue, reply_queue), _responder(&_ctx) {
  _rpc_read_args.read_args = (read_args_t *) malloc(sizeof(read_args_t));
  _status = PROCESS;
  _service->RequestRead(&_ctx, &_request, &_responder, reply_queue, request_queue, this);
}

GRPCServer::ReadHandler::~ReadHandler() {
  free(_rpc_read_args.read_args);
}

void GRPCServer::ReadHandler::setReply() {
  ReadReply read_reply;

  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  read_reply.set_value(_rpc_read_args.value);
  read_reply.set_status(_rpc_read_args.status);
  _status = FINISH;
  _responder.Finish(read_reply, Status::OK, this);
}

request_t GRPCServer::ReadHandler::getRequest() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been " << PROCESS << " instead of " << _status << "." << std::endl;
    exit(1);
  }
  return READ;
}

void *GRPCServer::ReadHandler::getArgs() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  _rpc_read_args.read_args->tid = _request.tid();
  _rpc_read_args.read_args->start_ts = _request.start_ts();
  _rpc_read_args.read_args->key = _request.key();
  return &_rpc_read_args;
}

GRPCServer::WriteHandler::WriteHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *request_queue,
                                       ServerCompletionQueue *reply_queue)
    : RequestHandler(service, request_queue, reply_queue), _responder(&_ctx) {
  _rpc_write_args.write_args = (write_args_t *) malloc(sizeof(write_args_t));
  _rpc_write_args.write_args->value = new std::string();
  _status = PROCESS;
  _service->RequestWrite(&_ctx, &_request, &_responder, reply_queue, request_queue, this);
}

GRPCServer::WriteHandler::~WriteHandler() {
  delete _rpc_write_args.write_args->value;
  free(_rpc_write_args.write_args);
}

void GRPCServer::WriteHandler::setReply() {
  WriteReply write_reply;

  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  write_reply.set_status(_rpc_write_args.status);
  _status = FINISH;
  _responder.Finish(write_reply, Status::OK, this);
}

request_t GRPCServer::WriteHandler::getRequest() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  return WRITE;
}

void *GRPCServer::WriteHandler::getArgs() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  _rpc_write_args.write_args->tid = _request.tid();
  _rpc_write_args.write_args->key = _request.key();
  _rpc_write_args.write_args->value->assign(_request.value());
  return &_rpc_write_args;
}

GRPCServer::PhaseOneCommitHandler::PhaseOneCommitHandler(Mvtkvs::AsyncService *service,
                                                         ServerCompletionQueue *request_queue,
                                                         ServerCompletionQueue *reply_queue)
    : RequestHandler(service, request_queue, reply_queue), _responder(&_ctx) {
  _rpc_p1c_args.p1c_args = (p1c_args_t *) malloc(sizeof(p1c_args_t));
  _rpc_p1c_args.p1c_args->read_nodes = new std::set<uint64_t> ();
  _rpc_p1c_args.p1c_args->write_nodes = new std::set<uint64_t> ();
  _rpc_p1c_args.nodes = new std::set<uint64_t> ();
  _status = PROCESS;
  _service->RequestP1C(&_ctx, &_request, &_responder, reply_queue, request_queue, this);
}

GRPCServer::PhaseOneCommitHandler::~PhaseOneCommitHandler() {
  delete _rpc_p1c_args.p1c_args->read_nodes;
  delete _rpc_p1c_args.p1c_args->write_nodes;
  free(_rpc_p1c_args.p1c_args);
  delete _rpc_p1c_args.nodes;
}

void GRPCServer::PhaseOneCommitHandler::setReply() {
  PhaseOneCommitReply p1c_reply;

  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  for (std::set<uint64_t>::iterator it = _rpc_p1c_args.nodes->begin(); it != _rpc_p1c_args.nodes->end(); ++it)
    p1c_reply.add_node(*it);
  p1c_reply.set_vote(_rpc_p1c_args.vote);
  _status = FINISH;
  _responder.Finish(p1c_reply, Status::OK, this);
}

request_t GRPCServer::PhaseOneCommitHandler::getRequest() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  return P1C;
}

void *GRPCServer::PhaseOneCommitHandler::getArgs() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  _rpc_p1c_args.p1c_args->tid = _request.tid();
  _rpc_p1c_args.p1c_args->start_ts = _request.start_ts();
  _rpc_p1c_args.p1c_args->commit_ts = _request.commit_ts();
  for (int i = 0; i < _request.read_node_size(); i++)
    _rpc_p1c_args.p1c_args->read_nodes->insert(_request.read_node(i));
  for (int i = 0; i < _request.write_node_size(); i++)
    _rpc_p1c_args.p1c_args->write_nodes->insert(_request.write_node(i));
  return &_rpc_p1c_args;
}

GRPCServer::PhaseTwoCommitHandler::PhaseTwoCommitHandler(Mvtkvs::AsyncService *service,
                                                         ServerCompletionQueue *request_queue,
                                                         ServerCompletionQueue *reply_queue)
    : RequestHandler(service, request_queue, reply_queue), _responder(&_ctx) {
  _rpc_p2c_args.p2c_args = (p2c_args_t *) malloc(sizeof(p2c_args_t));
  _rpc_p2c_args.nodes = new std::set<uint64_t> ();
  _status = PROCESS;
  _service->RequestP2C(&_ctx, &_request, &_responder, reply_queue, request_queue, this);
}

GRPCServer::PhaseTwoCommitHandler::~PhaseTwoCommitHandler() {
  free(_rpc_p2c_args.p2c_args);
  delete _rpc_p2c_args.nodes;
}

void GRPCServer::PhaseTwoCommitHandler::setReply() {
  PhaseTwoCommitReply p2c_reply;

  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  for (std::set<uint64_t>::iterator it = _rpc_p2c_args.nodes->begin(); it != _rpc_p2c_args.nodes->end(); ++it)
    p2c_reply.add_node(*it);
  _status = FINISH;
  _responder.Finish(p2c_reply, Status::OK, this);
}

request_t GRPCServer::PhaseTwoCommitHandler::getRequest() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  return P2C;
}

void *GRPCServer::PhaseTwoCommitHandler::getArgs() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  _rpc_p2c_args.p2c_args->tid = _request.tid();
  _rpc_p2c_args.p2c_args->vote = _request.vote();
  return &_rpc_p2c_args;
}

GRPCServer::GRPCServer(int port)
    : RPCServer() {
  std::string server_address("0.0.0.0:" + std::to_string(port));
  ServerBuilder builder;

  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&_service);
  _request_queue = builder.AddCompletionQueue();
  _reply_queue = builder.AddCompletionQueue();
  _server = builder.BuildAndStart();
  std::cout << "Server listening on " << server_address << std::endl;

  new ReadHandler(&_service, _request_queue.get(), _reply_queue.get());
  new WriteHandler(&_service, _request_queue.get(), _reply_queue.get());
  new PhaseOneCommitHandler(&_service, _request_queue.get(), _reply_queue.get());
  new PhaseTwoCommitHandler(&_service, _request_queue.get(), _reply_queue.get());
}

GRPCServer::~GRPCServer() {
  _server->Shutdown();
  _request_queue->Shutdown();
  _reply_queue->Shutdown();
}

void GRPCServer::nextRequest(uint64_t *rid, request_t *request, void **args) {
  void *tag;
  bool ok;
  RequestHandler *request_handler;

  _request_queue->Next(&tag, &ok);
  assert(ok);
  request_handler = static_cast<RequestHandler *>(tag);
  *rid = reinterpret_cast<uint64_t>(tag);
  *request = request_handler->getRequest();
  *args = request_handler->getArgs();
  prepareNextRequest(*request);
}

bool GRPCServer::asyncNextRequest(uint64_t *rid, request_t *request, void **args) {
  void *tag;
  bool ok;
  RequestHandler *request_handler;
  grpc::CompletionQueue::NextStatus next_status =
      _request_queue->AsyncNext(&tag, &ok, std::chrono::system_clock::now());

  if (next_status != grpc::CompletionQueue::NextStatus::GOT_EVENT)
    return false;

  request_handler = static_cast<RequestHandler *>(tag);
  *rid = reinterpret_cast<uint64_t>(request_handler);
  *request = request_handler->getRequest();
  *args = request_handler->getArgs();
  prepareNextRequest(*request);
  return true;
}

void GRPCServer::sendReply(uint64_t rid) {
  RequestHandler *request_handler = reinterpret_cast<RequestHandler *> (rid);

  request_handler->setReply();
}

void GRPCServer::nextCompletedReply(uint64_t *rid) {
  void *tag;
  bool ok;

  _reply_queue->Next(&tag, &ok);
  assert(ok);
  *rid= reinterpret_cast<uint64_t> (tag);
}

bool GRPCServer::asyncNextCompletedReply(uint64_t *rid) {
  void *tag;
  bool ok;
  grpc::CompletionQueue::NextStatus next_status = _reply_queue->AsyncNext(&tag, &ok, std::chrono::system_clock::now());

  if (next_status != grpc::CompletionQueue::NextStatus::GOT_EVENT)
      return false;
  *rid = reinterpret_cast<uint64_t> (tag);
  return true;
}

void GRPCServer::deleteRequest(uint64_t rid) {
  delete reinterpret_cast<RequestHandler *> (rid);
}

void GRPCServer::prepareNextRequest(request_t request) {
  switch (request) {
    case (READ):
      new ReadHandler(&_service, _request_queue.get(), _reply_queue.get());
      break;
    case (WRITE):
      new WriteHandler(&_service, _request_queue.get(), _reply_queue.get());
      break;
    case (P1C):
      new PhaseOneCommitHandler(&_service, _request_queue.get(), _reply_queue.get());
      break;
    case (P2C):
      new PhaseTwoCommitHandler(&_service, _request_queue.get(), _reply_queue.get());
      break;
  }
}

/*
static void *HandleRpcs(void *args) {
  thread_args_t *thread_args = (thread_args_t *) args;
  GRPCServer *grpc_server = thread_args->grpc_server;

  free(thread_args);
  while (true) {
    uint64_t rid1 = 0;
    uint64_t rid2 = 0;
    request_t request;
    void *args;

    while ((!grpc_server->asyncNextRequest(&rid1, &request, &args)) && (!grpc_server->asyncNextCompletedReply(&rid2)));
    if (rid1 != 0) {
      assert (rid2 == 0);
      switch (request) {
        case READ: {
          rpc_read_args_t *rpc_read_args = (rpc_read_args_t *) args;

          rpc_read_args->value = "100";
          rpc_read_args->status = true;
          break;
        }
        case WRITE: {
          rpc_write_args_t *rpc_write_args = (rpc_write_args_t *) args;

          rpc_write_args->status = true;
          break;
        }
        case P1C: {
          rpc_p1c_args_t *rpc_p1c_args = (rpc_p1c_args_t *) args;

          rpc_p1c_args->nodes->insert(0);
          rpc_p1c_args->vote = true;
          break;
        }
        case P2C: {
          rpc_p2c_args_t *rpc_p2c_args = (rpc_p2c_args_t *) args;

          rpc_p2c_args->nodes->insert(0);
          break;
        }
      }
      grpc_server->sendReply(rid1);
    } else {
      assert (rid1 == 0);
      grpc_server->deleteRequest(rid2);
    }
  }
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <port_num> <nr_threads>" << std::endl;
    exit(1);
  }

  GRPCServer grpc_server(atoi(argv[1]));
  int nr_threads = atoi(argv[2]);
  pthread_t threads[nr_threads];

  for (int i = 0; i < nr_threads; i++) {
    thread_args_t *thread_args = (thread_args_t *) malloc(sizeof(thread_args_t));

    thread_args->id = i;
    thread_args->grpc_server = &grpc_server;
    if (pthread_create(&threads[i], NULL, HandleRpcs, thread_args)) {
       std::cout << "Error:unable to create thread" << std::endl;
       free(thread_args);
       exit(1);
    }
  }

  for (int i = 0; i < nr_threads; i++)
    pthread_join(threads[i], NULL);
}
*/
