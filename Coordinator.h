/**
 * Coordinator.h
 *
 *  Created on: Jun 14, 2016
 *      Author: theo
 */

#ifndef COORDINATOR_H_
#define COORDINATOR_H_

#include <map>
#include <set>
#include "KeyMapper.h"
#include "ResolutionClient.h"
#include "TimestampGenerator.h"
#include "TransactionIDGenerator.h"

/**
 * Implementation of a transaction coordinator.
 */
class Coordinator
{
protected:
  ResolutionClient *_rsl_client;                  ///< Resolution client for this coordinator.
  KeyMapper *_key_mapper;                         ///< Mapper from keys to nodes.
  TransactionIDGenerator *_id_gen;                ///< Generator of transaction IDs.
  TimestampGenerator *_ts_gen;                    ///< Generator of timestamps.
  uint64_t _tid;                                  ///< Transaction ID.
  uint64_t _start_ts;                             ///< Starting timestamp of this transaction.
  std::set<uint64_t> _read_nodes;                 ///< Nodes for which there are only read operations.
  std::set<uint64_t> _write_nodes;                ///< Nodes which are updated.
  std::map<uint64_t, std::string> _pend_writes;   ///< Pending writes during the transaction.

public:
  Coordinator(ResolutionClient *rsl_client, KeyMapper *key_mapper, TransactionIDGenerator *id_gen,
              TimestampGenerator *ts_gen);
  virtual ~Coordinator();

protected:
  /**
   * Read operation for users.
   * @param key - Key to read from.
   * @return    - Value that correspons to this specific key.
   */
  std::string read(uint64_t key);

  /**
   * Write operation for users.
   * @param key   - Key to update.
   * @param value - Value to update.
   */
  void write(uint64_t key, std::string value);

  /**
   * Commit transaction.
   * @return  - Whether or not the commit succeeded.
   */
  bool commit();

public:
  /**
   * Run this transaction.
   * @return  - Whether or not the transaction was successful.
   */
  virtual bool run() = 0;
};

#endif /* COORDINATOR_H_ */