/*
 * LockStatus.h
 *
 *  Created on: Nov 14, 2016
 *      Author: theo
 */

#ifndef LOCKSTATUS_H_
#define LOCKSTATUS_H_

/**
 * Lock status for LockManager lock.
 */
enum LockStatus
{
  FREE,               ///> No lock.
  READ_LOCK,          ///> Read lock that might be unlocked (FREE) or frozen (FREEZE_READ_LOCK)
  WRITE_LOCK,         ///> Write lock that might be unlocked (FREE) or frozen (FREEZE_WRITE_LOCK)
  FREEZE_READ_LOCK,   ///> Frozen read lock that cannot change status.
  FREEZE_WRITE_LOCK   ///> Frozen write lock that cannot change status.
};

#endif /* LOCKSTATUS_H_ */
