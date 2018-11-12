#ifndef __LRS_APPLICATION__H__
#define __LRS_APPLICATION__H__

typedef struct LRS_PLAY_PATH{
    struct lrs_rbtree_node_t node;

    char name[MAX_PATH];

    struct LRS_SESSION *publish;

    //play session
    struct lrs_rbtree_t tree;
    struct lrs_rbtree_node_t sentinel;

    
}LRS_PLAY_PATH;


struct LRS_PLAY_PATH* lrs_play_path_alloc();
void lrs_play_path_free(struct LRS_PLAY_PATH *path);

typedef struct LRS_APP{
    struct lrs_rbtree_node_t node;

    //playpath tree
    struct lrs_rbtree_t tree;
    struct lrs_rbtree_node_t sentinel;

    char name[MAX_PATH];

    //callbk  hls  dash rtmp
    
}LRS_APP;

struct LRS_APP* lrs_app_alloc();
void lrs_app_free(struct LRS_APP *app);

typedef struct LRS_APP_TREE{
    
    struct lrs_rbtree_t tree;
    struct lrs_rbtree_node_t sentinel;

    
}LRS_APP_TREE;

struct LRS_APP_TREE* lrs_app_tree_alloc();
void lrs_app_tree_free(struct LRS_APP_TREE *apptree);

#endif