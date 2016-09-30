/**
 * MinimumResolutionClient.cc
 *
 *  Created on: Jun 13, 2016
 *      Author: theo
 */

#include <assert.h>
#include <set>
#include "MinimumResolutionClient.h"

MinimumResolutionClient::MinimumResolutionClient(RPCClient *rpc_client)
    : ResolutionClient(rpc_client), _tag(0) { }

MinimumResolutionClient::~MinimumResolutionClient() {
  _node_to_address.clear();
}

void MinimumResolutionClient::findAddresses(const std::set<uint64_t> &nodes, std::set<std::string> &addresses) {
  std::unique_lock<std::mutex> lock(_mutex1);
  // TODO: Change code here when master is implemented.
  for (std::set<uint64_t>::iterator it = nodes.begin(); it != nodes.end(); ++it) {
    if (_node_to_address.find(*it) == _node_to_address.end())
      _node_to_address[*it] = "localhost:10001";
    addresses.insert(_node_to_address[*it]);
  }
}

void MinimumResolutionClient::invalidateAddresses(const std::set<uint64_t> &nodes) {
  std::unique_lock<std::mutex> lock(_mutex1);

  for (std::set<uint64_t>::iterator it = nodes.begin(); it != nodes.end(); ++it)
    _node_to_address.erase(*it);
}

void MinimumResolutionClient::request(std::set<uint64_t> nodes, request_t request, void *args) {
  std::string server_address;

  switch (request) {
    case (READ):
    {
      rsl_read_args_t *rsl_read_args = (rsl_read_args_t *) args;
      rpc_read_args_t rpc_read_args;
      std::set<std::string> addresses;

      rpc_read_args.read_args = rsl_read_args->read_args;
      assert (nodes.size() == 1);
      findAddresses(nodes, addresses);
      assert (addresses.size() == 1);
      server_address = *addresses.begin();
      while (!_rpc_client->syncRPC(server_address, READ, &rpc_read_args) || !rpc_read_args.status) {
        invalidateAddresses(nodes);
        findAddresses(nodes, addresses);
        server_address = *addresses.begin();
        return;
      }
      rsl_read_args->value = rpc_read_args.value;
      break;
    }
    case (WRITE):
    {
      rsl_write_args_t *rsl_write_args = (rsl_write_args_t *) args;
      rpc_write_args_t rpc_write_args;
      std::set<std::string> addresses;

      rpc_write_args.write_args = rsl_write_args->write_args;
      assert (nodes.size() == 1);
      findAddresses(nodes, addresses);
      assert(addresses.size() == 1);
      server_address = *addresses.begin();
      while (!_rpc_client->syncRPC(server_address, WRITE, &rpc_write_args) || !rpc_write_args.status) {
        invalidateAddresses(nodes);
        findAddresses(nodes, addresses);
        server_address = *addresses.begin();
        return;
      }
      break;
    }
    case (P1C):
    {
      rsl_p1c_args_t *rsl_p1c_args = (rsl_p1c_args_t *) args;
      std::set<std::string> addresses;
      std::map<std::string,rpc_p1c_args_t> address_to_reply;
      std::set<uint64_t> rem_nodes(nodes.begin(), nodes.end());
      int tag;

      rsl_p1c_args->vote = true;
      while (rem_nodes.size() != 0) {
        // Step One: Gather all the addresses of the remaining nodes.
        findAddresses(rem_nodes, addresses);
        // std::cout << "Addresses size: " << addresses.size() << std::endl;

        // Step Two: Send messages to all the addresses.
        _mutex2.lock();
        tag = _tag;
        for (std::set<std::string>::iterator it = addresses.begin(); it != addresses.end(); ++it) {
          rpc_p1c_args_t rpc_p1c_args;

          rpc_p1c_args.p1c_args = rsl_p1c_args->p1c_args;
          rpc_p1c_args.nodes = new std::set<uint64_t> ();
          address_to_reply[*it] = rpc_p1c_args;
          _rpc_client->asyncRPC(*it, _tag++, P1C, &address_to_reply[*it]);
        }
        _mutex2.unlock();
        // std::cout << "Send all messages." << std::endl;

        // Step Three: Collect all the replies.
        for (std::set<std::string>::iterator it1 = addresses.begin(); it1 != addresses.end(); ++it1) {
          std::set<uint64_t> *updated_nodes;

          _rpc_client->waitAsyncReply(tag++);
          updated_nodes = address_to_reply[*it1].nodes;
          // std::cout << "Reply Nodes Size: " << updated_nodes.size() << std::endl;
          for (std::set<uint64_t>::iterator it2 = updated_nodes->begin(); it2 != updated_nodes->end(); ++it2)
            rem_nodes.erase(*it2);
          delete updated_nodes;
          rsl_p1c_args->vote = rsl_p1c_args->vote && address_to_reply[*it1].vote;
        }
        // std::cout << "Reply all messages." << std::endl;

        // Step Four: Delete from cache all the remaining nodes (wrong addresses).
        invalidateAddresses(rem_nodes);
      }
      break;
    }
    case (P2C):
    {
      rsl_p2c_args_t *rsl_p2c_args = (rsl_p2c_args_t *) args;
      std::set<std::string> addresses;
      std::map<std::string,rpc_p2c_args_t> address_to_reply;
      std::set<uint64_t> rem_nodes(nodes.begin(), nodes.end());
      int tag;

      while (rem_nodes.size() != 0) {
        // Step One: Gather all the addresses of the remaining nodes.
        findAddresses(rem_nodes, addresses);
        // std::cout << "Addresses size: " << addresses.size() << std::endl;

        // Step Two: Send messages to all the addresses.
        _mutex2.lock();
        tag = _tag;
        for (std::set<std::string>::iterator it = addresses.begin(); it != addresses.end(); ++it) {
          rpc_p2c_args_t rpc_p2c_args;

          rpc_p2c_args.p2c_args = rsl_p2c_args->p2c_args;
          rpc_p2c_args.nodes = new std::set<uint64_t> ();
          address_to_reply[*it] = rpc_p2c_args;
          _rpc_client->asyncRPC(*it, _tag++, P2C, &address_to_reply[*it]);
        }
        _mutex2.unlock();
        // std::cout << "Send all messages." << std::endl;

        // Step Three: Collect all the replies.
        for (std::set<std::string>::iterator it = addresses.begin(); it != addresses.end(); ++it) {
          std::set<uint64_t> *updated_nodes;

          _rpc_client->waitAsyncReply(tag++);
          updated_nodes = address_to_reply[*it].nodes;
          // std::cout << "Reply Nodes Size: " << updated_nodes.size() << std::endl;
          for (std::set<uint64_t>::iterator it2 = updated_nodes->begin(); it2 != updated_nodes->end(); ++it2)
            rem_nodes.erase(*it2);
          delete updated_nodes;
        }
        // std::cout << "Reply all messages." << std::endl;

        // Step Four: Delete from cache all the remaining nodes (wrong addresses).
        invalidateAddresses(rem_nodes);
      }
      break;
    }
  }
}