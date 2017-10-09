/*
 * ServerEvent.h
 *
 *  Created on: Jul 13, 2016
 *      Author: theo
 */

#ifndef SERVEREVENT_H_
#define SERVEREVENT_H_

#include <cstdint>
#include "Event.h"
#include "Request.h"

class TServer;

/**
 * Class that instantiates all tasks/events that need to be executed in the server side.
 */
class ServerEvent : public Event
{
protected:
  ///> Server instance that runs the events.
  TServer *_tserver;
  ///> Request ID provided by RPC Server.
  uint64_t _rid;
  ///> Phase in which the event currently is.
  uint64_t _phase;

public:
  /**
   * Constructor of ServerEvent.
   * @param tserver - Server instance.
   * @param rid     - Request ID for which this event is based on.
   */
  ServerEvent(TServer *tserver, uint64_t rid);

  /**
   * Destructor of ServerEvent.
   */
  virtual ~ServerEvent();

  /**
   * Run this event.
   */
  virtual void run() = 0;
};

/**
 * Event for reads.
 */
class ReadEvent: public ServerEvent
{
private:
  ///> Arguments provided by coordinators for reads.
  rpc_read_args_t *_rpc_read_args;

public:
  /**
   * Constructor of ReadEvent.
   * @param tserver       - Server instance.
   * @param rid           - Request ID for which this event is based on.
   * @param rpc_read_args - Arguments provided by coordinators.
   */
  ReadEvent(TServer *tserver, uint64_t rid, rpc_read_args_t *rpc_read_args);

  void run() override;
};

/**
 * Event for writes.
 */
class WriteEvent: public ServerEvent
{
private:
  ///> Arguments provided by coordinators for writes.
  rpc_write_args_t *_rpc_write_args;

public:
  /**
   * Constructor of ReadEvent.
   * @param tserver         - Server instance.
   * @param rid             - Request ID for which this event is based on.
   * @param rpc_write_args  - Arguments provided by coordinators.
   */
  WriteEvent(TServer *tserver, uint64_t rid, rpc_write_args_t *rpc_write_args);

  void run() override;
};

/**
 * Event for phase one commit.
 */
class P1CEvent: public ServerEvent
{
private:
  ///> Arguments provided by coordinators for phase one commits.
  rpc_p1c_args_t *_rpc_p1c_args;

public:
  /**
   * Constructor of P1CEvent.
   * @param tserver       - Server instance.
   * @param rid           - Request ID for which this event is based on.
   * @param rpc_p1c_args  - Arguments provided by coordinators.
   */
  P1CEvent(TServer *tserver, uint64_t rid, rpc_p1c_args_t *rpc_p1c_args);

  void run() override;
};

/**
 * Event for phase two commit.
 */
class P2CEvent: public ServerEvent
{
private:
  ///> Arguments provided by coordinators for phase two commits.
  rpc_p2c_args_t *_rpc_p2c_args;

public:
  /**
   * Constructor of P2CEvent.
   * @param tserver       - Server instance.
   * @param rid           - Request ID for which this event is based on.
   * @param rpc_p2c_args  - Arguments provided by coordinators.
   */
  P2CEvent(TServer *tserver, uint64_t rid, rpc_p2c_args_t *rpc_p2c_args);

  void run() override;
};

#endif /* SERVEREVENT_H_ */
