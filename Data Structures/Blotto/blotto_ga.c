#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strategy.h"
#include "population.h"
#include "ga_helpers.h"
#include "arrays.h"

int count_lines(FILE *file);
void free_dist(int **dist, int num_locations, int line_count);

int main(int argc, char **argv)
{
  if (argc < 2)
    {
      fprintf(stderr, "USAGE: %s values... files... operations...\n", argv[0]);
      return 1;
    }
  
  // 0 if true, 1 if false
  int in_positive_int_list = 0;
  // for later reference of battleground numbers
  int battleground_array_end = 0;
  // 0 if false, 1 if true
  int in_file_list = 0;
  // start of file list
  int start_of_file_list = -1;
  // end of file list, 
  int end_of_file_list = -1;
  // part 3, 0 if false, 1 if true
  int part_three = 0;
  // this for loop is error checking the input
  for (int i = 1; i < argc; i++) {
    if (in_positive_int_list == 0) {
      if (atoi(argv[i]) > 0) {
        battleground_array_end++;
        continue;
      }
      if (argv[i][0] != '-' && atoi(argv[i]) == 0 && battleground_array_end > 0) {
        start_of_file_list = i;
        in_positive_int_list++;
        in_file_list++;
        continue;
      }
      fprintf(stderr, "USAGE: %s values... files... operations...\n", argv[0]);
      return 1; 
    }
    else if (in_file_list == 1) {
      if (argv[i][0] != '-') {
        end_of_file_list = i;
        continue;
      }
      end_of_file_list = i - 1;
      in_file_list--;
      part_three++;
      i--;
    }
    else if (part_three == 1) {
      if (strcmp(argv[i], "-x") == 0) {
        if (i == argc - 1 || atoi(argv[i + 1]) < 2) {
          fprintf(stderr, "-x error\n");
          return 1;
        }
        i++;
      }
      else if (strcmp(argv[i], "-d") == 0) {
        if (i == argc - 1 || atoi(argv[i + 1]) < 1) {
          fprintf(stderr, "-d error\n");
          return 1;
        }
        i++;
      }
      else if (strcmp(argv[i], "-o") == 0) {
        continue;
      }
    }
    else {
      fprintf(stderr, "USAGE: %s values... files... operations...\n", argv[0]);
      return 1;
    }
  }
  // number of units for the set. Based off of first line of first file
  int num_units = 0;
  // number of locations, based off of input
  int num_locations = battleground_array_end;
  // the weight of each one of the locations
  double values[num_locations];
  for (int i = 1; i <= battleground_array_end; i++) {
    values[i - 1] = atoi(argv[i]);
  }

  if (start_of_file_list == -1) {
    fprintf(stderr, "USAGE: %s values... files... operations...\n", argv[0]);
    return 1; 
  }
  else if (end_of_file_list == -1) {
    end_of_file_list = start_of_file_list;
  }
  // creates a population
  population *pop = population_create();
  for (int i = start_of_file_list; i <= end_of_file_list; i++) {
    FILE *file = NULL;
    file = fopen(argv[i], "r");
    if (file == NULL) {
      fprintf(stderr, "Invalid file: %s\n", argv[i]);
      population_destroy(pop);
      return 1;
    }
    int line_count = count_lines(file);
    if (line_count == 0) {
      fprintf(stderr, "Empty file: %s\n", argv[i]);
      population_destroy(pop);
      return 1;
    }
    file = fopen(argv[i], "r");
    // holds all the weights from the file
    double *weights = malloc(sizeof(double) * line_count);
    if (weights == NULL) {
      fprintf(stderr, "malloc error\n");
      population_destroy(pop);
      free(weights);
      fclose(file);
      return 1;
    }
    for (int u = 0; u < line_count; u++) {
      weights[u] = 0.0;
    }
    // holds all the distributions from a file
    int **distribution = malloc(sizeof(int *) * line_count);
    if (distribution == NULL) {
      fprintf(stderr, "malloc error\n");
      population_destroy(pop);
      free(weights);
      free(distribution);
      fclose(file);
      return 1;
    }
    for (int j = 0; j < line_count; j++) {
      fscanf(file, "%lf", &weights[j]);
      // fprintf(stderr, "%lf ", weights[j]);
      distribution[j] = malloc(sizeof(int) * num_locations);
      if (distribution[j] == NULL) {
        fprintf(stderr, "malloc error\n");
        population_destroy(pop);
        free(weights);
        free_dist(distribution, num_locations, line_count);
        free(distribution);
        fclose(file);
        return 1;
      }
      if (num_units == 0) {
        for (int k = 0; k < num_locations; k++) {
          fscanf(file, "%d", &distribution[j][k]);
          num_units += distribution[j][k];
          // fprintf(stderr, "%d\n", distribution[j][k]);
        }        
      }
      else {
        for (int k = 0; k < num_locations; k++) {
          fscanf(file, "%d", &distribution[j][k]);
        }
      }
    }
    strategy *s = strategy_create(num_locations, num_units);
    for (int w = 0; w < line_count; w++) {
      //print_dist(distribution[w], num_locations);
      if (!strategy_add_distribution(s, distribution[w], weights[w])) {
        fprintf(stderr, "strat_add error\n");
        strategy_destroy(s);
        population_destroy(pop);
        free(weights);
        free_dist(distribution, num_locations, line_count);
        free(distribution);
        fclose(file);
        return 1;
      }
      // strategy_print(stderr, s);
    }
    if (!population_add_strategy(pop, s)) {
      fprintf(stderr, "pop_add error\n");
        strategy_destroy(s);
        population_destroy(pop);
        free(weights);
        free_dist(distribution, num_locations, line_count);
        free(distribution);
        fclose(file);
      return 1;
    }
    // print_population(pop);
    // fprintf(stderr,"\n");
    fclose(file);
    free(weights);
    free_dist(distribution, num_locations, line_count);
    free(distribution);
  }
  // executes the command line instructions
  if (part_three == 1) {
    for (int i = end_of_file_list + 1; i < argc; i++) {
      if (strcmp(argv[i], "-o") == 0) {
        population_order(pop, values);
        continue;
      }
      else if (strcmp(argv[i], "-d") == 0) {
        if (atoi(argv[i + 1]) < population_size(pop) && population_size(pop) > 0) {
          strategy **holder = population_remove_last(pop, atoi(argv[i + 1]));
          for (int j = 0; j < atoi(argv[i + 1]); j++) {
            strategy_destroy(holder[j]);
          }
          free(holder);
          i++;  
          continue;        
        }
          fprintf(stderr, "-d size is too big\n");
          population_destroy(pop);
          return 1;
      }
      else if (strcmp(argv[i], "-x") == 0) {
        if (atoi(argv[i + 1]) < population_size(pop) && population_size(pop) > 1) {
          for (int j = 0; j < atoi(argv[i + 1]) - 1; j++) {
            for (int k = j + 1; k < atoi(argv[i + 1]); k++) {
              strategy **holder = strategy_crossover(population_get(pop, j), population_get(pop, k));
              population_add_strategy(pop, holder[0]);
              population_add_strategy(pop, holder[1]);
              free(holder);
            }
          }
          i++;
          continue;
        }
        fprintf(stderr, "-x size is wrong");
        population_destroy(pop);
        return 1;
      }
    }
  }
  print_population(pop);
  population_destroy(pop);
  return 0;
}
// counts the number of lines in a file to use inside the main function
int count_lines(FILE *file) {
  int line_count = 0;
  int c = 0;
  while ((c = getc(file)) != EOF) {
    if (c == '\n') {
      line_count++;
    }
  }
  fclose(file);
  // fprintf(stderr, "%d", line_count);
  return line_count;
}
// frees my array of distributions that I read in from the files
void free_dist(int **dist, int num_locations, int line_count) {
    for (int i = 0; i < line_count; i++) {
        free(dist[i]);
    }
}