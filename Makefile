CXX = g++
CPPFLAGS += -I/usr/local/include -pthread
CXXFLAGS += -std=c++11 -Wall
LDFLAGS += -L/usr/local/lib `pkg-config --libs grpc++` -lprotobuf -lpthread -ldl -lgrpc
RM = rm
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`
PROTOS_PATH = ./protos
OBJS=MvtkvsService.grpc.pb.cc MvtkvsService.pb.cc Coordinator Server
COORD_SOURCES=GRPCClient.o SimpleResolutionClient.o Coordinator.o SimpleKeyMapper.o SimpleTransactionIDGenerator.o \
              SimpleTimestampGenerator.o WithdrawCoordinator.o MvtkvsService.pb.o MvtkvsService.grpc.pb.o \
              SafeQueue.o WithdrawCoordinatorMain.o
SERVER_SOURCES=ServerMain.o ServerEvent.o SimpleTServer.o SimpleKeyMapper.o SafeQueue.o GRPCServer.o \
               MvtkvsService.pb.o MvtkvsService.grpc.pb.o
TESTS=MvtkvsService.grpc.pb.cc MvtkvsService.pb.cc RunTests

vpath %.proto $(PROTOS_PATH)

all: Doxygen $(OBJS)

tests: $(TESTS)

Doxygen : mvtx.doxyfile *.h *.cc
	doxygen mvtx.doxyfile

%.grpc.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

GRPCClient.o : RPCClient.h GRPCClient.h GRPCClient.cc MvtkvsService.grpc.pb.h MvtkvsService.pb.h Request.h

SimpleResolutionClient.o : ResolutionClient.h SimpleResolutionClient.h SimpleResolutionClient.cc RPCClient.h Request.h

Coordinator.o : Coordinator.h Coordinator.cc KeyMapper.h TransactionIDGenerator.h TimestampGenerator.h \
                ResolutionClient.h RPCClient.h Request.h

SimpleKeyMapper.o : KeyMapper.h SimpleKeyMapper.h SimpleKeyMapper.cc

SimpleTransactionIDGenerator.o : TransactionIDGenerator.h SimpleTransactionIDGenerator.h \
                                 SimpleTransactionIDGenerator.cc

SimpleTimestampGenerator.o : TimestampGenerator.h SimpleTimestampGenerator.h SimpleTimestampGenerator.cc

WithdrawCoordinator.o : Coordinator.h WithdrawCoordinator.h WithdrawCoordinator.cc SafeQueue.h KeyMapper.h \
                        TransactionIDGenerator.h TimestampGenerator.h ResolutionClient.h RPCClient.h Request.h

Coordinator : $(COORD_SOURCES)
	 $(CXX) $^ $(LDFLAGS) -o $@

GRPCServer.o : RPCServer.h GRPCServer.h GRPCServer.cc Request.h

SimpleTServer.o : TServer.h SimpleTServer.h SimpleTServer.cc KeyMapper.h SafeQueue.h ServerEvent.h RPCServer.h \
                  Request.h

ServerEvent.o :ServerEvent.h ServerEvent.cc TServer.h Request.h

ServerMain.o: ServerMain.cc GRPCServer.h SafeQueue.h ServerEvent.h SimpleKeyMapper.h SimpleTServer.h

Server : $(SERVER_SOURCES)
	$(CXX) $^ $(LDFLAGS) -o $@

LockManagerTest.o : Test.h LockManagerTest.h LockManagerTest.cc

AVLTreeLockNode.o : LockStatus.h AVLTreeLockNode.h AVLTreeLockNode.cc

AVLTreeLockManager.o : Event.h LockStatus.h LockManager.h AVLTreeLockManager.h AVLTreeLockManager.cc

TestEvent.o : Event.h TestEvent.h TestEvent.cc

RPCTest.o : Test.h RPCTest.h RPCTest.cc

ResolutionClientTest.o : Test.h ResolutionClient.h ResolutionClientTest.h ResolutionClientTest.cc

CoordinatorTest.o : Test.h Coordinator.h CoordinatorTest.h CoordinatorTest.cc

RunTests.o : RunTests.cc LockManagerTest.h RPCTest.h AVLTreeLockManager.h GRPCClient.h GRPCServer.h

RunTests : LockManagerTest.o AVLTreeLockNode.o AVLTreeLockManager.o TestEvent.o RPCTest.o GRPCClient.o GRPCServer.o \
					 MvtkvsService.pb.o MvtkvsService.grpc.pb.o ResolutionClientTest.o SimpleResolutionClient.o \
					 CoordinatorTest.o  SimpleKeyMapper.o SimpleTransactionIDGenerator.o SimpleTimestampGenerator.o \
					 WithdrawCoordinator.o Coordinator.o RunTests.o
	$(CXX) $^ $(LDFLAGS) -o $@


clean:
	$(RM) -r -f html latex *.o *.pb.cc *.pb.h $(OBJS) $(TESTS)