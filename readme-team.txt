Client Side Files.
Coordinator -> Module for coordinating the transaction protocol. Specialize run() for each transaction protocol.
WithdrawCoordinator -> Example of a Coordinator that withdraws amount from one out of two accounts.
ResolutionClient -> Module for deciding in which server(s) specific nodes reside.
MinimumResolutionClient -> Example of resolution client. Resolves all nodes to a fixed server. 
RPCClient -> Module for Coordinator RPC protocol. Supports both blocking and non-blocking requests.
Request -> Enumeration of different types of requests. Arguments that are used in each request type for RPC, Resolution and Coordinator/Server layers.
GRPCClient -> Example of RPC Client. Supports different requests defined in proto files of gRPC. The same requests should be matched with the request enumeration in Request.h.
SimpleKeyMapper -> Example that maps keys to 0 node.
TimestampGenerator -> Module for generating timestamps.
SimpleTimestampGenerator-> Example that creates timestamps according to Hardware Clock.
TransactionIDGenerator -> Module for generating Transaction IDs.SimpleTransactionIDGenerator -> Example that creates IDs incrementally (tid += 1).
CoordinatorMain -> Main function for client side. Combines all the examples.
-------------------------------------------------------------------------------------------------------------------------------------------------------------------
Server Side Files.
TServer -> Top-level module of the server side. It contains both Server and Node modules combined. Node module should be separated.
SimpleServer -> Example of server that uses simple modules (RPCServer, ServerEvent).
ServerEvent -> Module for defining request handlers for READ,WRITE,P1C,P2C. Also, example for MVTO request handlers. Example should be separated from module (TODO).
RPCServer -> Module for handling RPC requests in the server side. Exposes interface for blocking and non-blocking processing of requests.
GRPCServer -> Example of RPCServer using gRPC.
AVLTreeLock -> Helper Class for Lock Manager. Implements AVL Self-Balancing Tree.
LockManager -> Module for locking keys and timestamps.
SimpleLockManager -> Example that uses AVL self-balancing trees (AVLTreeLock).
MVtkvsAsyncServer -> Previous (unused) GRPC Server implementation.
Node -> Unused module for nodes.
ServerMain -> Main function for server side. Combines all the examples.
--------------------------------------------------------------------------------------------------------------------------------------------------
Common Side Files.
KeyMapper -> Module for mapping keys to nodes.
SafeQueue -> Helper class for MT safe queue for events in the server and requests in the coordinator side.
--------------------------------------------------------------------------------------------------------------------------------------------------
Lessons for gRPC experiments.
Pinning CPUs for 1-2 threads was much better than not pinning CPUs.
Running inside the same numa node is much better than running across multiple numa nodes. Actually one server with 6 threads in one numa node is doing better that 12 threads across two numa nodes.
Hyper-threading caused significant overhead (still better running with hyperthreading than splitting 6+6 threads across the two numa nodes).
Putting two servers in the same physical machine but different numa nodes increased throughput but did not double (network bottleneck).
