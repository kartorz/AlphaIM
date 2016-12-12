/** 
 *	@Copyright 2013  AlphaDict
 *	@Authors: LiQiong Lee
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */
#ifndef _TREE_NODE_1_HPP_
#define _TREE_NODE_1_HPP_

#include <list>
#include <iterator>

#include "tree_node.hpp"

namespace ktree {

#ifdef _LINUX
template<class ForwardIt>
ForwardIt next(ForwardIt it, typename std::iterator_traits<ForwardIt>::difference_type n = 1)
{
    std::advance(it, n);
    return it;
}
#endif

template <typename T> class tree_node;
template <typename T> class kary_tree;

template <typename T>
class tree_node_1 : public tree_node<T> {
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
		return child(pos);
	}
	
	treeNodePtr child(const int pos) const
	{
		typename std::list<treeNodePtr>::const_iterator iter;
    #ifdef WIN32
        iter = std::next(_children.begin(), pos);
    #else
		iter = ktree::next(_children.begin(), pos);
    #endif
		return *iter;
	}
	
	treeNodePtr insert(const T& d, int pos=-1/*pos:begin(0);end(-1);*/)
 	{
		treeNodePtr pTree = new tree_node_1<T>(d);
		if (pos == -1)
			_children.push_back(pTree);
		else if(pos == 0)
			_children.push_front(pTree);
		else {
			typename std::list<treeNodePtr>::iterator iter;
#ifdef WIN32
			iter = std::next(_children.begin(), pos);
#else
            iter = ktree::next(_children.begin(), pos);
#endif
			_children.insert(iter, pTree);
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
        typename std::list<treeNodePtr>::iterator iter;
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
        typename std::list<treeNodePtr>::iterator iter;
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
	tree_node_1(const T& d) {this->_data = d;}
	tree_node_1();
	//treeNodePtr  _parent;
    //T _data;

	std::list<treeNodePtr> _children;
    typename std::list<treeNodePtr>::const_iterator _iter;
};

};

#endif
