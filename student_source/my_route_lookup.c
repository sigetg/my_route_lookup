#include <stdio.h>
#include <stdlib.h>
#include "io.h"


typedef struct TrieNode TrieNode;

struct TrieNode {
    uint32_t prefix;
    int bit_position;
    int port;
    TrieNode* left_ptr;
    TrieNode* right_ptr;
};

static TrieNode* root;

TrieNode* make_trienode(int bit_position) { //we need a different function for making nodes with port and without
    // Allocate memory for a TrieNode
    TrieNode* node = (TrieNode*)malloc( sizeof(TrieNode));
    node->left_ptr = NULL;
    node->right_ptr = NULL;
    node->bit_position = bit_position;
    node->prefix = 0; // lets assume 0 at start, might be changed later
    node->port = 0; //port number is 0 as default, if we need to overwrite it we do it later
    return node;
};

void free_trienode(TrieNode* node) {
    // Free the trienode sequence
    if (node->left_ptr != NULL) {
        free_trienode(node->left_ptr);
    }
    if (node->right_ptr != NULL) {
        free_trienode(node->left_ptr);
    }
    free(node);
};

void print_node(TrieNode* node){
    printf("Prefix: ");
    if (node->bit_position == 0){
        printf("none");
    }
    else{
        for (int i = 0; i < node->bit_position; i++){
            uint32_t mask = 1U  << i;
            printf("%d", (node->prefix & mask) ? 1 : 0); // idk if that doesn't work or the prefix
        }
    }
    printf(", bit position: ");
    printf("%d", node->bit_position);
    printf(", port interface: ");
    printf("%d\n", node->port);
}

void print_trie(TrieNode* node) {
    // Allocate memory for a TrieNode
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

TrieNode* insert_trie(TrieNode* root, uint32_t prefix, int prefix_len, int interface) {
    // Inserts the prefix onto the Trie
    TrieNode* temp = root;
    //for (int i = 31; i >= 0; i--) {
    //  uint32_t mask = 1U << i; // Create a mask for the current bit
    //printf("%d", (prefix & mask) ? 1 : 0); // Print the bit (1 or 0)
    //}

    if (prefix_len == 0){
        root->port = interface;
        return root;
    }

    for (int i = 0; i < prefix_len; i++) {
        uint32_t bit = 1U << (31-i);
        if (prefix & bit) {  // if the bit we are checking is 1
            if (temp->right_ptr == NULL){ // if the right child doesn't exist
                temp->right_ptr = make_trienode(i+1); //new TrieNode, we don't need to store bitstring in every i think
            }
            temp = temp->right_ptr; // we go deeper
        }
        else { // bit is 0
            if (temp->left_ptr == NULL){
                temp->left_ptr = make_trienode(i+1);
            }
            temp = temp->left_ptr;
        }
    }
    // end of loop

    temp->prefix = prefix; // we only care about prefixes in port nodes idk if that works
    temp->port = interface; // go girl!!

    return root;
}

void createTrie(TrieNode *root) {
    uint32_t prefix;
    int prefixLength, outInterface;

    while (readFIBLine(&prefix, &prefixLength, &outInterface) == OK) {
        printf("FIB Entry: Prefix: %u, Prefix Length: %d, Out Interface: %d\n", prefix, prefixLength, outInterface);
        insert_trie(root, prefix, prefixLength, outInterface);
    }
    print_trie(root);
}

int main(int argc, char *argv[]) {

    //did that cause i cba by running the file from console
    int err = initializeIO("routing_table_simple.txt", "prueba0.txt");
    if (err != OK) {
        printIOExplanationError(err);
        exit(0);
    }
    root = make_trienode(0); // while making a trie node we only care about the bit position
                                        //prefixes and port numbers will be changed later
    createTrie(root);
    free_trienode(root);

    return 0;
}