/**
 * ServerMain.cc
 *
 *  Created on: Jul 11, 2016
 *      Author: theo
 */

#include "GRPCServer.h"
#include "SafeQueue.h"
#include "ServerEvent.h"
#include "SimpleKeyMapper.h"
#include "SimpleTServer.h"

void *RunThreads(void *args) {
  TServer *server = static_cast<TServer *> (args);
  ServerEvent *event = NULL;

  std::cout << "Starting Thread." << std::endl;
  while (true) {
    event = server->getEvent();
    event->run();
  }

  return 0;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <port no> <num threads>." << std::endl;
    exit(-1);
  }

  GRPCServer grpc_server(atoi(argv[1]));
  int nr_threads = atoi(argv[2]);
  SimpleKeyMapper key_mapper;
  SimpleTServer server(&grpc_server, &key_mapper, 1000);
  pthread_t threads[nr_threads];
  std::mutex lock;

  for (int i = 0; i < nr_threads; i++) {
    if (pthread_create(&threads[i], NULL, RunThreads, &server)) {
      std::cerr << "Error: Unable to create thread." << std::endl;
      exit(1);
    }
  }
  for (int i = 0; i < nr_threads; i++)
    pthread_join(threads[i], NULL);
  return 0;
}