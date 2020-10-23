/**
 * Implementation of AVL balanced binary tree with string keys and void* values.
 * @author Michael Noguera
 * @date 11/4/2020
 * @file bintree.h
 */

#include "bintree.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static int passed = 0;
static int failed = 0;

static int testno = 0;
static int testresultno = 0;
static char* testname;

static void test(char* name) {
    testname = name;
    testresultno = 0;
    printf("\n(%-2i) \e[1;4m%s\e[0m...\n", ++testno, testname);
    //printf("\n\e[3;2mstarting (%-2i) %s\e[0m...", testno, testname);
}

static void pass() {
    passed++;
    fflush(stdout);
    fflush(stderr);
    printf("\e[3;2mchecking %2i.%-2i... \e[0m\e[1;32m%s\e[0m\n", testno, ++testresultno, "OK  ");
}

static void fail() {
    failed++;
    fflush(stdout);
    fflush(stderr);
    printf("\e[3;2mchecking %2i.%-2i... \e[0m\e[1;31m%s\e[0m\n", testno, ++testresultno, "FAIL");
}

static void summarize() {
    printf("\n( \e[1;32m%i\e[0m passing | \e[1;31m%i\e[0m failed | \e[1;1m%i\e[0m total )\n", passed, failed, testno);
}

static void assert_true(int boolean) {
    if (errno != 0) fail();
    else if (boolean == 0) fail();
    else pass();
}

static void assert_int_equals(int a, int b) {
    if (errno != 0) fail();
    else if (a == b) pass();
    else fail();
}

static void assert_str_equals(char* a, char* b) {
    assert_true(a != NULL && b != NULL);
    assert_int_equals(strcmp(a, b), 0);
}

int main() {
    BTree* t = malloc(sizeof(BTree));
    assert(t != NULL);

    t = bt_initializeTree();
    
    char* a = "a";
    char* b = "b";
    char* c = "c";
    
    test("add and get 1");
    assert_int_equals(bt_insert(t, "b", b), 0);
    assert_str_equals((char*)bt_get(t, "b"), "b");

    test("add duplicate rejected 1");
    assert_int_equals(bt_insert(t, "b", "wrong"), 1);
    assert_str_equals((char*)bt_get(t, "b"), "b");

    test("add and get 2");
    assert_int_equals(bt_insert(t, "a", a), 0);
    assert_str_equals((char*)bt_get(t, "a"), "a");

    test("add duplicate rejected 2");
    assert_int_equals(bt_insert(t, "a", "wrong"), 1);
    assert_str_equals((char*)bt_get(t, "a"), "a");

    test("add and get 3");
    assert_int_equals(bt_insert(t, "c", c), 0);
    assert_str_equals((char*)bt_get(t, "c"), "c");

    test("add duplicate rejected 3");
    assert_int_equals(bt_insert(t, "c", "wrong"), 1);
    assert_str_equals((char*)bt_get(t, "c"), "c");

    test("get 1");
    assert_str_equals((char*)bt_get(t, "b"), "b");

    test("get 2");
    assert_str_equals((char*)bt_get(t, "a"), "a");

    test("get 3");
    assert_str_equals((char*)bt_get(t, "c"), "c");

    summarize();
}