/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __HAMMERTREE_H__
#define __HAMMERTREE_H__

#define HAMMERTREE_DEBUG

typedef struct _hammernode
{
    int    n_index;
    float  n_value;
    int    n_black;
    struct _hammernode  *n_left;
    struct _hammernode  *n_right;
    struct _hammernode  *n_parent;
    struct _hammernode  *n_prev;
    struct _hammernode  *n_next;
} t_hammernode;

typedef struct _hammertree
{
    t_hammernode  *t_root;
    t_hammernode  *t_first;
    t_hammernode  *t_last;
} t_hammertree;

t_hammernode *hammertree_insert(t_hammertree *tree, int ndx);
void hammertree_delete(t_hammertree *tree, t_hammernode *np);
t_hammernode *hammertree_search(t_hammertree *tree, int ndx);
t_hammernode *hammertree_closest(t_hammertree *tree, int ndx, int geqflag);
void hammertree_init(t_hammertree *tree, int freecount);
void hammertree_clear(t_hammertree *tree, int freecount);
void hammertree_debug(t_hammertree *tree, int level);

#endif
