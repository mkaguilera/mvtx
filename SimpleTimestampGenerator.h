/*
 * SimpleTimestampGenerator.h
 *
 *  Created on: Jun 15, 2016
 *      Author: theo
 */

#ifndef SIMPLETIMESTAMPGENERATOR_H_
#define SIMPLETIMESTAMPGENERATOR_H_

#include <time.h>
#include "TimestampGenerator.h"

/**
 * Simple implementation of timestamp generator.
 */
class SimpleTimestampGenerator : public TimestampGenerator
{
public:
  SimpleTimestampGenerator();
  ~SimpleTimestampGenerator();

private:
  uint64_t getCurrentTime();

public:
  uint64_t genStartTimestamp() override;
  uint64_t genCommitTimestamp() override;
};

#endif /* SIMPLETIMESTAMPGENERATOR_H_ */
