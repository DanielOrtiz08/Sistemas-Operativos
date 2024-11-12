#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<wait.h>
#include<stdbool.h>

struct buffer{
    int id;
    int posy;
    int posx;
};

void read_file(char ***map, int *rows, int *columns, char *file_name){
    FILE *file = fopen(file_name, "r");
    
    fscanf(file, "%d", rows);
    fscanf(file, "%d", columns);
    printf("%d\n", *rows);
    printf("%d\n", *columns);

    //Creacion de matrix dinamica
    (*map) = (char **)malloc(sizeof(char *) * (*rows));
    for(int i=0; i< (*rows); i++){
        (*map)[i] = (char *)malloc(sizeof(char)* (*columns));
    }
    char character;
    for(int i=0; i < (*rows); i++){
        for(int j=0; j < (*columns); j++){
            fscanf(file, "%c ", &character);
            (*map)[i][j] = character;
        }
    }

    for(int i=0; i < (*rows); i++){
        for(int j=0; j < (*columns); j++){
            printf("%c ",(*map)[i][j]);
        }
        printf("\n");
    }
    fclose(file);
}



void get_matrix_range(int id, int num_filas, int num_columnas, int *start_row, int *end_row, int *start_col, int *end_col) {
    // Calcular las filas para el proceso
    if (id < 2) {
        *start_row = 0;
        *end_row = num_filas / 2;
    } else {
        *start_row = num_filas / 2;
        *end_row = num_filas;
    }

    // Calcular las columnas para el proceso
    if (id % 2 == 0) {
        *start_col = 0;
        *end_col = num_columnas / 2;
    } else {
        *start_col = num_columnas / 2;
        *end_col = num_columnas;
    }
}

bool is_nest(char **map, int start_rows, int end_rows, int start_columns, int end_columns, int posy, int posx){
    if(map[posy][posx] != '1'){
        return false;
    }

    for(int i = posy-1; i<=posy+1; i++){
        for(int j= posx-1; j<=posx+1; j++){
            if( (i < start_rows || i > end_rows ) || (j < start_columns || j > end_columns ) || (i==posy && j==posx) ){
                continue;
            }
            if(map[i][j] == '2'){
                return true;
            }
        }
    }
    return false;

}

int main(){
    char **map = NULL;
    char *file_name = "nidos.dat";
    int rows, columns;
    read_file(&map, &rows, &columns, file_name);

    pid_t dad = getpid();

    int number_sons = 4;
    int i;

    int file_descriptor[2];
    pipe(file_descriptor);

    for(i=0; i<number_sons; i++){
        if(!fork()){ close(file_descriptor[0]); break;}
    }

    if(dad == getpid()){
        close(file_descriptor[1]);
        struct buffer customs;

        while(read(file_descriptor[0], &customs, sizeof(struct buffer)) > 0){
            printf("Nests in(%d, %d), by: %d\n", customs.posy, customs.posx, customs.id);
        }


        //Se cierra lectura
        close(file_descriptor[0]);
        for(int j=0; j<number_sons; j++) wait(NULL);

    }else{
        struct buffer mailman;
        int start_rows, end_rows;
        int start_columns, end_columns;
        get_matrix_range(i, rows, columns, &start_rows, &end_rows, &start_columns, &end_columns);

        for(int y = start_rows; y<end_rows; y++ ){
            for(int x = start_columns; x<end_columns; x++){
                if(map[y][x] == '1'){
                    if(is_nest(map, start_rows, end_rows, start_columns, end_columns, y, x)){
                        mailman.id = i;
                        mailman.posx = x;
                        mailman.posy = y;
                        write(file_descriptor[1], &mailman, sizeof(struct buffer));
                    }
                }
            }
        }
        close(file_descriptor[1]);
        

    }

    return EXIT_SUCCESS;
}