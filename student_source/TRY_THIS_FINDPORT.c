//
//that's probably not the fastest though
bool checkPrefix(uint32_t prefix, int prefix_len, uint32_t ip_address){
    uint32_t bit;

    //iterate through every bit
    for (int i = 0; i < prefix_len; i++){
        bit = 1U << (31-i);
        if ((bit & prefix) != (bit & ip_address)){
            return false;
        }
    }
    return true;
}


int findPort(TrieNode *root, uint32_t IPAddress, int *tableAccesses) {
    TrieNode* temp = root;
    uint32_t bit;
    int return_port = temp->port;

    while (temp->left_ptr != NULL || temp->right_ptr != NULL) { //don't know if i need this outer one
        bit = 1U << (31-temp->bit_position);
        // printf("\nAdr: ");
        // printBits(&IPAddress);
        // printf("Bit: ");
        // printBits(&bit);
        // printf("Bit Pos: %u\n", temp->bit_position);


        if (!(IPAddress & bit) && temp->left_ptr != NULL) { // If the address at the next bit position is 0 and there is another node, move left
            temp = temp->left_ptr;
            *tableAccesses += 1;
            // printf("LEFT!\n");

            //

            //check if the prefixes are matching -> if they are, we update return port, if they don't we return the BMP
            if (temp->port != 0) {
                if (checkPrefix(temp->prefix, temp->prefix_len, IPAddress)){
                    return_port = temp->port;
                }
            } else {
                return return_port;
            }

            // printf("Prefix Length: %i\nShiAddy: ", temp->prefix_len);
            // printBits(&shifted_addr);
            // printf("Prefixx: ");
            // printBits(&temp->prefix);
            // printf("RegAddy: ");
            // printBits(&IPAddress);
            // printf("Port: %i\n", temp->port);

        } else if ((IPAddress & bit) && temp->right_ptr != NULL) { // If the address at the next bit position is 1 and there is another node, move right
            temp = temp->right_ptr;
            *tableAccesses += 1;
            // printf("RIGHT!\n");

            //same drill
            if (temp->port != 0) {
                if (checkPrefix(temp->prefix, temp->prefix_len, IPAddress)){
                    return_port = temp->port;
                }
            } else { //if prefix isnt matching -> we return and arrivederci
                return return_port;
            }

            // printf("Prefix Length: %i\nShiAddy: ", temp->prefix_len);
            // printBits(&shifted_addr);
            // printf("Prefixx: ");
            // printBits(&temp->prefix);
            // printf("RegAddy: ");
            // printBits(&IPAddress);
            // printf("Port: %i\n", temp->port);
        } else { // No further matching bit in trie, return
            return return_port;
        }
    }
    return return_port;
}
// Created by domin on 08.04.2024.
//
