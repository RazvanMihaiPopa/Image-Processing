#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <inttypes.h>

typedef struct QuadtreeNode {
    unsigned char blue, green, red;
    uint32_t area;
    int32_t top_left, top_right;
    int32_t bottom_left, bottom_right;
} __attribute__((packed)) QuadtreeNode;

typedef struct Pixel {
    unsigned char blue, green, red;
} __attribute__((packed)) Pixel;

typedef struct DynamicQuadtree {
    Pixel pixel;
    uint32_t area;
    int32_t index_array;
    struct DynamicQuadtree *top_left, *top_right;
    struct DynamicQuadtree *bottom_left, *bottom_right;
} __attribute__((packed)) DynamicQuadtree;

typedef struct queue_node {
    DynamicQuadtree *data;
    struct queue_node *next;
    struct queue_node *prev;
} __attribute__((packed)) node_t;

typedef struct queue_list {
    node_t *head;
    node_t *tail;
    int len;
} queue_t;

// functie de eliberare memorie pentru un arbore
void destroyTree(DynamicQuadtree **root) {
    if(*root == NULL)
        return;
    destroyTree(&(*root)->top_left);
    destroyTree(&(*root)->top_right);
    destroyTree(&(*root)->bottom_right);
    destroyTree(&(*root)->bottom_left);
    free(*root);
    *root = NULL;
}

// functie de initializare a unui nod din coada
node_t *init_qNode(DynamicQuadtree *root) {
    node_t *node = malloc(sizeof(node_t));
    node->data = root;
    node->next = NULL;
    node->prev = NULL;
    return node;
}

// functie de initializare coada
queue_t* initqList() {
    queue_t *q = malloc(sizeof(queue_t));
    q->head = NULL;
    q->tail = NULL;
    q->len = 0;
    return q;
}

// functie de eliberare memorie
void destroyList(queue_t *list) {
    node_t *aux;
    while (list->len > 0) {
        aux = list->head;
        list->head = aux->next;
        free(aux);
        list->len--;
    }
    free(list);
}

// functie de adaugare element in coada
void enqueue(queue_t *queue, DynamicQuadtree *node) {
    node_t *nou = init_qNode(node);
    if(queue->len == 0) {
        queue->head = nou;
        queue->tail = nou;
        queue->len++;
    }
    else {
        nou->next = queue->head;
        queue->head->prev = nou;
        queue->head = nou;
        queue->len++;
    }
    return;
}

// functie care scoate un element din coada si il returneaza
DynamicQuadtree* dequeue(queue_t *queue) {
    DynamicQuadtree *node;
    node_t *aux = queue->tail;
    if(queue->len == 0) {
        return 0;
    }
    else if(queue->len == 1) {
        node = queue->tail->data;
        free(aux);
        queue->head = NULL;
        queue->tail = NULL;
        queue->len = 0;
        return node;
    }
    else {
        node=queue->tail->data;
        queue->tail = aux->prev;
        queue->tail->next = NULL;
        free(aux);
        queue->len--;
        return node;
    }
    return 0;
}

// functie pentru a introduce arborele in vector
void BFSqueue(DynamicQuadtree *root, QuadtreeNode *array) {
    if(root == NULL)
        return;
    int i = 0;
    queue_t *BFSqueue = initqList();
    enqueue(BFSqueue, root);
    while(BFSqueue->len != 0) {
        DynamicQuadtree *node = dequeue(BFSqueue);
        node->index_array = i;
        array[i].area = node->area;
        array[i].red=node->pixel.red;
        array[i].green = node->pixel.green;
        array[i].blue = node->pixel.blue;
        if(node->top_left != NULL) {
            enqueue(BFSqueue, node->top_left);
        }
        if(node->top_right != NULL) {
            enqueue(BFSqueue, node->top_right);
        }
        if(node->bottom_right != NULL) {
            enqueue(BFSqueue, node->bottom_right);
        }
        if(node->bottom_left != NULL) {
            enqueue(BFSqueue, node->bottom_left);
        }
        i++;
    }
    destroyList(BFSqueue);
    BFSqueue = initqList();
    enqueue(BFSqueue, root);
    i = 0;
    while(BFSqueue->len != 0) {
        DynamicQuadtree *node = dequeue(BFSqueue);
        if(node->top_left != NULL) {
            array[i].top_left = node->top_left->index_array;
            enqueue(BFSqueue, node->top_left);
        }
        else {
            array[i].top_left = -1;
        }
        if(node->top_right != NULL) {
            array[i].top_right = node->top_right->index_array;
            enqueue(BFSqueue, node->top_right);
        }
        else {
            array[i].top_right = -1;
        }
        if(node->bottom_right != NULL) {
            array[i].bottom_right = node->bottom_right->index_array;
            enqueue(BFSqueue, node->bottom_right);
        }
        else {
            array[i].bottom_right = -1;
        }
        if(node->bottom_left != NULL) {
            array[i].bottom_left = node->bottom_left->index_array;
            enqueue(BFSqueue, node->bottom_left);
        }
        else {
            array[i].bottom_left = -1;
        }
        i++;
    } 
    destroyList(BFSqueue);
}

// functie de initializare a unui nod din arbore
DynamicQuadtree* init_node(int height, int width) {
    DynamicQuadtree *node = malloc(sizeof(DynamicQuadtree));
    node->top_left = NULL;
    node->top_right = NULL;
    node->bottom_left = NULL;
    node->bottom_right = NULL;
    node->index_array = 0;
    node->area=height * width;
    node->pixel.red = 0;
    node->pixel.blue = 0;
    node->pixel.green = 0;
    return node;
}

// functie pentru creearea arborelui cuaternar
void create_Quadtree(DynamicQuadtree *parent, Pixel **matrix, int size, double factor) {
    int i, j;
    uint64_t red = 0, green = 0, blue = 0;
    // aflu culorile medii pentru un nod
    for(i = 0; i < size; i++)
        for(j = 0; j < size; j++) {
            red += matrix[i][j].red;
            green += matrix[i][j].green;
            blue += matrix[i][j].blue;
        }
    red = red/(size * size);
    green = green/(size * size);
    blue = blue/(size * size);
    parent->pixel.red = red;
    parent->pixel.green = green;
    parent->pixel.blue = blue;
    uint64_t mean = 0;
    for(i = 0; i < size; i++)
        for(j = 0; j < size; j++) {
            mean += pow(parent->pixel.red - matrix[i][j].red, 2);
            mean += pow(parent->pixel.green - matrix[i][j].green, 2);
            mean += pow(parent->pixel.blue - matrix[i][j].blue, 2);
        }
    mean = mean/(3 * size * size);
    // formez copii daca pragul impus este mai mic sau egal
    if(mean > factor) {
        // aloc o matrice noua pentru a o folosi in apelarea recursiva
        Pixel **newmatrix = malloc((size/2) * sizeof(Pixel*));
        for(i = 0; i < size/2; i++)
            newmatrix[i] = malloc((size/2) * sizeof(Pixel));
        parent->top_left = init_node(size/2, size/2);
        for(i = 0; i < size/2; i++)
            for(j = 0; j < size/2; j++)
                newmatrix[i][j] = matrix[i][j];
        create_Quadtree(parent->top_left, newmatrix, size/2, factor);
        parent->top_right = init_node(size/2, size/2);
        for(i = 0; i < size/2; i++)
            for(j = 0; j < size/2; j++)
                newmatrix[i][j] = matrix[i][j + size/2];
        create_Quadtree(parent->top_right, newmatrix, size/2, factor);
        parent->bottom_left = init_node(size/2, size/2);
        for(i = 0; i < size/2; i++)
            for(j = 0; j < size/2; j++)
                newmatrix[i][j] = matrix[i + size/2][j];
        create_Quadtree(parent->bottom_left, newmatrix, size/2, factor);
        parent->bottom_right = init_node(size/2, size/2);
        for(i = 0; i < size/2; i++)
            for(j = 0; j < size/2; j++)
                newmatrix[i][j] = matrix[i + size/2][j + size/2];
        create_Quadtree(parent->bottom_right, newmatrix, size/2, factor);
        for(i = 0; i < size/2; i++)
            free(newmatrix[i]);
        free(newmatrix);
    }
}

// fucntie pentru a crea arborele cuaternar implementat dinamic din vector
void create_quadtree_from_array(DynamicQuadtree *root, QuadtreeNode *array, int i) {
    if(array[i].top_left != -1) {
        DynamicQuadtree *child = init_node(sqrt(array[array[i].top_left].area), sqrt(array[array[i].top_left].area));
        child->pixel.red = array[array[i].top_left].red;
        child->pixel.green = array[array[i].top_left].green;
        child->pixel.blue = array[array[i].top_left].blue;
        root->top_left = child;
        create_quadtree_from_array(child, array, array[i].top_left);
    }
    if(array[i].top_right != -1) {
        DynamicQuadtree *child = init_node(sqrt(array[array[i].top_right].area), sqrt(array[array[i].top_right].area));
        child->pixel.red = array[array[i].top_right].red;
        child->pixel.green = array[array[i].top_right].green;
        child->pixel.blue = array[array[i].top_right].blue;
        root->top_right = child;
        create_quadtree_from_array(child, array, array[i].top_right);
    }
    if(array[i].bottom_right != -1) {
        DynamicQuadtree *child = init_node(sqrt(array[array[i].bottom_right].area), sqrt(array[array[i].bottom_right].area));
        child->pixel.red = array[array[i].bottom_right].red;
        child->pixel.green = array[array[i].bottom_right].green;
        child->pixel.blue = array[array[i].bottom_right].blue;
        root->bottom_right = child;
        create_quadtree_from_array(child, array, array[i].bottom_right);
    }
    if(array[i].bottom_left != -1) {
        DynamicQuadtree *child = init_node(sqrt(array[array[i].bottom_left].area), sqrt(array[array[i].bottom_left].area));
        child->pixel.red = array[array[i].bottom_left].red;
        child->pixel.green = array[array[i].bottom_left].green;
        child->pixel.blue = array[array[i].bottom_left].blue;
        root->bottom_left = child;
        create_quadtree_from_array(child, array, array[i].bottom_left);
    }
}

// functie pentru a verifica daca un nod este frunza
int isLeaf(DynamicQuadtree *node) {
    return (node->top_left == NULL && node->top_right == NULL && node->bottom_left == NULL && node->bottom_right == NULL);
}

// functie pentru a afla numarul de frunze al unui arbore
int getNumberLeaves(DynamicQuadtree *root) {
    if(isLeaf(root))
        return 1;
    else if(root==NULL)
        return 0;
    else return getNumberLeaves(root->top_left)+getNumberLeaves(root->top_right)+getNumberLeaves(root->bottom_left)+getNumberLeaves(root->bottom_right);
}
// functie pentru a afla numarul de noduri al unui arbore
int getNumberNodes(DynamicQuadtree *root) {
    if(root == NULL)
        return 0;
    return 1 + getNumberNodes(root->top_left) + getNumberNodes(root->top_right)
        + getNumberNodes(root->bottom_left) + getNumberNodes(root->bottom_right);
}

void create_pixels_matrix(DynamicQuadtree *root, Pixel **matrix, int size, int x, int y) {
    unsigned int i,j;
    // daca se ajunge la frunze, atribui culorile frunzelor bloculurilor respective din matricea de pixeli
    if(isLeaf(root->top_left)) {
        for(i = x; i < x + size/2; i++)
            for(j = y; j < y + size/2; j++) {
                matrix[i][j].red = root->top_left->pixel.red;
                matrix[i][j].green = root->top_left->pixel.green;
                matrix[i][j].blue = root->top_left->pixel.blue;
            }
    }
    else {
        create_pixels_matrix(root->top_left, matrix, size/2, x, y);
    }
    if(isLeaf(root->top_right)) {
        for(i = x; i < x + size/2; i++)
            for(j = y + size/2; j < y + size; j++) {
                matrix[i][j].red = root->top_right->pixel.red;
                matrix[i][j].green = root->top_right->pixel.green;
                matrix[i][j].blue = root->top_right->pixel.blue;
            }
    }
    else {
        create_pixels_matrix(root->top_right, matrix, size/2, x, y+size/2);
    }
    if(isLeaf(root->bottom_right)) {
        for(i = x + size/2; i < x + size; i++)
            for(j = y + size/2; j < y + size; j++) {
                matrix[i][j].red = root->bottom_right->pixel.red;
                matrix[i][j].green = root->bottom_right->pixel.green;
                matrix[i][j].blue = root->bottom_right->pixel.blue;
            }
    }
    else {
        create_pixels_matrix(root->bottom_right, matrix, size/2, x+size/2, y+size/2);
    }
    if(isLeaf(root->bottom_left)) {
        for(i = x + size/2; i < x + size; i++)
            for(j = y; j < y + size/2; j++) {
                matrix[i][j].red = root->bottom_left->pixel.red;
                matrix[i][j].green = root->bottom_left->pixel.green;
                matrix[i][j].blue = root->bottom_left->pixel.blue;
            }
    }
    else {
        create_pixels_matrix(root->bottom_left, matrix, size/2, x+size/2, y);
    }
}
void horizontal_flip(DynamicQuadtree *root) {
    // cat timp nodul nu este frunza
    if(!isLeaf(root)) {
        // interschimbare copil top_left cu top_right
        DynamicQuadtree *aux = root->top_left;
        root->top_left = root->top_right;
        root->top_right = aux;
        // interschimbare copil bottom_left cu bottom_right
        aux = root->bottom_left;
        root->bottom_left = root->bottom_right;
        root->bottom_right = aux;
        horizontal_flip(root->top_left);
        horizontal_flip(root->top_right);
        horizontal_flip(root->bottom_left);
        horizontal_flip(root->bottom_right);
    }
}

void vertical_flip(DynamicQuadtree *root) {
    // cat timp nodul nu este frunza
    if(!isLeaf(root)) {
        // interschimbare copil top_left cu bottom_left
        DynamicQuadtree *aux = root->top_left;
        root->top_left = root->bottom_left;
        root->bottom_left = aux;
        // interschimbare copil top_right cu bottom_right
        aux = root->bottom_right;
        root->bottom_right = root->top_right;
        root->top_right = aux;
        vertical_flip(root->top_left);
        vertical_flip(root->top_right);
        vertical_flip(root->bottom_left);
        vertical_flip(root->bottom_right);
    }
}
int main(int argc, char *argv[]) {
    int j = 1, i, factor = 0, task = 0;
    char type;
    FILE *input_file, *output_file;
    while(j < argc) {
        if(strcmp(argv[j], "-c") == 0) {
            j++;
            factor = atoi(argv[j]);
            task = 1;
        }
        if(strcmp(argv[j], "-d") == 0) {
            task = 2;
        }
        if(strcmp(argv[j], "-m") == 0) {
            j++;
            task = 3;
            type = argv[j][0];
            j++;
            factor = atoi(argv[j]);
        }
        if(j == argc - 2) {
            input_file = fopen(argv[j], "rb");
        }
        if(j == argc - 1) {
            output_file = fopen(argv[j], "wb");
        }
        j++;
    }
    if(task == 1) {
        int width, height, maxvalue;
        char buffer[3], buffer2;
        // citesc P6\n ca sa scap de el
        fread(buffer, sizeof(char), 3, input_file);
        // citesc width, height si valoarea maxima a culorii
        fscanf(input_file, "%d %d\n%d", &width, &height, &maxvalue);
        // trec peste newline 
        fread(&buffer2, sizeof(char), 1, input_file);
        // alocare matrice de pixeli
        Pixel **matrix = malloc(height * sizeof(Pixel*));
        for(i = 0; i < width; i++)
            matrix[i] = malloc(width * sizeof(Pixel));
        // citesc din fisierul binar in matricea de pixeli
        for(i = 0; i < width; i++) 
            for(j = 0; j < height; j++) {
                fread(&matrix[i][j], sizeof(Pixel), 1, input_file);
            } 
        // initializez radacina cu heigth-ul si width-ul dat si formez arborele
        DynamicQuadtree *parent = init_node(height, width);
        create_Quadtree(parent, matrix, height, factor);
        uint32_t number_colors = getNumberLeaves(parent), number_nodes = getNumberNodes(parent);
        // aloc memorie pentru vector
        QuadtreeNode *array = malloc(number_nodes*sizeof(QuadtreeNode));
        // pun in vector arborele cuaternar implementat dinamic
        BFSqueue(parent, array);
        // scriu in fisierul comprimat
        fwrite(&number_colors, sizeof(uint32_t), 1, output_file);
        fwrite(&number_nodes, sizeof(uint32_t), 1, output_file);
        for(i=0; i<number_nodes; i++) {
            fwrite(&array[i].red, sizeof(unsigned char), 1, output_file);
            fwrite(&array[i].green, sizeof(unsigned char), 1, output_file);
            fwrite(&array[i].blue, sizeof(unsigned char), 1, output_file);
            fwrite(&array[i].area, sizeof(uint32_t), 1, output_file);
            fwrite(&array[i].top_left, sizeof(int32_t), 1, output_file);
            fwrite(&array[i].top_right, sizeof(int32_t), 1, output_file);
            fwrite(&array[i].bottom_left, sizeof(int32_t), 1, output_file);
            fwrite(&array[i].bottom_right, sizeof(int32_t), 1, output_file);
        } 
        for(i=0; i<width; i++)
            free(matrix[i]);
        free(matrix);
        free(array);
        destroyTree(&parent);
    }
    if(task==2) {
        uint32_t number_colors, number_nodes;
        unsigned int i=0;
        fread(&number_colors, sizeof(uint32_t), 1, input_file);
        fread(&number_nodes, sizeof(uint32_t), 1, input_file);
        // aloc matricea cu numarul de noduri si o citesc din fisier
        QuadtreeNode *array=malloc(number_nodes*sizeof(QuadtreeNode));
        for(i=0; i<number_nodes; i++) {
            fread(&array[i].red, sizeof(unsigned char), 1,  input_file);
            fread(&array[i].green, sizeof(unsigned char), 1,  input_file);
            fread(&array[i].blue, sizeof(unsigned char), 1,  input_file);
            fread(&array[i].area, sizeof(uint32_t), 1,  input_file);
            fread(&array[i].top_left, sizeof(int32_t), 1,  input_file);
            fread(&array[i].top_right, sizeof(int32_t), 1,  input_file);
            fread(&array[i].bottom_left, sizeof(int32_t), 1,  input_file);
            fread(&array[i].bottom_right, sizeof(int32_t), 1,  input_file); 
        }
        // initializez radacina
        DynamicQuadtree *root = init_node(sqrt(array[0].area), sqrt(array[0].area));
        // creez arbore cuaternar implementat dinamic din vector
        create_quadtree_from_array(root, array, 0);
        unsigned int size = sqrt(array[0].area);
        Pixel **matrix = malloc(sizeof(Pixel*) * size);
        for(i = 0; i < size; i++) {
            matrix[i] = malloc(size * sizeof(Pixel));
        }
        // creez matrice de pixeli dintr-un arbore cuaternar
        if(isLeaf(root)) {
            for(i = 0; i < size; i++)
                for(j = 0; j < size; j++) {
                    matrix[i][j].red = root->pixel.red;
                    matrix[i][j].green = root->pixel.green;
                    matrix[i][j].blue = root->pixel.blue;
                }
        }
        else {
            create_pixels_matrix(root, matrix, size, 0, 0);
        }
        // scriu in fisierul de tip PPM pentru a forma imaginea
        fwrite("P6", sizeof(char), 2, output_file);
        fwrite("\n", sizeof(char), 1, output_file);
        fprintf(output_file, "%d %d", size, size);
        fwrite("\n", sizeof(char), 1, output_file);
        fprintf(output_file, "%d", 255);
        fwrite("\n", sizeof(char), 1, output_file);
        for(i = 0; i < size; i++)
            for(j = 0; j < size; j++) {
                fwrite(&matrix[i][j].blue, sizeof(unsigned char), 1, output_file);
                fwrite(&matrix[i][j].green, sizeof(unsigned char), 1, output_file);
                fwrite(&matrix[i][j].red, sizeof(unsigned char), 1, output_file);
            } 
        for(i = 0; i < size; i++)
            free(matrix[i]);
        free(matrix);
        free(array);
        destroyTree(&root);
    }
    if(task == 3) {
        int width, height, maxvalue;
        char buffer[3],buffer2;
        fread(buffer, sizeof(char), 3, input_file); // sar peste P6\n
        // citesc width, height si valoarea maxima a culorii
        fscanf(input_file, "%d %d\n%d", &width, &height, &maxvalue);
        // trec peste newline 
        fread(&buffer2, sizeof(char), 1, input_file);
        // alocare matrice de pixeli
        Pixel **matrix=malloc(height*sizeof(Pixel*));
        for(i=0; i<width; i++)
            matrix[i]=malloc(width*sizeof(Pixel));
        // citesc din fisierul binar in matricea de pixeli
        for(i = 0; i < width; i++) 
            for(j = 0; j<height; j++) {
                fread(&matrix[i][j], sizeof(Pixel), 1, input_file);
            }
        // initializez radacina cu heigth-ul si width-ul dat si formez arborele
        DynamicQuadtree *parent = init_node(height, width);
        create_Quadtree(parent, matrix, height, factor);
        if(type == 'h') {
            horizontal_flip(parent);
        }
        if(type == 'v') {
            vertical_flip(parent);
        }
        unsigned int size = height;
        if(isLeaf(parent)) {
            for(i = 0; i < size; i++)
                for(j = 0; j < size; j++) {
                    matrix[i][j].red = parent->pixel.red;
                    matrix[i][j].green = parent->pixel.green;
                    matrix[i][j].blue = parent->pixel.blue;
                }
        }
        else {
            create_pixels_matrix(parent, matrix, size, 0, 0);
        }
        fprintf(output_file, "%s", "P6");
        fwrite("\n", sizeof(char), 1, output_file);
        fprintf(output_file, "%d %d", size, size);
        fwrite("\n", sizeof(char), 1, output_file);
        fprintf(output_file, "%d", 255);
        fwrite("\n", sizeof(char), 1, output_file);
        for(i = 0; i < size; i++)
            for(j = 0; j < size; j++) {
                fwrite(&matrix[i][j].blue, sizeof(unsigned char), 1, output_file);
                fwrite(&matrix[i][j].green, sizeof(unsigned char), 1, output_file);
                fwrite(&matrix[i][j].red, sizeof(unsigned char), 1, output_file);
            }
        for(i = 0; i < size; i++)
            free(matrix[i]);
        free(matrix);
        destroyTree(&parent);
    }
    fclose(input_file);
    fclose(output_file);
}
