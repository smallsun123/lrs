#include "lrs_application.h"

struct LRS_PLAY_PATH* lrs_play_path_alloc()
{
    struct LRS_PLAY_PATH *path = NULL;
    path = (struct LRS_PLAY_PATH*)malloc(sizeof(struct LRS_PLAY_PATH));
    if(!path){
        printf("lrs_play_path_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(path, 0, sizeof(struct LRS_PLAY_PATH));

    lrs_rbtree_init(&path->tree, &path->sentinel, lrs_rbtree_insert_value);
    return path;
}

void lrs_play_path_free(struct LRS_PLAY_PATH *path)
{
    if(path){
        path->publish = NULL;
        free(path);
        path = NULL;
    }
    return;
}

struct LRS_APP* lrs_app_alloc()
{
    struct LRS_APP *app = NULL;
    app = (struct LRS_APP*)malloc(sizeof(struct LRS_APP));
    if(!app){
        printf("lrs_app_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(app, 0, sizeof(struct LRS_APP));

    lrs_rbtree_init(&app->tree, &app->sentinel, lrs_rbtree_insert_value);
    return app;
}

void lrs_app_free(struct LRS_APP *app)
{
    if(app){
        free(app);
        app = NULL;
    }
    return;
}

struct LRS_APP_TREE* lrs_app_tree_alloc()
{
    struct LRS_APP_TREE *tree = NULL;
    tree = (struct LRS_APP_TREE*)malloc(sizeof(struct LRS_APP_TREE));
    if(!tree){
        printf("lrs_app_tree_alloc failed! error msg : %s\n", strerror(errno));
        return NULL;
    }
    memset(tree, 0, sizeof(struct LRS_APP_TREE));

    lrs_rbtree_init(&tree->tree, &tree->sentinel, lrs_rbtree_insert_value);
    return tree;
}

void lrs_app_tree_free(struct LRS_APP_TREE *apptree)
{
    if(apptree){
        free(apptree);
        apptree = NULL;
    }
    return;
}

