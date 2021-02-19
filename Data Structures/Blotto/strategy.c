#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "strategy.h"
#include "blotto.h"
#include "arrays.h"

struct _strategy
{
  size_t num_locations;
  int num_units;
  int **distro;
  double *weight;
  size_t size;
  size_t capacity;
};

#define STRATEGY_INITIAL_CAPACITY 1

strategy *strategy_create(size_t num_locations, size_t num_units)
{
  strategy *result = malloc(sizeof(strategy));

  if (result != NULL)
    {
      result->distro = malloc(sizeof(int *) * num_locations * STRATEGY_INITIAL_CAPACITY);
      result->weight = malloc(sizeof(double) * STRATEGY_INITIAL_CAPACITY);
      result->num_locations = num_locations;
      result->num_units = num_units;
      result->size = 0;
      result->capacity = STRATEGY_INITIAL_CAPACITY;
    }

  return result;
}

size_t strategy_count_locations(const strategy *s)
{
  if (s != NULL)
    {
      return s->num_locations;
    }
  else
    {
      return 0;
    }
}

int strategy_count_units(const strategy *s)
{
  if (s != NULL)
    {
      return s->num_units;
    }
  else
    {
      return 0;
    }
}

bool strategy_add_distribution(strategy *s, const int *dist, double w)
{
// MIGHT NEED TO ERROR CHECK IF DIST IS SAME LENGTH AS S->NUM_LOCATIONS

  if (s == NULL || w <= 0.0)
    {
      return false;
    }

  // make a copy of the given distribution
  int *copy = malloc(sizeof(int) * s->num_locations);
  if (copy == NULL)
    {
      return false;
    }

  // compute the total number of units in the distribution
  int sum = 0;
  for (size_t i = 0; i < s->num_locations; i++)
    {
      sum += dist[i];
      copy[i] = dist[i];
    }

  // make sure the total is crrect
  if (sum != s->num_units)
    {
      free(copy);
      return false;
    }

  if (s->size == s->capacity)
  {
    // TO DO -- mixed strategy should never be full
    int **expand = realloc(s->distro, sizeof(int *) * s->capacity * 2);
    if (expand == NULL) {
      return false;
    }
    double *x_weight = realloc(s->weight, sizeof(double) * s->capacity * 2);
    if (x_weight == NULL) {
      return false;
    }
    s->distro = expand;
    s->weight = x_weight;
    s->capacity *= 2;
  }

  for (int i = 0; i < s->size; i++) {
    if (arrays_compare(s->num_locations, s->distro[i], copy) == 0) {
      s->weight[i] += w;
      free(copy);
      return true;
    }
    else if (arrays_compare(s->num_locations, s->distro[i], copy) > 0) {
      for (int j = s->size; j > i; j--) {
        s->distro[j] = s->distro[j-1];
        s->weight[j] = s->weight[j-1];
      }
      s->distro[i] = copy;
      s->weight[i] = w;
      s->size += 1;
      return true;
    }
  }
    // save the copy of the distribution
    s->distro[s->size] = copy;
    s->weight[s->size] = w;
    s->size += 1;
    return true;
}
      
double strategy_expected_wins(const strategy *s1, const strategy *s2, const double *values)
{
  if (s1 == NULL || s2 == NULL || values == NULL
      || s1->num_locations != s2->num_locations
      || s1->num_units != s2->num_units)
    {
      return 0.0;
    }

  if (s1->size == 0 && s2->size == 0)
    {
      return 0.5;
    }
  else if (s1->size == 0)
    {
      return 0.0;
    }
  else if (s2->size == 0)
    {
      return 1.0;
    }

  double s1_weight = 0;
  double s2_weight = 0;
  for (int i = 0; i < s1->size; i++) {
    s1_weight += s1->weight[i];
  }
  for (int i = 0; i < s2->size; i++) {
    s2_weight += s2->weight[i];
  }

  double expected_wins = 0;
  for (int i = 0; i < s1->size; i++) {
    for (int j = 0; j < s2->size; j++) {
      expected_wins += blotto_play_game(s1->num_locations, s1->distro[i], s2->distro[j], values) * ((s1->weight[i] * s2->weight[j])/(s1_weight * s2_weight));
    }
  }
  
  // return blotto_play_game(s1->num_locations, s1->distro, s2->distro, values);
  return expected_wins;
}

strategy *strategy_copy(const strategy *s)
{
  if (s == NULL)
    {
      return NULL;
    }

  strategy *result = strategy_create(s->num_locations, s->num_units);
  if (result == NULL)
    {
      return NULL;
    }
  if (result->capacity < s->size)
  {
    int **make_bigger = realloc(result->distro, sizeof(int *) * s->size);
    if (make_bigger == NULL) {
      return NULL;
    }
    double *weight_bigger = realloc(result->weight, sizeof(double) * s->size);
    if (weight_bigger == NULL) {
      return NULL;
    }
    result->distro = make_bigger;
    result->weight = weight_bigger;
    result->capacity = s->size;
  }


  // TO DO -- this should perform a deep copy
  result->num_locations = s->num_locations;
  result->num_units = s-> num_units;
  for (size_t i = 0; i < s->size; i++) {
    result->weight[i] = s->weight[i];
    int *copy = malloc(sizeof(int) * s->num_locations);
    if (copy == NULL)
    {
      return false;
    }
    for (size_t j = 0; j < s->num_locations; j++)
    {
      copy[j] = s->distro[i][j];
    }
    result->distro[i] = copy;
    result->size++;
  }

  if (result->size != s->size) {
    return NULL;
  }

  return result;
}

strategy **strategy_crossover(const strategy *s1, const strategy *s2)
{
  if (s1 == NULL || s2 == NULL
      || s1->num_locations != s2->num_locations
      || s1->num_units != s2->num_units)
    {
      return NULL;
    }

  // make array to hold two offspring
  strategy **offspring = malloc(sizeof(strategy *) * 2);
  if (offspring == NULL)
    {
      return NULL;
    }
  
  // 1st offspring is a copy of 1st parent
  offspring[0] = strategy_create(s1->num_locations, s1->num_units);
  if (offspring[0] == NULL)
    {
      free(offspring);
      return NULL;
    }

  // 2nd offspring is a copy of 2nd parent
  offspring[1] = strategy_create(s2->num_locations, s2->num_units);
  if (offspring[1] == NULL)
    {
      strategy_destroy(offspring[0]);
      free(offspring);
      return NULL;
    }

  for (int i = 0; i < s1->size; i++) {
    if (i % 2 == 1 && i < s2->size) {
      if (!strategy_add_distribution(offspring[0], s2->distro[i], s2->weight[i])) {
        strategy_destroy(offspring[0]);
        strategy_destroy(offspring[1]);
        free(offspring);
        return NULL;
      }
      
    }
    else {
      if (!strategy_add_distribution(offspring[0], s1->distro[i], s1->weight[i])) {
        strategy_destroy(offspring[0]);
        strategy_destroy(offspring[1]);
        free(offspring);
        return NULL;
      }
      
    }
  }

  for (int i = 0; i < s2->size; i++) {
    if (i % 2 == 1 && i < s1->size) {
      if (!strategy_add_distribution(offspring[1], s1->distro[i], s1->weight[i])) {
        strategy_destroy(offspring[0]);
        strategy_destroy(offspring[1]);
        free(offspring);
        return NULL;
      }
    }
    else {
      if (!strategy_add_distribution(offspring[1], s2->distro[i], s2->weight[i])) {
        strategy_destroy(offspring[0]);
        strategy_destroy(offspring[1]);
        free(offspring);
        return NULL;
      }
      
    }
  }


  return offspring;
}

void strategy_print(FILE *out, const strategy *s)
{
  if (out == NULL || s == NULL)
    {
      return;
    }
  
  for (int i = 0; i < s->size; i++) {
    arrays_print(out, s->num_locations, s->distro[i]);
    fprintf(out, " %.3f\n", s->weight[i]);
  }
    

}

void strategy_destroy(strategy *s)
{
  // might need to watch valgrind to see if it should be s->capacity
  if (s != NULL)
    {
      for (int i = 0; i < s->size; i++) {
        free(s->distro[i]);
      }

      free(s->distro);
      free(s->weight);
      free(s);
    }
}

