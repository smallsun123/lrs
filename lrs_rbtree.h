#ifndef __RB_TREE__H__
#define __RB_TREE__H__


#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof1(type,member) );})
        

#define lrs_hash(key, c)   ((ngx_uint_t) key * 31 + c)


typedef unsigned int    lrs_uint_t;
typedef int             lrs_int_t;
typedef unsigned char   lrs_u_char;
typedef inline          lrs_inline;
typedef void            lrs_void;

typedef lrs_uint_t       lrs_rbtree_key_t;
typedef lrs_int_t        lrs_rbtree_key_int_t;


typedef struct lrs_rbtree_node_s  lrs_rbtree_node_t;

struct lrs_rbtree_node_s {
    lrs_rbtree_key_t       key;
    lrs_rbtree_node_t     *left;
    lrs_rbtree_node_t     *right;
    lrs_rbtree_node_t     *parent;
    lrs_u_char             color;
    lrs_void              *data;
};


typedef struct lrs_rbtree_s  lrs_rbtree_t;

typedef void (*lrs_rbtree_insert_pt) (lrs_rbtree_node_t *root, lrs_rbtree_node_t *node, lrs_rbtree_node_t *sentinel);

struct lrs_rbtree_s {
    lrs_rbtree_node_t        *root;
    lrs_rbtree_node_t        *sentinel;  //из╠Ь
    lrs_rbtree_insert_pt     insert;
};


#define lrs_rbtree_init(tree, s, i) \
    lrs_rbtree_sentinel_init(s);    \
    (tree)->root = s;              \
    (tree)->sentinel = s;          \
    (tree)->insert = i


void lrs_rbtree_insert(lrs_rbtree_t *tree, lrs_rbtree_node_t *node);
void lrs_rbtree_delete(lrs_rbtree_t *tree, lrs_rbtree_node_t *node);
void lrs_rbtree_insert_value(lrs_rbtree_node_t *root, lrs_rbtree_node_t *node, lrs_rbtree_node_t *sentinel);

#define lrs_rbt_red(node)               ((node)->color = 1)
#define lrs_rbt_black(node)             ((node)->color = 0)
#define lrs_rbt_is_red(node)            ((node)->color)
#define lrs_rbt_is_black(node)          (!lrs_rbt_is_red(node))
#define lrs_rbt_copy_color(n1, n2)      (n1->color = n2->color)


/* a sentinel must be black */

#define lrs_rbtree_sentinel_init(node)  lrs_rbt_black(node)


#define tree_entry(ptr, type, member) \
	container_of(ptr, type, member)


uint32_t lrs_hash_key(char *data, int len);

#endif
