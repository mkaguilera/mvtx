/*
 * DummyResolutionClient.h
 *
 *  Created on: Jun 13, 2016
 *      Author: theo
 */

#ifndef DUMMYRESOLUTIONCLIENT_H_
#define DUMMYRESOLUTIONCLIENT_H_

#include <map>
#include <mutex>
#include "ResolutionClient.h"

/**
 * Dummy implementation of ResolutionClient. All clients are supposed to go to localhost::10001.
 */
class DummyResolutionClient: public ResolutionClient
{
  private:
    ///> Maps nodes to addresses.
    std::map<uint64_t, std::string> _node_to_address;
    ///> Tag to pass to RPCClient.
    uint64_t _tag;
    ///> Locks for thread safe implementation.
    std::mutex _mutex1, _mutex2;

  public:
    /**
     * Constructor for DummyResolutionClient.
     * @param rpc_client  - RPC Client to forward the requests.
     */
    DummyResolutionClient(RPCClient *rpc_client);

    /**
     * Destructor for DummyResolutionClient. Cleans the dictionary from partitions to physical addresses.
     */
    ~DummyResolutionClient();

  private:
    /**
     * Finds the addresses of the requested nodes by searching _node_to_address or/and by quering the master.
     * @param nodes     - Nodes which location is needed.
     * @param addresses - Location of the requested nodes.
     */
    void findAddresses(const std::set<uint64_t> &nodes, std::set<std::string> &addresses);

    /**
     * Invalidates addresses from the cache.
     * @param nodes - Addresses to invalidate.
     */
    void invalidateAddresses(const std::set<uint64_t> &nodes);
  public:
    void request(std::set<uint64_t> nodes, request_t request, void *args) override;
};

#endif /* DUMMYRESOLUTIONCLIENT_H_ */
