#ifndef VISUALIZER_H
#define VISUALIZER_H

#ifndef VIS_DEFINES
#define VIS_DEFINES
// Used to expose defines to main: values should be changed in visualizer.c too
#define VIS_WIDTH 300
#define VIS_HEIGHT 200
#define VIS_INPUT_SIZE 100

#endif

void visualizer(double input[VIS_INPUT_SIZE]);

#endif
