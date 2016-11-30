/** 
 *	@Copyright 2013  AlphaDict
 *	@Authors: LiQiong Lee
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */
#ifndef _TREE_NODE_HPP_
#define _TREE_NODE_HPP_

#include <list>
#include <iterator>

namespace ktree {

template <typename T>
class tree_node {

public:
	typedef tree_node<T>*  	           treeNodePtr;
	typedef const tree_node<T>*  const_treeNodePtr;

	treeNodePtr parent() const
	{
		return _parent;
	}

	T& value()
	{
		return _data;
	}

	void operator=(const T& d)
	{
		_data = d;
	}

    virtual int size() = 0;

	virtual treeNodePtr operator[](const int pos) const = 0;
	
	virtual treeNodePtr child(const int pos) const = 0;
	
	virtual treeNodePtr insert(const T& d, int pos=-1/*pos:begin(0);end(-1);*/) = 0;

	virtual void clear() = 0;

    virtual treeNodePtr begin() = 0;

    virtual treeNodePtr next() = 0;
    
    virtual void remove(int pos) = 0;

    virtual void remove(treeNodePtr pTree) = 0;

	treeNodePtr  _parent = NULL;
    T _data;
};

};

#endif
