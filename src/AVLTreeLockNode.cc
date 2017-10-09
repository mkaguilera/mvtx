/*
 * AVLTreeLockNode.cc
 *
 *  Created on: Jul 25, 2016
 *      Author: theo
 */

#include <cassert>
#include <iostream>
#include <limits>
#include <vector>

#include "AVLTreeLockNode.h"

AVLTreeLockNode::AVLTreeLockNode() {
  _status = FREE;
  _start = 0;
  _end = UINTMAX_MAX;
  _tids = new std::set<uint64_t> ();
  _parent = nullptr;
  _left = nullptr;
  _right = nullptr;
  _left_height = 0;
  _right_height = 0;
}

AVLTreeLockNode::AVLTreeLockNode(AVLTreeLockNode *lock) {
  _status = lock->_status;
  _start = lock->_start;
  _end = lock->_end;
  _tids = new std::set<uint64_t> (lock->_tids->begin(), lock->_tids->end());
  _parent = nullptr;
  _left = nullptr;
  _right = nullptr;
  _left_height = 0;
  _right_height = 0;
}

AVLTreeLockNode::~AVLTreeLockNode() {
  delete _tids;
}

AVLTreeLockNode *AVLTreeLockNode::getRoot() {
  if (_parent != nullptr)
    return (_parent->getRoot());
  return (this);
}

void AVLTreeLockNode::updateHeight() {
  _left_height =
      _left == nullptr ?
          0 : (_left->_left_height >= _left->_right_height ? _left->_left_height + 1 : _left->_right_height + 1);
  _right_height =
      _right == nullptr ?
          0 : (_right->_left_height >= _right->_right_height ? _right->_left_height + 1 : _right->_right_height + 1);
}

void AVLTreeLockNode::swap(bool is_left) {
  AVLTreeLockNode *child = is_left ? _left : _right;
  std::string child_str = is_left ? "left" : "right";

  if (child == nullptr) {
    std::cerr << "AVLTreeLock Error: Swap of " + child_str + " child of node:" << std::endl << toString() << std::endl;
    return;
  }
  child->_parent = _parent;
  if (_parent != nullptr) {
    if (_parent->_left == this)
      _parent->_left = child;
    else
      _parent->_right = child;
  }
  _parent = child;
  if (is_left) {
    _left = child->_right;
    if (_left != nullptr)
      _left->_parent = this;
    child->_right = this;
  } else {
    _right = child->_left;
    if (_right != nullptr)
      _right->_parent = this;
    child->_left = this;
  }
  updateHeight();
  child->updateHeight();
}

void AVLTreeLockNode::balance() {
  AVLTreeLockNode *parent = _parent;
  int diff;

  updateHeight();
  diff = _right_height - _left_height;
  if (diff < -1) {
    if (_left->_right_height - _left->_left_height == 1) {
      _left->swap(false);
    } else if (_left->_right_height - _left->_left_height < -1) {
      std::cerr << "AVLTreeLock Error: Balance factor is " << _left->_right_height - _left->_left_height << ". It "
                << "should be greater or equal to -1." << std::endl;
      return;
    }
    swap(true);
  } else if (diff > 1) {
    if (_right->_right_height - _right->_left_height == -1) {
      _right->swap(true);
    } else if (_right->_right_height - _right->_left_height > 1) {
      std::cerr << "AVLTreeLock Error: Balance factor is " << _right->_right_height - _right->_left_height << ". It "
                << "should be greater or equal to -1." << std::endl;
      return;
    }
    swap(false);
  }
  if (parent != nullptr)
    parent->balance();
}

void AVLTreeLockNode::find(std::vector<AVLTreeLockNode*> *locks, uint64_t ts) {
  if (ts < _start) {
    if (_left != nullptr)
      _left->find(locks,ts);
  } else if (ts > _end) {
    if (_right != nullptr) {
      locks->push_back(this);
      _right->find(locks,ts);
    }
  } else {
    locks->push_back(this);
  }
}

AVLTreeLockNode *AVLTreeLockNode::insert(AVLTreeLockNode *new_lock) {
  if (new_lock->_start < _start) {
    if (_left == nullptr) {
      _left = new_lock;
      new_lock->_parent = this;
      balance();
    } else {
      return (_left->insert(new_lock));
    }
  } else {
    if (_right == nullptr) {
      _right = new_lock;
      new_lock->_parent = this;
      balance();
    } else {
      return (_right->insert(new_lock));
    }
  }
  return (getRoot());
}

AVLTreeLockNode *AVLTreeLockNode::remove() {
  if (_left != nullptr) {
    AVLTreeLockNode *new_lock = _left;
    AVLTreeLockNode *parent;

    while (new_lock->_right != nullptr)
      new_lock = new_lock->_right;
    if (new_lock->_parent == this) {
      if (_parent->_left == this)
        _parent->_left = new_lock;
      else
        _parent->_right = new_lock;
      new_lock->_parent = _parent;
      if (_right != nullptr)
        _right->_parent = new_lock;
      new_lock->_right = _right;
      new_lock->balance();
    } else {
      parent = new_lock->_parent;
      if (new_lock->_left != nullptr)
        new_lock->_left->_parent = parent;
      parent->_right = new_lock->_left;
      parent->balance();
      new_lock->_parent = _parent;
      new_lock->_left = _left;
      new_lock->_right = _right;
      new_lock->_left_height = _left_height;
      new_lock->_right_height = _right_height;
      if (_parent != nullptr) {
        if (_parent->_left == this)
          _parent->_left = new_lock;
        else
          _parent->_right = new_lock;
      }
      if (_left != nullptr)
        _left->_parent = new_lock;
      if (_right != nullptr)
        _right->_parent = new_lock;
    }
  } else if (_right != nullptr) {
    if (_parent != nullptr) {
      if (_parent->_left == this)
        _parent->_left = _right;
      else
        _parent->_right = _right;
      _right->_parent = _parent;
    }
    _parent->balance();
  } else {
    if (_parent != nullptr) {
      if (_parent->_left == this)
        _parent->_left = nullptr;
      else
        _parent->_right = nullptr;
    }
    _parent->balance();
  }

  return (getRoot());
}

std::string AVLTreeLockNode::toString() {
  std::string res = "";

  res += "Interval: [" + std::to_string(_start) + "," + std::to_string(_end) + "]\n";
  res += "Status: ";
  switch (_status) {
    case FREE:
      res += "FREE";
      break;
    case READ_LOCK:
      res += "READ_LOCK";
      break;
    case WRITE_LOCK:
      res += "WRITE_LOCK";
      break;
    case FREEZE_READ_LOCK:
      res += "FREEZE_READ_LOCK";
      break;
    case FREEZE_WRITE_LOCK:
      res += "FREEZE_WRITE_LOCK";
      break;
    default:
      std::cerr << "Status is necessary." << std::endl;
      exit(-1);
  }
  res += "\n";
  res += "Height: (" + std::to_string(_left_height) + "," + std::to_string(_right_height) + ")\n";
  if (_left != nullptr)
    res += "\n" + _left->toString();
  if (_right != nullptr)
    res += "\n" + _right->toString();
  return (res);
}
