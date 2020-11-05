/**
 * CS 537 Programming Assignment 3 (Fall 2020)
 * @author Michael Noguera
 * @date 11/4/2020
 *
 * @file bintree.h
 * @brief Implementation of binary tree with string keys and void* values.
 * @version 1.1
 */

#include "bintree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Constructs an empty BTNode (goes in a BTree), with specified values.
 *
 * @param[in] key string key identifying this node. CANNOT BE NULL.
 * @param[in] value value to associate with key. Can be NULL without issue.
 *
 * @return pointer to heap-allocated tree node struc
 */
static BTNode* initializeNode(const char* key, const void* value) {
    if (key == NULL) {
        perror("NULL is not a valid key");
        exit(EXIT_FAILURE);
    }
    BTNode* n =
      (BTNode*)malloc(sizeof(BTNode) + (sizeof(char) * (strlen(key) + 1)));
    if (n == NULL) {
        perror("Failed to allocate memory for new tree node");
        exit(EXIT_FAILURE);
    }
    n->key   = key;
    n->value = value;
    n->left  = NULL;
    n->right = NULL;

    return n;
}

/**
 * Constructs an empty BTree binary tree with default values.
 * @return pointer to heap-allocated tree
 */
BTree* bt_initializeTree() {
    BTree* t = (BTree*)malloc(sizeof(BTree));
    if (t == NULL) {
        perror("Failed to allocate memory for new binary tree");
        exit(EXIT_FAILURE);
    }
    t->root = NULL;
    return t;
}

void bt_print_in_order_helper(const BTNode* n) {
    if (n == NULL) return;
    bt_print_in_order_helper(n->left);
    bt_print_in_order_helper(n->right);
}

void bt_print(const BTree* bt) {
    bt_print_in_order_helper(bt->root);
}

/**
 * Recursive case in add operation.
 *
 * @details A reminant of a failed attempt to make this an AVL tree, this
 * function would return the node that should go in p's location. Because this
 * is not an AVL tree, that is always p.
 *
 * @param[in/out] p parent node, currently being traversed
 * @param[in/out] n new node, to be inserted
 * @param[out] err is set to 1 if error occurs
 *
 * @return pointer to p, or the node that should go in p's old place
 */
static BTNode* insertNodeRecursive(BTNode* p, BTNode* n, int* err) {
    // BASE CASE: correct empty location found --> add here
    if (p == NULL) return n;

    // RECURSIVE CASE: recur left or right based on sign of strcmp
    switch ((0 < strcmp(n->key, p->key)) - (strcmp(n->key, p->key) < 0)) {
        case 0:
            perror("[HELPER] Prevented add with duplicate key, returning 1");
            *err = 1; // already a node with this key -> return with error
            return p; // do not need to rebalance because nothing happened
        case -1:
            p->left = insertNodeRecursive(p->left, n, err); // recur left
            break;
        case 1:
            p->right = insertNodeRecursive(p->right, n, err); // recur right
            break;
    }

    return p; // return reference to this portion of the tree so that changes
              // can persist
}

/**
 * Error handler for add operation, initiates recursive helper
 * `insertNodeRecursive`.
 * @see insertNodeRecursive
 *
 * @param tree BTree to insert in
 * @param n node to add
 *
 * @return Error value. 0 on success, 1 if there is already a node with the
 * specified key or key is NULL. Exits with EXIT_FAILURE upon spontaneous
 * failure.
 */
static int insertNode(BTree* tree, BTNode* n) {
    // Input validation, important because recursive case doesn't check
    if (tree == NULL) {
        perror("NULL is not a valid tree");
        exit(EXIT_FAILURE);
    } else if (n == NULL) {
        perror("NULL is not a valid node");
        exit(EXIT_FAILURE);
    }

    // Refuse to add nodes that already have children, as there is no guarantee
    // that they won't break the tree
    if (n->left != NULL || n->right != NULL) {
        perror("Provided node is already in a tree or has children");
        return 1;
    }

    // Perform insert and detect errors, if any
    int err    = 0;
    tree->root = insertNodeRecursive(tree->root, n, &err);
    if (err) perror("Tried to insert node with duplicate key, returning 1");
    return err;
}

/**
 * Inserts a value identified by a string key to the tree.
 *
 * @param tree BTree to insert to
 * @param key string key for accessing, duplicate keys not allowed
 * @param value void* value associated with the key
 *
 * @return Error value. 0 on success, 1 if there is already a node with the
 * specified key or key is NULL. Exits with EXIT_FAILURE upon spontaneous
 * failure.
 */
int bt_insert(BTree* tree, const char* key, const void* value) {
    if (key == NULL) {
        perror("Node key cannot be null.");
        return 1;
    }

    BTNode* n = initializeNode(key, value);
    int err   = insertNode(tree, n);

    return err;
}

/**
 * Recursive case for get operation.
 *
 * @param n current node in traversal
 * @param t target key
 *
 * @return value corresponding to target key, if found, else null
 */
static void* getHelper(const BTNode* n, const char* t) {
    // BASE CASE: fell off tree -> not present
    if (n == NULL) return NULL;

    // RECURSIVE CASE: recur left or right
    int cmp = strcmp(t, n->key);
    switch ((0 < cmp) - (cmp < 0)) { // = sign of cmp
        case -1:
            return getHelper(n->left, t); // recur left
        case 1:
            return getHelper(n->right, t); // recur right
    }

    // BASE CASE: target found
    return (void*)(n->value);
}

/**
 * Retrieves a value identified by a string key from the tree.
 *
 * @param tree BTree to search
 * @param key string key to search for
 *
 * @return The value associated wih the node, or NULL if none found.
 * Note that null does not neccesarily mean failure because it is a valid value
 * as well.
 */
const void* bt_get(const BTree* tree, const char* key) {
    if (tree == NULL || key == NULL) {
        perror("Invalid parameter. Both tree and key must be non-null.");
        exit(EXIT_FAILURE);
    }

    // initate recursive search from tree root and return result
    return getHelper(tree->root, key);
}