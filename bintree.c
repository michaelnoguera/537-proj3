/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera
 * @date 11/4/2020
 * @brief Implementation of binary tree with string keys and void* values.
 * @file bintree.c
 */

#include "bintree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This was supposed to be an AVL tree, but it was not maintaining balance, so
// the rotate code is commented out

// node constructor
static BTNode* initializeNode(const char* key, const void* value) {
    if (key == NULL) {
        perror("NULL is not a valid key");
        exit(EXIT_FAILURE);
    }
    BTNode* n = (BTNode*)malloc(sizeof(BTNode) + (sizeof(char) * (strlen(key)+1)));
    if (n == NULL) {
        perror("Failed to allocate memory for new tree node");
        exit(EXIT_FAILURE);
    }
    n->key = key;
    n->value = value;
    n->bf = 0;
    n->left = NULL;
    n->right = NULL;
    //n->height = 0;

    return n;
}

// tree constructor
BTree* bt_initializeTree() {
    BTree* t = (BTree*)malloc(sizeof(BTree));
    if (t == NULL) {
        perror("Failed to allocate memory for new binary tree");
        exit(EXIT_FAILURE);
    }
    t->root = NULL;
    return t;
}

/*static int heightOfNode(const BTNode* n) {
    if (n == NULL) return 0;
    if (heightOfNode(n->left) > heightOfNode(n->right))
        return heightOfNode(n->left) + 1;
    else return heightOfNode(n->right) + 1;
}*/

void bt_print_in_order_helper(const BTNode* n) {
    if (n == NULL) return;
    bt_print_in_order_helper(n->left);
    //printf("%s (%i) [%i]\n", n->key, heightOfNode(n), heightOfNode(n->left)-heightOfNode(n->right));
    bt_print_in_order_helper(n->right);
}

void bt_print(const BTree* bt) {
    bt_print_in_order_helper(bt->root);
}

static BTNode* rightRotate(BTNode* a) {

    if (a == NULL || a->right == NULL) {
        perror("Invalid rotation requested, involves a null node");
        exit(EXIT_FAILURE);
    }

    // move subtrees
    //BTNode* b = a->left;
    //a->left = b->right;
    //b->left = a;

    // fix balance factors
    //a->bf = heightOfNode(a->left)-heightOfNode(a->right);
    //b->bf = heightOfNode(b->left)-heightOfNode(b->right);
    //a->bf = a->bf + 1 - (b->bf < 0 ? b->bf : 0);
    //b->bf = b->bf + 1 + (a->bf > 0 ? a->bf : 0);

    //assert(a->bf < 2 && a->bf > -2);
    //assert(b->bf < 2 && b->bf > -2);

    // point to the new top node
    return a;
}

static BTNode* leftRotate(BTNode* a) {
    if (a == NULL || a->left == NULL) {
        perror("Invalid rotation requested, involves a null node.");
        exit(EXIT_FAILURE);
    }

    // move subtrees
    //BTNode* b = a->right;
    //a->right = b->left;
    //b->left = a;

    // fix balance factors
    //a->bf = heightOfNode(a->left)-heightOfNode(a->right);
    //b->bf = heightOfNode(b->left)-heightOfNode(b->right);
    //a->bf = a->bf - 1 - (b->bf > 0 ? b->bf : 0);
    //b->bf = b->bf - 1 + (a->bf < 0 ? b->bf : 0);

    //assert(a->bf < 2 && a->bf > -2);
    //assert(b->bf < 2 && b->bf > -2);
    // point to the new top node

    return a;
}

// returns the node that is in old's location
// @param p parent
// @param n new node
static BTNode* insertNodeRecursive(BTNode* p, BTNode* n, int* err) {
    // BASE CASE: correct empty location found --> add here
    if (p == NULL) return n;

    // RECURSIVE CASE: recur left or right
    int cmp = strcmp(n->key, p->key);
    switch ((0 < cmp) - (cmp < 0)) {  // = sign of cmp
    case 0:
        perror("[HELPER] Prevented add with duplicate key, returning 1");
        *err = 1;  // already a node with this key -> return with error
        return p;  // do not need to rebalance because nothing happened
    case -1:
        p->left = insertNodeRecursive(p->left, n, err);  // recur left
        p->bf++;
        break;
    case 1:
        p->right = insertNodeRecursive(p->right, n, err);  // recur right
        p->bf--;
        break;
    }

    // REBALANCE after successful insert
    if (n->bf > 1) {
        if (n->key < p->left->key) {
            //printf("SINGLE RIGHT ROTATE\n");
            n = rightRotate(n);
        } else if (n->key > p->left->key) {
            //printf("LEFT RIGHT ROTATE\n");
            n->left = leftRotate(n->left);
            n = rightRotate(n);
        }
    } else if (n->bf < -1) {
        if (n->key < p->right->key) {
            //printf("SINGLE LEFT ROTATE\n");
            n = leftRotate(n);
        } else if (n->key < p->right->key) {
            //printf("RIGHT LEFT ROTATE\n");
            n->right = rightRotate(n->right);
            n = leftRotate(n);
        }
    }

    return p;  // return reference to this portion of the tree so that changes can persist
}

static int insertNode(BTree* tree, BTNode* n) {
    if (tree == NULL) {
        perror("NULL is not a valid tree");
        exit(EXIT_FAILURE);
    }
    if (n == NULL) {
        perror("NULL is not a valid node");
        exit(EXIT_FAILURE);
    }
    if (n->left != NULL || n->right != NULL) {
        perror("Provided node is already in a tree or has children");
        return 1;
    }

    int err = 0;
    tree->root = insertNodeRecursive(tree->root, n, &err);
    if (err) perror("Tried to insert node with duplicate key, returning 1");
    return err;
}

// returns 0 on success, or 1 if a node with the specified key already exists
// in the tree
int bt_insert(BTree* tree, const char* key, const void* value) {
    if (key == NULL) {
        perror("Node key cannot be null.");
        return 1;
    }

    BTNode* n = initializeNode(key, value);
    int err = insertNode(tree, n);

    return err;
}

static void* getHelper(const BTNode* n, const char* t) {
    // BASE CASE: fell off tree -> not present
    if (n == NULL) return NULL;

    // RECURSIVE CASE: recur left or right
    int cmp = strcmp(t, n->key);
    switch ((0 < cmp) - (cmp < 0)) {  // = sign of cmp
    case -1:
        return getHelper(n->left, t);  // recur left
    case 1:
        return getHelper(n->right, t);  // recur right
    }

    // TARGET FOUND
    return (void*)(n->value);
}

const void* bt_get(const BTree* tree, const char* key) {
    if (tree == NULL) {
        perror("NULL is not a valid tree");
        exit(EXIT_FAILURE);
    }
    if (key == NULL) {
        perror("NULL is not a valid key");
        exit(EXIT_FAILURE);
    }

    return getHelper(tree->root, key);
}


/* TODO implement remove if we need it
BTNode bt_removeHelper(BTNode p, char** key, int err) {
    // BASE CASE: fell off tree --> not found
    if (p == NULL) return p;

    // RECURSIVE CASE: recur left or right
    int cmp = strcmp(key, p->key);
    switch ((T(0) < cmp) - (cmp < T(0))) {  // = sign of cmp
        case 0: // found target
            
        case -1:
            p->left = bt_removeHelper(p->left, key, err);  // recur left
        case 1:
            p->right = bt_removeHelper(p->right, key, err);  // recur right
        default:

    }

    tree->root = bt_removeNode(tree->root, key, &err);
    return err;
}

int bt_remove(BTree* tree, char** key) {
    if (key == NULL) {
        perror("Node key cannot be null.");
        return 1;
    }

    int err = 0;
    tree->root = bt_removeNode(tree->root, key, &err);
    return err;
}
*/