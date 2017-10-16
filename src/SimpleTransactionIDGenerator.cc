/*
 * SimpleTransactionIDGenerator.cc
 *
 *  Created on: Jun 15, 2016
 *      Author: theo
 */

#include "SimpleTransactionIDGenerator.h"

SimpleTransactionIDGenerator::SimpleTransactionIDGenerator()
    : _cur_id(0) {
}

SimpleTransactionIDGenerator::~SimpleTransactionIDGenerator() {}

uint64_t SimpleTransactionIDGenerator::genTransactionID() {
  std::unique_lock<std::mutex> lock(_mutex);

  return (_cur_id++);
}
