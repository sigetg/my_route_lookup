#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> //USING BOOL FOR toCompress()
#include "io.h"


typedef struct TrieNode TrieNode;

struct TrieNode {
    uint32_t prefix;
    int prefix_len;
    int bit_position;
    int port;
    TrieNode* left_ptr;
    TrieNode* right_ptr;
};

TrieNode* make_trienode(int bit_position) {
    // Allocate memory for a TrieNode
    TrieNode* node = (TrieNode*)malloc( sizeof(TrieNode));
    node->left_ptr = NULL;
    node->right_ptr = NULL;
    node->bit_position = bit_position;
    node->prefix = 0; // if a node doesn't have a port assigned it goes 0
    node->prefix_len = 0;
    node->port = 0; //port number is 0 as default

    return node;
};

void free_trienode(TrieNode* node) {
    // Free the trienode sequence
    if (node->left_ptr != NULL) {
        free_trienode(node->left_ptr);
    }
    if (node->right_ptr != NULL) {
        free_trienode(node->right_ptr);
    }
    free(node);
};

void insert_trie(TrieNode* root, uint32_t prefix, int prefix_len, int interface) {
    // Inserts the prefix into the trie
    TrieNode* temp = root;

    // if we have a default port assigned (default gateway?)
    if (prefix_len == 0){
        root->port = interface;
        return;
    }

    for (int i = 0; i < prefix_len; i++) {
        uint32_t bit = 1U << (31-i);

        if (prefix & bit) {  // if the bit we are checking is 1
            if (temp->right_ptr == NULL){ // if the right child doesn't exist
                temp->right_ptr = make_trienode(i+1); //new TrieNode, we don't need to store bitstring in every i think
            }
            temp = temp->right_ptr; // we go deeper
        }
        else { //if the bit we are checking is 0
            if (temp->left_ptr == NULL){
                temp->left_ptr = make_trienode(i+1);
            }
            temp = temp->left_ptr;
        }
    }
    // end of loop
    temp->prefix = prefix; // we only care about prefixes in port nodes
    temp->prefix_len = prefix_len; //Added this so I could detect weather a prefix matches a IPAddress (as a shift amount)
    temp->port = interface;
}

int countNodes(TrieNode* node){
    //returns the number of nodes in the trie, starting from the given node (including that node)
    if (node == NULL){
        return 0;
    }
    return 1 + countNodes(node->left_ptr) + countNodes(node->right_ptr);
}

int countChildren(TrieNode* node){
    //counts all the children of a given node
    int result = 0;

    if (node->left_ptr != NULL){
        result ++;
    }

    if (node->right_ptr != NULL){
        result++;
    }

    return result;
}

bool toCompress(TrieNode* node){
    //returns true if a node should be compressed, otherwise false
    TrieNode* childNode;
    if (countChildren(node) == 1){
        if (node->left_ptr != NULL){
            childNode = node->left_ptr;
        }
        else{
            childNode = node->right_ptr;
        }

        if (node->port * childNode->port == 0){ // at least one of the nodes doesn't have a port assigned
            return true;
        }
    }
    return false;
}

void deleteAndUpdate(TrieNode* child, TrieNode* node, TrieNode* parent){
    //compresses a single link between nodes and deletes redundant node

    if (node->port != 0){ //if node has a port assigned to it, we assign it to the child
        child->port = node->port;
        child->prefix = node->prefix;
        child->prefix_len = node->prefix_len;
    }

    //linking parent of a node to the correct child
    if (parent->left_ptr == node){
        parent->left_ptr = child;
    }
    else{
        parent->right_ptr = child;
    }

    free(node); //free the memory space used for the deleted node
}

void compressTrie(TrieNode* root){
    // compresses the whole structure of the trie
    if (root->left_ptr != NULL){
        while(toCompress(root->left_ptr)){
            if (root->left_ptr->left_ptr != NULL){
                deleteAndUpdate(root->left_ptr->left_ptr, root->left_ptr, root);
            }
            else{
                deleteAndUpdate(root->left_ptr->right_ptr, root->left_ptr, root);
            }
        }

        compressTrie(root->left_ptr);
    }

    if (root->right_ptr != NULL){
        while(toCompress(root->right_ptr)){
            if (root->right_ptr->left_ptr != NULL){
                deleteAndUpdate(root->right_ptr->left_ptr, root->right_ptr, root);
            }
            else{
                deleteAndUpdate(root->right_ptr->right_ptr, root->right_ptr, root);
            }
        }
    }
}

void createTrie(TrieNode *root) {
    // creates and compresses trie
    uint32_t prefix;
    int prefixLength, outInterface;

    while (readFIBLine(&prefix, &prefixLength, &outInterface) == OK) {
        insert_trie(root, prefix, prefixLength, outInterface);
    }

    compressTrie(root);
}

int findPort(TrieNode *root, uint32_t IPAddress, int *tableAccesses) {
    // finds and returns a port given an IPAddress within the routing table and tracks table accesses
    TrieNode* temp = root;
    uint32_t bit;
    uint32_t shifted_addr;
    int return_port = temp->port;

    while (temp->left_ptr != NULL || temp->right_ptr != NULL) {
        bit = 1U << (31-temp->bit_position);

        if (!(IPAddress & bit) && temp->left_ptr != NULL) { // If the address at the next bit position is 0 and there is another node, move left
            temp = temp->left_ptr;
            *tableAccesses += 1;
            // shift it right and then back left by 32-prefix_len to compare addr to prefix
            if (temp->prefix_len == 0) {
                shifted_addr = 0;
            } else {
                shifted_addr = (IPAddress >> (32 - temp->prefix_len)) << (32 - temp->prefix_len);
            }
            // updating return port if bitstring matches shifted address and has a port
            if (temp->port != 0 && temp->prefix == shifted_addr) {
                return_port = temp->port;
            }

        } else if ((IPAddress & bit) && temp->right_ptr != NULL) { // If the address at the next bit position is 1 and there is another node, move right
            temp = temp->right_ptr;
            *tableAccesses += 1;
            // shift it right and then back left by 32-prefix_len to compare addr to prefix
            if (temp->prefix_len == 0) {
                shifted_addr = 0;
            } else {
                shifted_addr = (IPAddress >> (32 - temp->prefix_len)) << (32 - temp->prefix_len);
            }
            // updating return port if bitstring matches shifted address and has a port
            if (temp->port != 0 && temp->prefix == shifted_addr) {
                return_port = temp->port;
            }

        } else { // No further matching bit in trie, return
            return return_port;
        }
    }
    return return_port;
}

void processAddresses(TrieNode *root, int *numPktsProcessed, int *totalTableAccesses, double *totalPacketProcessingTime) {
    uint32_t IPAddress;
    int port;
    struct timespec initialTime, finalTime;
    double searchingTime;
    int tableAccesses = 0;

    while (readInputPacketFileLine(&IPAddress) == OK) {

        clock_gettime(CLOCK_MONOTONIC_RAW, &initialTime);
        port = findPort(root, IPAddress, &tableAccesses);
        clock_gettime(CLOCK_MONOTONIC_RAW, &finalTime);

        printOutputLine(IPAddress, port, &initialTime, &finalTime, &searchingTime, tableAccesses);

        *numPktsProcessed += 1;
        *totalTableAccesses += tableAccesses;
        *totalPacketProcessingTime += searchingTime;
    }
}



int main(int argc, char *argv[]) {

    int numPktsProcessed = 0;
    int totalTableAccesses = 0;
    double totalPacketProcessingTime = 0;

    int err = initializeIO(argv[1], argv[2]);
    if (err != OK) {
        printIOExplanationError(err);
        exit(0);
    }

    TrieNode* root = make_trienode(0);

    createTrie(root);

    int nodes = countNodes(root);

    processAddresses(root, &numPktsProcessed, &totalTableAccesses, &totalPacketProcessingTime);

    printf("\nNumber of nodes in the tree= %d", nodes);
    printSummary(numPktsProcessed, totalTableAccesses/numPktsProcessed, totalPacketProcessingTime/numPktsProcessed);

    free_trienode(root);

    return 0;
}