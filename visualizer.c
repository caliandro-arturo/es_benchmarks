/**
 * @file visualizer.c
 * @author Arturo Caliandro (arturo.caliandro AT mail.polimi.it)
 * @brief Time series visualizer.
 * Input:
 * - path to a file containing a time series;
 * - width of the output image;
 * - height of the output image.
 * Output:
 * - the image representing the signal, in .pbm (Portable BitMap) format.
 */

#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int width;       // The width of the image
    int height;      // The height of the image
    int x_factor;    // The scaling factor for the x axis
    double y_factor; // The scaling factor for the y axis
    double min;      // The minimum value found so far
} image_data;

#define MAX_WIDTH 1920
#define MAX_HEIGHT 1080

void abort_program(FILE *input, FILE *output);
int count_lines(FILE *input);
double *get_values(FILE *input, int *n, double *min, double *max);
void draw_line(image_data im_data, char image[im_data.height][im_data.width],
               int x_0, double y_0, double y_1);

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
    int x_max;
    double min, max;
    double *y = get_values(input, &x_max, &min, &max);
    // From now on, errors are not due to input
    image_data im_data = {.width = width, .height = height, .min = min};
    char image[im_data.height][im_data.width];
    for (int i = 0; i < im_data.height; ++i) {
        for (int j = 0; j < im_data.width; ++j) {
            image[i][j] = '0';
        }
    }
    // Check if time series fits horizontally
    if (x_max > im_data.width) {
        printf("Input too big for the picture: truncating to the first %d "
               "values\n",
               im_data.width);
        x_max = im_data.width;
        // Find the maximum and the minimum, again
        min = INFINITY;
        max = -INFINITY;
        for (int i = 0; i < x_max; ++i) {
            if (y[i] > max) {
                max = y[i];
            }
            if (y[i] < min) {
                min = y[i];
            }
        }
        im_data.min = min;
    }
    // Scale maximum and minimum according to size of the image
    im_data.x_factor = width / x_max; // >= 1
    if (im_data.min == max) {
        // Constant value, to be plot in the half of the image as a line
        im_data.min -= (height - 1) / 2.0;
        im_data.y_factor = 1;
    } else {
        // To scale a y value, the used formula is:
        // y_scaled = (height-1)*(y-min)/(max - min)
        im_data.y_factor = (im_data.height - 1) / (max - im_data.min);
    }
    // For each couple of point of the input draw a line
    for (int i = 1; i < x_max; ++i) {
        draw_line(im_data, image, i, y[i - 1], y[i]);
    }
    // Write image on a file
    FILE *output = fopen("output.pbm", "w");
    fputs("P1\n", output);
    fprintf(output, "%d %d\n", im_data.width, im_data.height);
    for (int i = 0; i < im_data.height; ++i) {
        for (int j = 0; j < im_data.width; ++j) {
            fputc(image[i][j], output);
        }
        fputc('\n', output);
    }
    // Close every file
    fclose(input);
    fclose(output);
    free(y);
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
 *        If there is an error while reading the file, the program is aborted.
 *
 * @param input the input file
 * @param n the address of a variable in which the size of the array will
 *          be stored
 * @param min the minimum found input value
 * @param max the maximum found input value
 * @return double *: the array with the input values, to be freed by the caller.
 */
double *get_values(FILE *input, int *n, double *min, double *max) {
    *n = count_lines(input);
    if (*n == -1) {
        perror("count_lines");
        abort_program(input, NULL);
    }
    double *values = malloc(*n * sizeof(double));
    char line[26];
    char *endptr;
    *min = INFINITY;
    *max = -INFINITY;
    for (int i = 0; i < *n; ++i) {
        values[i] = strtod(fgets(line, sizeof(line), input), &endptr);
        if (values[i] == 0 && endptr == line) {
            fprintf(stderr, "Error at input line %d: invalid data.\n", i);
            free(values);
            abort_program(input, NULL);
        }
        if (values[i] < *min) {
            *min = values[i];
        }
        if (values[i] > *max) {
            *max = values[i];
        }
    }
    return values;
}

/**
 * @brief Draw a line, using the Bresenham's Line Drawing algorithm.
 *
 * @param im_data the image metadata
 * @param image the image representation
 * @param x_0 the final x value (used to find the start)
 * @param y_0 the first y value
 * @param y_1 the second y value
 */
void draw_line(image_data im_data, char image[im_data.height][im_data.width],
               int x_1, double y_0, double y_1) {
    x_1 *= im_data.x_factor;
    int x = x_1 - im_data.x_factor;
    int dx = im_data.x_factor;
    int sign_x = 1;
    int y = im_data.height - (im_data.y_factor * (y_0 - im_data.min)) - 1;
    int y1 = im_data.height - (im_data.y_factor * (y_1 - im_data.min)) - 1;
    int dy = -abs(y1 - y);
    int sign_y = y < y1 ? 1 : -1;
    int err = dx + dy;
    int e2;
    while (1) {
        image[y][x] = '1';
        if (x == x_1 && y == y1) {
            break;
        }
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x += sign_x;
        }
        if (e2 <= dx) {
            err += dx;
            y += sign_y;
        }
    }
}
