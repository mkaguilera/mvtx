/*
 * WithdrawCoordinator.cc
 *
 *  Created on: Jun 15, 2016
 *      Author: theo
 */

#include "WithdrawCoordinator.h"

WithdrawCoordinator::WithdrawCoordinator(ResolutionClient *rsl_client, KeyMapper *key_mapper,
                                         TransactionIDGenerator *id_gen, TimestampGenerator *ts_gen, uint64_t amount)
    : Coordinator(rsl_client, key_mapper, id_gen, ts_gen), _amount(amount) {
}

WithdrawCoordinator::~WithdrawCoordinator() {}

bool WithdrawCoordinator::run() {
  int balance1 = atoi(read(1).c_str());
  int balance2 = atoi(read(2).c_str());

  if (balance1 + balance2 >= (int) _amount) {
    balance1 -= _amount;
    write(1, std::to_string(balance1));
  }
  return commit();
}
