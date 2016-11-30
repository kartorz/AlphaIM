/** 
 *	@Copyright 2013  AlphaDict
 *	@Authors: LiQiong Lee
 *
 * Distributed under the GNU GENERAL PUBLIC LICENSE, version 3 (GPLv3)
 * (See accompanying file LICENSE.txt or copy at
 * http://www.gnu.org/licenses/gpl.txt)
 *
 */
#ifndef _KARY_TREE_HPP_
#define _KARY_TREE_HPP_

#include "tree_node.hpp"
#include "tree_node_1.hpp"
#include "tree_node_2.hpp"
#include <stdio.h>

namespace ktree {
template <typename T>
class kary_tree {
	typedef typename tree_node<T>::treeNodePtr  treeNodePtr;
public:
    kary_tree():_root(NULL)
    {
    }

	kary_tree(const T& d, int type = 2)
	{
        _root = newNode(d, type);
	}

	~kary_tree() { remove(_root); }

    treeNodePtr newNode(const T& d, int type = 2)
    {
        treeNodePtr ret;
        if (type == 1)
            ret = new tree_node_1<T>(d);
        else
            ret = new tree_node_2<T>(d);
        return ret;
    }

	treeNodePtr root() const { return _root; }

    // Keep _root
	void clear() const {
        tree_node<T> *child = _root->begin();
        while (child != NULL) {
            tree_node<T> *next = _root->next();
            remove(child);
            child = next;
        }
    }

private:
	treeNodePtr _root;
};

template <typename T>
void erase(tree_node<T> *pTree)
{
  pTree->clear();
  delete pTree;
  pTree = NULL;
}

#if 0
template <typename T>
void remove(tree_node<T> *pTree)
{
    if(!pTree) return;

    if(!pTree->children().empty()) {
        typename std::list<tree_node<T> *>::const_iterator iter;
        const std::list<tree_node<T> *>& children = pTree->children();
        for (iter = children.begin();  iter != children.end(); ++iter) {
            remove(*iter);
        }
    }
    pTree->clear();
    delete pTree;
}
#else
template <typename T>
void remove(tree_node<T> *pTree)
{
    tree_node<T> *p = pTree->parent();
    if(p != NULL)
        p->remove(pTree); // Remove node from parent.

	traverse(pTree, erase);
}
#endif
template <typename T>
void traverse(tree_node<T> *pTree, void(*visit)(tree_node<T> *pTree))
{
	if(!pTree) return;
    if (pTree->size() == 0) return;

    tree_node<T> *child = pTree->begin();
    while (child != NULL) {
        tree_node<T> *next = pTree->next();
        traverse(child, visit);
        child = next;
    }

	visit(pTree);
}

};
#endif
