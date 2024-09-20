/**
 * PWM fan speed controller
 *
 * This program is a simulation of the effect of a fan, if it was applied
 * on the system for which a energy production time series is given.
 *
 * Some assumptions have been made on the system to simplify the model:
 * - the system consists in a vertical aluminium square of area
 *   10x10 cm^2 and a small volume;
 * - the ambient temperature is constant and equal to 25.0 °C;
 * - the fan is a classic round fan with a diameter of 12cm, and it is
 *   set in front of the aluminium surface, at a distance of 10cm.
 *
 * @author Arturo Caliandro <arturo.caliandro AT mail.polimi DOT it>
 *
 */

#include <math.h>

#define INPUT_SIZE 100
double input[INPUT_SIZE] = {
    5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
    5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0,
    5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, .0,  .0,  .0,  .0,  .0,
    .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,
    .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,
    .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,
    .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0,  .0}; // [J]
#define TEMP_TH 50                                    // [°C]
#define AIRFLOW 0.07                                  // [m^3/s]
#define DT 1                                          // [s]

// PID controller values
float Kp = 1;
float Ki = 1;
float Kd = 0;

// An aluminium square
#define SURFACE_AREA 0.01  // [m^2]
#define CHARACT_LEN 0.1    // [m] (length of the surface)
#define ALUMINIUM_CP 0.897 // [J/(Kg*K)]

// Air constants
#define AIR_CP 1.012                  // [J/(Kg*K)]
#define AMBIENT_TEMP 25.0             // [°C]
#define AIR_THERM_COND 0.026          // [W/(m*K)]
#define AIR_VISCOSITY 2.791E-7        // x T^0.7355 [Pa*s]
#define AIR_THERMAL_DIFF_COEFF 1.9E-5 // [m^2/s]
#define AIR_DENSITY 1.1839            // [kg/m^3] at 25°C
#define AIR_Pr 0.71                   // Air Prandtl number

// Fan constants
#define FAN_AREA 0.0113  // [m^2] circular area of a 12x12cm fan
#define FAN_DISTANCE 0.1 // [m] distance of the fan from the surface

// General constants
#define g 9.81    // [m/s^2]
#define K0 273.15 // [K] Kelvin absolute zero

// Fan characteristics
typedef struct {
    const double speed; // speed of blown air
    float DC;           // duty-cycle
} fan_t;

// Status of the system, including the parameters for the PID controller
typedef struct {
    double current_temp;  // Temperature of the naturally cooled system
    double expected_temp; // Temperature of the system if cooled by the fan
    double __integral;    // Integral error accumulation
    double __prev_err;    // Previous error
} status_t;

// int get_next_input_value(double *value, FILE *fp);
double evaluate_temperature_increment(double heat_diff);
double evaluate_natural_cooling(double temp);
double evaluate_fan_cooling(fan_t fan, status_t status);
double evaluate_new_dc(status_t *status, double th);
double grashof(double temp);
double reynolds(fan_t fan, double temp);

int main(int argc, char *argv[]) {
    const double temp_th = TEMP_TH;
    double airflow = AIRFLOW;
    fan_t fan = {airflow / FAN_AREA, 0.0};
    status_t status = {AMBIENT_TEMP, AMBIENT_TEMP, 0, 0};
    double heat_diff, temp_delta, cooling, richardson;
    for (int i = 0; i < INPUT_SIZE; ++i) {
        // Temperature increment
        heat_diff = input[i];
        temp_delta = evaluate_temperature_increment(heat_diff);
        status.current_temp += temp_delta;
        status.expected_temp += temp_delta;
        // Natural convection
        cooling = evaluate_natural_cooling(status.current_temp);
        status.current_temp -= evaluate_temperature_increment(cooling);
        // Forced convection via fan
        richardson = grashof(status.expected_temp) /
                     pow(reynolds(fan, status.expected_temp), 2);
        if (richardson > 16) {
            // Forced convection is negligible
            cooling = evaluate_natural_cooling(status.expected_temp);
        } else {
            // Consider forced convection only
            cooling = evaluate_fan_cooling(fan, status);
        }
        status.expected_temp -= evaluate_temperature_increment(cooling);
        // Evaluate new duty cycle
        fan.DC = evaluate_new_dc(&status, temp_th);
    }
    return 0;
}

/**
 * @brief Compute the new duty-cycle value for the fan
 *
 * @param status the status of the system
 * @param th the threshold temperature
 * @return double: a value in the interval [0.0, 1.0]
 */
double evaluate_new_dc(status_t *status, double th) {
    double err = status->expected_temp - th;
    // Clipping to values above 0 since temperatures below the threshold
    // are ok
    double derivative = fmax((err - status->__prev_err) / DT, 0);
    status->__integral = fmax(status->__integral + err * DT, 0);
    status->__prev_err = err;
    double dc =
        (Kp * err + Ki * (status->__integral) + Kd * derivative) / 100.0;
    if (dc > 1.0)
        dc = 1.0;
    else if (dc < 0.0)
        dc = 0.0;
    return dc;
}

/**
 * @brief Compute the temperature increment
 *
 * @param heat_diff the variation of heat
 * @return double: the temperature delta
 */
double evaluate_temperature_increment(double heat_diff) {
    return (heat_diff / ALUMINIUM_CP) * DT;
}

/**
 * @brief Compute the variation of heat due to natural convection
 *
 * @param temp the current temperature
 * @return double: the heat variation
 */
double evaluate_natural_cooling(double temp) {
    double t_film = (temp + AMBIENT_TEMP) / 2 + K0;
    // Rayleigh number for natural convection
    double Ra = (g * (1 / t_film)) /
                (AIR_VISCOSITY * pow(t_film, 0.7355) * AIR_THERMAL_DIFF_COEFF) *
                fabs(temp - AMBIENT_TEMP) * pow(CHARACT_LEN, 3);
    double h;
    if (Ra > 1E9) {
        // Turbulent
        h = AIR_THERM_COND / CHARACT_LEN *
            pow(0.825 + (0.387 * pow(Ra, 1.0 / 6)) /
                            (1 + pow(0.492 / pow(AIR_Pr, 9.0 / 16), 8.0 / 27)),
                2);
    } else {
        // Laminar
        h = AIR_THERM_COND / CHARACT_LEN *
            (0.68 + (0.67 * pow(Ra, 1.0 / 4)) /
                        (1 + pow(0.492 / pow(AIR_Pr, 9.0 / 16), 4.0 / 9)));
    }
    return (h * SURFACE_AREA * (temp - AMBIENT_TEMP));
}

/**
 * @brief Compute the Grashof number
 *
 * @param temp the system's temperature
 * @return double: the Grashof number
 */
double grashof(double temp) {
    double t_film = (temp + AMBIENT_TEMP) / 2 + K0;
    return ((g * (1 / t_film) * (temp - AMBIENT_TEMP) * pow(CHARACT_LEN, 3)) /
            pow(AIR_VISCOSITY * pow(t_film, 0.7355), 2));
}

/**
 * @brief compute the variation of heat due to the cooling action of the fan
 *
 * @param fan the fan instance
 * @param status the status of the system
 * @return double: the heat variation
 */
double evaluate_fan_cooling(fan_t fan, status_t status) {
    // Reynolds number for forced convection
    double Re = reynolds(fan, status.expected_temp);
    double C, m, n;
    // Assuming that the laminar-turbulent threshold is 3000
    if (Re > 3000.0) {
        // Turbulent
        C = 0.037, m = 0.8, n = 1.0 / 3;
    } else {
        // Laminar
        C = 0.664, m = 0.5, n = 1.0 / 3;
    }
    double Nu = C * pow(Re, m) * pow(AIR_Pr, n);
    double h = Nu * AIR_THERM_COND / FAN_DISTANCE;
    return (h * SURFACE_AREA * (status.expected_temp - AMBIENT_TEMP));
}

/**
 * @brief Compute the Reynolds number
 *
 * @param fan the fan parameters
 * @param temp the surface temperature
 * @return double: the Reynolds number
 */
double reynolds(fan_t fan, double temp) {
    double t_film = (temp + AMBIENT_TEMP) / 2 + K0;
    return (AIR_DENSITY * (fan.speed * fan.DC) * FAN_DISTANCE) /
           (AIR_VISCOSITY * pow(t_film, 0.7355));
}