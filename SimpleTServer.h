/**
 * SimpleTServer.h
 *
 *  Created on: Jul 13, 2016
 *      Author: theo
 */

#ifndef SIMPLETSERVER_H_
#define SIMPLETSERVER_H_

#include "TServer.h"

class SimpleTServer : public TServer
{
public:
  SimpleTServer(RPCServer *rpc_server, KeyMapper *key_mapper, uint64_t queue_size);
  ~SimpleTServer();

  ServerEvent *getEvent() override;
  void addEvent(ServerEvent *event) override;
  void sendReply(ServerEvent *event, uint64_t rid) override;
};

#endif /* SIMPLETSERVER_H_ */
