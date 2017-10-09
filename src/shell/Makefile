OBJ_DIR=../../obj
BIN_DIR=../../bin
PROTOS_OBJ=$(addprefix $(OBJ_DIR)/,MvtkvsService.grpc.pb.o MvtkvsService.pb.o)
COORD_OBJ =$(addprefix $(OBJ_DIR)/,GRPCClient.o SimpleResolutionClient.o Coordinator.o SimpleKeyMapper.o \
		   SimpleTransactionIDGenerator.o SimpleTimestampGenerator.o SafeQueue.o)
LDFLAGS=-L/usr/local/lib `pkg-config --libs grpc++` -lprotobuf -lpthread -ldl -lgrpc -lfl

all: ${BIN_DIR}/shell

shell.tab.cc shell.tab.hh : shell.y shell.h
	bison -o shell.tab.cc -vd shell.y

shell.lex.cc : shell.l shell.tab.hh
	flex -o shell.lex.cc -l shell.l

${BIN_DIR}/shell: shell.lex.cc shell.tab.cc shell.tab.hh
	g++ -std=c++11 -o ${BIN_DIR}/shell shell.lex.cc shell.tab.cc $(PROTOS_OBJ) $(COORD_OBJ) $(LDFLAGS)

clean:
	rm -f ${BIN_DIR}/shell shell.tab.* shell.lex.* shell.output