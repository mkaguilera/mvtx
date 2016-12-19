/*
 * WithdrawCoordinatorMain.cc
 *
 *  Created on: Jun 17, 2016
 *      Author: theo
 */

#include "DummyResolutionClient.h"
#include "GRPCClient.h"
#include "SafeQueue.h"
#include "SimpleKeyMapper.h"
#include "SimpleTimestampGenerator.h"
#include "SimpleTransactionIDGenerator.h"
#include "WithdrawCoordinator.h"

void *RunThreads(void *arg) {
  SafeQueue<void *> *queue = (SafeQueue<void *> *) arg;
  Coordinator * coord;
  time_t old_time, new_time;
  uint64_t count = 0;

  time(&old_time);
  while (true) {
    coord = reinterpret_cast<Coordinator *> (queue->dequeue());
    coord->run();
    delete coord;
    time(&new_time);
    while (new_time > old_time) {
      std::cout << old_time << "," << count << std::endl;
      count = 0;
      old_time++;
    }
    count++;
  }
}

int main(int argc, char **argv) {
  GRPCClient rpc_client;
  DummyResolutionClient rsl_client(&rpc_client);
  SimpleKeyMapper key_mapper;
  SimpleTransactionIDGenerator id_gen;
  SimpleTimestampGenerator ts_gen;
  SafeQueue<void *> queue(100000);
  pthread_t *threads;
  int nr_threads, nr_repeats;

  if (argc != 3) {
    std::cerr << "Usage: ./WithdrawCoordinator <nr threads> <nr repeats>" << std::endl;
    exit(0);
  }
  nr_threads = atoi(argv[1]);
  nr_repeats = atoi(argv[2]);
  for (int i = 0; i < nr_repeats; i++) {
    Coordinator *coord = new WithdrawCoordinator(&rsl_client, &key_mapper, &id_gen, &ts_gen, 10);

    queue.enqueue(coord);
  }
  threads = (pthread_t *) malloc(nr_threads*sizeof(pthread_t));
  for (int i = 0; i < nr_threads; i++) {
    if (pthread_create(&threads[i], NULL, RunThreads, (void *) &queue)) {
      std::cerr << "Error:unable to create thread" << std::endl;
      exit(1);
    }
  }
  for (int i = 0; i < nr_threads; i++)
    pthread_join(threads[i], NULL);
  return 0;
}

