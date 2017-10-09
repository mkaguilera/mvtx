/*
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
private:
  std::map<uint64_t, std::map<uint64_t,std::string> *> _store;
  std::map<uint64_t, std::vector<std::pair<uint64_t, std::string *>> *> _pend_writes;
  std::map<uint64_t, uint64_t> _rdy_to_commit;

public:
  SimpleTServer(RPCServer *rpc_server, KeyMapper *key_mapper, LockManager *lock_manager, uint64_t queue_size);
  ~SimpleTServer();

  ServerEvent *getEvent() override;
  void addEvent(ServerEvent *event) override;
  void sendReply(ServerEvent *event, uint64_t rid) override;
  std::string *get(uint64_t key, uint64_t ts) override;
  void set(uint64_t key, uint64_t ts, std::string *value) override;
  void add(uint64_t tid, uint64_t key, std::string *value) override;
  std::vector<uint64_t> *getKeys(uint64_t tid) override;
  void prepare(uint64_t tid, uint64_t ts) override;
  void finalize(uint64_t tid, bool success) override;
};

#endif /* SIMPLETSERVER_H_ */
