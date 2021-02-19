#ifndef __MERGESORT_H__
#define __MERGESORT_H__

#include <stdlib.h>

/**
 * Makes a sorted copy of the given input array in the given output array.
 *
 * @param n a nonnegative integer
 * @param elt_size the size of the elements in in
 * @param in an array of n cities
 * @param out an array that can hold n cities
 * @param compare a function that takes pointers to two elements and compares
 * them, returning negative if the first comes before the second, positive
 * for the other way around, and 0 if they are the same
 */
void merge_sort(size_t n, size_t elt_size, void *in, void *out, int (*compare)(const void *, const void *));

/**
 * Merges the two given sorted lists into the given output array.

 * @param elt_size the size of the elements in left and right
 * @param left_n the size of left_out
 * @param left an array
 * @param right_n the size of right_out
 * @param right an array
 * @param compare a function that takes pointers to two elements and compares
 * them, returning negative if the first comes before the second, positive
 * for the other way around, and 0 if they are the same
 * @param out an array that can hold left_n + right_n cities
 */
void merge(size_t elt_size, size_t left_n, void *left, size_t right_n, void *right, void *out, int (*compare)(const void *, const void *));

#endif
