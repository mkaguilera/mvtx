# Directories.
SRC_DIR=./src
PROTOS_DIR=$(SRC_DIR)/protos
OBJ_DIR=./obj
BIN_DIR=./bin

# Flags and command macros.
CXX = g++
RM = rm
CPPFLAGS += -I/usr/local/include
CXXFLAGS += -std=c++11 -Wall -pthread
LDFLAGS += -L/usr/local/lib `pkg-config --libs grpc++` -lprotobuf -lpthread -ldl -lgrpc

# GRPC and protocol buffer macros.
PROTOC = protoc
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`

# Protocol buffer source files.
PROTOS_SRC=$(addprefix $(SRC_DIR)/,MvtkvsService.grpc.pb.cc MvtkvsService.pb.cc)

# Object files.
PROTOS_OBJ=$(addprefix $(OBJ_DIR)/,MvtkvsService.grpc.pb.o MvtkvsService.pb.o)
COORD_OBJ=$(addprefix $(OBJ_DIR)/,GRPCClient.o SimpleResolutionClient.o Coordinator.o SimpleKeyMapper.o \
		  SimpleTransactionIDGenerator.o SimpleTimestampGenerator.o WithdrawCoordinator.o SafeQueue.o \
		  WithdrawCoordinatorMain.o)
SERVER_OBJ=$(addprefix $(OBJ_DIR)/,ServerMain.o ServerEvent.o SimpleTServer.o SimpleKeyMapper.o SafeQueue.o \
      GRPCServer.o AVLTreeLockManager.o AVLTreeLockNode.o)
TESTS_OBJ=$(addprefix $(OBJ_DIR)/,LockManagerTest.o TestEvent.o RPCTest.o ResolutionClientTest.o CoordinatorTest.o)

# Binary files.
BINS=$(addprefix $(BIN_DIR)/,Server Coordinator)

# Commands.
all: $(OBJ_DIR) $(BIN_DIR) $(BINS)

docs: $(SRC_DIR)/LegoStore.doxyfile $(SRC_DIR)/*.h $(SRC_DIR)/*.cc
	doxygen $(SRC_DIR)/LegoStore.doxyfile

$(SRC_DIR)/%.pb.cc: $(PROTOS_DIR)/%.proto
	$(PROTOC) -I $(PROTOS_DIR) --cpp_out=$(SRC_DIR) $<

$(SRC_DIR)/%.grpc.pb.cc: $(PROTOS_DIR)/%.proto
	$(PROTOC) -I $(PROTOS_DIR) --grpc_out=$(SRC_DIR) --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN_PATH) $<

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc
	$(CXX) -I $(SRC_DIR) $(CXXFLAGS) $(CPP_FLAFS) -c -o $@ $<

$(BIN_DIR)/Server: $(PROTOS_OBJ) $(SERVER_OBJ)
	$(CXX) $^ $(LDFLAGS) -o $@

$(BIN_DIR)/Coordinator: $(PROTOS_OBJ) $(COORD_OBJ)
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	$(RM) -r -f $(OBJ_DIR) $(BIN_DIR) $(PROTOS_SRC)
