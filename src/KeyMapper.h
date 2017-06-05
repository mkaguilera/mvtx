/*
 * KeyMapper.h
 *
 *  Created on: Jun 15, 2016
 *      Author: theo
 */

#ifndef KEYMAPPER_H_
#define KEYMAPPER_H_

#include <cstdint>

/**
 * Interface for mapping keys to logical partitions (nodes).
 */
class KeyMapper
{
  public:
    /**
     * Destructor of KeyMapper.
     */
    virtual ~KeyMapper() {
    }
    ;

    /**
     * Maps keys to nodes.
     * @param key - Key for which node is needed.
     * @return    - Node for the specific key.
     */
    virtual uint64_t getNode(uint64_t key) = 0;
};

#endif /* KEYMAPPER_H_ */
