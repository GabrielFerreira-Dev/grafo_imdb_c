#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "avl_tree.h"

#define MAX_LINE_LENGTH 1024
#define INITIAL_CAPACITY 100

typedef struct actor {
    int id;
    char *name;
    avl_node *movies;
} actor;

typedef struct movie {
    int id;
    char *title;
    avl_node *neighbors;
} movie;

void destruct(avl_node *n) {
    if (n) {
        destruct(n->left);
        destruct(n->right);
        free(n);
    }
}

int get_weight(avl_node *n) {
    if (!n) {
        return 0;
    }
    int left_level = n->left ? n->left->level : -1;
    int right_level = n->right ? n->right->level : -1;
    return right_level - left_level;
}

void update_level(avl_node *n) {
    if (n) {
        int left_level = n->left ? n->left->level : -1;
        int right_level = n->right ? n->right->level : -1;
        int max_level = left_level > right_level ? left_level : right_level;
        n->level = max_level + 1;
    }
}

avl_node *rotate_left(avl_node *n, int weight) {
    if (!n) {
        return NULL;
    }

    int right_weight = get_weight(n->right);

    if ((weight < 0 && right_weight > 0) || (weight > 0 && right_weight < 0)) {
        n->right = rotate_right(n->right, right_weight);
    }

    avl_node *temp = n->right;
    n->right = temp->left;
    temp->left = n;

    update_level(temp->left);
    update_level(temp);

    return temp;
}

avl_node *rotate_right(avl_node *n, int weight) {
    if (!n) {
        return NULL;
    }

    int left_weight = get_weight(n->left);

    if ((weight < 0 && left_weight > 0) || (weight > 0 && left_weight < 0)) {
        n->left = rotate_left(n->left, left_weight);
    }

    avl_node *temp = n->left;
    n->left = temp->right;
    temp->right = n;

    update_level(temp->right);
    update_level(temp);

    return temp;
}

void balance(avl_node **n) {
    if (*n) {
        int weight = get_weight(*n);
        if (weight > 1) {
            *n = rotate_left(*n, weight);
        } else if (weight < -1) {
            *n = rotate_right(*n, weight);
        } else {
            update_level(*n);
        }
    }
}

void insert_node(avl_node **n, avl_node *m) {
    if (!m) {
        return;
    }

    if (!*n) {
        *n = m;
        return;
    }

    if (m->id < (*n)->id) {
        insert_node(&(*n)->left, m);
    } else {
        insert_node(&(*n)->right, m);
    }

    balance(n);
}

void insert(avl_node **n, int id) {
    avl_node *m = (avl_node *)malloc(sizeof(avl_node));
    if (m) {
        m->id = id;
        m->level = 0;
        m->left = NULL;
        m->right = NULL;
    }
    insert_node(n, m);
}

void add_movie(actor *actor, int movie_id) {
    insert(&actor->movies, movie_id);
}

void add_neighbor(movie *movie, int neighbor_id) {
    insert(&movie->neighbors, neighbor_id);
}

char *trim(char *str) {
    char *end;

    // Trim leading space
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;

    // Write new null terminator character
    end[1] = '\0';

    return str;
}

actor *read_artists(const char *file_path, size_t *num_actors) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Cannot open file");
        return NULL;
    }

    char line[1024];
    int line_number = 0;
    size_t actor_count = 0;
    size_t actor_capacity = 10;
    actor *actors = malloc(actor_capacity * sizeof(actor));
    if (!actors) {
        perror("Cannot allocate memory for actors");
        fclose(file);
        return NULL;
    }

    while (fgets(line, sizeof(line), file)) {
        line_number++;
        if (line_number == 1) {
            // Skip header
            continue;
        }

        char *token = strtok(line, "\t");
        int column = 0;
        actor new_actor = {0};
        while (token != NULL) {
            if (column == 0) {
                // Process the first column (ID)
                token += 2; // Remove the first two characters
                new_actor.id = atoi(token);
            } else if (column == 1) {
                // Process the second column (name)
                new_actor.name = strdup(trim(token));
            } else if (column == 5) {
                // Process the sixth column (array of movie IDs)
                char *movie_id_token = strtok(token, ",");
                while (movie_id_token != NULL) {
                    movie_id_token += 2; // Remove the first two characters
                    int movie_id = atoi(movie_id_token);
                    add_movie(&new_actor, movie_id);
                    movie_id_token = strtok(NULL, ",");
                }
            }
            token = strtok(NULL, "\t");
            column++;
        }

        if (actor_count >= actor_capacity) {
            actor_capacity *= 2;
            actor *temp = realloc(actors, actor_capacity * sizeof(actor));
            if (!temp) {
                perror("Cannot reallocate memory for actors");
                for (size_t i = 0; i < actor_count; i++) {
                    free(actors[i].name);
                    // Free AVL tree nodes if necessary
                }
                free(actors);
                fclose(file);
                return NULL;
            }
            actors = temp;
        }

        actors[actor_count++] = new_actor;
        if (actor_count == 5) {
            break;
        }
    }

    fclose(file);
    *num_actors = actor_count;
    return actors;
}

movie *read_movies(const char *file_path, size_t *num_movies) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        perror("Cannot open file");
        return NULL;
    }

    char line[1024];
    int line_number = 0;
    size_t movie_count = 0;
    size_t movie_capacity = 10;
    movie *movies = malloc(movie_capacity * sizeof(movie));
    if (!movies) {
        perror("Cannot allocate memory for movies");
        fclose(file);
        return NULL;
    }

    while (fgets(line, sizeof(line), file)) {
        line_number++;
        if (line_number == 1) {
            // Skip header
            continue;
        }

        char *token = strtok(line, "\t");
        int column = 0;
        int isMovie = 0;
        movie new_movie = {0};
        while (token != NULL) {
            if (column == 0) {
                // Process the first column (ID)
                token += 2; // Remove the first two characters
                new_movie.id = atoi(token);
                isMovie = 0; // Reset isMovie to 0
            } else if (column == 1) {
                // Process the second column (type)
                if (strcmp(trim(token), "movie") == 0) {
                    isMovie = 1;
                }
            } else if (column == 2 && isMovie == 1) {
                // Process the third column (title) if it is a movie
                new_movie.title = strdup(trim(token));
            }
            token = strtok(NULL, "\t");
            column++;
        }

        if (isMovie) {
            if (movie_count >= movie_capacity) {
                movie_capacity *= 2;
                movie *temp = realloc(movies, movie_capacity * sizeof(movie));
                if (!temp) {
                    perror("Cannot reallocate memory for movies");
                    for (size_t i = 0; i < movie_count; i++) {
                        free(movies[i].title);
                        // Free AVL tree nodes if necessary
                    }
                    free(movies);
                    fclose(file);
                    return NULL;
                }
                movies = temp;
            }

            movies[movie_count++] = new_movie;
        } else {
            free(new_movie.title); // Free the allocated title if not a movie
        }
    }

    fclose(file);
    *num_movies = movie_count;
    return movies;
}

movie *find_movie_by_id(movie *movies, size_t num_movies, int id) {
    for (size_t i = 0; i < num_movies; ++i) {
        if (movies[i].id == id) {
            return &movies[i];
        }
    }
    return NULL; // Filme não encontrado
}

void add_all_neighbors(avl_node *root, movie *current_movie, movie *movies, size_t num_movies) {
    if (!root) return;
    
    if (root->id != current_movie->id) {
        add_neighbor(current_movie, root->id);
    }

    add_all_neighbors(root->left, current_movie, movies, num_movies);
    add_all_neighbors(root->right, current_movie, movies, num_movies);
}

void traverse_and_add_neighbors(avl_node *movie_node, movie *movies, size_t num_movies, actor *actor) {
    if (!movie_node) return;

    // Traverse left subtree
    traverse_and_add_neighbors(movie_node->left, movies, num_movies, actor);

    int movie_id = movie_node->id;
    movie *current_movie = find_movie_by_id(movies, num_movies, movie_id);

    if (current_movie) {
        // Add all other movies of the actor as neighbors to the current movie
        add_all_neighbors(actor->movies, current_movie, movies, num_movies);
    }

    // Traverse right subtree
    traverse_and_add_neighbors(movie_node->right, movies, num_movies, actor);
}

void associate_movies_with_actors(actor *actors, size_t num_actors, movie *movies, size_t num_movies) {
    for (size_t i = 0; i < num_actors; i++) {
        avl_node *movie_list = actors[i].movies;
        traverse_and_add_neighbors(movie_list, movies, num_movies, &actors[i]);
    }
}

void free_avl_tree(avl_node *node) {
    if (node) {
        free_avl_tree(node->left);
        free_avl_tree(node->right);
        free(node);
    }
}

void write_dot_file(const char *filename, movie *movies, size_t num_movies) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo para escrita");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "graph {\n");
    fprintf(file, "    concentrate=true;\n");

    for (size_t i = 0; i < num_movies; i++) {
        movie *filme = &movies[i];
        avl_node *neighbor = filme->neighbors;
        while (neighbor != NULL) {
            movie *neighbor_movie = find_movie_by_id(movies, num_movies, neighbor->id);
            if (neighbor_movie) {
                fprintf(file, "    \"%s\" -- \"%s\";\n", filme->title, neighbor_movie->title);
            }
            neighbor = neighbor->left ? neighbor->left : neighbor->right;
        }
    }

    fprintf(file, "}\n");
    fclose(file);
}

int main() {
    size_t num_actors, num_movies;
    actor *actors = read_artists("C:\\Users\\gabri\\OneDrive\\Desktop\\trabalhos\\Trabalho Grafo\\name.basics.tsv", &num_actors);
    movie *movies = read_movies("C:\\Users\\gabri\\OneDrive\\Desktop\\trabalhos\\Trabalho Grafo\\title.basics.tsv", &num_movies);

    if (actors && movies) {
        associate_movies_with_actors(actors, num_actors, movies, num_movies);
        write_dot_file("input.dot", movies, num_movies);

        for (size_t i = 0; i < num_actors; i++) {
            free(actors[i].name);
            free_avl_tree(actors[i].movies);
        }
        free(actors);

        for (size_t i = 0; i < num_movies; i++) {
            free(movies[i].title);
            free_avl_tree(movies[i].neighbors);
        }
        free(movies);
    }

    return EXIT_SUCCESS;
}
