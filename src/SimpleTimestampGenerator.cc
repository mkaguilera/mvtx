/*
 * SimpleTimestampGenerator.cc
 *
 *  Created on: Jun 15, 2016
 *      Author: theo
 */

#include "SimpleTimestampGenerator.h"

SimpleTimestampGenerator::SimpleTimestampGenerator() {}

SimpleTimestampGenerator::~SimpleTimestampGenerator() {}

uint64_t SimpleTimestampGenerator::getCurrentTime() {
  time_t timev;

  time(&timev);
  return ((uint64_t) timev);
}

uint64_t SimpleTimestampGenerator::genStartTimestamp() {
  return (getCurrentTime());
}

uint64_t SimpleTimestampGenerator::genCommitTimestamp() {
  return (getCurrentTime());
}
