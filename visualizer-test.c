/**
 * @file visualizer.c
 * @author Arturo Caliandro (arturo.caliandro AT mail.polimi.it)
 * @brief Time series visualizer.
 * The input is read and then drawn as a graph using the Bresenham algorithm.
 */

#include <math.h>
#include <stdio.h>

typedef struct {
    int width;       // The width of the image
    int height;      // The height of the image
    int x_factor;    // The scaling factor for the x axis
    double y_factor; // The scaling factor for the y axis
    double min;      // The minimum value found so far
} image_data;

#define INPUT_SIZE 100
double input[INPUT_SIZE] = {3.869142818076724666e+00, 2.841240192715680735e+00,
                            3.483596684378727382e+00, 3.374880936613904758e+00,
                            3.462841926188640063e+00, 3.971900700455767819e+00,
                            4.487176776887483065e+00, 7.445005609974871819e-01,
                            2.571702504147584722e+00, 1.997184222132831088e+00,
                            4.479472507723974317e+00, 7.138910016166825301e+00,
                            9.205652655329789269e+00, 7.899842457474038682e+00,
                            5.442229212227376323e+00, 1.320651671334561961e+00,
                            3.640403815985101765e+00, 6.125096255084139241e-01,
                            4.674671713856629829e+00, 3.840888872016131828e+00,
                            3.032146072140213366e+00, 3.594445619712988815e+00,
                            1.782528105378214267e+00, 4.897143460907924073e+00,
                            4.595499906613166985e+00, 3.490005042076675057e+00,
                            8.242819383957048274e+00, 7.912690478227514923e+00,
                            2.444308312885862478e+00, 2.187659286528051528e+00,
                            4.265252526669438105e+00, 3.487268875573542370e+00,
                            6.884198011780201520e-01, 3.434412733401206008e+00,
                            6.587461279108257628e+00, 1.030327560142615795e+01,
                            1.093801712010700022e+01, 7.386354743033987980e+00,
                            5.080016556519407089e+00, 5.030656895578113463e+00,
                            3.858457358123344783e+00, 5.198720623951137654e+00,
                            1.847616729676342384e+00, 0.000000000000000000e+00,
                            1.978731192012662454e+00, 6.666743281746511762e+00,
                            6.067095076203656845e+00, 6.198526313926985942e+00,
                            9.503096283952434220e+00, 8.376361749030662551e+00,
                            9.112312916889987235e+00, 8.903113771732359183e+00,
                            1.489866333744331861e+00, 1.280529919893908186e+00,
                            4.981086228820915451e+00, 4.332318380409435399e+00,
                            9.529053158885730568e+00, 1.092658528111184424e+01,
                            1.225580568779378865e+01, 1.022649575319498716e+01,
                            1.252170854829885194e+01, 1.147732400875366565e+01,
                            1.478805964475810342e+01, 1.258717729459071855e+01,
                            7.083368398932622156e+00, 5.368315747705430852e+00,
                            3.915277460612154758e+00, 2.603466522568715469e+00,
                            1.641425581905664455e-01, 6.760693091495694418e+00,
                            3.441490874976572911e+00, 5.818781311869681616e+00,
                            5.851490432670162889e+00, 4.098702857116828469e+00,
                            4.905286615857199273e+00, 7.959179571644271256e+00,
                            4.642632672039132657e+00, 5.356402160098403087e+00,
                            3.078411785360566366e+00, 2.307598154684447955e+00,
                            0.000000000000000000e+00, 1.923860793987666629e+00,
                            3.841956591007800625e+00, 9.841549654354654342e-01,
                            0.000000000000000000e+00, 0.000000000000000000e+00,
                            0.000000000000000000e+00, 2.513016913506181393e-01,
                            0.000000000000000000e+00, 1.579965723494272112e+00,
                            2.543264628410263128e+00, 8.191522405106015370e+00,
                            7.881856824628884262e+00, 7.178803122663416580e+00,
                            9.011493658470167034e+00, 8.833482709777433328e+00,
                            9.938109436085706960e+00, 6.990229225213370867e+00,
                            2.243190590393101758e+00, 2.107190745065088855e+00};
image_data im_data = {.width = 1920, .height = 1080};

void get_values(int n, double *min, double *max);
void draw_line(char image[im_data.height][im_data.width], int x_0, double y_0,
               double y_1);

int main(int argc, char *argv[]) {
    char image[im_data.height][im_data.width];
    for (int i = 0; i < im_data.height; ++i) {
        for (int j = 0; j < im_data.width; ++j) {
            image[i][j] = '0';
        }
    }
    double y_max;
    // Check if time series fits horizontally
    int x_max = INPUT_SIZE;
    if (INPUT_SIZE > im_data.width) {
        // Input too big for the picture: truncating to the values compatible
        // with the width of the image
        x_max = im_data.width;
        // Find the maximum and the minimum, again
        // Scale maximum and minimum according to size of the image
    }
    get_values(x_max, &y_max, &im_data.min);
    im_data.x_factor = im_data.width / x_max; // >= 1
    if (im_data.min == y_max) {
        // Constant value, to be plot in the half of the image as a line
        im_data.min -= (im_data.height - 1) / 2.0;
        im_data.y_factor = 1;
    } else {
        // To scale a y value, the used formula is:
        // y_scaled = (height-1)*(y-min)/(max - min)
        im_data.y_factor = (im_data.height - 1) / (y_max - im_data.min);
    }
    // For each couple of point of the input draw a line
    for (int i = 1; i < x_max; ++i) {
        draw_line(image, i, input[i - 1], input[i]);
    }
    // Write image on a file in PBM format
    FILE *output = fopen("output.pbm", "w");
    fputs("P1\n", output);
    fprintf(output, "%d %d\n", im_data.width, im_data.height);
    for (int i = 0; i < im_data.height; ++i) {
        for (int j = 0; j < im_data.width; ++j) {
            fputc(image[i][j], output);
        }
        fputc('\n', output);
    }
    fclose(output);
    return 0;
}

/**
 * @brief Finds the min-max values inside the first n values of the input.
 *
 * @param n the number of values to consider
 * @param min the minimum found input value
 * @param max the maximum found input value
 */
void get_values(int n, double *min, double *max) {
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
void draw_line(char image[im_data.height][im_data.width], int x_1, double y_0,
               double y_1) {
    x_1 *= im_data.x_factor;
    int x = x_1 - im_data.x_factor;
    int dx = im_data.x_factor;
    int sign_x = 1;
    int y = im_data.height - (im_data.y_factor * (y_0 - im_data.min)) - 1;
    int y1 = im_data.height - (im_data.y_factor * (y_1 - im_data.min)) - 1;
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
