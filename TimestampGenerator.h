/**
 * TimestampGenerator.h
 *
 *  Created on: Jun 15, 2016
 *      Author: theo
 */

#ifndef TIMESTAMPGENERATOR_H_
#define TIMESTAMPGENERATOR_H_

#include <cstdint>

/**
 * Interface for timestamp generators.
 */
class TimestampGenerator {
public:
  TimestampGenerator() {};
  virtual ~TimestampGenerator() {};

  /**
   * Generate starting timestamp for transactions.
   * @return - starting timestamp.
   */
  virtual uint64_t genStartTimestamp() = 0;

  /**
   * Generate commit timestamp for transactions.
   * @return - commit timestamp.
   */
  virtual uint64_t genCommitTimestamp() = 0;
};

#endif /* TIMESTAMPGENERATOR_H_ */
