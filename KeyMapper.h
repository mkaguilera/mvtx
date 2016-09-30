/**
 * KeyMapper.h
 *
 *  Created on: Jun 15, 2016
 *      Author: theo
 */

#ifndef KEYMAPPER_H_
#define KEYMAPPER_H_

#include <stdint.h>

/**
 * Interface for mapper from keys to nodes.
 */
class KeyMapper
{
public:
  virtual ~KeyMapper() {}

  /**
   * Map keys to nodes.
   * @param key - Key for which node is needed.
   * @return    - Node for the specific key.
   */
  virtual uint64_t getNode(uint64_t key) = 0;
};

#endif /* KEYMAPPER_H_ */