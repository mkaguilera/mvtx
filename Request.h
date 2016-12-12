/*
 * Request.h
 *
 *  Created on: Jun 9, 2016
 *      Author: theo
 */

#ifndef REQUEST_H_
#define REQUEST_H_

#include <cstdint>
#include <set>
#include <string>
#include <vector>

/**
 * Different request types.
 */
enum request_t
{
  READ, WRITE, P1C, P2C
};

/**
 * Arguments needed for read request.
 */
struct read_args_t {
  uint64_t tid;             ///> Transaction ID.
  uint64_t start_ts;        ///> Starting timestamp of transaction.
  uint64_t key;             ///> Key to read from.
};

/**
 * Arguments needed for Write RPC.
 */
struct write_args_t {
  uint64_t tid;             ///> Transaction ID.
  uint64_t key;             ///> Key to update.
  std::string *value;       ///> Value to update.
};

/**
 * Arguments needed for Phase 1 Commit RPC.
 */
struct p1c_args_t {
  uint64_t tid;                             ///> Transaction ID.
  uint64_t start_ts;                        ///> Starting timestamp of transaction.
  uint64_t commit_ts;                       ///> Commit timestamp of transaction.
  std::set<uint64_t> *read_nodes;           ///> Participating nodes from which there are read requests.
  std::set<uint64_t> *write_nodes;          ///> Participating nodes from which there are write requests (this
                                            ///  includes participants from other servers and not only for the
                                            ///  specified one (unlike nodes).
};

/**
 * Arguments needed for Phase 2 Commit RPC.
 */
struct p2c_args_t {
  uint64_t tid;                       ///> Transaction ID.
  bool vote;                          ///> Whether the transaction has beem commited or aborted.
};

/**
 * Fields needed for Reads.
 */
struct rsl_read_args_t {
  read_args_t *read_args;   ///> Arguments needed for a read request.
  std::string value;        ///> Return value.
};

/**
  * Fields needed for Reads.
  */
struct rsl_write_args_t {
  write_args_t *write_args;   ///> Arguments needed for a write request.
};

/**
 * Fields needed for Phase One Commits.
 */
struct rsl_p1c_args_t {
  p1c_args_t *p1c_args;   ///> Arguments needed for Phase One Commit request.
  bool vote;              ///> The vote for the specified nodes (COMMIT if all of them COMMIT, else ABORT).
};

/**
 * Fields needed for Phase Two Commits.
 */
struct rsl_p2c_args_t {
  p2c_args_t *p2c_args;   ///> Arguments needed for Phase Two Commit request.
};

/**
 * Arguments needed for Read RPC.
 */
struct rpc_read_args_t {
  read_args_t *read_args;   ///> Arguments needed for a read request.
  std::string value;        ///> Return value.
  bool status;              ///> Result of the request (true if the key was found).
};

/**
 * Arguments needed for Write RPC.
 */
struct rpc_write_args_t {
  write_args_t *write_args; ///> Arguments needed for a write request.
  bool status;              ///> Result of the request (true if the key was found).
};

/**
 * Arguments needed for Phase 1 Commit RPC.
 */
struct rpc_p1c_args_t {
  p1c_args_t *p1c_args;         ///> Arguments needed for Phase One Commit request.
  std::set<uint64_t> *nodes;    ///> Nodes that were associated with this transaction in the server.
  bool vote;                    ///> The vote for the specified nodes (COMMIT if all of them COMMIT, else ABORT).
};

/**
 * Arguments needed for Phase 2 Commit RPC.
 */
struct rpc_p2c_args_t {
  p2c_args_t *p2c_args;         ///> Arguments needed for Phase Two Commit request.
  std::set<uint64_t> *nodes;    ///> Nodes that were associated with this transaction in the server.
};

#endif /* REQUEST_H_ */
