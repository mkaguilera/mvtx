/*
 * ServerMain.cc
 *
 *  Created on: Jul 11, 2016
 *      Author: theo
 */

#include "AVLTreeLockManager.h"
#include "GRPCServer.h"
#include "SafeQueue.h"
#include "ServerEvent.h"
#include "SimpleKeyMapper.h"
#include "SimpleTServer.h"

void *run(void *args) {
  TServer *server = static_cast<TServer *> (args);
  ServerEvent *event = NULL;

  while (true) {
    event = server->getEvent();
    event->run();
  }

  return (nullptr);
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <port no> <num threads>." << std::endl;
    exit(-1);
  }

  GRPCServer grpc_server(atoi(argv[1]));
  int nr_threads = atoi(argv[2]);
  SimpleKeyMapper key_mapper;
  AVLTreeLockManager lock_manager;
  SimpleTServer server(&grpc_server, &key_mapper, &lock_manager, 1000);
  pthread_t threads[nr_threads];
  std::mutex lock;

  for (int i = 0; i < nr_threads; i++) {
    if (pthread_create(&threads[i], NULL, run, &server)) {
      std::cerr << "Error: Unable to create thread." << std::endl;
      exit(1);
    }
  }
  for (int i = 0; i < nr_threads; i++)
    pthread_join(threads[i], NULL);
  return (0);
}
