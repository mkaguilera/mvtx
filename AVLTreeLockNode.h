/*
 * AVLTreeLockNode.h
 *
 *  Created on: Jul 25, 2016
 *      Author: theo
 */

#ifndef AVLTREELOCKNODE_H_
#define AVLTREELOCKNODE_H_

#include <cstdint>
#include <set>
#include <string>
#include <vector>

#include "Event.h"
#include "LockStatus.h"

/**
 * Tree implementation for the AVLTreeLockNode implementing the logic behind the AVLTreeLockManager.
 */
class AVLTreeLockNode
{
  public:
    ///> Status of the current node.
    LockStatus _status;
    ///> Starting point of associated time interval.
    uint64_t _start;
    ///> Ending point of associated time interval.
    uint64_t _end;
    ///> Transaction IDs that keep a lock on this node.
    std::set<uint64_t> *_tids;

  private:
    ///> Height of left subtree (root is the left child) of this node.
    int _left_height;
    ///> Height of right subtree (root is the right child) of this node.
    int _right_height;
    ///> Parent of this node. Null if node is the root.
    AVLTreeLockNode *_parent;
    ///> Left child of this node. Null if there is no left child.
    AVLTreeLockNode *_left;
    ///> Right child of this node. Null if there is no left child.
    AVLTreeLockNode *_right;

  public:
    /**
     * Constructor of AVLTreeLockNode. Initializes the tree with a root node with time interval [0,MAX_UINT].
     */
    AVLTreeLockNode();

    /**
     * Constructor of AVLTreeLockNode. It copies all the fields from another AVLTreeLock.
     * @param lock  - AVLTreeLockNode to copy from.
     */
    AVLTreeLockNode(AVLTreeLockNode *lock);

    /**
     * Destructor of AVLTreeLockNode. It destroys all transaction IDs that hold for this interval.
     */
    ~AVLTreeLockNode();

  private:
    /**
     * Finds the root of the AVLTreeLock.
     * @return  - Root of AVLTreeLock
     */
    AVLTreeLockNode *getRoot();

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
    void find(std::vector<AVLTreeLockNode*> *locks, uint64_t ts);

    /**
     * Insert a new node according to AVL tree rules. The nodes are placed in respect to the starting point of their
     * time interval.
     * @param new_lock  - AVLTreeLock to be inserted in the tree.
     * @return          - Root of the corresponding AVLTreeLock.
     */
    AVLTreeLockNode *insert(AVLTreeLockNode *new_lock);

    /**
     * Remove this node from AVLTreeLock.
     * @return  - Root of the corresponding AVLTreeLock.
     */
    AVLTreeLockNode *remove();

    /**
     * Serialize the fields of AVLTreeLock as a string.
     * @return  - Representative string for this AVLTreeLock.
     */
    std::string toString();
};

#endif /* AVLTREELOCKNODE_H_ */
