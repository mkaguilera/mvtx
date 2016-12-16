/*
 * LockManagerTest.cc
 *
 *  Created on: Dec 8, 2016
 *      Author: theo
 */

#include <cassert>
#include <iostream>

#include "AVLTreeLockManager.h"
#include "LockManager.h"
#include "LockManagerTest.h"
#include "TestEvent.h"

LockManagerTest::LockManagerTest(LockManager *lock_manager) {
  _lock_manager = lock_manager;
}

void LockManagerTest::run() {
  uint64_t last = 1;

  // Unit tests for tryBackwardLock.
  assert(_lock_manager->tryBackwardLock(0, 0, 3, false, new TestEvent(1), &last));
  assert(last == 0);
  assert(!_lock_manager->tryBackwardLock(0, 1, 4, true, new TestEvent(2), &last));
  assert(last == 3);

  // Unit tests for tryLock.
  assert(_lock_manager->tryLock(1, 0, 5, 15, false));
  assert(!_lock_manager->tryLock(1, 1, 1, 5, true));

  // Unit tests for freeze.
  _lock_manager->freeze(0);
  assert(_lock_manager->tryBackwardLock(0, 1, 3, true, new TestEvent(2), &last));
  assert(last == 3);

  // Unit tests for unlock.
  _lock_manager->unlock(1);
  assert(_lock_manager->tryBackwardLock(0, 2, 5, false, new TestEvent(3), &last));
  assert(last == 3);
  _lock_manager->unlock(2);

  // More sophisticated test.
  assert(_lock_manager->tryBackwardLock(2, 3, 15, true, new TestEvent(3), &last));
  assert(last == 0);
  assert(_lock_manager->tryBackwardLock(2, 4, 30, true, new TestEvent(4), &last));
  assert(last == 0);
  assert(_lock_manager->tryLock(2, 4, 31, 35, false));
  assert(!_lock_manager->tryLock(2, 5, 5, 5, false));
  assert(!_lock_manager->tryBackwardLock(2, 6, 40, true, new TestEvent(6), &last));
  assert(last == 35);
  _lock_manager->freeze(3);
  _lock_manager->freeze(4);
  assert(_lock_manager->tryBackwardLock(2, 6, 35, true, new TestEvent(6), &last));
  assert(last == 35);
  assert(_lock_manager->tryBackwardLock(2, 7, 50, true, new TestEvent(7), &last));
  assert(_lock_manager->tryLock(2, 7, 51, 55, false));
  assert(!_lock_manager->tryBackwardLock(2, 8, 70, true, new TestEvent(8), &last));
  assert(last == 55);
  _lock_manager->unlock(6);
  _lock_manager->unlock(7);
  assert(_lock_manager->tryBackwardLock(2, 8, 55, true, new TestEvent(8), &last));
  assert(last == 35);
  _lock_manager->freeze(8);

  std::cout << "Lock Manager Test Successful." << std::endl;
}

int main(int argc, char **argv) {
  AVLTreeLockManager lock_manager;
  LockManagerTest lock_manager_test(&lock_manager);

  lock_manager_test.run();

  return 0;
}
