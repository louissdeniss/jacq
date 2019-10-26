#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "Grid.h"

#define min(a,b) (a<=b?a:b)
#define max(a,b) (a<=b?b:a)

static void placeBombs(Grid *grid, int x, int y);
static int calculValue(Grid *grid, int x, int y);
static int verifyAppartenance(Grid *grid, int x, int y);

//Création d'un type structurée représentant les cases du tableau
typedef struct{
    int value; //nbr bombes adjacentes ou -1 si bombe.
    int flag; //case marquée d'un drapeau:1 non:0
    int revealed; //case révélée:1 non:0
} cases;
//Définition concrète du type grid
struct Grid_t {
cases **el;
int width, height, nbrBombs;
int played; // grille pas encore jouée:0 ,sinon:1
int exploded; // grille a explosé:1, sinon:0
};

Grid *gridInit(int width, int height, int nbrBombs){
    if(width<1 || height<1 || nbrBombs<0){
        printf("Erreur, paramètres non-valables.\n");
        return NULL;
    }
    // Vérification de la contrainte des bombes; QUESTION: PEUT-ON ALLOUER UN TABLEAU <5  ???
    if( height * width < (min(5,height) * min(5,width) +nbrBombs) ){
        printf("Erreur, la contrainte initiale liée aux bombes n'est pas respectée.\n");
        return NULL;
    }
    
    Grid *new;
    new = malloc(sizeof(Grid));
    if(!new){
        printf("Erreur d'allocation de mémoire dans GridInit.\n");
        return NULL;
    }
    new->el = malloc(width * sizeof(cases*));
    if(!new->el){ //allocation dynamique d'une ligne de pointeurs vers des cases
        printf("Erreur d'allocation de mémoire dans GridInit.\n");
        free(new);
        return NULL;
    } 
    int i;
    int j;
    for(i=0;i<width;i++){
        new->el[i]=malloc(height * sizeof(cases));
        if(!new->el[i]){ //allocation dynamique de chaque colonne de cases
            printf("Erreur d'allocation de mémoire dans GridInit.\n");
            for(j=0;j<i;j++)
                free(new->el[j]);
            free(new);
            return NULL;
        }  
        // initialisation des cases du tableau
        for(j=0;j<height;j++){
            new->el[i][j].value=0;
            new->el[i][j].flag=0;
            new->el[i][j].revealed=0;
        }
    }

    new->width = width;
    new->height = height;
    new->nbrBombs = nbrBombs;
    new->played = 0;

    return new;
}

//Comment implémenter la programmation défensive?? Typeof??
void gridFree(Grid *grid){
    int i;
    int width=grid->width;

    for(i=0;i<width;i++)
        free(grid->el[i]);

    free(grid->el);

    free(grid);
}

/* void gridPrint(Grid *grid){
    
} */

int gridReveal(Grid *grid, int x, int y){
    int width = grid->width;
    int height = grid->height;
    //gestion du cas où la case à révéler n'appartient pas au tableau
    if(!(verifyAppartenance(grid,x,y))){
        printf("Erreur dans gridReveal: cette case n'appartient pas à la grille.\n");
        return -1;
    }

    //gestion du cas où la grille est jouée pour la première fois (grid->played=0) -> on doit initialiser le tableau
    if(!grid->played){
        placeBombs(grid, x, y);
        
        //bombes placées, il faut maintenant calculer les valeurs de chaque case
        int i,j;
        for(i = 0; i < width ;i++){
            for(j = 0; j < height; j++){
                //si la case est une bombe (value=-1), il ne faut pas calculer sa valeur
                if(grid->el[i][j].value + 1){
                    grid->el[i][j].value = calculValue(grid, i, j);
                }
            }
        }
        // la grille est maintenant initialisée, on peut commencer à révéler des cases
        grid->played = 1;
        return gridReveal(grid, x, y);
    }

    //gestion du cas où la case à révéler est déja révélée
    if(grid->el[x][y].revealed){
        printf("Erreur dans gridReveal: cette case est déja révélée. \n");
        return -1;
    }
    
    //Révélation de la case
    grid->el[x][y].revealed = 1;

    //cas où la case est une bombe
    if(grid->el[x][y].value == -1)
        return 1;
    
    //cas où aucune bombe dans les cases adjacentes
    if(!grid->el[x][y].value){
        //on révèle les cases adjacentes, si celles-ci appartiennent à la grille et ne sont pas encore révélées
        int i,j;
        for(i = x - 1; i <= x + 1; i++){
            for(j = y - 1; j <= y + 1; j++){
                if(verifyAppartenance(grid,i,j) && grid->el[x][y].revealed == 0)
                    gridReveal(grid, i, j);
            }
        }
    }

    return 0;
}

//fonction plaçant les bombes aléatoirement dans la grille
static void placeBombs(Grid *grid, int x, int y){  
    int nbrBombs = grid->nbrBombs;

    int placedBombs = 0;
    int newx,newy;
    int width = grid->width;
    int height = grid->height;

    //on place les bombes une par une
    while(placedBombs < nbrBombs){
        newx=rand() % width;
        newy=rand() % height;

        //on s'assure que la case générée n'appartienne pas au carré de longueur 5 centré en (x,y) et ne soit pas déjà une bombe
        if((!(newx >= x - 2 && newx <= x + 2 && newy >= y - 2 && newy <= y + 2)) && (grid->el[newx][newy].value + 1)){
            grid->el[newx][newy].value = -1;
            placedBombs++;
        }
    }
}

//fonction qui calcule les valeurs (le nombre de bombes adjacentes) de chaque case du tableau
static int calculValue(Grid *grid, int x, int y){
    // on va parcourir les cases adjacentes, on suppose que la case x,y appartient au tableau et n'est pas une bombe
    int i,j;
    int Value = 0;
    for(i = x - 1; i <= x + 1; i++){
        for(j = y - 1; j <= y + 1; j++){
            //on teste si la case qu'on parcourt appartient bien au tableau
            if(verifyAppartenance(grid,i,j)){
                //on teste si la case parcoure est une bombe
                if(grid->el[i][j].value == -1){
                    Value++;
                }
            }
        }
    }
    return Value;
}

//fonction qui vérifie si la case de cooordonnées x,y appartient bien à la grille
static int verifyAppartenance(Grid *grid, int x, int y){
    int width=grid->width;
    int height=grid->height;
    if(x >= 0 && x < width && y >= 0 && y < height)
        return 1;
    else
    {
        return 0;
    }
    
}