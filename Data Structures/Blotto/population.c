#include <stdlib.h>
#include <stdbool.h>

#include "population.h"
#include "strategy.h"

// please make this smaller!
#define POPULATION_INITIAL_CAPACITY 4

struct _population
{
  // FIX ME -- use dynamic allocation
  strategy **elements;
  size_t size;
  size_t capacity;
};

/**
 * Resizes the array held by the given population.  (Since this is
 * private to this module, you can do whatever you want with this
 * function.)
 *
 * @param pop a pointer to a population, non-NULL
 */
bool embiggen(population *pop);

// should be done
population *population_create()
{
  population *result = malloc(sizeof(population));
  if (result != NULL)
    { 
      result->elements = malloc(sizeof(strategy *) * POPULATION_INITIAL_CAPACITY);
      result->capacity = POPULATION_INITIAL_CAPACITY;
      result->size = 0;
    }
  return result;
}

size_t population_size(const population *pop)
{
  if (pop == NULL)
    {
      return 0;
    }
  else
    {
      return pop->size;
    }
}

bool population_add_strategy(population *pop, strategy *s)
{
  if (pop == NULL || s == NULL)
    {
      return false;
    }

  if (pop->size > 0 && (strategy_count_locations(s) != strategy_count_locations(pop->elements[0])
			|| strategy_count_units(s) != strategy_count_units(pop->elements[0])))
    {
      return false;
    }

  if (pop->size == pop->capacity)
    {
      embiggen(pop);
    }

  pop->elements[pop->size] = s;
  pop->size++;

  return true;
}

// should be done
bool embiggen(population *pop)
{
  strategy **bigger = realloc(pop->elements, sizeof(strategy *) * pop->capacity * POPULATION_INITIAL_CAPACITY);
  if (bigger == NULL) {
    return false;
  }
  pop->elements = bigger;
  pop->capacity *= POPULATION_INITIAL_CAPACITY;
  return true;

  // a noble spirit embiggens the smallest man
  // the embiggen function should resize the dynamically allocated array
}

strategy *population_get(population *pop, size_t i)
{
  if (pop == NULL || i < 0 || i >= pop->size)
    {
      return NULL;
    }
  else
    {
      return pop->elements[i];
    }
}

strategy **population_remove_last(population *pop, size_t n)
{
  if (pop == NULL || n <= 0)
    {
      return NULL;
    }
  else if (n > pop->size)
    {
      n = pop->size;
    }

  // FIX ME -- make array to return removed strategies in
  strategy **removed = malloc(sizeof(strategy *)*n);
  if (removed == NULL) {
    return NULL;
  }

  size_t starting_point = pop->size - n;
  for (size_t i = 0; i < n; i++) {
    removed[i] = pop->elements[starting_point + i];
  }
  pop->size -= n;
  
  return removed;
}

void population_order(population *pop, const double *values)
{
  double score[pop->size];
  if (score == NULL)
    {
      return;
    }
  for (size_t i = 0; i < pop->size; i++)
    {
      score[i] = 0.0;
    }

  // compute scores for all individuals in population
  for (size_t i = 0; i < pop->size; i++)
    {
      for (size_t j = i + 1; j < pop->size; j++)
      {
        double wins = strategy_expected_wins(pop->elements[i], pop->elements[j], values);
        score[i] += wins;
        score[j] += (1.0 - wins);
      }
    }

  // bubble sort meets the time bound since the above loop
  // already takes Theta(n^2 * m * p) time
  for (size_t pass = 0; pass < pop->size; pass++)
    {
      for (size_t i = 0; i < pop->size - 1 - pass; i++)
        {
          if (score[i] < score[i + 1])
            {
              double temp_d = score[i];
              strategy *temp_s = pop->elements[i];
              score[i] = score[i + 1];
              pop->elements[i] = pop->elements[i + 1];
              score[i + 1] = temp_d;
              pop->elements[i + 1] = temp_s;
            }
        }
    }
}

void population_destroy(population *pop)
{
  //might need to watch valgrind to see if it should be pop->capacity instead
  if (pop != NULL)
    {
      for (size_t i = 0; i < pop->size; i++)
        {
          strategy_destroy(pop->elements[i]);
        }
      pop->size = 0;
      pop->capacity = 0;
      free(pop->elements);
      free(pop);
    }
}


