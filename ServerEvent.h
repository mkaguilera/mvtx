/**
 * ServerEvent.h
 *
 *  Created on: Jul 13, 2016
 *      Author: theo
 */

#ifndef SERVEREVENT_H_
#define SERVEREVENT_H_

#include <stdint.h>
#include "Request.h"

class TServer;

class ServerEvent
{
protected:
  TServer *_tserver;
  uint64_t _rid;
  uint64_t _phase;

public:
  ServerEvent(TServer *tserver, uint64_t rid);
  virtual ~ServerEvent();

  virtual void run() = 0;
};

class ReadEvent: public ServerEvent
{
private:
  rpc_read_args_t *_rpc_read_args;

public:
  ReadEvent(TServer *tserver, uint64_t rid, rpc_read_args_t *rpc_read_args);

  void run() override;
};

class WriteEvent: public ServerEvent
{
private:
  rpc_write_args_t *_rpc_write_args;

public:
  WriteEvent(TServer *tserver, uint64_t rid, rpc_write_args_t *rpc_write_args);

  void run() override;
};

class P1CEvent: public ServerEvent
{
private:
  rpc_p1c_args_t *_rpc_p1c_args;

public:
  P1CEvent(TServer *tserver, uint64_t rid, rpc_p1c_args_t *rpc_p1c_args);

  void run() override;
};

class P2CEvent: public ServerEvent
{
private:
  rpc_p2c_args_t *_rpc_p2c_args;

public:
  P2CEvent(TServer *tserver, uint64_t rid, rpc_p2c_args_t *rpc_p2c_args);

  void run() override;
};

#endif /* SERVEREVENT_H_ */