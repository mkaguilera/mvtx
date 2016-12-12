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
COORD_SOURCES=GRPCClient.o MinimumResolutionClient.o Coordinator.o SimpleKeyMapper.o SimpleTransactionIDGenerator.o \
              SimpleTimestampGenerator.o WithdrawCoordinator.o MvtkvsService.pb.o MvtkvsService.grpc.pb.o \
              SafeQueue.o WithdrawCoordinatorMain.o
SERVER_SOURCES=ServerMain.o ServerEvent.o SimpleTServer.o SimpleKeyMapper.o SafeQueue.o GRPCServer.o \
               MvtkvsService.pb.o MvtkvsService.grpc.pb.o

vpath %.proto $(PROTOS_PATH)

all: Doxygen $(OBJS) test

test: RunTests

%.grpc.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --grpc_out=. --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

%.pb.cc: %.proto
	$(PROTOC) -I $(PROTOS_PATH) --cpp_out=. $<

GRPCClient.o : RPCClient.h GRPCClient.h GRPCClient.cc MvtkvsService.grpc.pb.h MvtkvsService.pb.h Request.h

MinimumResolutionClient.o : ResolutionClient.h MinimumResolutionClient.h MinimumResolutionClient.cc RPCClient.h \
                            Request.h

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

AVLTreeLockNode.o : LockStatus.h AVLTreeLockNode.h AVLTreeLockNode.cc

AVLTreeLockManager.o : Event.h LockStatus.h LockManager.h AVLTreeLockManager.h AVLTreeLockManager.cc

TestEvent.o : Event.h TestEvent.h TestEvent.cc

LockManagerTest.o : Test.h LockManagerTest.h LockManagerTest.cc

RPCTest.o : Test.h RPCTest.h RPCTest.cc

RunTests.o : LockManager.h RPCClient.h RPCServer.h RunTests.cc

RunTests : AVLTreeLockManager.o AVLTreeLockNode.o TestEvent.o ServerEvent.o SimpleTServer.o \
           SimpleKeyMapper.o SafeQueue.o GRPCServer.o MvtkvsService.pb.o MvtkvsService.grpc.pb.o LockManagerTest.o \
           RunTests.o GRPCClient.o RPCTest.o
	$(CXX) $^ $(LDFLAGS) -o $@

Doxygen : mvtx.doxyfile *.h *.cc
	doxygen mvtx.doxyfile

clean:
	$(RM) -r -f *.o *.pb.cc *.pb.h $(OBJS) RunTests html latex
