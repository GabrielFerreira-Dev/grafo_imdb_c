#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

struct node {
    int id;
    struct node *next;
};

struct actor {
    int id;
    char *name;
    struct node *movies;
};

struct movie {
    int id;
    char *title;
    struct node *neighbors;
};

void add_movie(struct actor *actor, int movie_id) {
    struct node *new_node = (struct node *)malloc(sizeof(struct node));
    if (new_node == NULL) {
        perror("Erro ao alocar memória para o nó do filme");
        exit(EXIT_FAILURE);
    }
    new_node->id = movie_id;
    new_node->next = actor->movies;
    actor->movies = new_node;
}

void add_neighbor(struct movie *movie, int neighbor_id) {
    struct node *new_node = (struct node *)malloc(sizeof(struct node));
    if (new_node == NULL) {
        perror("Erro ao alocar memória para o nó do vizinho");
        exit(EXIT_FAILURE);
    }
    new_node->id = neighbor_id;
    new_node->next = movie->neighbors;
    movie->neighbors = new_node;
}

void read_artists(const char *file_path, struct actor **actors, int *num_actors, int max_read) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    char line[MAX_LINE_LENGTH];
    int count = 0;

    if (fgets(line, sizeof(line), file) == NULL) {
        perror("Erro ao ler o cabeçalho");
        fclose(file);
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL && count < max_read) {
        line[strcspn(line, "\n")] = '\0';

        char *token = strtok(line, "\t");
        int column = 0;

        struct actor new_actor;
        new_actor.movies = NULL;

        while (token != NULL) {
            if (column == 0) {
                new_actor.id = atoi(token + 2);
            } else if (column == 1) {
                new_actor.name = strdup(token);
                if (new_actor.name == NULL) {
                    perror("Erro ao alocar memória para o nome do ator");
                    exit(EXIT_FAILURE);
                }
            } else if (column == 5) {
                char *movie_id_str = strtok(token, ",");
                while (movie_id_str != NULL) {
                    int movie_id = atoi(movie_id_str + 2);
                    add_movie(&new_actor, movie_id);
                    movie_id_str = strtok(NULL, ",");
                }
            }
            token = strtok(NULL, "\t");
            column++;
        }

        actors[*num_actors] = (struct actor *)malloc(sizeof(struct actor));
        if (actors[*num_actors] == NULL) {
            perror("Erro ao alocar memória para o ator");
            exit(EXIT_FAILURE);
        }
        *actors[*num_actors] = new_actor;
        (*num_actors)++;
        count++;
    }

    fclose(file);
}

void read_movies(const char *file_path, struct movie **movies, int *num_movies, int max_movies) {
    FILE *file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return;
    }

    char line[MAX_LINE_LENGTH];

    if (fgets(line, sizeof(line), file) == NULL) {
        perror("Erro ao ler o cabeçalho");
        fclose(file);
        return;
    }

    while (*num_movies < max_movies && fgets(line, sizeof(line), file) != NULL) {
        line[strcspn(line, "\n")] = '\0';

        char *token = strtok(line, "\t");
        int column = 0;

        struct movie new_movie;
        new_movie.neighbors = NULL;
        new_movie.title = NULL;
        int isFilme = 0;

        while (token != NULL) {
            if (column == 0) {
                isFilme = 0;
                new_movie.id = atoi(token + 2);
            } else if (column == 1 && strcmp(token, "movie") == 0) {
                isFilme = 1;
            } else if (column == 2 && isFilme == 1) {
                new_movie.title = strdup(token);
                if (new_movie.title == NULL) {
                    perror("Erro ao alocar memória para o título do filme");
                    exit(EXIT_FAILURE);
                }
            }
            token = strtok(NULL, "\t");
            column++;
        }

        if (new_movie.title != NULL) {
            movies[*num_movies] = (struct movie *)malloc(sizeof(struct movie));
            if (movies[*num_movies] == NULL) {
                perror("Erro ao alocar memória para o filme");
                exit(EXIT_FAILURE);
            }
            *movies[*num_movies] = new_movie;
            (*num_movies)++;
        } else {
            if (new_movie.title != NULL) {
                free(new_movie.title);
            }
        }
    }

    fclose(file);
}

struct movie *find_movie_by_id(struct movie **movies, int num_movies, int id) {
    for (int i = 0; i < num_movies; i++) {
        if (movies[i]->id == id) {
            return movies[i];
        }
    }
    return NULL;
}

void form_cliques(struct actor **actors, int num_actors, struct movie **movies, int num_movies) {
    for (int i = 0; i < num_actors; i++) {
        struct node *movie1 = actors[i]->movies;
        while (movie1 != NULL) {
            struct node *movie2 = movie1->next;
            while (movie2 != NULL) {
                struct movie *m1 = find_movie_by_id(movies, num_movies, movie1->id);
                struct movie *m2 = find_movie_by_id(movies, num_movies, movie2->id);
                if (m1 && m2) {
                    add_neighbor(m1, m2->id);
                    add_neighbor(m2, m1->id);
                }
                movie2 = movie2->next;
            }
            movie1 = movie1->next;
        }
    }
}

void write_dot_file(const char *filename, struct movie **movies, int num_movies) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        perror("Erro ao abrir o arquivo para escrita");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "graph {\n");
    fprintf(file, "    concentrate=true;\n");

    for (int i = 0; i < num_movies; i++) {
        struct movie *movie = movies[i];
        struct node *neighbor = movie->neighbors;
        while (neighbor != NULL) {
            struct movie *neighbor_movie = find_movie_by_id(movies, num_movies, neighbor->id);
            if (neighbor_movie) {
                fprintf(file, "    \"%s\" -- \"%s\";\n", movie->title, neighbor_movie->title);
            }
            neighbor = neighbor->next;
        }
    }

    fprintf(file, "}\n");
    fclose(file);
}

int main() {
    const char *file_path_artists = "C:\\Users\\gabri\\OneDrive\\Desktop\\trabalhos\\Trabalho Jango Arvore\\name.basics.tsv";
    const char *file_path_movies = "C:\\Users\\gabri\\OneDrive\\Desktop\\trabalhos\\Trabalho Jango Arvore\\title.basics.tsv";
    int max_read = 10;
    int max_movies = 100000;

    struct actor *actors[1000];
    struct movie **movies = (struct movie **)malloc(max_movies * sizeof(struct movie *));
    if (movies == NULL) {
        perror("Erro ao alocar memória para o array de filmes");
        exit(EXIT_FAILURE);
    }

    int num_actors = 0;
    int num_movies = 0;

    read_artists(file_path_artists, actors, &num_actors, max_read);
    read_movies(file_path_movies, movies, &num_movies, max_movies);
    form_cliques(actors, num_actors, movies, num_movies);
    write_dot_file("input.dot", movies, num_movies);

    for (int i = 0; i < num_actors; i++) {
        free(actors[i]->name);
        struct node *movie = actors[i]->movies;
        while (movie != NULL) {
            struct node *temp = movie;
            movie = movie->next;
            free(temp);
        }
        free(actors[i]);
    }

    for (int i = 0; i < num_movies; i++) {
        free(movies[i]->title);
        struct node *neighbor = movies[i]->neighbors;
        while (neighbor != NULL) {
            struct node *temp = neighbor;
            neighbor = neighbor->next;
            free(temp);
        }
        free(movies[i]);
    }
    free(movies); 

    return 0;
}
