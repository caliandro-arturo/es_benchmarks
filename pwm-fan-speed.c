#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 *
 */
typedef struct {
    const double airflow; // expressed as m^3/min
    float DC;             // duty-cycle
} fan_t;

typedef struct {
    double current_temp;
    double expected_temp;
} status_t;

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

void update_temperature(char *value, fan_t *fan, status_t *status, double th) {
    // TODO
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s temps_file threshold_temp fan_airflow\n", argv[0]);
        exit(1);
    }
    char *filename = argv[1];
    FILE *input = fopen(filename, "r");
    if (input == NULL) {
        printf("Error: file %s not found.\n", filename);
        exit(1);
    }
    char *endptr;
    double temp_th = strtod(argv[2], &endptr);
    if (temp_th == 0 && endptr == argv[2]) {
        puts("Error: the threshold temperature must be a float number.");
        exit(1);
    }
    double airflow = strtod(argv[3], &endptr);
    if (airflow == 0 && endptr == argv[3]) {
        puts("Error: the airflow must be a float number.");
        exit(1);
    }
    fan_t fan = {airflow, 0.0};

    status_t status;
    double value;
    int ret_code = get_next_input_value(&value, input);
    while (ret_code == 0) {
        // TODO Perform main part of the bench here
        // update_temperature(char *value, fan_t *fan, status_t *status);
        ret_code = get_next_input_value(&value, input);
    }
    puts("End of the program.");
    fclose(input);
    return 0;
}
