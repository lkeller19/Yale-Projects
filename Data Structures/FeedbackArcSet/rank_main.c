#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lugraph.h"
#include "gmap.h"
#include "string_key.h"

void free_value(const void *key, void *value, void *arg);

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Only two arguments allowed\n");
        return 1;
    }

    gmap *vertices = gmap_create(duplicate, compare_keys, hash29, free);
    if (vertices == NULL) {
        fprintf(stderr, "gmap vertices fail\n");
        return 1;
    }

    size_t n = 0;
    size_t total = 0;
    char **names_holder = NULL;
    char** games = read_input(stdin, &n, &vertices, &total, &names_holder);
    if (games == NULL || names_holder == NULL) {
        gmap_for_each(vertices, free_value, NULL);
        gmap_destroy(vertices);
        fprintf(stderr, "read_error\n");
        return 1;
    }
    for (int i=0; i < total; i++) {
        fprintf(stderr, "%s, %s\n", games[i], games[i + 1]);
        i++;
    }

    // INSTEAD NEED TO CREATE A GMAP WITH A STRUCT THAT CAN ACT LIKE THE ARRAY
    // typedef struct value {
    //     int value;
    //     int tracker;
    // } value;

    // typedef struct node {
    //     int vertex;
    //     value value;
    // } node;
    // ^^ this is the gmap that i need to put into
    
    for (int i = 0; i < n; i++) {
        fprintf(stderr, "%s ", names_holder[i]);
        fprintf(stderr, "%ld ", *(size_t*)gmap_get(vertices, names_holder[i]));
    }
    fprintf(stderr, "\n");

    gmap** adjset = malloc(sizeof(gmap*)*n);
    if (adjset == NULL) {
        gmap_for_each(vertices, free_value, NULL);
        gmap_destroy(vertices);
        return 1;
    }
    for (int i = 0; i < n; i++) {
        adjset[i] = gmap_create(duplicate, compare_keys, hash29, free);
        if (adjset[i] == NULL) {
            free(adjset);
            gmap_for_each(vertices, free_value, NULL);
            gmap_destroy(vertices);
            return 1;
        }
    }
    for (int i = 0; i < total; i++) {
        char* winner = games[i];
        char* loser = games[i+1];
        size_t winner_vertex = *(size_t*)gmap_get(vertices, winner);
        //int loser_vertex = *(int*)gmap_get(vertices, loser);
        if (gmap_contains_key(adjset[winner_vertex], loser)) {
            size_t* reciever = (size_t*)gmap_get(adjset[winner_vertex], loser);
            //int other = *(int*)gmap_get(adjset[loser_vertex], winner);
            (*reciever)++;
            // fprintf(stderr, "winner - loser: %s - %s %d\n", winner, loser, *(int*)gmap_get(adjset[winner_vertex], loser));
        }
        else {
            size_t* ptr = malloc(sizeof(size_t));
            if (ptr == NULL) {
                free(adjset);
                gmap_for_each(vertices, free_value, NULL);
                gmap_destroy(vertices);
            }
            *ptr = 1;

            if (!gmap_put(adjset[winner_vertex], loser, ptr)) {
                // ADD FREES
                return 1;
            }
            // fprintf(stderr, "winner: %s %d\n", winner, *ptr);
        }
        i++;
    }



    // gmap *checker = gmap_create(duplicate, compare_keys, hash29, free);
    // if (checker == NULL) {
    //     fprintf(stderr, "checker fail\n");
    //     return 1;
    // }
    // for (int i = 0; i < n; i++) {
    //     int *arr = calloc(n, sizeof(int));
    //     gmap_put(checker, names_holder[i], arr);
    // }

    // int *reciever;
    // for (int i = 0; i < total; i++) {
    //     char* winner = games[i];
    //     char* loser = games[i+1];
    //     int loser_vertex = *(int*)gmap_get(vertices, loser);
    //     reciever = (int*)gmap_get(checker, winner);
    //     reciever[loser_vertex]++;
    //     i++;
    // }
    // fprintf(stderr, "\n");


    //PRINTS OUT MATRIX BUT ITS TOO SLOW ANYWAY SO NEED TO CHANGE
    // for (int i = 0; i < n; i++) {
    //     fprintf(stderr, "%s: ", names_holder[i]);
    //     reciever = (int *)gmap_get(checker, names_holder[i]);
    //     for (int j = 0; j < n; j++) {
    //         fprintf(stderr, "%d, ", reciever[j]);
    //     }
    //     fprintf(stderr, "\n");
    // }

    // char** final = malloc(sizeof(char*)*total);
    char* final[total];
    // int *other;
    size_t number = 0;
    for (size_t i = 0; i < total; i++) {
        char* winner = games[i];
        char* loser = games[i+1];

        size_t loss = *(size_t*)gmap_get(vertices, loser);
        size_t win = *(size_t*)gmap_get(vertices, winner);

        size_t* other = (size_t*)gmap_get(adjset[win], loser);
        size_t* reciever = (size_t*)gmap_get(adjset[loss], winner);

        if (reciever == NULL) {
            if (*other != 0) {
                final[number++] = names_holder[win];
                final[number++] = names_holder[loss];
                *other = 0;
                
                fprintf(stderr, "here: %ld\n",*(size_t*)gmap_get(adjset[win], loser));
                //fprintf(stderr,"HEEEEEE\n");
                }
        }
        //fprintf(stderr, "# of wins by winner: %d...# of wins by loser: %d\n", *other, *reciever);
        else if (*reciever < *other) {
            final[number++] = names_holder[win];
            final[number++] = names_holder[loss];
            *other = 0;
            *reciever = 0;
            //fprintf(stderr, "here: %ld\n",*(size_t*)gmap_get(adjset[loss], winner));
        }

        i++;
    }
    //fprintf(stderr, "%d\n", number);

    // char** final = malloc(sizeof(char*)*total);
    // int *other;
    // int number = 0;
    // for (int i = 0; i < total; i++) {
    //     char* winner = games[i];
    //     char* loser = games[i+1];
    //     reciever = (int*)gmap_get(checker, winner);
    //     other = (int*)gmap_get(checker, loser);
    //     int loss = *(int*)gmap_get(vertices, loser);
    //     int win = *(int*)gmap_get(vertices, winner);
    //     if (reciever[loss] > other[win]) {
    //         final[number++] = names_holder[win];
    //         final[number++] = names_holder[loss];
    //         reciever[loss] = 0;
    //         other[win] = 0;
    //     }
    //     i++;
    // }
    for (size_t i=0; i < number; i++) {
        fprintf(stderr, "%s, %s\n", final[i], final[i + 1]);
        i++;
    }

    lugraph* g = lugraph_create(n, vertices, names_holder);
    if (g == NULL) {
        fprintf(stderr, "Graph create error\n");
        return 1;
    }

    for (size_t i = 0; i < number; i++) {
        lugraph_add_edge(g, *(size_t*)gmap_get(vertices, final[i]), *(size_t*)gmap_get(vertices, final[i+1]));
        i++;
    }
    fprintf(stderr, "\n");
    print_adj_list(g, names_holder);
    fprintf(stderr, "\n");

    lugraph_add_degrees(g);

    size_t* outdegrees = lugraph_outdegrees(g);
    size_t* indegrees = lugraph_indegrees(g);
    float* ratios = lugraph_ratios(g);

    for (size_t i = 0; i < n; i++) {
        fprintf(stderr, "Name: %s Outdegree: %ld Indegree: %ld Ratio: %f\n", names_holder[i], outdegrees[i], indegrees[i], ratios[i]);
    }

    // for (int i = 0; i < total; i++) {

    // }

    if (strcmp(argv[1], "-degree") == 0) {
        // gmap *holder = degree(g, names_holder);
        // int *ordered = order(g, holder, names_holder);
        int *ordered = order_degrees(g);
        int wrong = wrong_way(g, ordered);
        fprintf(stderr, "\n");
        printf("%d\n", wrong);
        for (int i = 0; i < n; i++) {
            printf("%s\n", names_holder[ordered[i]]);
        }
        fprintf(stderr, "\n");
        free(ordered);
        
    }
    else if (strcmp(argv[1], "-topo") == 0) {
        if (!cycle(g)) {
            int *ordered = topological(g);
            if (ordered == NULL) {
                goto destroy;
            }
            int wrong = wrong_way(g, ordered);
            fprintf(stderr, "\n");
            printf("%d\n", wrong);
            for (int i = 0; i < n; i++) {
                printf("%s\n", names_holder[ordered[i]]);
            }
            fprintf(stderr, "\n");
            free(ordered);
        }
        else {
            fprintf(stderr, "there is a cycle so can't sort topologically\n");
        }
        
        
    }
    else if (strcmp(argv[1], "-dfs") == 0) {
        
        int *ordered = malloc(sizeof(int)*n);
        if (ordered == NULL) {
            goto destroy;
        }
        //ordered = NULL;
        dfs(g, ordered);
        int wrong = wrong_way(g, ordered);
        
        fprintf(stderr, "\n");
        printf("%d\n", wrong);
        for (int i = 0; i < n; i++) {
            //fprintf(stderr, "%d ", ordered[i]);
            printf("%s\n", names_holder[ordered[i]]);
        }
        fprintf(stderr, "\n");
        free(ordered);
    }
    else {
        fprintf(stderr, "Invalid method\n");
    }

    destroy:
    for (size_t i = 0; i < total; i++) {
        free(games[i]);
        free(games[i+1]);
        i++;
    }
    free(games);

    for (size_t i = 0; i < n; i++) {
        gmap_for_each(adjset[i], free_value, NULL);
        gmap_destroy(adjset[i]);
        free(names_holder[i]);
    }
    free(names_holder);
    free(adjset);
    gmap_for_each(vertices, free_value, NULL);
    gmap_destroy(vertices);
    lugraph_destroy(g);
    fprintf(stderr, "success\n");
    return 0;
}

void free_value(const void *key, void *value, void *arg)
{
  free(value);
}

