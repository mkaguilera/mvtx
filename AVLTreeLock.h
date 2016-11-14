/*
 * AVLTreeLock.h
 *
 *  Created on: Jul 25, 2016
 *      Author: theo
 */

#ifndef AVLTREELOCK_H_
#define AVLTREELOCK_H_

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "Event.h"
#include "LockStatus.h"

/**
 * Tree implementation for the AVLTreeLock implementing the logic behind the AVLTreeLockManager.
 */
class AVLTreeLock
{
  public:
    ///> Status of the current node.
    LockStatus _status;
    ///> Starting point of associated time interval.
    uint64_t _start;
    ///> Ending point of associated time interval.
    uint64_t _end;
    ///> Transactions that have locked the root node. Only filled when status is READ_LOCK or WRITE_LOCK.
    std::set<uint64_t> *_tids;
    ///> Events that should be triggered if node is frozen or unlocked. Only filled when status is WRITE_LOCK.
    std::set<Event *> *_events;

  private:
    ///> Height of left subtree (root is the left child) of this node.
    int _left_height;
    ///> Height of right subtree (root is the right child) of this node.
    int _right_height;
    ///> Parent of this node. Null if node is the root.
    AVLTreeLock *_parent;
    ///> Left child of this node. Null if there is no left child.
    AVLTreeLock *_left;
    ///> Right child of this node. Null if there is no left child.
    AVLTreeLock *_right;

  public:
    /**
     * Constructor of AVLTreeLock. Initializes the tree with a root node with time interval [0,MAX_UINT].
     */
    AVLTreeLock();

    /**
     * Constructor of AVLTreeLock. It copies all the fields from another AVLTreeLock.
     * @param lock  - AVLTreeLock to copy from.
     */
    AVLTreeLock(AVLTreeLock *lock);

    /**
     * Destructor of AVLTreeLock.
     */
    ~AVLTreeLock();

  private:
    /**
     * Finds the root of the AVLTreeLock.
     * @return  - Root of AVLTreeLock
     */
    AVLTreeLock *getRoot();

    /**
     * Updates the left and right heights of this tree.
     */
    void updateHeight();

    /**
     * Swap node inside an AVLTreeLock with one of its children.
     * @param is_left - Whether or not the swapped child is the left child.
     * @return        - Root of the corresponding AVLTreeLock.
     */
    void swap(bool is_left);

    /**
     * Balance this tree according to AVL tree rules. The tree is balanced in respect to the starting point of their
     * time interval. If tree changes balance the predecessors.
     */
    void balance();

  public:
    /**
     * Finds the path from this node to node which contains a specific time value in its time interval. Maintains all
     * nodes with lower starting point than this time value.
     * @param locks - Vector to add the corresponding nodes.
     * @param ts    - Time value for which this search is enabled.
     */
    void find(std::vector<AVLTreeLock*> *locks, uint64_t ts);

    /**
     * Insert a new node according to AVL tree rules. The nodes are placed in respect to the starting point of their
     * time interval.
     * @param new_lock  - AVLTreeLock to be inserted in the tree.
     * @return          - Root of the corresponding AVLTreeLock.
     */
    AVLTreeLock *insert(AVLTreeLock *new_lock);

    /**
     * Remove this node from AVLTreeLock.
     * @return  - Root of the corresponding AVLTreeLock.
     */
    AVLTreeLock *remove();

    /**
     * Serialize the fields of AVLTreeLock as a string.
     * @return  - Representative string for this AVLTreeLock.
     */
    std::string toString();
};

#endif /* AVLTREELOCK_H_ */
