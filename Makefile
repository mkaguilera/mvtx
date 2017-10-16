# Directories.
SRC_DIR=./src
SHELL_DIR=$(SRC_DIR)/shell
PROTOS_DIR=$(SRC_DIR)/protos
OBJ_DIR=./obj
BIN_DIR=./bin
DOC_DIR=./docs

# Flags and command macros.
CXX = g++
RM = rm
CPPFLAGS += -I/usr/local/include
CXXFLAGS += -std=c++11 -Wall -pthread
LDFLAGS += -L/usr/local/lib `pkg-config --libs grpc++` -lprotobuf -lpthread -ldl -lgrpc -lfl

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
SHELL_OBJ=$(addprefix $(OBJ_DIR)/,GRPCClient.o SimpleResolutionClient.o Coordinator.o SimpleKeyMapper.o \
		  SimpleTransactionIDGenerator.o SimpleTimestampGenerator.o SafeQueue.o)
TESTS_OBJ=$(addprefix $(OBJ_DIR)/,LockManagerTest.o TestEvent.o RPCTest.o ResolutionClientTest.o CoordinatorTest.o)

# Binary files.
BINS=$(addprefix $(BIN_DIR)/,Server Coordinator)
SHELL_BIN=$(BIN_DIR)/Shell

# Commands.
all: $(OBJ_DIR) $(BIN_DIR) $(BINS)

shell: $(SHELL_BIN)

docs: LegoStore.doxyfile $(SRC_DIR)/*.h $(SRC_DIR)/*.cc
	doxygen LegoStore.doxyfile

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

$(SHELL_DIR)/shell.tab.cc $(SHELL_DIR)/shell.tab.hh : $(SHELL_DIR)/shell.y $(SHELL_DIR)/shell.h
	bison -o $(SHELL_DIR)/shell.tab.cc -vd $(SHELL_DIR)/shell.y

$(SHELL_DIR)/shell.lex.cc : $(SHELL_DIR)/shell.l $(SHELL_DIR)/shell.tab.hh
	flex -o $(SHELL_DIR)/shell.lex.cc -l $(SHELL_DIR)/shell.l

$(BIN_DIR)/Shell: $(SHELL_DIR)/shell.lex.cc $(SHELL_DIR)/shell.tab.cc $(SHELL_DIR)/shell.tab.hh
	g++ -std=c++11 -o $(BIN_DIR)/Shell $(SHELL_DIR)/shell.lex.cc $(SHELL_DIR)/shell.tab.cc $(PROTOS_OBJ) $(SHELL_OBJ) $(LDFLAGS)

clean:
	$(RM) -r -f $(OBJ_DIR) $(BIN_DIR) $(PROTOS_SRC) $(addprefix $(SHELL_DIR)/,shell.lex.cc shell.output shell.tab.cc shell.tab.hh) $(DOC_DIR)

