/**
 * PWM fan speed controller
 *
 * This program is a simulation of the effect of a fan, if it was applied
 * on the system for which a temperature time serie is applied.
 *
 * Some assumptions are made on the system to simplify the model.
 *
 * @author Arturo Caliandro <arturo.caliandro AT mail.polimi DOT it>
 *
 */

#include <stdio.h>
#include <stdlib.h>

float Kp;
float Ki;
float Kd;

#define AIR_HEAT_CAPACITY 1.012 // [J/(Kg*°C)] at room conditions (source:
// https://en.wikipedia.org/wiki/Table_of_specific_heat_capacities
// where it is expressed in [J*g^-1*K^-1])
#define DT 1 // in seconds
#define AMBIENT_TEMP 20.0

typedef enum {
    false,
    true,
} bool;

typedef struct {
    const double airflow; // expressed as m^3/min
    float DC;             // duty-cycle
} fan_t;

typedef struct {
    double current_temp;
    double expected_temp;
    double __integral;
    double __prev_err;
    double time;
} status_t;

int get_next_input_value(double *value, FILE *fp);
void update_temperature(FILE *fp, fan_t *fan, status_t *status, double new_temp,
                        double th);

int main(int argc, char *argv[]) {
    if (argc != 7) {
        printf("Usage: %s temps_file threshold_temp fan_airflow Kp Ki Kd\n",
               argv[0]);
        exit(1);
    }
    char *filename = argv[1];
    FILE *input = fopen(filename, "r");
    if (input == NULL) {
        printf("Error: file %s not found.\n", filename);
        exit(1);
    }
    char *endptr;
    const double temp_th = strtod(argv[2], &endptr);
    if (temp_th == 0 && endptr == argv[2]) {
        puts("Error: the threshold temperature must be a float number.");
        exit(1);
    }
    const double airflow = strtod(argv[3], &endptr);
    if (airflow == 0 && endptr == argv[3]) {
        puts("Error: the airflow must be a float number.");
        exit(1);
    }
    Kp = strtof(argv[4], &endptr);
    Ki = strtof(argv[5], &endptr);
    Kd = strtof(argv[6], &endptr);
    fan_t fan = {airflow, 0.0};

    status_t status = {0, 0, 0, 0, 0};
    double new_temperature;
    int ret_code = get_next_input_value(&new_temperature, input);
    if (ret_code != 0) {
        puts("Error: invalid file format.");
        exit(1);
    }
    FILE *output = fopen("output.csv", "w");
    status.expected_temp = new_temperature;
    while (ret_code == 0) {
        update_temperature(output, &fan, &status, new_temperature, temp_th);
        ret_code = get_next_input_value(&new_temperature, input);
    }
    puts("End of the program.");
    fclose(input);
    return 0;
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

double get_new_dc(double err, double *integral, double *prev_err) {
    double derivative = (err - *prev_err) / DT;
    *integral += err * DT;
    *prev_err = err;
    return Kp * err + Ki * (*integral) + Kd * derivative;
}

double eval_fan_effect(fan_t *fan, status_t *status, double new_temp) {
    double temp_diff = status->expected_temp - AMBIENT_TEMP;
    double airflow_effect =
        fan->airflow * AIR_HEAT_CAPACITY * temp_diff * fan->DC / 100.0;
    return new_temp - airflow_effect * DT;
}

void update_temperature(FILE *fp, fan_t *fan, status_t *status, double new_temp,
                        double th) {
    double error = status->expected_temp - th;
    fan->DC = get_new_dc(error, &status->__integral, &status->__prev_err);
    if (fan->DC > 100.0)
        fan->DC = 100.0;
    else if (fan->DC < 0.0)
        fan->DC = 0.0;
    status->expected_temp = eval_fan_effect(fan, status, new_temp);
    int len = fprintf(fp, "%le, %.3f\n", status->expected_temp, fan->DC);
}
