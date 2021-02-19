#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "gmap.h"

#define LEFT (0)
#define RIGHT (1)
#define TREE_NUM_CHILDREN (2)



#define TREE_EMPTY (0)
#define TREE_EMPTY_HEIGHT (-1)



typedef struct tree {
  struct tree *child[TREE_NUM_CHILDREN];
  void *value;
  char *key;
  int height;
  size_t size;
} tree;

// typedef struct _node
// {
//   void *key;
//   void *value;
//   struct _node *next;
// } node;

struct gmap
{
  size_t capacity;
  size_t size;
  //tree **table;
  tree **table;
  
  void *(*copy)(const void *);
  int (*compare)(const void *, const void *);
  size_t (*hash)(const void *);
  void (*free)(void *);
};

#define GMAP_INITIAL_CAPACITY 100

// fix all the heights and sizes
static void treeAggregateFix(tree *root);
// rebalance the tree
static void treeRebalance(tree **root);
// adding on embiggen
void embiggenHelper(tree **table, tree *curr, size_t (*hash)(const void *), size_t capacity);
// search down all tree and apply function
void downTree(tree *curr, void (*f)(const void *, void *, void *), void *arg);
/* free all elements of a tree, replacing it with TREE_EMPTY */
void treeDestroy(tree **root);
/* insert an element into a tree pointed to by root */
void treeInsert(tree **root, tree *n);
/* return 1 if target is in tree, 0 otherwise */
/* we allow root to be modified to allow for self-balancing trees */
tree* treeContains(tree *root, const char* target);
/* return height of tree */
int treeHeight(const struct tree *root);
/* return size of tree */
size_t treeSize(const struct tree *root);
void treePrint(tree *root);
/* check that aggregate data is correct throughout the tree */
void treeSanityCheck(tree *root);

size_t gmap_compute_index(const void *key, size_t (*hash)(const void *), size_t size);
void gmap_embiggen(gmap *m, size_t n);
void gmap_table_add(tree **table, tree *n, size_t (*hash)(const void *), size_t capacity);
tree *gmap_table_find_key(tree **table, const void *key, int (*compare)(const void *, const void *), size_t (*hash)(const void *), size_t capacity);

gmap *gmap_create(void *(*cp)(const void *), int (*comp)(const void *, const void *), size_t (*h)(const void *s), void (*f)(void * k))
{
  gmap *result = malloc(sizeof(gmap));
  if (result != NULL)
    {
      result->size = 0;
      result->copy = cp;
      result->compare = comp;
      result->hash = h;
      result->free = f;
      result->table = malloc(sizeof(tree *) * GMAP_INITIAL_CAPACITY);
      result->capacity = (result->table != NULL ? GMAP_INITIAL_CAPACITY : 0);
      for (size_t i = 0; i < result->capacity; i++)
	{
	  result->table[i] = NULL;
	}
    }
  return result;
}

size_t gmap_size(const gmap *m)
{
  if (m == NULL)
    {
      return 0;
    }
  
  return m->size;
}

/**
 * Returns the tree where the given key is located, or NULL if it is not present.
 * where it would go if it is not present.
 * 
 * @param table a table with at least one free slot
 * @param key a string, non-NULL
 * @param capacity the capacity of table
 * @param hash a hash function for strings
 * @return a pointer to the tree containing key, or NULL
 */
tree *gmap_table_find_key(tree **table, const void *key, int (*compare)(const void *, const void *), size_t (*hash)(const void *), size_t capacity)
{
  // compute starting location for search from hash function
  size_t i = gmap_compute_index(key, hash, capacity);
  tree *curr = table[i];
  curr = treeContains(curr, (char*)key);
  // while (curr != NULL && compare(curr->key, key) != 0)
  //   {
  //     curr = curr->next;
  //   }
  return curr;
}

bool gmap_put(gmap *m, const void *key, void *value)
{
  if (m == NULL || key == NULL)
    {
      return false;
    }

  tree *n = gmap_table_find_key(m->table, key, m->compare, m->hash, m->capacity);
  if (n != NULL)
    {
      // key already present
      n->value = value;
      return false;
    }
  else
    {
      // make a copy of the key
      void *copy = m->copy(key);
      
      if (copy != NULL)
	{
	  // new key, value pair -- check capacity
	  if (m->size >= m->capacity)
	    {
	      // grow
        //fprintf(stderr, "%s %ld %ld", (char *)copy, m->size, m->capacity);
	      gmap_embiggen(m, m->capacity * 2);
	    }
	      
	  // add to table
	  tree *n = malloc(sizeof(tree));
	  if (n != NULL)
	    {
        size_t i = gmap_compute_index(key, m->hash, m->capacity);
    
	      n->key = copy;
	      n->value = value;
        treeInsert(&m->table[i], n);
	      // gmap_table_add(m->table, n, m->hash, m->capacity);
	      m->size++;
	      return true;
	    }
	  else
	    {
	      free(copy);
	      return false;
	    }
	}
      else
	{
	  return false;
	}
    }
}

void treeInsert(tree **root, tree *n) {
  if (*root == 0) {
    *root = n;
    n->child[LEFT] = n->child[RIGHT] = 0;
  // } else if (strcmp((*root)->key, n->key) == 0) {
  //   return;
  } else {
    //fprintf(stderr, "there I am\n");
    treeInsert(&(*root)->child[strcmp((*root)->key,n->key) < 0], n);
  }

  treeAggregateFix(*root);
  treeRebalance(root);
}

// /**
//  * Adds the given tree containing a key and value into the appropriate chain the in the
//  * given hash table.
//  *
//  * @param table a hash table
//  * @param n a tree containing a key and value
//  * @param hash a hash function for keys
//  * @param capacity the size of the hash table
//  */
// void gmap_table_add(tree **table, tree *n, size_t (*hash)(const void *), size_t capacity)
// {
//   size_t i = gmap_compute_index(n->key, hash, capacity);
//   treeInsert(&n, key, value);
//   n->next = table[i];
//   table[i] = n;
// }

void embiggenHelper(tree **table, tree *curr, size_t (*hash)(const void *), size_t capacity) {
  if (curr != 0) {  
    size_t i = gmap_compute_index(curr->key, hash, capacity);

    if (curr->child[LEFT] != 0) {
      embiggenHelper(table, curr->child[LEFT],hash,capacity);
    }
    
    if (curr->child[RIGHT] != 0) {
      embiggenHelper(table, curr->child[RIGHT],hash,capacity);
    }
    treeInsert(&table[i], curr);
  }
}

void gmap_embiggen(gmap *m, size_t n)
{
  //fprintf(stderr,"HEREHREHRHEHHE\n");
  size_t bigger_capacity = n;
  tree **bigger = calloc(bigger_capacity, sizeof(tree *));
  if (bigger != NULL)
    {
      // would be better to do this without creating new trees
      for (size_t i = 0; i < m->capacity; i++)
      {
        tree *curr = m->table[i];
        //fprintf(stderr, "%s\n", curr->key);
        embiggenHelper(bigger, curr, m->hash, bigger_capacity);
        //fprintf(stderr, "ddday");
        // while (curr != NULL)
        //   {
        //     tree *next = curr->next;
        //     gmap_table_add(bigger, curr, m->hash, bigger_capacity);
        //     curr = next;
        //   }
      }
      free(m->table);
      m->table = bigger;
      m->capacity = bigger_capacity;
      //fprintf(stderr, "%ld\n", m->capacity);
    }
}



size_t gmap_compute_index(const void *key, size_t (*hash)(const void *), size_t size)
{
  return (hash(key) % size + size) % size;
}

bool gmap_contains_key(const gmap *m, const void *key)
{
  if (m == NULL || key == NULL)
    {
      return false;
    }

  return gmap_table_find_key(m->table, key, m->compare, m->hash, m->capacity) != NULL;
}

void *gmap_get(gmap *m, const void *key)
{
  if (m == NULL || key == NULL)
    {
      return NULL;
    }
  
  tree *n = gmap_table_find_key(m->table, key, m->compare, m->hash, m->capacity);
  if (n != NULL)
    {
      return n->value;
    }
  else
    {
      return NULL;
    }
}

void gmap_for_each(gmap *m, void (*f)(const void *, void *, void *), void *arg)
{
  if (m == NULL || f == NULL)
    {
      return;
    }

  // TO DO: iterate over all chains as in Ex. 7
  for (int i = 0; i < m->capacity; i++) {
    tree *curr = m->table[i];

    downTree(curr, f, arg);
    // while (curr != NULL)
    // {
    //   f(curr->key, curr->value, arg);
    //   curr = curr->next;
    // }
  }
}  

void downTree(tree *curr, void (*f)(const void *, void *, void *), void *arg) {
  if (curr != NULL) {
    if (curr->child[LEFT] != NULL) {
      downTree(curr->child[LEFT],f,arg);
      //f(curr->key, curr->value, arg);
    }
    
    if (curr->child[RIGHT] != NULL) {
      downTree(curr->child[RIGHT], f, arg);
      //f(curr->key, curr->value,arg);
    }
    f(curr->key, curr->value, arg);
    
  }
}

void gmap_destroy(gmap *m)
{
  if (m == NULL)
    {
      return;
    }

  //gmap_validate(m);
  for (int i = 0; i < m->capacity; i++) {
    treeDestroy(&m->table[i]);
  }
    
  // for (size_t i = 0; i < m->capacity; i++)
  //   {
  //     tree *curr = m->table[i];
  //     while (curr != NULL)
	// {
	//   m->free(curr->key);
	//   tree *next = curr->next;
	//   free(curr);
	//   curr = next;
	// }
  //   }

  // TO DO: fix memory leak from Ex. 7
  free(m->table);
  free(m);
}

int treeHeight(const struct tree *root)
{
    if(root == 0) {
        return TREE_EMPTY_HEIGHT;
    } else {
        return root->height;
    }
}

/* recompute height from height of kids */
static int
treeComputeHeight(const struct tree *root)
{
    int childHeight;
    int maxChildHeight;
    int i;

    if(root == 0) {
        return TREE_EMPTY_HEIGHT;
    } else {
        maxChildHeight = TREE_EMPTY_HEIGHT;

        for(i = 0; i < TREE_NUM_CHILDREN; i++) {
            childHeight = treeHeight(root->child[i]);
            if(childHeight > maxChildHeight) {
                maxChildHeight = childHeight;
            }
        }

        return maxChildHeight + 1;
    }
}

size_t treeSize(const struct tree *root)
{
    if(root == 0) {
        return 0;
    } else {
        return root->size;
    }
} 

/* recompute size from size of kids */
static int
treeComputeSize(const struct tree *root)
{
    int size;
    int i;

    if(root == 0) {
        return 0;
    } else {
        size = 1;

        for(i = 0; i < TREE_NUM_CHILDREN; i++) {
            size += treeSize(root->child[i]);
        }

        return size;
    }
}

/* fix aggregate data in root */
/* assumes children are correct */
static void
treeAggregateFix(struct tree *root)
{
    if(root) {
        root->height = treeComputeHeight(root);
        root->size = treeComputeSize(root);
    }
}

/* rotate child in given direction to root */
static void
treeRotate(struct tree **root, int direction)
{
    struct tree *x;
    struct tree *y;
    struct tree *b;

    /*
     *      y           x 
     *     / \         / \
     *    x   C  <=>  A   y
     *   / \             / \
     *  A   B           B   C
     */

    y = *root;                  assert(y);
    x = y->child[direction];    assert(x);
    b = x->child[!direction];

    /* do the rotation */
    *root = x;
    x->child[!direction] = y;
    y->child[direction] = b;

    /* fix aggregate data for y then x */
    treeAggregateFix(y);
    treeAggregateFix(x);
}

/* restore AVL property at *root after an insertion or deletion */
/* assumes subtrees already have AVL property */
static void
treeRebalance(struct tree **root)
{
    int tallerChild;

    if(*root) {
        for(tallerChild = 0; tallerChild < TREE_NUM_CHILDREN; tallerChild++) {
            if(treeHeight((*root)->child[tallerChild]) >= treeHeight((*root)->child[!tallerChild]) + 2) {

                /* which grandchild is the problem? */
                if(treeHeight((*root)->child[tallerChild]->child[!tallerChild]) 
                            > treeHeight((*root)->child[tallerChild]->child[tallerChild])) {
                    /* opposite-direction grandchild is too tall */
                    /* rotation at root will just change its parent but not change height */
                    /* so we rotate it up first */
                    treeRotate(&(*root)->child[tallerChild], !tallerChild);
                }

                /* now rotate up the taller child */
                treeRotate(root, tallerChild);

                /* don't bother with other child */
                break;
            }
        }

        /* test that we actually fixed it */
        assert(abs(treeHeight((*root)->child[LEFT]) - treeHeight((*root)->child[RIGHT])) <= 1);

#ifdef PARANOID_REBALANCE
        treeSanityCheck(*root);
#endif
    }
}

void treeDestroy(tree **root) {
  int i; 
  
  if(*root) {
    for (i = 0; i < TREE_NUM_CHILDREN; i++) {
      treeDestroy(&(*root)->child[i]);
    }
    free((*root)->key);
    free(*root);
    *root = TREE_EMPTY;
  }
}



tree* treeContains(tree *t, const char *target) {
  while (t && strcmp(t->key, target) != 0) {
    t = t->child[strcmp(t->key, target) < 0];
  }

  return t;
}

#define INDENTATION_LEVEL (2)

static void
treePrintIndented(tree *root, int depth)
{
    int i;

    if(root != 0) {
        treePrintIndented(root->child[LEFT], depth+1);

        for(i = 0; i < INDENTATION_LEVEL*depth; i++) {
            fprintf(stderr," ");
        }
        fprintf(stderr, "%s Height: %d Size: %zu (%p)\n", (char*)root->key, root->height, root->size, (void *) root);

        treePrintIndented(root->child[RIGHT], depth+1);
    }
}

/* print the contents of a tree */
void 
treePrint(tree *root)
{
    treePrintIndented(root, 0);
}

/* check that aggregate data is correct throughout the tree */
void
treeSanityCheck(tree *root)
{
    int i;

    if(root) {
        assert(root->height == treeComputeHeight(root));
        assert(root->size == treeComputeSize(root));

        assert(abs(treeHeight(root->child[LEFT]) - treeHeight(root->child[RIGHT])) < 2);

        for(i = 0; i < TREE_NUM_CHILDREN; i++) {
            treeSanityCheck(root->child[i]);
        }
    }
}