/*
 * Test.h
 *
 *  Created on: Dec 8, 2016
 *      Author: theo
 */
#ifndef TEST_H_
#define TEST_H_

/**
 * Generic class for tests. Test classes are supposed to implement test cases for the corresponding main classes.
 */
class Test
{
  public:
    /**
     * Destructor of Test.
     */
    virtual ~Test() {
    }
    ;

    /**
     * Run all tests that correspond to this class.
     */
    virtual void run() = 0;
};

#endif /* TEST_H_ */
