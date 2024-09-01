/**
 * PWM fan speed controller
 *
 * This program is a simulation of the effect of a fan, if it was applied
 * on the system for which a energy production time series is given.
 *
 * Input:
 * - path to a file containing the energy production time series, measured
 *   at intervals of time of length DT (defined in code), in Joule;
 * - a threshold temperature in degrees Celsius, used to compute the error
 *   for the fan PID controller;
 * - the maximum airflow of the fan in cubic meters per second;
 * - the three parameters of the PID controller: Kp, Ki and Kd, defining
 *   respectively the proportional, integral and derivative contribute.
 *
 * Output:
 * - a file, called "output.csv", containing the same number of rows as
 *   the input file, and three columns, respectively:
 *   - the temperature of the aluminium surface, subject to natural
 *     convection only;
 *   - the temperature of the aluminium surface, subject to both natural
 *     and forced convection (due to the fan action);
 *   - the duty-cycle used to control the fan.
 *
 * Some assumptions have been made on the system to simplify the model:
 * - the system consists in a vertical aluminium square of area
 *   10x10 cm^2 and a small volume;
 * - the ambient temperature is constant and equal to 25.0 °C;
 * - the fan is a classic round fan with a diameter of 12cm.
 *
 * @author Arturo Caliandro <arturo.caliandro AT mail.polimi DOT it>
 *
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// PID controller values
float Kp;
float Ki;
float Kd;

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
#define FAN_AREA 0.0113 // [m^2] circular area of a 12x12cm fan

// General constants
#define g 9.81    // [m/s^2]
#define K0 273.15 // [K] Kelvin absolute zero
#define DT 1      // [s]

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

int get_next_input_value(double *value, FILE *fp);
double evaluate_temperature_increment(double heat_diff);
double evaluate_natural_cooling(double temp);
double evaluate_fan_cooling(fan_t *fan, status_t *status, double th);
double evaluate_new_dc(status_t *status, double th);

int main(int argc, char *argv[]) {
    // Parsing the input
    if (argc != 7) {
        printf("Usage: %s temps_file threshold_temp fan_airflow_m^3/sec Kp Ki "
               "Kd\n",
               argv[0]);
        exit(1);
    }
    char *filename = argv[1];
    FILE *input = fopen(filename, "r");
    if (input == NULL) {
        perror(filename);
        exit(EXIT_FAILURE);
    }
    char *endptr;
    const double temp_th = strtod(argv[2], &endptr);
    if (temp_th == 0 && endptr == argv[2]) {
        puts("Error: the threshold temperature must be a float number.");
        exit(EXIT_FAILURE);
    }
    double airflow = strtod(argv[3], &endptr);
    if (airflow == 0 && endptr == argv[3]) {
        puts("Error: the airflow must be a float number.");
        exit(EXIT_FAILURE);
    }
    fan_t fan = {airflow / FAN_AREA, 0.0};
    Kp = strtof(argv[4], &endptr);
    if (Kp == 0 && endptr == argv[4]) {
        puts("Error: the Kp parameter must be a float number.");
        exit(EXIT_FAILURE);
    }
    Ki = strtof(argv[5], &endptr);
    if (Ki == 0 && endptr == argv[5]) {
        puts("Error: the Ki parameter must be a float number.");
        exit(EXIT_FAILURE);
    }
    Kd = strtof(argv[6], &endptr);
    if (Kp == 0 && endptr == argv[6]) {
        puts("Error: the Kd parameter must be a float number.");
        exit(EXIT_FAILURE);
    }
    // Assuming that the system starts at ambient temperature
    status_t status = {AMBIENT_TEMP, AMBIENT_TEMP, 0, 0};
    double heat_diff;
    int ret_code = get_next_input_value(&heat_diff, input);
    if (ret_code != 0) {
        puts("Error at input line 1: invalid data.");
        exit(EXIT_FAILURE);
    }
    double temp_delta, natural_cooling;
    FILE *output = fopen("output.csv", "w");
    int count = 0;
    while (ret_code == 0) {
        // Temperature increment
        temp_delta = evaluate_temperature_increment(heat_diff);
        status.current_temp += temp_delta;
        status.expected_temp += temp_delta;
        // Natural convection
        natural_cooling = evaluate_natural_cooling(status.current_temp);
        status.current_temp -= evaluate_temperature_increment(natural_cooling);
        // Forced convection via fan
        natural_cooling = evaluate_natural_cooling(status.expected_temp);
        status.expected_temp -= evaluate_temperature_increment(
            evaluate_fan_cooling(&fan, &status, temp_th) + natural_cooling);
        fan.DC = evaluate_new_dc(&status, temp_th);
        fprintf(output, "%le, %le, %.3f\n", status.current_temp,
                status.expected_temp, fan.DC);
        ret_code = get_next_input_value(&heat_diff, input);
        count++;
    }
    if (ret_code != EOF) {
        printf("Error at input line %d: invalid data.\n", count);
        exit(EXIT_FAILURE);
    }
    fclose(input);
    fclose(output);
    return EXIT_SUCCESS;
}

/**
 * @brief Reads the next value from the input file.
 *
 * @param value the read value, casted to double
 * @param fd the file descriptor
 *
 * @return 0 if the value has been read correctly, EOF if the EOF has been
 *         reached, 1 if there was an error during the conversion.
 */
int get_next_input_value(double *value, FILE *fp) {
    char *line = NULL;
    size_t len;
    int status = 0;
    if (getline(&line, &len, fp) == EOF) {
        status = EOF;
    } else if (sscanf(line, "%lf", value) != 1) {
        status = 1;
    }
    free(line);
    return status;
}

/**
 * @brief compute the new duty-cycle value for the fan
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
 * @brief compute the temperature increment
 *
 * @param heat_diff the variation of heat
 * @return double: the temperature delta
 */
double evaluate_temperature_increment(double heat_diff) {
    return (heat_diff / ALUMINIUM_CP) * DT;
}

/**
 * @brief compute the variation of heat due to natural convection
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
 * @brief compute the variation of heat due to the cooling action of the fan
 *
 * @param fan the fan instance
 * @param status the status of the system
 * @param th the threshold temperature
 * @return double: the heat variation
 */
double evaluate_fan_cooling(fan_t *fan, status_t *status, double th) {
    double t_film = (status->expected_temp + AMBIENT_TEMP) / 2 + K0;
    // Reynolds number for forced convection
    double Re = (AIR_DENSITY * (fan->speed * fan->DC) * CHARACT_LEN) /
                (AIR_VISCOSITY * pow(t_film, 0.7355));
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
    double h = Nu * AIR_THERM_COND / CHARACT_LEN;
    return (h * SURFACE_AREA * (status->expected_temp - AMBIENT_TEMP));
}
