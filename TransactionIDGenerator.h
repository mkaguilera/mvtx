/**
 * TransactionIDGenerator.h
 *
 *  Created on: Jun 15, 2016
 *      Author: theo
 */

#ifndef TRANSACTIONIDGENERATOR_H_
#define TRANSACTIONIDGENERATOR_H_

#include <stdint.h>

/**
 * Interface for transaction ID generators.
 */
class TransactionIDGenerator
{
public:
  TransactionIDGenerator() {};
  virtual ~TransactionIDGenerator() {};

  /**
   * Generate transaction ID for the next transaction.
   * @return - transaction ID.
   */
  virtual uint64_t genTransactionID() = 0;
};

#endif /* TRANSACTIONIDGENERATOR_H_ */
