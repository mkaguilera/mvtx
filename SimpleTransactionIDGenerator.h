/**
 * SimpleTransactionIDGenerator.h
 *
 *  Created on: Jun 15, 2016
 *      Author: theo
 */

#ifndef SIMPLETRANSACTIONIDGENERATOR_H_
#define SIMPLETRANSACTIONIDGENERATOR_H_

#include <mutex>
#include "TransactionIDGenerator.h"

/**
 * Simple implementation for transaction ID generators.
 */
class SimpleTransactionIDGenerator : public TransactionIDGenerator
{
private:
  uint64_t _cur_id;
  std::mutex _mutex;

public:
  SimpleTransactionIDGenerator();
  virtual ~SimpleTransactionIDGenerator();
  uint64_t genTransactionID() override;
};

#endif /* SIMPLETRANSACTIONIDGENERATOR_H_ */
