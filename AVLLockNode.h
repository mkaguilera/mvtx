/**
 * AVLTreeLock.h
 *
 *  Created on: Jul 25, 2016
 *      Author: theo
 */

#ifndef AVLLOCKNODE_H_
#define AVLLOCKNODE_H_

#include <cstdint>
#include <set>
#include <string>
#include <vector>

/**
 * Node implementation for the AVLLockTree implementing the logic behind the AVLLockManager.
 */
class AVLLockNode
{
  public:

    /**
     * Status of an AVLTreeLock node.
     */
    enum Status
    {
      FREE, READ_LOCK, WRITE_LOCK, FREEZE_READ_LOCK, FREEZE_WRITE_LOCK
    };

    Status _status;               ///> Status of the current node.
    uint64_t _start;              ///> Starting point of associated time interval.
    uint64_t _end;                ///> Ending point of associated time interval.
    std::set<uint64_t> *_tids;    ///> Transactions that have locked the node. Only filled when status is READ_LOCK or
                                  ///  WRITE_LOCK.
    std::set<void *> *_events;    ///> Events that should be triggered if node is frozen or unlocked. Only filled when
                                  ///  status is WRITE_LOCK.

  private:
    int _left_height;             ///> Height of left subtree (root is the left child) of this node.
    int _right_height;            ///> Height of right subtree (root is the right child) of this node.
    AVLLockNode *_parent;         ///> Parent of this node. Null if node is the root.
    AVLLockNode *_left;           ///> Left child of this node. Null if there is no left child.
    AVLLockNode *_right;          ///> Right child of this node. Null if there is no left child.

  public:
    ///> Constructors
    AVLLockNode();
    AVLLockNode(AVLLockNode *lock);
    ///> Destructor
    ~AVLLockNode();

  private:
    /**
     * Finds the root of the ALVLockTree in which this node belongs to.
     * @return  - root of AVLLockTree
     */
    AVLLockNode *getRoot();

    /**
     * Updates the left and right heights of this node. In case of change updates the predecessors as well.
     */
    void updateHeight();

    /**
     * Balance the sub-tree with this node as a root according to AVL tree rules. The tree is balanced in respect to
     * the starting point of their time interval.
     */
    void balance();

  public:
    /**
     * Finds the path from this node to node which contains a specific time value in its time interval. Maintains all
     * nodes with lower starting point than this time value.
     * @param locks - Vector to add the corresponding nodes
     * @param ts    - Time value
     */
    void find(std::vector<AVLLockNode*> *locks, uint64_t ts);

    /**
     * Creates and initializes an AVLLockTree. There are two nodes, one which has FREEZE_WRITE_LOCK status and [0,0]
     * interval and another which has FREE status and [1,MAX_INT] interval.
     * @return root of the newly constructed tree.
     */
    static AVLLockNode *createLockTree();

    /**
     * Insert a new node according to AVL tree rules. The nodes are placed in respect to the starting point of their
     * time interval.
     * @param new_lock  - AVLLockNode to be inserted in the tree
     * @return          - Root of the corresponding AVLLockTree
     */
    AVLLockNode *insert(AVLLockNode *new_lock);

    /**
     * Remove this node from AVLLockTree.
     */
    void remove();

    /**
     *
     * @param parent
     * @param child
     * @param is_left
     */
    static void swap(AVLLockNode *parent, AVLLockNode *child, bool is_left);
    std::string toString();
};

#endif /* AVLLOCKNODE_H_ */
