#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> //USING BOOL FOR toCompress()
#include "io.h"


typedef struct TrieNode TrieNode;

struct TrieNode {
    uint32_t prefix; //we store the whole prefix bcs its only 32 bits, so basically the same as 4 chars
    int bit_position; //starting from 0, not from 1 for quality of our life, care for that
    int port; // if a node doesn't have a port assigned it goes 0
    TrieNode* left_ptr;
    TrieNode* right_ptr;
};

static TrieNode* root; //idk if that's the way to go, i don't have access to memory data and stuff from the online compiler

TrieNode* make_trienode(int bit_position) { //we need a different function for making nodes with port and without
    // Allocate memory for a TrieNode
    TrieNode* node = (TrieNode*)malloc( sizeof(TrieNode));
    node->left_ptr = NULL;
    node->right_ptr = NULL;
    node->bit_position = bit_position;
    node->prefix = 0; // let's assume 0 at start, might be changed later
    node->port = 0; //port number is 0 as default
    //NOTE: WE ONLY CARE ABOUT PREFIXES OF NODES WITH PORTS ASSIGNED TO THEM


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

void print_node(TrieNode* node){
    //PRINTS INFO ABOUT A SINGLE NODE
    printf("Prefix: ");
    if (node->port == 0){ //if a node doesn't have a port assigned, we don't print prefix to avoid confusion
        printf("none");
    }
    else{
        for (int i = 0; i < node->bit_position; i++){
            uint32_t mask = 1U  << (31-i); //that gives us a 00000000000001000000000 uint with 1 at the position of i
            printf("%d", (node->prefix & mask) ? 1 : 0); //PRINTS THE PREFIX
        }
    }
    printf(", bit position: %d, port interface: %d\n", node->bit_position, node->port);
}

void print_trie(TrieNode* node) {
    // PRINTS THE WHOLE TREE USING print_node() (PRE-ORDER)
    print_node(node);
    if (node->left_ptr != NULL) {
        printf("Left: ");
        print_trie(node->left_ptr);
    } else {
        printf("Left is null\n");
    }
    if (node->right_ptr != NULL) {
        printf("Right: ");
        print_trie(node->right_ptr);
    } else {
        printf("Right is null\n");
    }
};

void insert_trie(TrieNode* root, uint32_t prefix, int prefix_len, int interface) {
    //INSERTS THE PREFIX INTO THE TRIE
    TrieNode* temp = root;

    //IF YOU WANT TO PRINT THE WHOLE ADDRESS IN BINARY FOR SOME REASON:

    //for (int i = 31; i >= 0; i--) {
    //  uint32_t mask = 1U << i; // Create a mask for the current bit
    //printf("%d", (prefix & mask) ? 1 : 0); // Print the bit (1 or 0)
    //}


    //IF WE HAVE A DEFAULT PORT ASSIGNED (default gateway?)
    if (prefix_len == 0){
        root->port = interface;
        return;
    }

    //PROMPT FOR PRINTING OUT THE PROCESSED PREFIX
    printf("calculating for: ");

    for (int i = 0; i < prefix_len; i++) {
        uint32_t bit = 1U << (31-i);
        printf("%d", (prefix & bit) ? 1 : 0); //PRINTING OUT THE PROCESSED PREFIX

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
    temp->port = interface; // go girl!!

    printf("\n");
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

        if (node->port * childNode->port == 0){ //atleast one of the nodes doesn't have a port assigned
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

        compressTrie(root->right_ptr);
    }
}


void createTrie(TrieNode *root) {
    //CREATES THE WHOOOOOOOOOOOOOOOOOOOLE TRIE AND COMPRESSES IT
    uint32_t prefix;
    int prefixLength, outInterface;

    while (readFIBLine(&prefix, &prefixLength, &outInterface) == OK) {
        //printf("FIB Entry: Prefix: %u, Prefix Length: %d, Out Interface: %d\n", prefix, prefixLength, outInterface);
        insert_trie(root, prefix, prefixLength, outInterface);
    }
    //print_trie(root);

    compressTrie(root);
}


int main(int argc, char *argv[]) {

    int err = initializeIO("routing_table_presentation.txt", "prueba0.txt");
    if (err != OK) {
        printIOExplanationError(err);
        exit(0);
    }
    root = make_trienode(0); // CREATING A ROOT NOODE
    createTrie(root); //CREATING THE TREE

    print_trie(root); //PRINTING OUT THE TRIE

    int nodes = countNodes(root); //INFO ABOUT THE AMOUNT OF NODES
    printf("countNodes: %d\n", nodes);
    //int children = countChildren(root);
    //printf("countChildren root: %d\n", children);

    free_trienode(root); // FREE THE TRIE

    return 0;
}

//I LIKE WRITING IT CAPS IT FEELS LIKE IM SCREAMING