/**
 * @file huffman-compression.c
 * @brief Implementation of the Huffman coding algorithm. A heap and a binary
 * tree are used to implement the queue and the main structure of the coder.
 */

#include "huffman-compression.h"
#include <math.h>

#define INT_BIT_SIZE sizeof(int) * 8

// 'word' here means a int-sized variable.

// Set the specified bit of the word to 1.
#define BIT_SET(word, bit) ((word) |= (1 << (bit)))
// Set the specified bit to 0.
#define BIT_CLEAR(word, bit) ((word) &= ~(1 << (bit)))
// Get the value of the read bit. Use it to compare to 0 only.
#define BIT_READ(word, bit) ((word) & (1 << (bit)))
// Get the index of the character bit inside a byte.
#define CHAR_BIT_INDEX(chr) (((chr) - ' ') % (INT_BIT_SIZE))
// Get the index of the byte associated to the character.
#define CHAR_MAP_INDEX(chr) (((chr) - ' ') / (INT_BIT_SIZE))

#define CHAR_DOMAIN_LEN 95 // == '~' - ' ' + 1;

// Adapt if the architecture's int size is not 4 bytes
#define SYMBOL_BYTE_LEN 3 // (ceil(CHAR_DOMAIN_LEN / INT_BIT_SIZE));

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

unsigned int compute_input_statistics(unsigned int input[HUFFMAN_INPUT_SIZE],
                                      unsigned int freq[CHAR_DOMAIN_LEN]);

// Heap

// Initialize the heap
void init_heap(unsigned int size, Node heap[size],
               unsigned int freq[CHAR_DOMAIN_LEN]);
// Insert a node in the heap
void insert_in_heap(unsigned int *size, Node *heap, Node node);
// Pop a node out of the heap
Node pop(unsigned int *size, Node *heap);

// Tree

void init_huffman_tree(Node *heap, unsigned int *heap_size, Node *tree,
                       unsigned int *tree_size);
// Merge the symbol of two nodes, given their position in the tree, and return
// the merged node
Node merge_nodes(Node *tree, unsigned int node_a, unsigned int node_b);
// Insert a node in the tree, at the end of the tree
void insert_in_tree(unsigned int *size, Node *tree, Node *node);
unsigned int encode_input(unsigned int input[HUFFMAN_INPUT_SIZE], Node *tree,
                          unsigned int tree_size, unsigned int *code);
void decode_code(unsigned int *code, unsigned int code_len, Node *tree,
                 unsigned int tree_size, char output[HUFFMAN_INPUT_SIZE + 1]);

void huffman_compression(unsigned int input[HUFFMAN_INPUT_SIZE]) {
    // Compress the input
    // Evaluate character statistics
    unsigned int freq[CHAR_DOMAIN_LEN] = {0};
    // Total amount of unique characters
    unsigned int total = compute_input_statistics(input, freq);
    Node priority_queue[total];
    init_heap(total, priority_queue, freq);
    unsigned int heap_size = total;
    // Make the tree
    unsigned int tree_size = 0;
    Node tree[2 * total - 1];
    init_huffman_tree(priority_queue, &heap_size, tree, &tree_size);
    // Encode the input
    unsigned int huffman_code_space =
        ceilf((float)HUFFMAN_INPUT_SIZE /
              sizeof(int)); // The size of the code is not bigger
                            // than the size of the input.
    unsigned int code[huffman_code_space];
    unsigned int code_len = encode_input(input, tree, tree_size, code);

    // Ensure that the decoded string matches with the original one
    char decoded[HUFFMAN_INPUT_SIZE + 1];
    decoded[HUFFMAN_INPUT_SIZE] = '\0';
    decode_code(code, code_len, tree, tree_size, decoded);
}

/**
 * @brief Counts the amount of different characters in the input and their
 * frequencies.
 *
 * @param input: the input array
 * @param freq: the array to use as histogram
 * @return int: the amount of unique characters
 */
unsigned int compute_input_statistics(unsigned int input[HUFFMAN_INPUT_SIZE],
                                      unsigned int freq[CHAR_DOMAIN_LEN]) {
    int total = 0;
    int next_char;
    for (unsigned int i = 0; i < HUFFMAN_INPUT_SIZE; ++i) {
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
unsigned int parent(unsigned int pos) { return (pos - 1) / 2; }

/**
 * @brief Left child of the node. No check against its actual existence is
 * performed.
 *
 * @param pos the position of the parent
 * @return int: the position of the left child
 */
unsigned int left(int pos) { return 2 * pos + 1; }

/**
 * @brief Right child of the node. No check against its actual existence is
 * performed.
 *
 * @param pos the position of the parent
 * @return int: the position of the right child
 */
unsigned int right(int pos) { return 2 * pos + 2; }

void swap(Node *a, Node *b) {
    Node t = *a;
    *a = *b;
    *b = t;
}

void heapify(unsigned int size, Node heap[size]) {
    unsigned int start = size >> 1;
    unsigned int root, child;
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
void init_heap(unsigned int size, Node heap[size],
               unsigned int freq[CHAR_DOMAIN_LEN]) {
    unsigned int cur = 0;
    for (unsigned int i = 0; i < CHAR_DOMAIN_LEN && cur < size; ++i) {
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

void insert_in_heap(unsigned int *size, Node *heap, Node node) {
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

Node pop(unsigned int *size, Node *heap) {
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

// TREE

void init_huffman_tree(Node *heap, unsigned int *heap_size, Node *tree,
                       unsigned int *tree_size) {
    while (*heap_size > 1) {
        Node a = pop(heap_size, heap);
        if (a.inserted_at == -1) {
            insert_in_tree(tree_size, tree, &a);
        }
        Node b = pop(heap_size, heap);
        if (b.inserted_at == -1) {
            insert_in_tree(tree_size, tree, &b);
        }
        Node merge = merge_nodes(tree, a.inserted_at, b.inserted_at);
        insert_in_tree(tree_size, tree, &merge);
        insert_in_heap(heap_size, heap, merge);
    }
}

Node merge_nodes(Node *tree, unsigned int node_a, unsigned int node_b) {
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

unsigned int encode(unsigned int size, Node *tree, char ch, unsigned int *len) {
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

unsigned int encode_input(unsigned int input[HUFFMAN_INPUT_SIZE], Node *tree,
                          unsigned int tree_size, unsigned int *code) {
    unsigned int code_len = 0;
    unsigned int curr_cell = 0, curr_cell_bit = 0;
    // Used to store single encoded characters
    unsigned int piece, piece_len = 0;
    // If information is fragmented, use these
    unsigned int first_fragment_len;
    for (unsigned int i = 0; i < HUFFMAN_INPUT_SIZE; ++i) {
        piece = encode(tree_size, tree, input[i], &piece_len);
        // If the next chunk overlaps between two cells, cut it in two
        if (curr_cell_bit + piece_len > INT_BIT_SIZE) {
            first_fragment_len = INT_BIT_SIZE - curr_cell_bit;
            code[curr_cell] = (code[curr_cell] << first_fragment_len) |
                              (piece >> (piece_len - first_fragment_len));
            code_len += first_fragment_len;
            piece_len -= first_fragment_len;
            piece &= ((1 << piece_len) - 1);
            curr_cell_bit += first_fragment_len;
        }
        if (curr_cell_bit == INT_BIT_SIZE) {
            ++curr_cell;
            curr_cell_bit = 0;
            code[curr_cell] = 0;
        }
        code[curr_cell] = (code[curr_cell] << piece_len) | piece;
        curr_cell_bit += piece_len;
        code_len += piece_len;
        piece_len = 0;
    }
    // Align to the left
    code[curr_cell] <<= (INT_BIT_SIZE - curr_cell_bit);
    return code_len;
}

char decode(unsigned int size, Node *tree, unsigned int input,
            unsigned int *len) {
    // Input goes from MSB to LSB
    Node node = tree[size - 1];
    unsigned int index;
    unsigned int MSB = INT_BIT_SIZE - 1;
    do {
        if (BIT_READ(input, MSB) != 0) {
            node = tree[node.right];
        } else {
            node = tree[node.left];
        }
        ++(*len);
        input <<= 1;
    } while (node.left != -1 && node.right != -1);
    for (index = 0; index < SYMBOL_BYTE_LEN; ++index) {
        if (node.symbol[index] != 0) {
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
    char ch = bit + INT_BIT_SIZE * index + ' ';
    return ch;
}

void decode_code(unsigned int *code, unsigned int code_len, Node *tree,
                 unsigned int tree_size, char output[HUFFMAN_INPUT_SIZE + 1]) {
    unsigned int next_ch_index = 0;
    unsigned int to_decode = code_len;
    unsigned int first_fragment_len;
    unsigned int next_cell = 1;
    unsigned int next_cell_bit = 0;
    unsigned int piece_len = 0;
    unsigned int input_chunk = code[0];
    while (to_decode > 0) {
        // Assuming that each symbol is not going to take more than 4 bytes
        output[next_ch_index++] =
            decode(tree_size, tree, input_chunk, &piece_len);
        to_decode -= piece_len;
        if (next_cell_bit + piece_len > INT_BIT_SIZE) {
            first_fragment_len = INT_BIT_SIZE - next_cell_bit;
            input_chunk = (input_chunk << first_fragment_len) |
                          (code[next_cell] << next_cell_bit >>
                           (INT_BIT_SIZE - first_fragment_len));
            next_cell_bit = INT_BIT_SIZE;
            piece_len -= first_fragment_len;
        }
        if (next_cell_bit == INT_BIT_SIZE) {
            ++next_cell;
            next_cell_bit = 0;
        }
        input_chunk =
            (input_chunk << piece_len) |
            (code[next_cell] << next_cell_bit >> (INT_BIT_SIZE - piece_len));
        next_cell_bit += piece_len;
        piece_len = 0;
    }
}
