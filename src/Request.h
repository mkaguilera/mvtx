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
  TREAD, TWRITE, TP1C, TP2C
};

/**
 * Arguments needed for READ request.
 */
struct read_args_t
{
    ///> Transaction ID.
    uint64_t tid;
    ///> Starting timestamp of transaction.
    uint64_t start_ts;
    ///> Key to read from.
    uint64_t key;
};

/**
 * Arguments needed for Write RPC.
 */
struct write_args_t
{
    ///> Transaction ID.
    uint64_t tid;
    ///> Key to update.
    uint64_t key;
    ///> Value to update.
    std::string *value;
};

/**
 * Arguments needed for Phase 1 Commit RPC.
 */
struct p1c_args_t
{
    ///> Transaction ID.
    uint64_t tid;
    ///> Starting timestamp of transaction.
    uint64_t start_ts;
    ///> Commit timestamp of transaction.
    uint64_t commit_ts;
    ///> Participating nodes from which there are read requests.
    std::set<uint64_t> *read_nodes;
    ///> Participating nodes from which there are write requests (this includes participants from other servers and not
    ///  only for the specified one (unlike nodes).
    std::set<uint64_t> *write_nodes;
};

/**
 * Arguments needed for Phase 2 Commit RPC.
 */
struct p2c_args_t
{
    ///> Transaction ID.
    uint64_t tid;
    ///> Whether the transaction has been committed or aborted.
    bool vote;
};

/**
 * Fields needed for Reads.
 */
struct rsl_read_args_t
{
    ///> Arguments needed for a read request.
    read_args_t *read_args;
    ///> Return value.
    std::string *value;
};

/**
 * Fields needed for Writes.
 */
struct rsl_write_args_t
{
    ///> Arguments needed for a write request.
    write_args_t *write_args;
};

/**
 * Fields needed for Phase One Commits.
 */
struct rsl_p1c_args_t
{
    ///> Arguments needed for Phase One Commit request.
    p1c_args_t *p1c_args;
    ///> The vote for the specified nodes (COMMIT if all of them COMMIT, else ABORT).
    bool vote;
};

/**
 * Fields needed for Phase Two Commits.
 */
struct rsl_p2c_args_t
{
    ///> Arguments needed for Phase Two Commit request.
    p2c_args_t *p2c_args;
};

/**
 * Arguments needed for Read RPC.
 */
struct rpc_read_args_t
{
    ///> Arguments needed for a read request.
    read_args_t *read_args;
    ///> Return value.
    std::string *value;
    ///> Result of the request (true if the key was found).
    bool status;
};

/**
 * Arguments needed for Write RPC.
 */
struct rpc_write_args_t
{
    ///> Arguments needed for a write request.
    write_args_t *write_args;
    ///> Result of the request (true if the key was found).
    bool status;
};

/**
 * Arguments needed for Phase 1 Commit RPC.
 */
struct rpc_p1c_args_t
{
    ///> Arguments needed for Phase One Commit request.
    p1c_args_t *p1c_args;
    ///> Nodes that were associated with this transaction in the server.
    std::set<uint64_t> *nodes;
    ///> The vote for the specified nodes (COMMIT if all of them COMMIT, else ABORT).
    bool vote;
};

/**
 * Arguments needed for Phase 2 Commit RPC.
 */
struct rpc_p2c_args_t
{
    ///> Arguments needed for Phase Two Commit request.
    p2c_args_t *p2c_args;
    ///> Nodes that were associated with this transaction in the server.
    std::set<uint64_t> *nodes;
};

#endif /* REQUEST_H_ */
