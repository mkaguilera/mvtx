/**
 * GRPCClient.cc
 *
 *  Created on: Jun 7, 2016
 *      Author: theo
 */

#include <assert.h>
#include "GRPCClient.h"

GRPCClient::RequestHandler::RequestHandler(GRPCClient *grpc_client, const std::string &addr)
    : _grpc_client(grpc_client), _addr(addr), _rstatus(INIT) {
}

GRPCClient::RequestHandler::~RequestHandler() {
  _cq.Shutdown();
}

Status GRPCClient::RequestHandler::getStatus() {
  return _status;
}

GRPCClient::ReadRequestHandler::ReadRequestHandler(GRPCClient *grpc_client, const std::string &addr,
                                                   rpc_read_args_t *rpc_read_args)
    : GRPCClient::RequestHandler::RequestHandler(grpc_client, addr), _rpc_read_args(rpc_read_args),
      _reply_reader(_grpc_client->getStub(_addr)->
          AsyncRead(&_ctx, GRPCClient::makeReadRequest(_rpc_read_args->read_args), &_cq)) {
}

void GRPCClient::ReadRequestHandler::proceed() {
  switch (_rstatus) {
    case INIT:
    {
      _rstatus = WAIT;
      _reply_reader->Finish(&_reply, &_status, (void *) 1);
      break;
    }
    case WAIT:
    {
      void *tag;
      bool ok;

      _cq.Next(&tag, &ok);

      // assert (tag == (void *) 1);
      // assert (ok);

      if (ok) {
        _rpc_read_args->status = _reply.status();
        if (_rpc_read_args->status)
          _rpc_read_args->value = _reply.value();
      }
      break;
    }
  }
}

GRPCClient::WriteRequestHandler::WriteRequestHandler(GRPCClient *grpc_client, const std::string &addr,
                                                     rpc_write_args_t *rpc_write_args)
    : GRPCClient::RequestHandler::RequestHandler(grpc_client, addr), _rpc_write_args(rpc_write_args),
      _reply_reader(_grpc_client->getStub(_addr)->
          AsyncWrite(&_ctx, GRPCClient::makeWriteRequest(_rpc_write_args->write_args), &_cq)) {
}

void GRPCClient::WriteRequestHandler::proceed() {
  switch (_rstatus) {
    case INIT:
    {
      _rstatus = WAIT;
      _reply_reader->Finish(&_reply, &_status, (void *) 1);
      break;
    }
    case WAIT:
    {
      void *tag;
      bool ok;

      _cq.Next(&tag, &ok);

      // assert (tag == (void *) 1);
      // assert (ok);

      if (ok)
        _rpc_write_args->status = _reply.status();
      break;
    }
  }
}

GRPCClient::PhaseOneCommitRequestHandler::PhaseOneCommitRequestHandler(GRPCClient *grpc_client, const std::string &addr,
                                                                       rpc_p1c_args_t *rpc_p1c_args)
    : GRPCClient::RequestHandler::RequestHandler(grpc_client, addr), _rpc_p1c_args(rpc_p1c_args),
      _reply_reader(_grpc_client->getStub(_addr)->
          AsyncP1C(&_ctx, GRPCClient::makePhaseOneCommitRequest(_rpc_p1c_args->p1c_args), &_cq)) {
}

void GRPCClient::PhaseOneCommitRequestHandler::proceed() {
  switch (_rstatus) {
    case INIT:
    {
      _rstatus = WAIT;
      _reply_reader->Finish(&_reply, &_status, (void *) 1);
      break;
    }
    case WAIT:
    {
      void *tag;
      bool ok;

      _cq.Next(&tag, &ok);

      // assert (tag == (void *) 1);
      // assert (ok);

      if (ok) {
        for (int i = 0; i < _reply.node_size(); i++)
          _rpc_p1c_args->nodes->insert(_reply.node(i));
        _rpc_p1c_args->vote = _reply.vote();
      }
      break;
    }
  }
}

GRPCClient::PhaseTwoCommitRequestHandler::PhaseTwoCommitRequestHandler(GRPCClient *grpc_client, const std::string &addr,
                                                                       rpc_p2c_args_t *rpc_p2c_args)
    : GRPCClient::RequestHandler::RequestHandler(grpc_client, addr), _rpc_p2c_args(rpc_p2c_args),
      _reply_reader(_grpc_client->getStub(_addr)->
          AsyncP2C(&_ctx, GRPCClient::makePhaseTwoCommitRequest(_rpc_p2c_args->p2c_args), &_cq)) {
}

void GRPCClient::PhaseTwoCommitRequestHandler::proceed() {
  switch (_rstatus) {
    case INIT:
    {
      _rstatus = WAIT;
      _reply_reader->Finish(&_reply, &_status, (void *) 1);
      break;
    }
    case WAIT:
    {
      void *tag;
      bool ok;

      _cq.Next(&tag, &ok);

      // assert (tag == (void *) 1);
      // assert (ok);

      if (ok)
        for (int i = 0; i < _reply.node_size(); i++)
          _rpc_p2c_args->nodes->insert(_reply.node(i));
      break;
    }
  }
}

GRPCClient::~GRPCClient () {
  _address_to_stub.clear();
  _tag_to_handler.clear();
}

void GRPCClient::makeStub(const std::string &addr) {
  std::unique_lock<std::mutex> lock(_mutex1);

  if (_address_to_stub.find(addr) == _address_to_stub.end())
    _address_to_stub[addr] = Mvtkvs::NewStub(grpc::CreateChannel(addr, grpc::InsecureChannelCredentials()));
}

Mvtkvs::Stub *GRPCClient::getStub(std::string addr) {
  std::unique_lock<std::mutex> lock(_mutex1);

  return _address_to_stub[addr].get();
}

ReadRequest GRPCClient::makeReadRequest(const read_args_t *read_args) {
  ReadRequest res;

  res.set_tid(read_args->tid);
  res.set_start_ts(read_args->start_ts);
  res.set_key(read_args->key);
  return res;
}

WriteRequest GRPCClient::makeWriteRequest(const write_args_t *write_args) {
  WriteRequest res;

  res.set_tid(write_args->tid);
  res.set_key(write_args->key);
  res.set_value(*(write_args->value));
  return res;
}

PhaseOneCommitRequest GRPCClient::makePhaseOneCommitRequest(const p1c_args_t *p1c_args) {
  PhaseOneCommitRequest res;

  res.set_tid(p1c_args->tid);
  res.set_start_ts(p1c_args->start_ts);
  res.set_commit_ts(p1c_args->commit_ts);
  for (std::set<uint64_t>::iterator it = p1c_args->read_nodes->begin(); it != p1c_args->read_nodes->end(); ++it)
    res.add_read_node(*it);
  for (std::set<uint64_t>::iterator it = p1c_args->write_nodes->begin(); it != p1c_args->write_nodes->end(); ++it)
    res.add_write_node(*it);
  return res;
}

PhaseTwoCommitRequest GRPCClient::makePhaseTwoCommitRequest(const p2c_args_t *p2c_args) {
  PhaseTwoCommitRequest res;

  res.set_tid(p2c_args->tid);
  res.set_vote(p2c_args->vote);
  return res;
}

bool GRPCClient::syncRPC(const std::string &addr, request_t request, void *args) {
  ClientContext ctx;
  Status status;

  makeStub(addr);
  switch (request) {
    case (READ):
    {
      rpc_read_args_t *rpc_read_args = (rpc_read_args_t *) args;
      ReadRequest request = GRPCClient::makeReadRequest(rpc_read_args->read_args);
      ReadReply reply;

      _mutex1.lock();
      status = _address_to_stub[addr]->Read(&ctx, request, &reply);
      _mutex1.unlock();
      rpc_read_args->status = status.ok() && reply.status();
      if (rpc_read_args->status)
        rpc_read_args->value = reply.value();
      break;
    }
    case (WRITE):
    {
      rpc_write_args_t *rpc_write_args = (rpc_write_args_t *) args;
      WriteRequest request = GRPCClient::makeWriteRequest(rpc_write_args->write_args);
      WriteReply reply;

      _mutex1.lock();
      status = _address_to_stub[addr]->Write(&ctx, request, &reply);
      _mutex1.unlock();
      rpc_write_args->status = status.ok() && reply.status();
      break;
    }
    case (P1C):
    {
      rpc_p1c_args_t *rpc_p1c_args = (rpc_p1c_args_t *) args;
      PhaseOneCommitRequest request = GRPCClient::makePhaseOneCommitRequest(rpc_p1c_args->p1c_args);
      PhaseOneCommitReply reply;

      _mutex1.lock();
      status = _address_to_stub[addr]->P1C(&ctx, request, &reply);
      _mutex1.unlock();
      if (status.ok()) {
        for (int i = 0; i < reply.node_size(); i++)
          rpc_p1c_args->nodes->insert(reply.node(i));
        rpc_p1c_args->vote = reply.vote();
      }
      break;
    }
    case (P2C):
    {
      rpc_p2c_args_t *rpc_p2c_args = (rpc_p2c_args_t *) args;
      PhaseTwoCommitRequest request = GRPCClient::makePhaseTwoCommitRequest(rpc_p2c_args->p2c_args);
      PhaseTwoCommitReply reply;

      _mutex1.lock();
      status = _address_to_stub[addr]->P2C(&ctx, request, &reply);
      _mutex1.unlock();
      if (status.ok()) {
        for (int i = 0; i < reply.node_size(); i++)
          rpc_p2c_args->nodes->insert(reply.node(i));
      }
      break;
    }
  }

  return status.ok();
}

void GRPCClient::asyncRPC(const std::string &addr, uint64_t tag, request_t request, void *args) {
  GRPCClient::RequestHandler *handler = NULL;

  switch(request) {
    case (READ):
    {
      handler = new ReadRequestHandler(this, addr, (rpc_read_args_t *) args);
      break;
    }
    case (WRITE):
    {
      handler = new WriteRequestHandler(this, addr, (rpc_write_args_t *) args);
      break;
    }
    case (P1C):
    {
      handler = new PhaseOneCommitRequestHandler(this, addr, (rpc_p1c_args_t *) args);
      break;
    }
    case (P2C):
    {
      handler = new PhaseTwoCommitRequestHandler(this, addr, (rpc_p2c_args_t *) args);
      break;
    }
  }
  handler->proceed();
  _mutex2.lock();
  _tag_to_handler[tag] = handler;
  _mutex2.unlock();
}

bool GRPCClient::waitAsyncReply(uint64_t tag) {
  bool res;
  std::unique_lock<std::mutex> lock(_mutex2);

  _tag_to_handler[tag]->proceed();
  res = _tag_to_handler[tag]->getStatus().ok();
  delete _tag_to_handler[tag];
  _tag_to_handler.erase(tag);
  return res;
}