/*
 * TestEvent.h
 *
 *  Created on: Nov 21, 2016
 *      Author: theo
 */
#ifndef TESTEVENT_H_
#define TESTEVENT_H_

#include <cstdint>

#include "Event.h"

class TestEvent : public Event {
  private:
    uint64_t _event_id;

  public:
    TestEvent(uint64_t event_id);
    void run() override;
};

#endif /* TESTEVENT_H_ */
