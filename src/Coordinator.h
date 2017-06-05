/*
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
 * Coordinator implementation for transactional systems. A Coordinator should be responsible for running one associated
 * transaction. It makes requests to Servers and waits for the replies.
 */
class Coordinator
{
  protected:
    ///> Resolution client for this coordinator.
    ResolutionClient *_rsl_client;
    ///> Map from keys to nodes.
    KeyMapper *_key_mapper;
    ///> Generator of timestamps.
    TimestampGenerator *_ts_gen;
    ///> Transaction ID.
    uint64_t _tid;
    ///> Starting timestamp of this transaction.
    uint64_t _start_ts;
    ///> Nodes for which there are only read operations.
    std::set<uint64_t> _read_nodes;
    ///> Nodes which are updated.
    std::set<uint64_t> _write_nodes;
    ///> Pending writes during the transaction.
    std::map<uint64_t, std::string *> _pend_writes;

    /**
     * Constructor of Coordinator.
     * @param rsl_client  - Bottom layer that decides in which physical nodes the requests should go.
     * @param key_mapper  - Map between keys and partitions.
     * @param id_gen      - Transaction ID generator.
     * @param ts_gen      - Timestamp generator.
     */
    Coordinator(ResolutionClient *rsl_client, KeyMapper *key_mapper, TransactionIDGenerator *id_gen,
        TimestampGenerator *ts_gen);

  public:
    /**
     * Destructor of Coordinator.
     */
    virtual ~Coordinator();

  protected:
    /**
     * Read operation for a specific key.
     * @param key - Key to read from.
     * @return    - Value that corresponds to this specific key.
     */
    std::string *read(uint64_t key);

    /**
     * Write operation that updates the value of a key.
     * @param key   - Key to update.
     * @param value - Value to update.
     */
    void write(uint64_t key, std::string *value);

    /**
     * Commit transaction.
     * @return  - Whether or not the transaction committed or aborted.
     */
    bool commit();

  public:
    /**
     * Run this transaction.
     * @return  - Whether or not the transaction committed or aborted.
     */
    virtual bool run() = 0;
};

#endif /* COORDINATOR_H_ */
