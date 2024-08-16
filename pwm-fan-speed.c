#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
  double airflow;

} fan;

/**
 * @brief Reads the next value from the input file.
 * @param value the read value
 * @param fd the file descriptor
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

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Usage: %s inputfile\n", argv[0]);
    exit(1);
  }
  char *filename = argv[1];
  FILE *input = fopen(filename, "r");
  if (input == NULL) {
    printf("Error: file %s not found", filename);
    exit(1);
  }
  double value;
  int status = get_next_input_value(&value, input);
  while (status == 0) {
    // TODO Perform main part of the bench here
    //  printf("%le\n", value);
    status = get_next_input_value(&value, input);
  }
  printf("End of the program.\n");
  fclose(input);
  return 0;
}
