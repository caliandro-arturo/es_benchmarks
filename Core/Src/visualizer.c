/**
 * @file visualizer.c
 * @author Arturo Caliandro (arturo.caliandro AT mail.polimi.it)
 * @brief Time series visualizer.
 * The input is read and then drawn as a graph using the Bresenham algorithm.
 */

#include <math.h>

typedef struct {
    int x_factor;    // The scaling factor for the x axis
    double y_factor; // The scaling factor for the y axis
    double min;      // The minimum value found so far
} image_data;

image_data im_data;

#ifndef VIS_DEFINES
#define VIS_DEFINES

#define VIS_WIDTH 300
#define VIS_HEIGHT 200
#define VIS_INPUT_SIZE 100

#endif


void get_values(double input[VIS_INPUT_SIZE], int n, double *min, double *max);
void draw_line(char image[VIS_HEIGHT][VIS_WIDTH], int x_0, double y_0,
               double y_1);

void visualizer(double input[VIS_INPUT_SIZE]) {
    char image[VIS_HEIGHT][VIS_WIDTH];
    for (int i = 0; i < VIS_HEIGHT; ++i) {
        for (int j = 0; j < VIS_WIDTH; ++j) {
            image[i][j] = '0';
        }
    }
    double y_max;
    // Check if time series fits horizontally
    int x_max = VIS_INPUT_SIZE;
    if (VIS_INPUT_SIZE > VIS_WIDTH) {
        // Input too big for the picture: truncating to the values compatible
        // with the width of the image
        x_max = VIS_WIDTH;
        // Find the maximum and the minimum, again
        // Scale maximum and minimum according to size of the image
    }
    get_values(input, x_max, &y_max, &im_data.min);
    im_data.x_factor = VIS_WIDTH / x_max; // >= 1
    if (im_data.min == y_max) {
        // Constant value, to be plot in the half of the image as a line
        im_data.min -= (VIS_HEIGHT - 1) / 2.0;
        im_data.y_factor = 1;
    } else {
        // To scale a y value, the used formula is:
        // y_scaled = (height-1)*(y-min)/(max - min)
        im_data.y_factor = (VIS_HEIGHT - 1) / (y_max - im_data.min);
    }
    // For each couple of point of the input draw a line
    for (int i = 1; i < x_max; ++i) {
        draw_line(image, i, input[i - 1], input[i]);
    }
}

/**
 * @brief Finds the min-max values inside the first n values of the input.
 *
 * @param n the number of values to consider
 * @param min the minimum found input value
 * @param max the maximum found input value
 */
void get_values(double input[VIS_INPUT_SIZE], int n, double *min, double *max) {
    *min = INFINITY;
    *max = -INFINITY;
    for (int i = 0; i < n; ++i) {
        if (input[i] < *min) {
            *min = input[i];
        }
        if (input[i] > *max) {
            *max = input[i];
        }
    }
}

/**
 * @brief Draw a line, using the Bresenham's Line Drawing algorithm.
 *
 * @param image the image representation
 * @param x_0 the final x value (used to find the start)
 * @param y_0 the first y value
 * @param y_1 the second y value
 */
void draw_line(char image[VIS_HEIGHT][VIS_WIDTH], int x_1, double y_0,
               double y_1) {
    x_1 *= im_data.x_factor;
    int x = x_1 - im_data.x_factor;
    int dx = im_data.x_factor;
    int sign_x = 1;
    int y = VIS_HEIGHT - (im_data.y_factor * (y_0 - im_data.min)) - 1;
    int y1 = VIS_HEIGHT - (im_data.y_factor * (y_1 - im_data.min)) - 1;
    int dy = -(y1 - y);
    if (dy > 0) {
        dy *= -1;
    }
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
