/*
 * LockManagerTest.h
 *
 *  Created on: Dec 8, 2016
 *      Author: theo
 */
#ifndef LOCKMANAGERTEST_H_
#define LOCKMANAGERTEST_H_

#include "LockManager.h"
#include "Test.h"

/**
 * Test class for LockManager. Tests the functionality of each individual function (unit tests) as well as more
 * sophisticated scenarios.
 */
class LockManagerTest: public Test
{
  private:
    LockManager *_lock_manager;

  public:
    /**
     * Initiates test for LockManager.
     * @param lock_manager - LockManager to test its implementation.
     */
    LockManagerTest(LockManager *lock_manager);

    void run() override;
};

#endif /* LOCKMANAGERTEST_H_ */
