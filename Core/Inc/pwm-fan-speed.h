#ifndef PWM_H
#define PWM_H



// Config 1
#define PWM_INPUT_SIZE 100
#define PWM_INPUT_SCALE 5
// Fan parameters
#define PWM_TEMP_TH 50    // [°C]
#define PWM_AIRFLOW 0.07  // [m^3/s]
// Fan constants
#define FAN_AREA 0.0113   // [m^2] circular area of a 12x12cm fan
#define FAN_DISTANCE 0.1  // [m] distance of the fan from the surface

// Config 2
//#define PWM_INPUT_SIZE 200
//#define PWM_INPUT_SCALE 5
//#define PWM_TEMP_TH 30    // [°C]
//#define PWM_AIRFLOW 0.1   // [m^3/s]
//#define FAN_AREA 0.0113   // [m^2] circular area of a 12x12cm fan
//#define FAN_DISTANCE 0.07 // [m] distance of the fan from the surface

// Config 3
//#define PWM_INPUT_SIZE 300
//#define PWM_INPUT_SCALE 5
//#define PWM_TEMP_TH 25      // [°C]
//#define PWM_AIRFLOW 0.15    // [m^3/s]
//#define FAN_AREA 0.0113     // [m^2] circular area of a 12x12cm fan
//#define FAN_DISTANCE 0.05   // [m] distance of the fan from the surface


// PID controller values
#define PWM_Kp 1
#define PWM_Ki 1
#define PWM_Kd 0
#define PWM_DT 1          // [s]
// Aluminum parameters
#define SURFACE_AREA 0.01  // [m^2]
#define CHARACT_LEN 0.1    // [m] (length of the surface)
#define ALUMINIUM_CP 0.897 // [J/(Kg*K)]

void pwm_fan_speed(double input[PWM_INPUT_SIZE]);

#endif
