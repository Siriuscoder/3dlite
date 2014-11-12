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
#include <3dlite/rb_tree.h>
#include <3dlite/alloc.h>

#include <string.h>
#include <assert.h>

int lite3d_rb_tree_c_string_comparator(const void* a, const void* b)
{
    return strcmp((const char *) a, (const char *) b);
}

lite3d_rb_tree* lite3d_rb_tree_create(lite3d_rb_tree_comparator comparator,
    lite3d_rb_node_free deleter)
{
    lite3d_rb_tree* newTree;
    lite3d_rb_node* temp;

    newTree = (lite3d_rb_tree*) lite3d_malloc(sizeof (lite3d_rb_tree));
    newTree->compare = comparator;
    newTree->free = deleter;
    /* out of memory */
    assert(newTree != NULL);

    /*  see the comment in the lite3d_rb_tree structure in red_black_tree.h */
    /*  for information on nil and root */
    temp = newTree->nil = &newTree->nilEntity;
    temp->parent = temp->left = temp->right = temp;
    temp->red = 0;
    temp->key = 0;
    temp = newTree->root = &newTree->rootEntity;
    temp->parent = temp->left = temp->right = newTree->nil;
    temp->key = 0;
    temp->red = 0;
    return (newTree);
}

/***********************************************************************/
/*  FUNCTION:  LeftRotate */
/**/
/*  INPUTS:  This takes a tree so that it can access the appropriate */
/*           root and nil pointers, and the node to rotate on. */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input: tree, x */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly. */

/***********************************************************************/

static void lite3d_rb_tree_left_rotate(lite3d_rb_tree* tree, lite3d_rb_node* x)
{
    lite3d_rb_node* y;
    lite3d_rb_node* nil = tree->nil;

    /*  I originally wrote this function to use the sentinel for */
    /*  nil to avoid checking for nil.  However this introduces a */
    /*  very subtle bug because sometimes this function modifies */
    /*  the parent pointer of nil.  This can be a problem if a */
    /*  function which calls LeftRotate also uses the nil sentinel */
    /*  and expects the nil sentinel's parent pointer to be unchanged */
    /*  after calling this function.  For example, when RBDeleteFixUP */
    /*  calls LeftRotate it expects the parent pointer of nil to be */
    /*  unchanged. */

    y = x->right;
    x->right = y->left;

    if (y->left != nil) y->left->parent = x; /* used to use sentinel here */
    /* and do an unconditional assignment instead of testing for nil */

    y->parent = x->parent;

    /* instead of checking if x->parent is the root as in the book, we */
    /* count on the root sentinel to implicitly take care of this case */
    if (x == x->parent->left)
    {
        x->parent->left = y;
    }
    else
    {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}


/***********************************************************************/
/*  FUNCTION:  RighttRotate */
/**/
/*  INPUTS:  This takes a tree so that it can access the appropriate */
/*           root and nil pointers, and the node to rotate on. */
/**/
/*  OUTPUT:  None */
/**/
/*  Modifies Input?: tree, y */
/**/
/*  EFFECTS:  Rotates as described in _Introduction_To_Algorithms by */
/*            Cormen, Leiserson, Rivest (Chapter 14).  Basically this */
/*            makes the parent of x be to the left of x, x the parent of */
/*            its parent before the rotation and fixes other pointers */
/*            accordingly. */

/***********************************************************************/

static void lite3d_rb_tree_right_rotate(lite3d_rb_tree* tree, lite3d_rb_node* y)
{
    lite3d_rb_node* x;
    lite3d_rb_node* nil = tree->nil;

    /*  I originally wrote this function to use the sentinel for */
    /*  nil to avoid checking for nil.  However this introduces a */
    /*  very subtle bug because sometimes this function modifies */
    /*  the parent pointer of nil.  This can be a problem if a */
    /*  function which calls LeftRotate also uses the nil sentinel */
    /*  and expects the nil sentinel's parent pointer to be unchanged */
    /*  after calling this function.  For example, when RBDeleteFixUP */
    /*  calls LeftRotate it expects the parent pointer of nil to be */
    /*  unchanged. */

    x = y->left;
    y->left = x->right;

    if (nil != x->right) x->right->parent = y; /*used to use sentinel here */
    /* and do an unconditional assignment instead of testing for nil */

    /* instead of checking if x->parent is the root as in the book, we */
    /* count on the root sentinel to implicitly take care of this case */
    x->parent = y->parent;
    if (y == y->parent->left)
    {
        y->parent->left = x;
    }
    else
    {
        y->parent->right = x;
    }
    x->right = y;
    y->parent = x;
}

/***********************************************************************/
/*  FUNCTION:  TreeInsertHelp  */
/**/
/*  INPUTS:  tree is the tree to insert into and z is the node to insert */
/**/
/*  OUTPUT:  none */
/**/
/*  Modifies Input:  tree, z */
/**/
/*  EFFECTS:  Inserts z into the tree as if it were a regular binary tree */
/*            using the algorithm described in _Introduction_To_Algorithms_ */
/*            by Cormen et al.  This funciton is only intended to be called */
/*            by the RBTreeInsert function and not by the user */

/***********************************************************************/

static void lite3d_rb_tree_insert_help(lite3d_rb_tree* tree, lite3d_rb_node* z)
{
    /*  This function should only be called by InsertRBTree (see above) */
    lite3d_rb_node* x;
    lite3d_rb_node* y;
    lite3d_rb_node* nil = tree->nil;

    z->left = z->right = nil;
    y = tree->root;
    x = tree->root->left;
    while (x != nil)
    {
        y = x;
        if (1 == tree->compare(x->key, z->key))
        { /* x.key > z.key */
            x = x->left;
        }
        else
        { /* x,key <= z.key */
            x = x->right;
        }
    }
    z->parent = y;
    if ((y == tree->root) ||
        (1 == tree->compare(y->key, z->key)))
    { /* y.key > z.key */
        y->left = z;
    }
    else
    {
        y->right = z;
    }
}

/*  Before calling Insert RBTree the node x should have its key set */

lite3d_rb_node *lite3d_rb_tree_insert(lite3d_rb_tree* tree, lite3d_rb_node *newNode)
{
    lite3d_rb_node * y;
    lite3d_rb_node * x = newNode;

    lite3d_rb_tree_insert_help(tree, x);
    x->red = 1;
    while (x->parent->red)
    { /* use sentinel instead of checking for root */
        if (x->parent == x->parent->parent->left)
        {
            y = x->parent->parent->right;
            if (y->red)
            {
                x->parent->red = 0;
                y->red = 0;
                x->parent->parent->red = 1;
                x = x->parent->parent;
            }
            else
            {
                if (x == x->parent->right)
                {
                    x = x->parent;
                    lite3d_rb_tree_left_rotate(tree, x);
                }
                x->parent->red = 0;
                x->parent->parent->red = 1;
                lite3d_rb_tree_right_rotate(tree, x->parent->parent);
            }
        }
        else
        { /* case for x->parent == x->parent->parent->right */
            y = x->parent->parent->left;
            if (y->red)
            {
                x->parent->red = 0;
                y->red = 0;
                x->parent->parent->red = 1;
                x = x->parent->parent;
            }
            else
            {
                if (x == x->parent->left)
                {
                    x = x->parent;
                    lite3d_rb_tree_right_rotate(tree, x);
                }
                x->parent->red = 0;
                x->parent->parent->red = 1;
                lite3d_rb_tree_left_rotate(tree, x->parent->parent);
            }
        }
    }
    tree->root->left->red = 0;
    return (newNode);
}

lite3d_rb_node* lite3d_rb_tree_successor(lite3d_rb_tree* tree, lite3d_rb_node* x)
{
    lite3d_rb_node* y;
    lite3d_rb_node* nil = tree->nil;
    lite3d_rb_node* root = tree->root;

    if (nil != (y = x->right))
    { /* assignment to y is intentional */
        while (y->left != nil)
        { /* returns the minium of the right subtree of x */
            y = y->left;
        }
        return (y);
    }
    else
    {
        y = x->parent;
        while (x == y->right)
        { /* sentinel used instead of checking for nil */
            x = y;
            y = y->parent;
        }
        if (y == root) return (nil);
        return (y);
    }
}

lite3d_rb_node* lite3d_rb_tree_predecessor(lite3d_rb_tree* tree, lite3d_rb_node* x)
{
    lite3d_rb_node* y;
    lite3d_rb_node* nil = tree->nil;
    lite3d_rb_node* root = tree->root;

    if (nil != (y = x->left))
    { /* assignment to y is intentional */
        while (y->right != nil)
        { /* returns the maximum of the left subtree of x */
            y = y->right;
        }
        return (y);
    }
    else
    {
        y = x->parent;
        while (x == y->left)
        {
            if (y == root) return (nil);
            x = y;
            y = y->parent;
        }
        return (y);
    }
}

/***********************************************************************/
/*  FUNCTION:  TreeDestHelper */
/**/
/*    INPUTS:  tree is the tree to destroy and x is the current node */
/**/
/*    OUTPUT:  none  */
/**/
/*    EFFECTS:  This function recursively destroys the nodes of the tree */
/*              postorder using the DestroyKey and DestroyInfo functions. */
/**/
/*    Modifies Input: tree, x */
/**/
/*    Note:    This function should only be called by RBTreeDestroy */

/***********************************************************************/

static void lite3d_rb_tree_destroy_help(lite3d_rb_tree* tree, lite3d_rb_node* x)
{
    lite3d_rb_node* nil = tree->nil;
    if (x != nil)
    {
        lite3d_rb_tree_destroy_help(tree, x->left);
        lite3d_rb_tree_destroy_help(tree, x->right);
        tree->free(x);
    }
}

void lite3d_rb_tree_destroy(lite3d_rb_tree* tree)
{
    lite3d_rb_tree_destroy_help(tree, tree->root);
    lite3d_free(tree);
}

lite3d_rb_node* lite3d_rb_tree_exact_query(lite3d_rb_tree* tree, void* q)
{
    lite3d_rb_node* x = tree->root->left;
    lite3d_rb_node* nil = tree->nil;
    int compVal;
    if (x == nil) return (0);
    compVal = tree->compare(x->key, q);
    while (0 != compVal)
    {/*assignemnt*/
        if (1 == compVal)
        { /* x->key > q */
            x = x->left;
        }
        else
        {
            x = x->right;
        }
        if (x == nil) return (0);
        compVal = tree->compare(x->key, q);
    }
    return (x);
}


/***********************************************************************/
/*  FUNCTION:  RBDeleteFixUp */
/**/
/*    INPUTS:  tree is the tree to fix and x is the child of the spliced */
/*             out node in RBTreeDelete. */
/**/
/*    OUTPUT:  none */
/**/
/*    EFFECT:  Performs rotations and changes colors to restore red-black */
/*             properties after a node is deleted */
/**/
/*    Modifies Input: tree, x */
/**/
/*    The algorithm from this function is from _Introduction_To_Algorithms_ */

/***********************************************************************/

static void lite3d_rb_tree_delete_fixup(lite3d_rb_tree* tree, lite3d_rb_node* x)
{
    lite3d_rb_node* root = tree->root->left;
    lite3d_rb_node* w;

    while ((!x->red) && (root != x))
    {
        if (x == x->parent->left)
        {
            w = x->parent->right;
            if (w->red)
            {
                w->red = 0;
                x->parent->red = 1;
                lite3d_rb_tree_left_rotate(tree, x->parent);
                w = x->parent->right;
            }
            if ((!w->right->red) && (!w->left->red))
            {
                w->red = 1;
                x = x->parent;
            }
            else
            {
                if (!w->right->red)
                {
                    w->left->red = 0;
                    w->red = 1;
                    lite3d_rb_tree_right_rotate(tree, w);
                    w = x->parent->right;
                }
                w->red = x->parent->red;
                x->parent->red = 0;
                w->right->red = 0;
                lite3d_rb_tree_left_rotate(tree, x->parent);
                x = root; /* this is to exit while loop */
            }
        }
        else
        { /* the code below is has left and right switched from above */
            w = x->parent->left;
            if (w->red)
            {
                w->red = 0;
                x->parent->red = 1;
                lite3d_rb_tree_right_rotate(tree, x->parent);
                w = x->parent->left;
            }
            if ((!w->right->red) && (!w->left->red))
            {
                w->red = 1;
                x = x->parent;
            }
            else
            {
                if (!w->left->red)
                {
                    w->right->red = 0;
                    w->red = 1;
                    lite3d_rb_tree_left_rotate(tree, w);
                    w = x->parent->left;
                }
                w->red = x->parent->red;
                x->parent->red = 0;
                w->left->red = 0;
                lite3d_rb_tree_right_rotate(tree, x->parent);
                x = root; /* this is to exit while loop */
            }
        }
    }
    x->red = 0;
}

void lite3d_rb_tree_delete(lite3d_rb_tree* tree, lite3d_rb_node* z)
{
    lite3d_rb_node* y;
    lite3d_rb_node* x;
    lite3d_rb_node* nil = tree->nil;
    lite3d_rb_node* root = tree->root;

    y = ((z->left == nil) || (z->right == nil)) ? z : lite3d_rb_tree_successor(tree, z);
    x = (y->left == nil) ? y->right : y->left;
    if (root == (x->parent = y->parent))
    { /* assignment of y->p to x->p is intentional */
        root->left = x;
    }
    else
    {
        if (y == y->parent->left)
        {
            y->parent->left = x;
        }
        else
        {
            y->parent->right = x;
        }
    }
    if (y != z)
    { /* y should not be nil in this case */
        /* y is the node to splice out and x is its child */

        if (!(y->red)) lite3d_rb_tree_delete_fixup(tree, x);

        y->left = z->left;
        y->right = z->right;
        y->parent = z->parent;
        y->red = z->red;
        z->left->parent = z->right->parent = y;
        if (z == z->parent->left)
        {
            z->parent->left = y;
        }
        else
        {
            z->parent->right = y;
        }

        tree->free(z);
    }
    else
    {
        if (!(y->red)) lite3d_rb_tree_delete_fixup(tree, x);
        tree->free(z);
    }
}

static void lite3d_rb_tree_iterate_help(lite3d_rb_tree* tree, 
    lite3d_rb_node* x, lite3d_rb_node_iter func)
{
    lite3d_rb_node* nil = tree->nil;
    if (x != nil)
    {
        lite3d_rb_tree_destroy_help(tree, x->left);
        lite3d_rb_tree_destroy_help(tree, x->right);
        func(tree, x);
    }
}

void lite3d_rb_tree_iterate(lite3d_rb_tree *tree, lite3d_rb_node_iter func)
{
    lite3d_rb_tree_iterate_help(tree, tree->root, func);   
}