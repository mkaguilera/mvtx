/*
 * SimpleKeyMapper.h
 *
 *  Created on: Jun 15, 2016
 *      Author: theo
 */

#ifndef SIMPLEKEYMAPPER_H_
#define SIMPLEKEYMAPPER_H_

#include "KeyMapper.h"

/**
 * Simple implementation for mapping keys to nodes.
 */
class SimpleKeyMapper : public KeyMapper
{
public:
  SimpleKeyMapper();
  ~SimpleKeyMapper();

  uint64_t getNode(uint64_t key) override;
};

#endif /* SIMPLEKEYMAPPER_H_ */
