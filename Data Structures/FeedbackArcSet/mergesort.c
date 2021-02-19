#include <stdlib.h>
#include <string.h>

#include "mergesort.h"

void merge_sort(size_t n, size_t elt_size, void *in, void *out, int (*compare)(const void *, const void *))
{
  if (n < 2)
    {
      // array of size 0 or 1 is already sorted
      memcpy(out, in, elt_size * n);
      return;
    }

  int left_n = n / 2;
  int right_n = n - left_n;

  void *left_out = malloc(elt_size * left_n);
  void *right_out = malloc(elt_size * right_n);
    
  merge_sort(left_n, elt_size, in, left_out, compare);
  merge_sort(right_n, elt_size, ((char *)in) + (elt_size * left_n), right_out, compare);

  merge(elt_size, left_n, left_out, right_n, right_out, out, compare);

  free(left_out);
  free(right_out);
}

void merge(size_t elt_size, size_t left_n, void *left, size_t right_n, void *right, void *out, int (*compare)(const void *, const void *))
{
  int left_i = 0;
  int right_i = 0;
  int out_i = 0;
  
  while (left_i < left_n || right_i < right_n)
    {
      if (right_i == right_n ||
	  (left_i < left_n
	   && compare(((char *)left) + (left_i * elt_size),
		      ((char *)right) + (right_i * elt_size)) < 0))
	{
	  // right doesn't exist or left comes first
	  memcpy(((char *)out) + out_i * elt_size,
		 ((char *)left) + left_i * elt_size,
		 elt_size);
	  left_i++;
	}
      else
	{
	  // left doesn't exist or right comes first
	  memcpy(((char *)out) + out_i * elt_size,
		 ((char *)right) + right_i * elt_size,
		 elt_size);
	  right_i++;
	}
      out_i++;
    }
}
