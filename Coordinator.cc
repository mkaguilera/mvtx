/**
 * Coordinator.cc
 *
 *  Created on: Jun 15, 2016
 *      Author: theo
 */

#include "Coordinator.h"

Coordinator::Coordinator(ResolutionClient *rsl_client, KeyMapper *key_mapper, TransactionIDGenerator *id_gen,
    TimestampGenerator *ts_gen)
    : _rsl_client(rsl_client), _key_mapper(key_mapper), _id_gen(id_gen), _ts_gen(ts_gen) {
  _tid = _id_gen->genTransactionID();
  _start_ts = _ts_gen->genStartTimestamp();
}

Coordinator::~Coordinator() {
  _read_nodes.clear();
  _write_nodes.clear();
  _pend_writes.clear();
}

std::string Coordinator::read(uint64_t key) {
  std::set<uint64_t> nodes;
  rsl_read_args_t rsl_read_args;
  read_args_t read_args;
  uint64_t node;

  if (_pend_writes.find(key) != _pend_writes.end())
    return _pend_writes[key];
  node = _key_mapper->getNode(key);
  nodes.insert(node);
  read_args.tid = _tid;
  read_args.start_ts = _start_ts;
  read_args.key = key;
  rsl_read_args.read_args = &read_args;
  _rsl_client->request(nodes, READ, &rsl_read_args);
  _read_nodes.insert(node);
  return rsl_read_args.value;
}

void Coordinator::write(uint64_t key, std::string value) {
  std::set<uint64_t> nodes;
  rsl_write_args_t rsl_write_args;
  write_args_t write_args;
  uint64_t node = _key_mapper->getNode(key);

  _pend_writes[key] = value;
  nodes.insert(node);
  write_args.tid = _tid;
  write_args.key = key;
  write_args.value = &value;
  rsl_write_args.write_args = &write_args;
  _rsl_client->request(nodes, WRITE, &rsl_write_args);
  _write_nodes.insert(node);
}

bool Coordinator::commit() {
  std::set<uint64_t> nodes = _write_nodes;
  rsl_p1c_args_t rsl_p1c_args;
  rsl_p2c_args_t rsl_p2c_args;
  p1c_args_t p1c_args;
  p2c_args_t p2c_args;
  bool vote;

  p1c_args.tid = _tid;
  p1c_args.start_ts = _start_ts;
  p1c_args.commit_ts = _ts_gen->genCommitTimestamp();
  p1c_args.read_nodes = &_read_nodes;
  p1c_args.write_nodes = &_write_nodes;
  rsl_p1c_args.p1c_args = &p1c_args;
  _rsl_client->request(nodes, P1C, &rsl_p1c_args);
  vote = rsl_p1c_args.vote;
  nodes.insert(_read_nodes.begin(), _read_nodes.end());
  p2c_args.tid = _tid;
  p2c_args.vote = vote;
  rsl_p2c_args.p2c_args = &p2c_args;
  _rsl_client->request(nodes, P2C, &rsl_p2c_args);
  return vote;
}