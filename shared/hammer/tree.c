/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "hammer/tree.h"

/* Since there is no sentinel node, the deletion routine has to have
   a few extra checks.  LATER rethink. */

/* LATER freelist */

#ifdef HAMMERTREE_DEBUG
/* returns bh or 0 if failed */
static int hammernode_verify(t_hammernode *np)
{
    if (np)
    {
	int bhl, bhr;
	if (((bhl = hammernode_verify(np->n_left)) == 0) ||
	    ((bhr = hammernode_verify(np->n_right)) == 0))
	    return (0);
	if (bhl != bhr)
	{
	    /* failure: two paths rooted in the same node
	       contain different number of black nodes */
	    bug("hammernode_verify: not balanced");
	    return (0);
	}
	if (np->n_black)
	    return (bhl + 1);
	else
	{
	    if ((np->n_left && !np->n_left->n_black) ||
		(np->n_right && !np->n_right->n_black))
	    {
		bug("hammernode_verify: adjacent red nodes");
		return (0);
	    }
	    return (bhl);
	}
    }
    else return (1);
}

/* returns bh or 0 if failed */
static int hammertree_verify(t_hammertree *tree)
{
    return (hammernode_verify(tree->t_root));
}

static void hammernode_post(t_hammernode *np)
{
    startpost("%d %g %d (", np->n_index, np->n_value, np->n_black);
    if (np->n_left)
	startpost("%d, ", np->n_left->n_index);
    else
	startpost("nul, ");
    if (np->n_right)
	post("%d)", np->n_right->n_index);
    else
	post("nul)");
}

/* this is a standard stackless traversal, not the best one, obviously...
   (used only for debugging) */
static int hammertree_traverse(t_hammertree *tree, int postit)
{
    t_hammernode *np = tree->t_root;
    int count = 0;
    while (np)
    {
	t_hammernode *prev = np->n_left;
	if (prev)
	{
	    while (prev->n_right && prev->n_right != np) prev = prev->n_right;
	    if (prev->n_right)
	    {
		prev->n_right = 0;
		if (postit) hammernode_post(np);
		count++;
		np = np->n_right;
	    }
	    else
	    {
		prev->n_right = np;
		np = np->n_left;
	    }
	}
	else
	{
	    if (postit) hammernode_post(np);
	    count++;
	    np = np->n_right;
	}
    }
    return (count);
}

static int hammernode_height(t_hammernode *np)
{
    if (np)
    {
	int lh = hammernode_height(np->n_left);
	int rh = hammernode_height(np->n_right);
	return (lh > rh ? lh + 1 : rh + 1);
    }
    else return (0);
}

void hammertree_debug(t_hammertree *tree, int level)
{
    t_hammernode *np;
    int count;
    post("------------------------");
    count = hammertree_traverse(tree, level);
    if (level > 1)
    {
	post("***");
	for (np = tree->t_last; np; np = np->n_prev)
	    startpost("%d ", np->n_index);
	endpost();
    }
    post("count %d, height %d, root %d:",
	 count, hammernode_height(tree->t_root),
	 (tree->t_root ? tree->t_root->n_index : 0));
    post("...verified (black-height is %d)", hammertree_verify(tree));
    post("------------------------");
}
#endif

/* assuming that target node (np->n_right) exists */
static void hammertree_lrotate(t_hammertree *tree, t_hammernode *np)
{
    t_hammernode *target = np->n_right;
    if (np->n_right = target->n_left)
	np->n_right->n_parent = np;
    if (!(target->n_parent = np->n_parent))
	tree->t_root = target;
    else if (np == np->n_parent->n_left)
	np->n_parent->n_left = target;
    else
	np->n_parent->n_right = target;
    target->n_left = np;
    np->n_parent = target;
}

/* assuming that target node (np->n_left) exists */
static void hammertree_rrotate(t_hammertree *tree, t_hammernode *np)
{
    t_hammernode *target = np->n_left;
    if (np->n_left = target->n_right)
	np->n_left->n_parent = np;
    if (!(target->n_parent = np->n_parent))
	tree->t_root = target;
    else if (np == np->n_parent->n_left)
	np->n_parent->n_left = target;
    else
	np->n_parent->n_right = target;
    target->n_right = np;
    np->n_parent = target;
}

/* returns a newly inserted or already existing node
   (or 0 if allocation failed) */
t_hammernode *hammertree_insert(t_hammertree *tree, int ndx)
{
    t_hammernode *np, *parent, *result;
    if (!(np = tree->t_root))
    {
	if (!(np = getbytes(sizeof(*np))))
	    return (0);
	np->n_index = ndx;
	np->n_black = 1;
	tree->t_root = tree->t_first = tree->t_last = np;
	return (np);
    }

    do
	if (np->n_index == ndx)
	    return (np);
	else
	    parent = np;
    while (np = (ndx < np->n_index ? np->n_left : np->n_right));

    if (!(np = getbytes(sizeof(*np))))
	return (0);
    np->n_index = ndx;
    np->n_parent = parent;
    if (ndx < parent->n_index)
    {
	parent->n_left = np;
	/* update the auxiliary linked list structure */
	np->n_next = parent;
	if (np->n_prev = parent->n_prev)
	    np->n_prev->n_next = np;
	else
	    tree->t_first = np;
	parent->n_prev = np;
    }
    else
    {
	parent->n_right = np;
	/* update the auxiliary linked list structure */
	np->n_prev = parent;
	if (np->n_next = parent->n_next)
	    np->n_next->n_prev = np;
	else
	    tree->t_last = np;
	parent->n_next = np;
    }
    result = np;

    /* balance the tree -- LATER clean this if possible... */
    np->n_black = 0;
    while (np != tree->t_root && !np->n_parent->n_black)
    {
	t_hammernode *uncle;
	/* np->n_parent->n_parent exists (we always paint root node in black) */
	if (np->n_parent == np->n_parent->n_parent->n_left)
	{
	    uncle = np->n_parent->n_parent->n_right;
	    if (!uncle  /* (sentinel not used) */
		|| uncle->n_black)
	    {
		if (np == np->n_parent->n_right)
		{
		    np = np->n_parent;
		    hammertree_lrotate(tree, np);
		}
		np->n_parent->n_black = 1;
		np->n_parent->n_parent->n_black = 0;
		hammertree_rrotate(tree, np->n_parent->n_parent);
	    }
	    else
	    {
		np->n_parent->n_black = 1;
		uncle->n_black = 1;
		np = np->n_parent->n_parent;
		np->n_black = 0;
	    }
	}
	else
	{
	    uncle = np->n_parent->n_parent->n_left;
	    if (!uncle  /* (sentinel not used) */
		|| uncle->n_black)
	    {
		if (np == np->n_parent->n_left)
		{
		    np = np->n_parent;
		    hammertree_rrotate(tree, np);
		}
		np->n_parent->n_black = 1;
		np->n_parent->n_parent->n_black = 0;
		hammertree_lrotate(tree, np->n_parent->n_parent);
	    }
	    else
	    {
		np->n_parent->n_black = 1;
		uncle->n_black = 1;
		np = np->n_parent->n_parent;
		np->n_black = 0;
	    }
	}
    }
    tree->t_root->n_black = 1;
    return (result);
}

/* assuming that requested node exists */
void hammertree_delete(t_hammertree *tree, t_hammernode *np)
{
    t_hammernode *gone, *parent, *child;
    /* gone is the actual node to be deleted
       -- it has to be the parent of no more than one child: */
    if (np->n_left && np->n_right)
    {
	gone = np->n_next;  /* gone always exists */
	child = gone->n_right;  /* there is no left child of gone */
	/* gone is not a requested node, so we replace fields to be
	   deleted with gone's fields: */
	np->n_index = gone->n_index;
	np->n_value = gone->n_value;
	/* update the auxiliary linked list structure */
	/* np->n_prev is up-to-date */
	if (np->n_prev)
	    np->n_prev->n_next = np;
	else tree->t_first = np;
	if (np->n_next = gone->n_next)
	    np->n_next->n_prev = np;
	else tree->t_last = np;
    }
    else
    {
	gone = np;
	if (gone->n_left)
	    child = gone->n_left;
	else
	    child = gone->n_right;
	/* update the auxiliary linked list structure */
	if (gone->n_prev)
	    gone->n_prev->n_next = gone->n_next;
	else
	    tree->t_first = gone->n_next;
	if (gone->n_next)
	    gone->n_next->n_prev = gone->n_prev;
	else
	    tree->t_last = gone->n_prev;
    }
    /* connect gone's child with gone's parent */
    if (!(parent = gone->n_parent))
    {
	if (tree->t_root = child)
	{
	    child->n_parent = 0;
	    child->n_black = 1;  /* LATER rethink */
	}
	goto done;
    }
    else
    {
	if (child)  /* (sentinel not used) */
	    child->n_parent = parent;
	if (gone == parent->n_left)
	    parent->n_left = child;
	else
	    parent->n_right = child;
    }

    if (gone->n_black)
    {
	/* balance the tree -- LATER clean this if possible... */
	/* on entry:  tree is not empty, parent always exists, child
	   not necessarily... */
	while (child != tree->t_root &&
	       (!child ||  /* (sentinel not used) */
		child->n_black))
	{
	    t_hammernode *other;  /* another child of the same parent */
	    if (child == parent->n_left)
	    {
		other = parent->n_right;
		if (other &&  /* (sentinel not used) */
		    !other->n_black)
		{
		    other->n_black = 1;
		    parent->n_black = 0;
		    hammertree_lrotate(tree, parent);
		    other = parent->n_right;
		}
		if (!other ||  /* (sentinel not used) */
		    (!other->n_left || other->n_left->n_black) &&
		    (!other->n_right || other->n_right->n_black))
		{
		    if (other)  /* (sentinel not used) */
			other->n_black = 0;
		    child = parent;
		    parent = parent->n_parent;
		}
		else
		{
		    if (!other ||  /* (sentinel not used) */
			!other->n_right || other->n_right->n_black)
		    {
			if (other)  /* (sentinel not used) */
			{
			    if (other->n_left) other->n_left->n_black = 1;
			    other->n_black = 0;
			    hammertree_rrotate(tree, other);
			    other = parent->n_right;
			}
		    }
		    if (other)  /* (sentinel not used) */
		    {
			if (other->n_right) other->n_right->n_black = 1;
			other->n_black = parent->n_black;
		    }
		    parent->n_black = 1;
		    hammertree_lrotate(tree, parent);
		    tree->t_root->n_black = 1;  /* LATER rethink */
		    goto done;
		}
	    }
	    else  /* right child */
	    {
		other = parent->n_left;
		if (other &&  /* (sentinel not used) */
		    !other->n_black)
		{
		    other->n_black = 1;
		    parent->n_black = 0;
		    hammertree_rrotate(tree, parent);
		    other = parent->n_left;
		}
		if (!other ||  /* (sentinel not used) */
		    (!other->n_left || other->n_left->n_black) &&
		    (!other->n_right || other->n_right->n_black))
		{
		    if (other)  /* (sentinel not used) */
			other->n_black = 0;
		    child = parent;
		    parent = parent->n_parent;
		}
		else
		{
		    if (!other ||  /* (sentinel not used) */
			!other->n_left || other->n_left->n_black)
		    {
			if (other)  /* (sentinel not used) */
			{
			    if (other->n_right) other->n_right->n_black = 1;
			    other->n_black = 0;
			    hammertree_lrotate(tree, other);
			    other = parent->n_left;
			}
		    }
		    if (other)  /* (sentinel not used) */
		    {
			if (other->n_left) other->n_left->n_black = 1;
			other->n_black = parent->n_black;
		    }
		    parent->n_black = 1;
		    hammertree_rrotate(tree, parent);
		    tree->t_root->n_black = 1;  /* LATER rethink */
		    goto done;
		}
	    }
	}
	if (child)  /* (sentinel not used) */
	    child->n_black = 1;
    }
done:
    freebytes(gone, sizeof(*gone));
#ifdef HAMMERTREE_DEBUG
    hammertree_verify(tree);
#endif
}

t_hammernode *hammertree_search(t_hammertree *tree, int ndx)
{
    t_hammernode *np = tree->t_root;
    while (np && np->n_index != ndx)
	np = (ndx < np->n_index ? np->n_left : np->n_right);
    return (np);
}

t_hammernode *hammertree_closest(t_hammertree *tree, int ndx, int geqflag)
{
    t_hammernode *np, *parent;
    if (!(np = tree->t_root))
	return (0);
    do
	if (np->n_index == ndx)
	    return (np);
	else
	    parent = np;
    while (np = (ndx < np->n_index ? np->n_left : np->n_right));
    if (geqflag)
	return (ndx > parent->n_index ? parent->n_next : parent);
    else
	return (ndx < parent->n_index ? parent->n_prev : parent);
}

/* LATER preallocate 'freecount' nodes */
void hammertree_init(t_hammertree *tree, int freecount)
{
    tree->t_root = tree->t_first = tree->t_last = 0;
}

/* LATER keep and/or preallocate 'freecount' nodes (if negative, keep all) */
void hammertree_clear(t_hammertree *tree, int freecount)
{
    t_hammernode *np, *next = tree->t_first;
    while (next)
    {
	np = next;
	next = next->n_next;
	freebytes(np, sizeof(*np));
    }
    hammertree_init(tree, 0);
}
