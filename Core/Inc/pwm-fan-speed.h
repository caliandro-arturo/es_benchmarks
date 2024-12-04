#ifndef PWM_H
#define PWM_H

#ifndef PWM_DEFINES
#define PWM_DEFINES

#define PWM_INPUT_SIZE 100
// Fan parameters
#define PWM_TEMP_TH 50    // [°C]
#define PWM_AIRFLOW 0.07  // [m^3/s]
#define PWM_DT 1          // [s]
// Fan constants
#define FAN_AREA 0.0113  // [m^2] circular area of a 12x12cm fan
#define FAN_DISTANCE 0.1 // [m] distance of the fan from the surface
// PID controller values
#define PWM_Kp 1
#define PWM_Ki 1
#define PWM_Kd 0
// Aluminum parameters
#define SURFACE_AREA 0.01  // [m^2]
#define CHARACT_LEN 0.1    // [m] (length of the surface)
#define ALUMINIUM_CP 0.897 // [J/(Kg*K)]

#endif

void pwm_fan_speed(double input[PWM_INPUT_SIZE]);

#endif
