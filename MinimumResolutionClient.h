/**
 * minimumResolutionClient.h
 *
 *  Created on: Jun 13, 2016
 *      Author: theo
 */

#ifndef MINIMUMRESOLUTIONCLIENT_H_
#define MINIMUMRESOLUTIONCLIENT_H_

#include <map>
#include <mutex>
#include "ResolutionClient.h"

/**
 * Straightforward implementation of ResolutionClient.
 */
class MinimumResolutionClient : public ResolutionClient
{
private:
  std::map<uint64_t, std::string> _node_to_address;     ///< Maps nodes to addresses.
  uint64_t _tag;                                        ///< Tag to pass to RPCClient.
  std::mutex _mutex1, _mutex2;                          ///< Lock for thread safe implementation.

public:
  MinimumResolutionClient(RPCClient *rpc_client);
  ~MinimumResolutionClient();

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

#endif /* MINIMUMRESOLUTIONCLIENT_H_ */
