#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080

void abort_program(FILE *input, FILE *output);

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
    // Check if size meets conditions

    // Find maximum and minimum of the signal

    // For each couple of point of the input draw a line (using e.g. Bresenham)

    // Write image on a file

    // Close every file
    fclose(input);
    return EXIT_SUCCESS;
}

void abort_program(FILE *input, FILE *output) {
    if (input != NULL) {
        fclose(input);
    }
    if (output != NULL) {
        fclose(output);
    }
    exit(EXIT_FAILURE);
}
