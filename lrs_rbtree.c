#include "lrs_rbtree.h"

static lrs_inline void lrs_rbtree_left_rotate(lrs_rbtree_node_t **root,
    lrs_rbtree_node_t *sentinel, lrs_rbtree_node_t *node);

static lrs_inline void lrs_rbtree_right_rotate(lrs_rbtree_node_t **root,
    lrs_rbtree_node_t *sentinel, lrs_rbtree_node_t *node);


void
lrs_rbtree_insert(lrs_rbtree_t *tree, lrs_rbtree_node_t *node)
{
    lrs_rbtree_node_t  **root, *temp, *sentinel;

    /* a binary tree insert */

    root = (lrs_rbtree_node_t **) &tree->root;
    sentinel = tree->sentinel;

    //first element
    if (*root == sentinel) {
        node->parent = NULL;
        node->left = sentinel;
        node->right = sentinel;
        lrs_rbt_black(node);
        *root = node;

        return;
    }

    tree->insert(*root, node, sentinel);

    /* re-balance tree */

    // node is red, node->parent is red
    while (node != *root && lrs_rbt_is_red(node->parent)) {

        // node->parent is left child
        if (node->parent == node->parent->parent->left) {

            // temp == uncle
            temp = node->parent->parent->right;

            // uncle is red
            if (lrs_rbt_is_red(temp)) {
                
                lrs_rbt_black(node->parent);         // b node->parent
                lrs_rbt_black(temp);                 // b node->uncle
                lrs_rbt_red(node->parent->parent);   // r node->parent->parent

                // new node == node->parent->parent
                node = node->parent->parent;

            } else {
            // uncle is black
                // node is right child
                if (node == node->parent->right) {
                    node = node->parent;
                    lrs_rbtree_left_rotate(root, sentinel, node);
                }

                lrs_rbt_black(node->parent);
                lrs_rbt_red(node->parent->parent);
                lrs_rbtree_right_rotate(root, sentinel, node->parent->parent);
            }

        } else {
            temp = node->parent->parent->left;

            if (lrs_rbt_is_red(temp)) {
                lrs_rbt_black(node->parent);
                lrs_rbt_black(temp);
                lrs_rbt_red(node->parent->parent);
                node = node->parent->parent;

            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    lrs_rbtree_right_rotate(root, sentinel, node);
                }

                lrs_rbt_black(node->parent);
                lrs_rbt_red(node->parent->parent);
                lrs_rbtree_left_rotate(root, sentinel, node->parent->parent);
            }
        }
    }

    lrs_rbt_black(*root);
}


void lrs_rbtree_insert_value(lrs_rbtree_node_t *temp, lrs_rbtree_node_t *node, lrs_rbtree_node_t *sentinel)
{
    lrs_rbtree_node_t  **p;

    // temp == root
    // node == new
    
    //find node to insert
    for ( ;; ) {

        p = (node->key < temp->key) ? &temp->left : &temp->right;

        if (*p == sentinel) {
            break;
        }

        temp = *p;
    }

    *p = node;
    node->parent = temp;
    node->left = sentinel;
    node->right = sentinel;

    //red new node
    lrs_rbt_red(node);
}

void
lrs_rbtree_delete(lrs_rbtree_t *tree, lrs_rbtree_node_t *node)
{
    lrs_uint_t           red;
    lrs_rbtree_node_t  **root, *sentinel, *subst, *temp, *w;

    /* a binary tree delete */

    root = (lrs_rbtree_node_t **) &tree->root;
    sentinel = tree->sentinel;

    if (node->left == sentinel) {
        temp = node->right;
        subst = node;

    } else if (node->right == sentinel) {
        temp = node->left;
        subst = node;

    } else {
        subst = lrs_rbtree_min(node->right, sentinel);

        if (subst->left != sentinel) {
            temp = subst->left;
        } else {
            temp = subst->right;
        }
    }

    if (subst == *root) {
        *root = temp;
        lrs_rbt_black(temp);

        /* DEBUG stuff */
        node->left = NULL;
        node->right = NULL;
        node->parent = NULL;
        node->key = 0;

        return;
    }

    red = lrs_rbt_is_red(subst);

    if (subst == subst->parent->left) {
        subst->parent->left = temp;

    } else {
        subst->parent->right = temp;
    }

    if (subst == node) {

        temp->parent = subst->parent;

    } else {

        if (subst->parent == node) {
            temp->parent = subst;

        } else {
            temp->parent = subst->parent;
        }

        subst->left = node->left;
        subst->right = node->right;
        subst->parent = node->parent;
        lrs_rbt_copy_color(subst, node);

        if (node == *root) {
            *root = subst;

        } else {
            if (node == node->parent->left) {
                node->parent->left = subst;
            } else {
                node->parent->right = subst;
            }
        }

        if (subst->left != sentinel) {
            subst->left->parent = subst;
        }

        if (subst->right != sentinel) {
            subst->right->parent = subst;
        }
    }

    /* DEBUG stuff */
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    node->key = 0;

    if (red) {
        return;
    }

    /* a delete fixup */

    while (temp != *root && lrs_rbt_is_black(temp)) {

        if (temp == temp->parent->left) {
            w = temp->parent->right;

            if (lrs_rbt_is_red(w)) {
                lrs_rbt_black(w);
                lrs_rbt_red(temp->parent);
                lrs_rbtree_left_rotate(root, sentinel, temp->parent);
                w = temp->parent->right;
            }

            if (lrs_rbt_is_black(w->left) && lrs_rbt_is_black(w->right)) {
                lrs_rbt_red(w);
                temp = temp->parent;

            } else {
                if (lrs_rbt_is_black(w->right)) {
                    lrs_rbt_black(w->left);
                    lrs_rbt_red(w);
                    lrs_rbtree_right_rotate(root, sentinel, w);
                    w = temp->parent->right;
                }

                lrs_rbt_copy_color(w, temp->parent);
                lrs_rbt_black(temp->parent);
                lrs_rbt_black(w->right);
                lrs_rbtree_left_rotate(root, sentinel, temp->parent);
                temp = *root;
            }

        } else {
            w = temp->parent->left;

            if (lrs_rbt_is_red(w)) {
                lrs_rbt_black(w);
                lrs_rbt_red(temp->parent);
                lrs_rbtree_right_rotate(root, sentinel, temp->parent);
                w = temp->parent->left;
            }

            if (lrs_rbt_is_black(w->left) && lrs_rbt_is_black(w->right)) {
                lrs_rbt_red(w);
                temp = temp->parent;

            } else {
                if (lrs_rbt_is_black(w->left)) {
                    lrs_rbt_black(w->right);
                    lrs_rbt_red(w);
                    lrs_rbtree_left_rotate(root, sentinel, w);
                    w = temp->parent->left;
                }

                lrs_rbt_copy_color(w, temp->parent);
                lrs_rbt_black(temp->parent);
                lrs_rbt_black(w->left);
                lrs_rbtree_right_rotate(root, sentinel, temp->parent);
                temp = *root;
            }
        }
    }

    lrs_rbt_black(temp);
}

/*
================================
            |
            |
            X
           / \
          /   \
         a     Y
              / \
             /   \
            b     c            
    -------------------
       节点 X  左旋
    -------------------
            |
            |
            Y
           / \
          /   \
         X     c
        / \
       /   \
      a     b
================================
*/
static lrs_inline void
lrs_rbtree_left_rotate(lrs_rbtree_node_t **root, lrs_rbtree_node_t *sentinel, lrs_rbtree_node_t *node)
{
    lrs_rbtree_node_t  *temp;

    temp = node->right;

    // node->right <---> node->right->left
    node->right = temp->left;
    if (temp->left != sentinel) {
        temp->left->parent = node;
    }

    // node->parent <---> node->right->parent
    temp->parent = node->parent;
    if (node == *root) {
        *root = temp;
    } else if (node == node->parent->left) {
        node->parent->left = temp;
    } else {
        node->parent->right = temp;
    }

    // node->right->left <---> node
    temp->left = node;
    node->parent = temp;
}



/*
================================
            |
            |
            Y
           / \
          /   \
         X     c
        / \
       /   \
      a     b
    -------------------
       节点 Y  右旋
    -------------------
            |
            |
            X
           / \
          /   \
         a     Y
              / \
             /   \
            b     c
================================
*/
static lrs_inline void
lrs_rbtree_right_rotate(lrs_rbtree_node_t **root, lrs_rbtree_node_t *sentinel, lrs_rbtree_node_t *node)
{
    lrs_rbtree_node_t  *temp;

    temp = node->left;

    // node->left <---> node->left->right
    node->left = temp->right;       // 1
    if (temp->right != sentinel) {
        temp->right->parent = node; // 1
    }

    // node->parent <---> node->left->parent
    temp->parent = node->parent;    // 2
    if (node == *root) {
        *root = temp;               // 2
    } else if (node == node->parent->right) {
        node->parent->right = temp; // 2
    } else {
        node->parent->left = temp;  // 2
    }

    // node->left->right <---> node
    temp->right = node;             // 3
    node->parent = temp;            // 3
}

uint32_t lrs_hash_key(char *data, int len)
{
    uint32_t  i, key;
    key = 0;
    for (i = 0; i < len; i++) {
        key = lrs_hash(key, data[i]);
    }
    return key;
}

