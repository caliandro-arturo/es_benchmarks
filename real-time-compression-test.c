/**
 * @file real-time-compression.c
 * @brief Implementation of the Huffman coding algorithm. A heap and a binary
 * tree are used to implement the queue and the main structure of the coder.
 */

#include <math.h>
#include <stdio.h>

char input[] =
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do "
    "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim "
    "ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut "
    "aliquip ex ea commodo consequat. Duis aute irure dolor in reprehenderit "
    "in voluptate velit esse cillum dolore eu fugiat nulla pariatur. "
    "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui "
    "officia deserunt mollit anim id est laborum.";
unsigned int INPUT_SIZE = sizeof(input) - 1;

// 'word' here means a int-sized variable.

// Set the specified bit of the word to 1.
#define BIT_SET(word, bit) ((word) |= (1 << (bit)))
// Set the specified bit to 0.
#define BIT_CLEAR(word, bit) ((word) &= ~(1 << (bit)))
// Get the value of the read bit. Use it to compare to 0 only.
#define BIT_READ(word, bit) ((word) & (1 << (bit)))
// Get the index of the character bit inside a byte.
#define CHAR_BIT_INDEX(chr) (((chr) - ' ') % (8 * sizeof(int)))
// Get the index of the byte associated to the character.
#define CHAR_MAP_INDEX(chr) (((chr) - ' ') / (8 * sizeof(int)))

#define CHAR_DOMAIN_LEN 95 // == '~' - ' ' + 1;

// Adapt if the architecture's int size is not 4 bytes
#define SYMBOL_BYTE_LEN 3 // (ceil(CHAR_DOMAIN_LEN / (8.0 * sizeof(int))));

/**
 * @brief Node to be used for both heap and tree.
 */
typedef struct {
    // Bit map for the characters. Each bit represents the presence of a
    // character into the symbol.
    unsigned int symbol[SYMBOL_BYTE_LEN];

    // Frequency of the symbol.
    // If -1, the node is considered empty.
    int weight;

    // Fields used by the tree. -1 means no child.
    int left, right;
    int inserted_at;
} Node;

int compute_input_statistics(int freq[CHAR_DOMAIN_LEN]);

// Heap

// Initialize the heap
void init_heap(int size, Node heap[size], int freq[CHAR_DOMAIN_LEN]);
// Insert a node in the heap
void insert_in_heap(int *size, Node *heap, Node node);
// Pop a node out of the heap
Node pop(int *size, Node *heap);

// Tree

// Merge the symbol of two nodes, given their position in the tree, and return
// the merged node
Node merge_nodes(Node *tree, int node_a, int node_b);
// Insert a node in the tree, at the end of the tree
void insert_in_tree(unsigned int *size, Node *tree, Node *node);
// Encode the character, and return in len the number of bits
int encode(int size, Node *tree, char ch, int *len); // TODO
// Decode until a character has been obtained, and return the amount of bits
// used to decode
char decode(int size, Node *tree, int input, int *len); // TODO

void print_symbol(unsigned int symbol[3]);

int main() {
    // Compress the input
    // Evaluate character statistics
    int freq[CHAR_DOMAIN_LEN] = {0};
    int total = compute_input_statistics(freq);
    Node priority_queue[total];
    init_heap(total, priority_queue, freq);

    // TODO: implement actual Huffman coding:
    // while(at least 2 nodes inside the queue/heap) {
    //   pop two nodes from the queue;
    //   merge them in a new node;
    //   add the three nodes in the three, the merge one as the parent;
    //   insert the merge node into the heap;
    // }
    // sort the tree (hardest part imho);

    // The size of the code is not bigger than the size of the input.
    int huffman_code_space = ceilf((float)INPUT_SIZE / sizeof(int));
    unsigned int code[huffman_code_space]; // The code is expressed bit-wise
    int code_len = 0;
    // TODO:
    // Encode the string as bits in the variable "code";
    // Print the output (on test version);
    // Decode the string;
    // Ensure that the decoding matches with the original string;
    return 0;
}

/**
 * @brief Counts the amount of different characters in the input and their
 * frequencies.
 *
 * @param freq the array to use as histogram
 * @return int: the amount of unique characters
 */
int compute_input_statistics(int freq[CHAR_DOMAIN_LEN]) {
    int total = 0;
    int next_char;
    for (unsigned int i = 0; i < INPUT_SIZE; ++i) {
        next_char = input[i] - ' ';
        if (freq[next_char] == 0) {
            total++;
        }
        freq[next_char]++;
    }
    return total;
}

/**
 * @brief Parent of the node, given the index. The root is the only node
 * that returns a negative index (namely -1).
 *
 * @param pos the position of the child
 * @return int: the position of the parent
 */
int parent(int pos) { return (pos - 1) / 2; }

/**
 * @brief Left child of the node. No check against its actual existence is
 * performed.
 *
 * @param pos the position of the parent
 * @return int: the position of the left child
 */
int left(int pos) { return 2 * pos + 1; }

/**
 * @brief Right child of the node. No check against its actual existence is
 * performed.
 *
 * @param pos the position of the parent
 * @return int: the position of the right child
 */
int right(int pos) { return 2 * pos + 2; }

void swap(Node *a, Node *b) {
    Node t = *a;
    *a = *b;
    *b = t;
}

void heapify(int size, Node heap[size]) {
    int start = size >> 1;
    int root, child;
    while (start > 0) {
        --start;
        // Sift down (start, end)
        root = start;
        while (left(root) < size) {
            child = left(root);
            if (child + 1 < size &&
                heap[child].weight > heap[child + 1].weight) {
                // here the '>' makes the difference between max and min sort
                ++child;
            }
            if (heap[root].weight > heap[child].weight) { // Here again
                swap(&heap[root], &heap[child]);
                root = child;
            } else {
                break;
            }
        }
    }
}

/**
 * @brief Initializes the heap with nodes such that each node contains a
 * character and its frequency.
 *
 * @param size the size of the heap
 * @param heap the heap
 * @param freq the reference histogram
 */
void init_heap(int size, Node heap[size], int freq[CHAR_DOMAIN_LEN]) {
    int cur = 0;
    for (int i = 0; i < CHAR_DOMAIN_LEN && cur < size; ++i) {
        if (freq[i] > 0) {
            // Make node:
            // 1. Set the symbol bitmap to 0
            for (int j = 0; j < 3; ++j) {
                heap[cur].symbol[j] = 0;
            }
            // 2. Set the character bit to 1
            char ch = i + ' ';
            int byte_index = CHAR_MAP_INDEX(ch);
            int bit_index = CHAR_BIT_INDEX(ch);
            BIT_SET(heap[cur].symbol[byte_index], bit_index);
            // 3. Initialize other parameters to default
            heap[cur].weight = freq[i];
            heap[cur].inserted_at = -1;
            heap[cur].left = -1;
            heap[cur].right = -1;
            ++cur;
        }
    }
    heapify(cur, heap);
    // if cur != size, there should be an error somewhere
}

void insert_in_heap(int *size, Node *heap, Node node) {
    // Inserts occurr always after two pops, so out-of-bound checks should
    // never be necessary.
    heap[*size] = node;
    int curr = *size;
    while (curr > 0 && heap[curr].weight < heap[parent(curr)].weight) {
        swap(&heap[parent(curr)], &heap[curr]);
        curr = parent(curr);
    }
    ++(*size);
}

Node pop(int *size, Node *heap) {
    Node to_extract = heap[0];
    --(*size);
    heap[0] = heap[*size];
    int curr = 0;
    int candidate;
    while (left(curr) < *size) {
        candidate = left(curr);
        if (right(curr) < *size &&
            heap[candidate].weight > heap[right(curr)].weight) {
            candidate = right(curr);
        }
        if (heap[candidate].weight > heap[curr].weight) {
            break;
        }
        swap(&heap[curr], &heap[candidate]);
        curr = candidate;
    }
    return to_extract;
}

Node merge_nodes(Node *tree, int node_a, int node_b) {
    Node a = tree[node_a];
    Node b = tree[node_b];
    Node merge = {.left = node_a,
                  .right = node_b,
                  .weight = a.weight + b.weight,
                  .inserted_at = -1};
    for (int i = 0; i < SYMBOL_BYTE_LEN; ++i) {
        merge.symbol[i] = a.symbol[i] | b.symbol[i];
    }
    return merge;
}

void insert_in_tree(unsigned int *size, Node *tree, Node *node) {
    if (node->inserted_at != -1) {
        // Already inserted
        return;
    }
    node->inserted_at = *size;
    tree[*size] = *node;
    ++(*size);
}

unsigned int encode(int size, Node *tree, char ch, unsigned int *len) {
    unsigned int ch_byte = CHAR_MAP_INDEX(ch);
    unsigned int ch_bit = CHAR_BIT_INDEX(ch);
    unsigned int code = 0;
    Node node = tree[size - 1];
    while (node.left != -1 && node.right != -1) {
        if (BIT_READ(tree[node.left].symbol[ch_byte], ch_bit) != 0) {
            code <<= 1;
            node = tree[node.left];
        } else {
            code = (code << 1) | 1;
            node = tree[node.right];
        } // all cases should have already been covered...
        ++(*len);
    }
    return code;
}

char decode(int size, Node *tree, unsigned int input, unsigned int *len) {
    // Input goes from MSB to LSB
    Node node = tree[size - 1];
    unsigned int index;
    unsigned int MSB = sizeof(int) * 8 - 1;
    do {
        if (BIT_READ(input, MSB) != 0) {
            node = tree[node.right];
        } else {
            node = tree[node.left];
        }
        ++(*len);
        input <<= 1;
    } while (node.left != -1 && node.right != -1);
    for (unsigned int i = 0; i < SYMBOL_BYTE_LEN; ++i) {
        if (node.symbol[i] != 0) {
            index = i;
            break;
        }
    }
    unsigned int bit;
    // ctz has UB if there is no trailing zero
    if (node.symbol[index] == 1) {
        bit = 0;
    } else {
        bit = __builtin_ctz(node.symbol[index]);
    }
    char ch = bit + sizeof(int) * 8 * index + ' ';
    return ch;
}
