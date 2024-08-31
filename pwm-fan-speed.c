/**
 * PWM fan speed controller
 *
 * This program is a simulation of the effect of a fan, if it was applied
 * on the system for which a heat draw is given.
 *
 * Some assumptions are made on the system to simplify the model:
 * - the system is approximated to a vertical aluminium square of area
 *   10x10 cm^2 and negligible volume;
 * - the ambient temperature is constant and equal to 25.0 °C;
 * - the fan is a classic 12x12 cm round fan.
 *
 * @author Arturo Caliandro <arturo.caliandro AT mail.polimi DOT it>
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// PID controller values
float Kp;
float Ki;
float Kd;

// an aluminium square
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
#define AIR_Pr 0.71

#define g 9.81    // [m/s^2]
#define K0 273.15 // [K] Kelvin absolute zero
#define DT 1      // [s]

#define FAN_AREA 0.0113 // [m^2] circular area of a 12x12cm fan

typedef enum {
    false,
    true,
} bool;

typedef struct {
    const double speed; // speed of moved air
    float DC;           // duty-cycle
} fan_t;

typedef struct {
    double current_temp;
    double expected_temp;
    double __integral;
    double __prev_err;
    double time;
} status_t;

int get_next_input_value(double *value, FILE *fp);
double evaluate_temperature_increment(double heat_diff);
double evaluate_natural_cooling(double temp);
double evaluate_fan_cooling(fan_t *fan, status_t *status, double th);

int main(int argc, char *argv[]) {
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
    Ki = strtof(argv[5], &endptr);
    Kd = strtof(argv[6], &endptr);
    status_t status = {AMBIENT_TEMP, AMBIENT_TEMP, 0, 0, 0};
    double heat_diff;
    int ret_code = get_next_input_value(&heat_diff, input);
    if (ret_code != 0) {
        puts("Error: invalid file format.");
        exit(EXIT_FAILURE);
    }
    int count = 1;
    double temp_delta, natural_cooling;
    FILE *output = fopen("output.csv", "w");
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
        fprintf(output, "%le, %le, %.3f\n", status.current_temp,
                status.expected_temp, fan.DC);
        ret_code = get_next_input_value(&heat_diff, input);
        count++;
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
    int status;
    if (getline(&line, &len, fp) == -1)
        status = EOF;
    else if (sscanf(line, "%lf", value) != 1)
        status = 1;
    else
        status = 0;
    free(line);
    return status;
}

/**
 * @brief compute the new duty-cycle value for the fan
 *
 * @return a value in the interval [0.0, 1.0]
 */
double get_new_dc(status_t *status, double th) {
    double err = status->expected_temp - th;
    double derivative = (err - status->__prev_err) / DT;
    status->__integral += err * DT;
    status->__prev_err = err;
    double dc =
        (Kp * err + Ki * (status->__integral) + Kd * derivative) / 100.0;
    if (dc > 1.0)
        dc = 1.0;
    else if (dc < 0.0)
        dc = 0.0;
    return dc;
}

double evaluate_temperature_increment(double heat_diff) {
    return (heat_diff / ALUMINIUM_CP) * DT;
}

double evaluate_natural_cooling(double temp) {
    double t_film = (temp + AMBIENT_TEMP) / 2 + K0;
    double Ra = (g * (1 / t_film)) /
                (AIR_VISCOSITY * pow(t_film, 0.7355) * AIR_THERMAL_DIFF_COEFF) *
                (temp - AMBIENT_TEMP) * pow(CHARACT_LEN, 3);
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

double evaluate_fan_cooling(fan_t *fan, status_t *status, double th) {
    double t_film = (status->expected_temp + AMBIENT_TEMP) / 2 + K0;
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
    fan->DC = get_new_dc(status, th);
    return (h * SURFACE_AREA * (status->expected_temp - AMBIENT_TEMP));
}
