#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080

void abort_program(FILE *input, FILE *output);
int count_lines(FILE *input);
double *get_values(FILE *input, int *n);

int main(int argc, char *argv[]) {
    if (argc - 1 != 3) {
        fprintf(stderr, "Usage: %s input_file img_width img_height\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Read image and input size
    FILE *input = fopen(argv[1], "r");
    if (input == NULL) {
        fprintf(stderr, "Error when opening %s: %s\n", argv[1],
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    char *endptr;
    int width = strtol(argv[2], &endptr, 10);
    if (endptr == argv[2]) {
        fputs("Width value in invalid format.\n", stderr);
        abort_program(input, NULL);
    } else if (width <= 0 || width > MAX_WIDTH) {
        fprintf(stderr, "Width value must be a number between 0 and %d.\n",
                MAX_WIDTH);
        abort_program(input, NULL);
    }
    int height = strtol(argv[3], &endptr, 10);
    if (endptr == argv[3]) {
        fputs("Height value in invalid format.\n", stderr);
        abort_program(input, NULL);
    } else if (width <= 0 || width > MAX_WIDTH) {
        fprintf(stderr, "Height value must be a number between 0 and %d.\n",
                MAX_HEIGHT);
        abort_program(input, NULL);
    }
    int x_len;
    double *x = get_values(input, &x_len);
    // Check if size meets conditions

    // Find maximum and minimum of the signal

    // For each couple of point of the input draw a line (using e.g. Bresenham)

    // Write image on a file

    // Close every file
    fclose(input);
    free(x);
    return EXIT_SUCCESS;
}

/**
 * @brief Close the open files, then exit with failure.
 *
 * @param input the input file
 * @param output the output file
 */
void abort_program(FILE *input, FILE *output) {
    if (input != NULL) {
        fclose(input);
    }
    if (output != NULL) {
        fclose(output);
    }
    exit(EXIT_FAILURE);
}

/**
 * @brief Count the lines of the input file.
 *
 * @param input the input file
 * @return int: the number of lines, or -1 if there was an error
 */
int count_lines(FILE *input) {
    int count = 0;
    char line[26];
    while (fgets(line, sizeof(line), input) != NULL) {
        count++;
    }
    if (ferror(input) != 0) {
        return -1;
    }
    // Then it is EOF
    rewind(input);
    return count;
}

/**
 * @brief Read the input file and put the values in a heap-allocated array.
 *        If there is an error in reading the file, the program is aborted.
 *
 * @param input the input file
 * @param n the address of a variable in which the size of the array will
 *          be stored
 * @return double *: the array with the input values, to be freed by the caller.
 */
double *get_values(FILE *input, int *n) {
    *n = count_lines(input);
    if (*n == -1) {
        perror("count_lines");
        abort_program(input, NULL);
    }
    double *values = malloc(*n * sizeof(double));
    char line[26];
    char *endptr;
    for (int i = 0; i < *n; ++i) {
        values[i] = strtod(fgets(line, sizeof(line), input), &endptr);
        if (values[i] == 0 && endptr == line) {
            fprintf(stderr, "Error at input line %d: invalid data.\n", i);
            free(values);
            abort_program(input, NULL);
        }
    }
    return values;
}
