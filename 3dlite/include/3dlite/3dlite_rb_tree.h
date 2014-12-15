/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
 *
 *	Foobar is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Foobar is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#ifndef LIGHT3D_RB_TREE_H
#define	LIGHT3D_RB_TREE_H

#include <3dlite/3dlite_common.h>

typedef struct lite3d_rb_node {
    void* key;
    uint8_t red; /* if red=0 then the node is black */
    struct lite3d_rb_node* left;
    struct lite3d_rb_node* right;
    struct lite3d_rb_node* parent;
} lite3d_rb_node;

typedef int (*lite3d_rb_tree_comparator)(const void* a, const void* b);
typedef void (*lite3d_rb_node_free)(lite3d_rb_node *x);

/* Compare(a,b) should return 1 if *a > *b, -1 if *a < *b, and 0 otherwise */

/* Destroy(a) takes a pointer to whatever key might be and frees it accordingly */
typedef struct lite3d_rb_tree {
    lite3d_rb_tree_comparator compare;
    lite3d_rb_node_free free;
    /*  A sentinel is used for root and for nil.  These sentinels are */
    /*  created when RBTreeCreate is caled.  root->left should always */
    /*  point to the node which is the root of the tree.  nil points to a */
    /*  node which should always be black but has aribtrary children and */
    /*  parent and no key or info.  The point of using these sentinels is so */
    /*  that the root and nil nodes do not require special cases in the code */
    lite3d_rb_node nilEntity;
    lite3d_rb_node rootEntity;
    lite3d_rb_node* root;
    lite3d_rb_node* nil;
} lite3d_rb_tree;

lite3d_rb_tree* lite3d_rb_tree_create(lite3d_rb_tree_comparator comparator,
    lite3d_rb_node_free deleter);

/***********************************************************************/
/*  FUNCTION:  RBTreeInsert */
/**/
/*  INPUTS:  tree is the red-black tree to insert a node which has a key */
/*           pointed to by key and info pointed to by info.  */
/**/
/*  OUTPUT:  This function returns a pointer to the newly inserted node */
/*           which is guarunteed to be valid until this node is deleted. */
/*           What this means is if another data structure stores this */
/*           pointer then the tree does not need to be searched when this */
/*           is to be deleted. */
/**/
/*  Modifies Input: tree */
/**/
/*  EFFECTS:  Creates a node node which contains the appropriate key and */
/*            info pointers and inserts it into the tree. */
/***********************************************************************/
lite3d_rb_node * lite3d_rb_tree_insert(lite3d_rb_tree *tree, lite3d_rb_node *newNode);

/***********************************************************************/
/*  FUNCTION:  RBDelete */
/**/
/*    INPUTS:  tree is the tree to delete node z from */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  Deletes z from tree and frees the key and info of z */
/*             using DestoryKey and DestoryInfo.  Then calls */
/*             RBDeleteFixUp to restore red-black properties */
/**/
/*    Modifies Input: tree, z */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */
/***********************************************************************/
void lite3d_rb_tree_delete(lite3d_rb_tree *tree, lite3d_rb_node *x);

/***********************************************************************/
/*  FUNCTION:  RBTreeDestroy */
/**/
/*    INPUTS:  tree is the tree to destroy */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  Destroys the key and frees memory */
/**/
/*    Modifies Input: tree */
/**/
/***********************************************************************/
void lite3d_rb_tree_destroy(lite3d_rb_tree *tree);

/***********************************************************************/
/*  FUNCTION:  Treepredecessor  */
/**/
/*    INPUTS:  tree is the tree in question, and x is the node we want the */
/*             the predecessor of. */
/**/
/*    OUTPUT:  This function returns the predecessor of x or NULL if no */
/*             predecessor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/
lite3d_rb_node* lite3d_rb_tree_predecessor(lite3d_rb_tree *tree, lite3d_rb_node *x);

/***********************************************************************/
/*  FUNCTION:  TreeSuccessor  */
/**/
/*    INPUTS:  tree is the tree in question, and x is the node we want the */
/*             the successor of. */
/**/
/*    OUTPUT:  This function returns the successor of x or NULL if no */
/*             successor exists. */
/**/
/*    Modifies Input: none */
/**/
/*    Note:  uses the algorithm in _Introduction_To_Algorithms_ */
/***********************************************************************/
lite3d_rb_node* lite3d_rb_tree_successor(lite3d_rb_tree *tree, lite3d_rb_node *x);

/***********************************************************************/
/*  FUNCTION:  RBExactQuery */
/**/
/*    INPUTS:  tree is the tree to print and q is a pointer to the key */
/*             we are searching for */
/**/
/*    OUTPUT:  returns the a node with key equal to q.  If there are */
/*             multiple nodes with key equal to q this function returns */
/*             the one highest in the tree */
/**/
/*    Modifies Input: none */
/**/
/***********************************************************************/
lite3d_rb_node* lite3d_rb_tree_exact_query(lite3d_rb_tree*, const void *q);

typedef void (*lite3d_rb_node_iter)(lite3d_rb_tree* tree, lite3d_rb_node *x);
void lite3d_rb_tree_iterate(lite3d_rb_tree *tree, lite3d_rb_node_iter func);

int lite3d_rb_tree_c_string_comparator(const void *a, const void *b);

#endif	/* RB_TREE_H */

