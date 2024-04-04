#include <stdio.h>
#include <stdlib.h>
#include "io.h"


typedef struct TrieNode TrieNode;

struct TrieNode {
  int prefix;
  int bit_position;
  int port;
  TrieNode* left_ptr;
  TrieNode* right_ptr;
};

static TrieNode* root;

TrieNode* make_trienode(int prefix, int port) {
    // Allocate memory for a TrieNode
    TrieNode* node = (TrieNode*)malloc( sizeof(TrieNode));
    node->left_ptr = NULL;
    node->right_ptr = NULL;
    node->prefix = prefix;
    node->port = port;
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

void print_trie(TrieNode* node) {
    // Allocate memory for a TrieNode
    printf("Prefix: %d\n", node->prefix);
    if (node->left_ptr != NULL) {
        print_trie(node->left_ptr);
    } else {
        printf("Left is null\n");
    }
    if (node->right_ptr != NULL) {
        print_trie(node->left_ptr);
    } else {
        printf("Right is null\n");
    }
};

// TrieNode* insert_trie(TrieNode* root, int prefix) {
//     // Inserts the prefix onto the Trie
//     TrieNode* temp = root;


//     for (int i = 31; i >= 0; i--) {
//         uint32_t mask = 1U << i; // Create a mask for the current bit
//         printf("%d", (num & mask) ? 1 : 0); // Print the bit (1 or 0)
//     }

//     for (int i=0; prefix[i] != NULL; i++) {
//         if (temp->left_ptr == NULL) {
//             // If the corresponding child doesn't exist,
//             // simply create that child!
//             temp->left_ptr[i] = make_trienode(prefix[i]);
//         }
//         else {
//             // Do nothing. The node already exists
//         }
//         // Go down a level, to the child referenced by idx
//         // since we have a prefix match
//         temp = temp->children[idx];
//     }
//     // At the end of the word, mark this node as the leaf node
//     temp->is_leaf = 1;
//     return root;
// }

void createTrie(TrieNode *root) {
    uint32_t prefix;
    int prefixLength, outInterface;

    while (readFIBLine(&prefix, &prefixLength, &outInterface) == OK) {
        printf("FIB Entry: Prefix: %u, Prefix Length: %d, Out Interface: %d\n", prefix, prefixLength, outInterface);
        // insert_trie(root, prefix);
    }
    print_trie(root);
}


int main(int argc, char *argv[]) {

    int err = initializeIO(argv[1], argv[2]);
    if (err != OK) {
        printIOExplanationError(err);
        exit(0);
    }

    root = make_trienode(NULL, 0);

    createTrie(root);

    free_trienode(root);

    return 0;
}