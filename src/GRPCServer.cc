/*
 * GRPCServer.cc
 *
 *  Created on: Jun 21, 2016
 *      Author: theo
 */

#include <chrono>
#include <thread>
#include "GRPCServer.h"

GRPCServer::RequestHandler::RequestHandler() {
  _status = CREATE;
}

GRPCServer::RequestHandler::~RequestHandler() {
  if (_status != FINISH) {
    std::cerr << "Status should have been 2 instead of " << _status << "." << std::endl;
    exit(1);
  }
}

GRPCServer::RequestHandler::RequestStatus GRPCServer::RequestHandler::getStatus() {
  return (_status);
}

GRPCServer::ReadRequestHandler::ReadRequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *queue)
    : RequestHandler(), _responder(&_ctx) {
  _rpc_read_args.read_args = (read_args_t *) malloc(sizeof(read_args_t));
  _status = PROCESS;
  service->RequestRead(&_ctx, &_request, &_responder, queue, queue, this);
}

GRPCServer::ReadRequestHandler::~ReadRequestHandler() {
  free(_rpc_read_args.read_args);
}

void GRPCServer::ReadRequestHandler::setReply() {
  ReadReply read_reply;

  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  std::cout << "Send value " << *(_rpc_read_args.value) << std::endl;
  read_reply.set_value(std::string(*(_rpc_read_args.value)));
  read_reply.set_status(_rpc_read_args.status);
  _status = FINISH;
  _responder.Finish(read_reply, Status::OK, this);
}

request_t GRPCServer::ReadRequestHandler::getRequest() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been " << PROCESS << " instead of " << _status << "." << std::endl;
    exit(1);
  }
  return (TREAD);
}

void *GRPCServer::ReadRequestHandler::getArgs() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  _rpc_read_args.read_args->tid = _request.tid();
  _rpc_read_args.read_args->start_ts = _request.start_ts();
  _rpc_read_args.read_args->key = _request.key();
  return (&_rpc_read_args);
}

GRPCServer::WriteRequestHandler::WriteRequestHandler(Mvtkvs::AsyncService *service, ServerCompletionQueue *queue)
    : RequestHandler(), _responder(&_ctx) {
  _rpc_write_args.write_args = (write_args_t *) malloc(sizeof(write_args_t));
  _rpc_write_args.write_args->value = new std::string();
  _status = PROCESS;
  service->RequestWrite(&_ctx, &_request, &_responder, queue, queue, this);
}

GRPCServer::WriteRequestHandler::~WriteRequestHandler() {
  delete _rpc_write_args.write_args->value;
  free(_rpc_write_args.write_args);
}

void GRPCServer::WriteRequestHandler::setReply() {
  WriteReply write_reply;

  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  write_reply.set_status(_rpc_write_args.status);
  _status = FINISH;
  _responder.Finish(write_reply, Status::OK, this);
}

request_t GRPCServer::WriteRequestHandler::getRequest() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  return (TWRITE);
}

void *GRPCServer::WriteRequestHandler::getArgs() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  _rpc_write_args.write_args->tid = _request.tid();
  _rpc_write_args.write_args->key = _request.key();
  _rpc_write_args.write_args->value->assign(_request.value());
  return (&_rpc_write_args);
}

GRPCServer::PhaseOneCommitRequestHandler::PhaseOneCommitRequestHandler(Mvtkvs::AsyncService *service,
    ServerCompletionQueue *queue)
    : RequestHandler(), _responder(&_ctx) {
  _rpc_p1c_args.p1c_args = (p1c_args_t *) malloc(sizeof(p1c_args_t));
  _rpc_p1c_args.p1c_args->read_nodes = new std::set<uint64_t>();
  _rpc_p1c_args.p1c_args->write_nodes = new std::set<uint64_t>();
  _rpc_p1c_args.nodes = new std::set<uint64_t>();
  _status = PROCESS;
  service->RequestP1C(&_ctx, &_request, &_responder, queue, queue, this);
}

GRPCServer::PhaseOneCommitRequestHandler::~PhaseOneCommitRequestHandler() {
  delete _rpc_p1c_args.p1c_args->read_nodes;
  delete _rpc_p1c_args.p1c_args->write_nodes;
  free(_rpc_p1c_args.p1c_args);
  delete _rpc_p1c_args.nodes;
}

void GRPCServer::PhaseOneCommitRequestHandler::setReply() {
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

request_t GRPCServer::PhaseOneCommitRequestHandler::getRequest() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  return (TP1C);
}

void *GRPCServer::PhaseOneCommitRequestHandler::getArgs() {
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
  return (&_rpc_p1c_args);
}

GRPCServer::PhaseTwoCommitRequestHandler::PhaseTwoCommitRequestHandler(Mvtkvs::AsyncService *service,
    ServerCompletionQueue *queue)
    : RequestHandler(), _responder(&_ctx) {
  _rpc_p2c_args.p2c_args = (p2c_args_t *) malloc(sizeof(p2c_args_t));
  _rpc_p2c_args.nodes = new std::set<uint64_t>();
  _status = PROCESS;
  service->RequestP2C(&_ctx, &_request, &_responder, queue, queue, this);
}

GRPCServer::PhaseTwoCommitRequestHandler::~PhaseTwoCommitRequestHandler() {
  free(_rpc_p2c_args.p2c_args);
  delete _rpc_p2c_args.nodes;
}

void GRPCServer::PhaseTwoCommitRequestHandler::setReply() {
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

request_t GRPCServer::PhaseTwoCommitRequestHandler::getRequest() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  return (TP2C);
}

void *GRPCServer::PhaseTwoCommitRequestHandler::getArgs() {
  if (_status != PROCESS) {
    std::cerr << "Status should have been 1 instead of " << _status << "." << std::endl;
    exit(1);
  }
  _rpc_p2c_args.p2c_args->tid = _request.tid();
  _rpc_p2c_args.p2c_args->vote = _request.vote();
  return (&_rpc_p2c_args);
}

GRPCServer::GRPCServer(int port)
    : RPCServer() {
  std::string server_address("0.0.0.0:" + std::to_string(port));
  ServerBuilder builder;

  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&_service);
  _queue = builder.AddCompletionQueue();
  _request_queue = new std::vector<RequestHandler *> ();
  _reply_queue = new std::vector<RequestHandler *> ();
  _server = builder.BuildAndStart();
  std::cout << "Server listening on " << server_address << std::endl;

  new ReadRequestHandler(&_service, _queue.get());
  new WriteRequestHandler(&_service, _queue.get());
  new PhaseOneCommitRequestHandler(&_service, _queue.get());
  new PhaseTwoCommitRequestHandler(&_service, _queue.get());
}

GRPCServer::~GRPCServer() {
  _queue->Shutdown();
  _request_queue->clear();
  delete _request_queue;
  _reply_queue->clear();
  delete _reply_queue;
  _server->Shutdown();
}

void GRPCServer::processSyncQueue() {
  void *tag;
  bool ok;
  RequestHandler *handler;

  _queue->Next(&tag, &ok);
  assert(ok);
  handler = static_cast<RequestHandler *>(tag);
  if (handler->getStatus() == RequestHandler::RequestStatus::PROCESS) {
    _request_queue->push_back(handler);
    prepareNextRequest(handler->getRequest());
  } else if (handler->getStatus() == RequestHandler::RequestStatus::FINISH) {
    _reply_queue->push_back(handler);
  }
}

void GRPCServer::processAsyncQueue() {
  void *tag;
  bool ok;
  RequestHandler *handler;
  grpc::CompletionQueue::NextStatus next_status;

  next_status = _queue->AsyncNext(&tag, &ok, std::chrono::system_clock::now());
  while (next_status == grpc::CompletionQueue::NextStatus::GOT_EVENT) {
    assert(ok);
    handler = static_cast<RequestHandler *>(tag);
    if (handler->getStatus() == RequestHandler::RequestStatus::PROCESS) {
      _request_queue->push_back(handler);
      prepareNextRequest(handler->getRequest());
    } else if (handler->getStatus() == RequestHandler::RequestStatus::FINISH) {
      _reply_queue->push_back(handler);
    }
    next_status = _queue->AsyncNext(&tag, &ok, std::chrono::system_clock::now());
  }
}

void GRPCServer::prepareNextRequest(request_t request) {
  switch (request) {
    case (TREAD):
      new ReadRequestHandler(&_service, _queue.get());
      break;
    case (TWRITE):
      new WriteRequestHandler(&_service, _queue.get());
      break;
    case (TP1C):
      new PhaseOneCommitRequestHandler(&_service, _queue.get());
      break;
    case (TP2C):
      new PhaseTwoCommitRequestHandler(&_service, _queue.get());
      break;
  }
}

void GRPCServer::nextRequest(uint64_t *rid, request_t *request, void **args) {
  RequestHandler *request_handler;

  while (_request_queue->empty())
    processSyncQueue();
  request_handler = *(_request_queue->begin());
  _request_queue->erase(_request_queue->begin());
  *rid = reinterpret_cast<uint64_t>(request_handler);
  *request = request_handler->getRequest();
  *args = request_handler->getArgs();
}

bool GRPCServer::asyncNextRequest(uint64_t *rid, request_t *request, void **args) {
  RequestHandler *request_handler;

  processAsyncQueue();
  if (_request_queue->empty())
    return (false);
  request_handler = *(_request_queue->begin());
  _request_queue->erase(_request_queue->begin());
  *rid = reinterpret_cast<uint64_t>(request_handler);
  *request = request_handler->getRequest();
  *args = request_handler->getArgs();
  return (true);
}

void GRPCServer::sendReply(uint64_t rid) {
  RequestHandler *request_handler = reinterpret_cast<RequestHandler *>(rid);

  request_handler->setReply();
}

void GRPCServer::nextCompletedReply(uint64_t *rid) {
  RequestHandler *reply_handler;

      processSyncQueue();
  reply_handler = *(_reply_queue->begin());
  _reply_queue->erase(_reply_queue->begin());
  *rid = reinterpret_cast<uint64_t>(reply_handler);
}

bool GRPCServer::asyncNextCompletedReply(uint64_t *rid) {
  RequestHandler *reply_handler;

  processAsyncQueue();
  if (_reply_queue->empty())
    return (false);
  reply_handler = *(_reply_queue->begin());
  _reply_queue->erase(_reply_queue->begin());
  *rid = reinterpret_cast<uint64_t>(reply_handler);
  return (true);
}

void GRPCServer::deleteRequest(uint64_t rid) {
  delete reinterpret_cast<RequestHandler *>(rid);
}
