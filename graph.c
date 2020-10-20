
#include "linkedlist.h"

const enum Status {VISITED, VISITING, UNVISITED};

typedef struct g_Graph_t {
    int size;
    LinkedList* /*of g_Node_t*s */ nodes;
} Graph;

struct g_Node_t {
    enum Status mark;
    void* contents;
    g_Node_t** successors;
};

g_Node_t* initializeGraphNode(Graph g, void* contents, int[] successors) {

}

g_Node_t* nextUnvisited(LinkedList /* of g_Node_ts*/ nodeList) {
    assert(nodeList != NULL);

    for (int i = 0; i < nodeList->size; i++) {
        g_Node_t() 
        ll_get(nodeList);
    }
}

LinkedList /*of content type*/ topologicalSort(Graph graph);
// helper func: ret first unvisited or null

// main func: topo order
// creates an empty list
// while next = helper != null
    // visit next

// visit function
// if has been visited -> return to recur upwards
// if is being visited -> return with circular dep. error
// mark as being visited currently
// visit all nodes that are the successors of n

function visit(node n)
    if n has a permanent mark then
        return // recur up
    if n has a temporary mark then
        stop   (not a DAG) // CIRCULAR DEPENDENCY ERROR OUT

    mark n with a temporary mark

    for each node m with an edge from n to m do
        visit(m) // recur down

    remove temporary mark from n
    mark n with a permanent mark // has been visited, ignore now
    add n to head of L // add to front of topo list

