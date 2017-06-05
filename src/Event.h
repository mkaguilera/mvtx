/*
 * Event.h
 *
 *  Created on: Nov 14, 2016
 *      Author: theo
 */

#ifndef EVENT_H_
#define EVENT_H_

/**
 * Events that are supposed to be triggered on specific cases.
 */
class Event
{
  public:
    /**
     * Destructor of Event.
     */
    virtual ~Event() {
    }
    ;

    /**
     * Code to run when event is triggered.
     */
    virtual void run() = 0;
};

#endif /* EVENT_H_ */
