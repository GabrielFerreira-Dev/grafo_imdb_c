#ifndef AVL_TREE_H
#define AVL_TREE_H

typedef struct avl_node avl_node;

struct avl_node {
    int id;
    int level;
    avl_node *left;
    avl_node *right;
};

void visit(avl_node *n);
void destruct(avl_node *n);
int get_weight(avl_node *n);
void update_level(avl_node *n);
avl_node *rotate_left(avl_node *n, int weight);
avl_node *rotate_right(avl_node *n, int weight);
void balance(avl_node **n);
void insert_node(avl_node **n, avl_node *m);
void insert(avl_node **n, int id);
void erase(avl_node **n, int id);

#endif // AVL_TREE_H
