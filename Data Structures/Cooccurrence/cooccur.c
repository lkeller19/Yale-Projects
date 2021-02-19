#include <stdlib.h>
#include <string.h>
#include "gmap.h"

#include "cooccur.h"
#include "string_key.h"

struct cooccurrence_matrix
{
  int size;
  gmap *indices;
  gmap *vectors;
  //char **keywords;
  int max;
  //int *check;
};

//taken from chomp_main.c to free the values in 
void free_value(const void *key, void *value, void *arg);

cooccurrence_matrix *cooccur_create(char *key[], size_t n)
{
  gmap *check = gmap_create(duplicate, compare_keys, hash29, free);
  if (check == NULL) {
    return NULL;
  }
  for (int i = 0; i < n; i++) {
    if (gmap_contains_key(check, key[i])) {
      gmap_destroy(check);
      return NULL;
    }
    else {
      if (!gmap_put(check, key[i], NULL)) {
        gmap_destroy(check);
        return NULL;
      }
    }
  }
  gmap_destroy(check);
  // need to add all the malloc fail checks!!
  cooccurrence_matrix *new = malloc(sizeof(cooccurrence_matrix));
  if (new == NULL) {
    return NULL;
  }
  new->indices = gmap_create(duplicate, compare_keys, hash29, free);
  if (new->indices == NULL) {
    free(new);
    return NULL;
  }
  new->vectors = gmap_create(duplicate, compare_keys, hash29, free);
  if (new->vectors == NULL) {
    gmap_destroy(new->indices);
    free(new);
    return NULL;
  }
  new->size = n;
  new->max = 0;
  //new->check = calloc(n, sizeof(int));
  // new->keywords = malloc(sizeof(char *)*n);
  // if (new->keywords == NULL) {
  //   gmap_destroy(new->indices);
  //   gmap_destroy(new->vectors);
  //   free(new);
  //   return NULL;
  // }
  for (int i = 0; i < n; i++) {
    int length = strlen(key[i]);
    if (length > new->max) {
      new->max = length;
    }
    // new->keywords[i] = duplicate(key[i]);
  }

  for (int i = 0; i < n; i++) {
    double *ptr = calloc(n, sizeof(double));
    if (ptr == NULL) {
      gmap_for_each(new->vectors, free_value, NULL);
      gmap_destroy(new->indices);
      gmap_destroy(new->vectors);
      free(new);
    }
    if (!gmap_put(new->vectors, key[i], ptr)) {
      // free(new->keywords);
      free(ptr);
      gmap_for_each(new->vectors, free_value, NULL);
      gmap_destroy(new->indices);
      gmap_destroy(new->vectors);
      free(new);
      return NULL;
    }
    // fprintf(stderr, "%lf\n", ptr[0]);
  }

  for (int i = 0; i < n; i++) {
    int *ptr = malloc(sizeof(int));
    if (ptr == NULL) {
      gmap_for_each(new->vectors, free_value, NULL);
      gmap_for_each(new->indices, free_value, NULL);
      gmap_destroy(new->indices);
      gmap_destroy(new->vectors);
      free(new); 
      return NULL;
    }
    *ptr = i;
    if (!gmap_put(new->indices, key[i], ptr)) {
      free(ptr);
      gmap_for_each(new->vectors, free_value, NULL);
      gmap_for_each(new->indices, free_value, NULL);
      gmap_destroy(new->indices);
      gmap_destroy(new->vectors);
      free(new); 
      return NULL;
    }
  }

  // if (gmap_size(new->vectors) != n) {
  //     gmap_for_each(new->vectors, free_value, NULL);
  //     gmap_for_each(new->indices, free_value, NULL);
  //     gmap_destroy(new->indices);
  //     gmap_destroy(new->vectors);
  //     free(new); 
  //     return NULL;
  // }

  return new;
}

void cooccur_update(cooccurrence_matrix *mat, char **context, size_t n)
{
  for (int i = 0; i < n; i++) {
    //fprintf(stderr, "%s\n", context[i]);
    if (!gmap_contains_key(mat->vectors, context[i])) {
      return;
    }
  }
  // fprintf(stderr, "first half\n");

  for (int i = 0; i < n; i++) {
    double *vec = gmap_get(mat->vectors, context[i]);
    // fprintf(stderr, "%lf\n", vec[0]);
    for (int j = 0; j < n; j++) {
      int *index = gmap_get(mat->indices, context[j]);
      // fprintf(stderr, "%d\n", *index);
      vec[*index]++;
    }
  }
}

char **cooccur_read_context(cooccurrence_matrix *mat, FILE *stream, size_t *n)
{
  gmap *check = gmap_create(duplicate, compare_keys, hash29, free);
  if (check == NULL) {
    return NULL;
  }
  // TAKE CARE OF DOUBLE SHOWINGS CONTEXT WORDS
  char **context = malloc(sizeof(char *) * mat->size);
  if (context == NULL) {
    gmap_destroy(check);
    return NULL;
  }
//which to do while loop
  // char format[mat->max];
  // sprintf(format, "%%%ds", mat->max);
  char word[mat->max + 1];
  int counter = 0;
  char c;
  int word_count = 0;
  if ((c = getc(stream)) == '\n') {
    *n = word_count;
    gmap_destroy(check);

    return context;
  }
  else if (c == EOF) {
    gmap_destroy(check);
    free(context);
    return NULL;
  }
  do {
    //fprintf(stderr, "next: %c\n", c);
    // if (c == EOF) {
    //   gmap_destroy(check);
    //   free(context);
    //   return NULL;
    // }
    if (c != ' ' && counter < mat->max + 1) {
      word[counter] = c;
      word[counter+1] = '\0';
      counter++;
    }
    else if (c == ' ' && counter > 0) {
      //fprintf(stderr, "first\n");
      //word[counter] = '\0';
      if (gmap_contains_key(mat->vectors, word)) {
        if (!gmap_contains_key(check, word)) {
          context[word_count] = duplicate(word);
          word_count++;
          if (!gmap_put(check, word, NULL)) {
            free(context);
            gmap_destroy(check);
            return NULL;
          }
        }
      }
      counter = 0;
    }
    else if (c == ' ' && counter == 0){
      continue;
    }
    else {
      //fprintf(stderr, "%c\n", c);
      counter++;
    }
  } while ((c = getc(stream)) != '\n' && c != EOF);
  if (counter != 0) {
    //fprintf(stderr, "last\n");
    //word[counter] = '\0';
    if (gmap_contains_key(mat->vectors, word)) {
      //int *index = gmap_get(mat->indices, word);
      //if (mat->check[*index] == 0) {
      if (!gmap_contains_key(check, word)) {
        context[word_count] = duplicate(word);
        //fprintf(stderr, "%s\n", context[word_count]);
        word_count++;
        //mat->check[*index]++;
        if (!gmap_put(check, word, NULL)) {
          free(context);
          gmap_destroy(check);
          return NULL;
        }
      }
    }
  } 
  //memset(mat->check, 0, mat->size*sizeof(mat->check[0]));
  *n = word_count;
  gmap_destroy(check);

  return context;
}

double *cooccur_get_vector(cooccurrence_matrix *mat, const char *word)
{
  // divide by diagonal
  double *get = malloc(sizeof(double) * mat->size);
  int *diag = gmap_get(mat->indices, word);
  if (diag == NULL) {
    for (int i = 0; i < mat->size; i++) {
      get[i] = 0.0;
    }
  }
  
  double *vector = gmap_get(mat->vectors, word);
  double value = vector[*diag];
  if (value == 0) {
    for (int i = 0; i < mat->size; i++) {
      get[i] = 0.0;
    }
  } 
  else {
    for (int i = 0; i < mat->size; i++) {
      get[i] = vector[i] / value;
      //fprintf(stderr, "%d %lf\n",i, get[i]);
    } 
  }

  
  return get;
}

void cooccur_destroy(cooccurrence_matrix *mat)
{
  if (mat != NULL) {
    // for (int i = 0; i < mat->size; i++) {
    //   // free(gmap_get(mat->indices, mat->keywords[i]));
    //   // free(gmap_get(mat->vectors, mat->keywords[i]));
    //   free(mat->keywords[i]);
    // }
    // free(mat->keywords);
    gmap_for_each(mat->indices, free_value, NULL);
    gmap_for_each(mat->vectors, free_value, NULL);
    gmap_destroy(mat->indices);
    gmap_destroy(mat->vectors);
    //free(mat->check);
    free(mat);
  }

}

void free_value(const void *key, void *value, void *arg)
{
  free(value);
}
