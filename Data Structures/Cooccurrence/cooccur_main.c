#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cooccur.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage error");
        return 1;
    }
    
    cooccurrence_matrix *matrix = cooccur_create(argv + 1, argc - 1);
    if (matrix == NULL) {
        fprintf(stderr, "Matrix create Error\n");
        return 1;
    }
    //fprintf(stderr, "here");
    size_t n;
    char **context;
    while ((context = cooccur_read_context(matrix, stdin, &n)) != NULL) {
        cooccur_update(matrix, context, n);
        for (int i = 0; i < n; i++) {
            free(context[i]);
        }
        free(context);
    }

    for (int i = 1; i < argc; i++) {
        double *out = cooccur_get_vector(matrix, argv[i]);
        printf("%s: [", argv[i]);
        for (int j = 0; j < argc - 2; j++) {
            printf("%lf, ", out[j]);
        }
        printf("%lf]\n", out[argc - 2]);
        free(out);
    }
    cooccur_destroy(matrix);

    

    return 0;
}
