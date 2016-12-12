/** 
 *	@Copyright 2013  AlphaDict
 *	@Authors: LiQiong Lee
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */
#ifndef _TREE_NODE_2_HPP_
#define _TREE_NODE_2_HPP_

#include <vector>
#include <iterator>

#include "tree_node.hpp"

namespace ktree {

template <typename T> class tree_node;
template <typename T> class kary_tree;

template <typename T>
class tree_node_2 : public tree_node<T> {
	friend class kary_tree<T>;
public:
	typedef tree_node<T>*  	           treeNodePtr;
	typedef const tree_node<T>*  const_treeNodePtr;

    int size()
    {
        return _children.size();
    }

	treeNodePtr operator[](const int pos) const
	{
        return _children[pos];
	}
	
	treeNodePtr child(const int pos) const
	{
        return _children[pos];
	}
	
	treeNodePtr insert(const T& d, int pos=-1/*pos:begin(0);end(-1);*/)
    {
		treeNodePtr pTree = new tree_node_2<T>(d);
		if (pos == -1)
			_children.push_back(pTree);
		else {
            typename std::vector<treeNodePtr>::iterator begin = _children.begin();
			_children.insert(begin+pos, pTree);
		}
		pTree->_parent = this;
		return pTree;
	}

	void clear()
	{
        _children.clear();
	}

    treeNodePtr begin()
    {
        if (_children.size() == 0)
            return NULL;

        _iter = _children.begin();
        return *_iter;
    }

    treeNodePtr next()
    {
        if (++_iter != _children.end())
            return *_iter;
        return NULL;
    }

    virtual void remove(int pos)
    {
        typename std::vector<treeNodePtr>::iterator iter;
        int i = 0;
        for (iter = _children.begin();
            iter != _children.end();
	         ++iter) {
             if (i++ == pos) {
                 _children.erase(iter);
                 break;
             }
	     }
    }

    virtual void remove(treeNodePtr tree)
    {
        typename std::vector<treeNodePtr>::iterator iter;
        for (iter = _children.begin(); 
             iter != _children.end();
	         ++iter) {
             if (tree == *iter) {
                 _children.erase(iter);
                 break;
             }
	     }
    }

private:
	tree_node_2(const T& d) {this->_data = d;}
	tree_node_2();

	std::vector<treeNodePtr> _children;
    typename std::vector<treeNodePtr>::const_iterator _iter;
};

};

#endif
