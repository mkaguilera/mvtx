/*
 * WithdrawCoordinator.h
 *
 *  Created on: Jun 15, 2016
 *      Author: theo
 */

#ifndef WITHDRAWCOORDINATOR_H_
#define WITHDRAWCOORDINATOR_H_

#include <set>
#include "Coordinator.h"

/**
 * Implementation of coordinator for withdrawals.
 */
class WithdrawCoordinator : public Coordinator
{
private:
  uint64_t _amount;                     ///< Amount to withdraw if there is still in your accounts.
  std::set<uint64_t> _account_numbers;  ///< Account numbers.
public:
  WithdrawCoordinator(ResolutionClient *rsl_client, KeyMapper *key_mapper, TransactionIDGenerator *id_gen,
                      TimestampGenerator *ts_gen, uint64_t amount);
  ~WithdrawCoordinator();

  bool run() override;
};

#endif /* WITHDRAWCOORDINATOR_H_ */
