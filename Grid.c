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
        exit(-1);
    }
    // Vérification de la contrainte des bombes; QUESTION: PEUT-ON ALLOUER UN TABLEAU <5  ???
    if( height * width < (min(5,height) * min(5,width) +nbrBombs) ){
        printf("Erreur, la contrainte initiale liée aux bombes n'est pas respectée.\n");
        exit(-1);
    }
    
    Grid *new;
    new = malloc(sizeof(Grid));
    if(!new){
        printf("Erreur d'allocation de mémoire dans GridInit.\n");
        exit(-1);
    }
    new->el = malloc(width * sizeof(cases*));
    if(!new->el){ //allocation dynamique d'une ligne de pointeurs vers des cases
        printf("Erreur d'allocation de mémoire dans GridInit.\n");
        free(new);
        exit(-1);
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
            exit(-1);
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
    new->exploded = 0;

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

//Afiche la grille
//Afiche la grille
void gridPrint(Grid *grid)
{
    int i, j,k;

    for(j = grid->height-1;j>=0;j--){//Le sens de parcours est un peu spécial car non semblable à la répresentation dans la mémoire.
                                     //Les lignes deviennent colones et vise-versa.
        for(k = 0; k<2*(grid->width)+1; k++)//Affiche '----' à chaque fois qu'on commence une nouvelle ligne.
            printf("-");
        printf("\n");
        for (i = 0;i<grid->width;i++){
            printf("|"); //Affiche "|" à chauqe fois qu'on veut afficher un nouveau caractère.
            if (grid->el[i][j].revealed && grid->el[i][j].value != -1) //Affiche la valeur de la case si celle-ci est révélée et que la grille n'a pas explosée.
                                                            //La case n'est donc pas une bombe.
                printf("%d", grid->el[i][j].value);
            else if (grid->el[i][j].flag && !grid->exploded)//Affiche un drapeau si cela est demandé et que la grille n'a pas explosé,
                                                            //car dans ce cas il faut afficher 'X'.
                printf("F");
            else if (grid->el[i][j].value == -1 && grid->exploded)//Si la case contient une bombe et que la grille a explosée.
                printf("X");
            else
                printf(" ");//la grille n'est pas révélée ou ne contient pas de bombe (dans le cas ou  la grille a explosée).
        }
        printf("|\n");
    }
    for(k = 0; k<2*(grid->width)+1; k++)
            printf("-");
    printf("\n");

}

int gridReveal(Grid *grid, int x, int y){
    int width = grid->width;
    int height = grid->height;
    //gestion du cas où la case à révéler n'appartient pas au tableau
    if(x < 0  ||  x >= width  ||  y < 0  ||  y >= height){
        printf("Erreur dans gridReveal: cette case n'appartient pas à la grille.\n");
       exit(-1); //MODIF LOUIS
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
        //MODIF LOUIS: ENLEVER -1
    }
    
    //Révélation de la case
    grid->el[x][y].revealed = 1;

    //pas de drapeau si révélé
    grid->el[x][y].flag = 0;

    //cas où la case est une bombe
    if(grid->el[x][y].value == -1){
        grid->exploded = 1;
        return 1;
    }
    
    //cas où aucune bombe dans les cases adjacentes
    if(!grid->el[x][y].value){
        //on révèle les cases adjacentes, si celles-ci appartiennent à la grille et ne sont pas encore révélées
        int i,j;
        for(i = x - 1; i <= x + 1; i++){
            for(j = y - 1; j <= y + 1; j++){
                if(verifyAppartenance(grid,i,j) && grid->el[i][j].revealed == 0)
                    gridReveal(grid, i, j);
            }
        }
    }

    return 0;
}

//fonction retournant le nombre de bombes adjacentes si la case est révélée, sinon -1.
int gridValue(Grid *grid, int x, int y){
    //test d'appartenance à la grille
    if(!verifyAppartenance(grid, x, y)){
        printf("Erreur dans gridValue, la case n'appartient pas au tableau.\n");
        return -2;
    }
    if(grid->el[x][y].revealed)
        return grid->el[x][y].value; //value=-1, ssi la case contient une bombe, ce qui n'est pas possible puisqu'on n'appellera pas gridValue si bombe il y a
    else
    {
        return -1;
    }
}

//fonction indiquant si une case (x,y) est révélée ou non
int gridIsExplored(Grid *grid, int x, int y){
    //test d'appartenance à la grille
    if(!verifyAppartenance(grid, x, y)){
        printf("Erreur dans gridIsExplored, la case n'appartient pas au tableau.\n");
        exit(-1);
    }
    return grid->el[x][y].revealed;
}

//fonction retournant la largeur d'une grille donnée
int gridWidth(Grid *g){
    return g->width;
}

//fonction retournant la hauteur d'une grille donnée
int gridHeight(Grid *g){
    return g->height;
}

int gridIsFlagged(Grid *grid, int x, int y){
    //test d'appartenance à la grille
    if(!verifyAppartenance(grid, x, y)){
        printf("Erreur dans gridIsFlagged, la case n'appartient pas au tableau.\n");
        return -2;
    }
    return grid->el[x][y].flag;
}

void gridSetFlag(Grid *grid, int x, int y){
    //test d'appartenance à la grille
    if(!verifyAppartenance(grid, x, y)){
        printf("Erreur dans gridSetFlag, la case n'appartient pas au tableau.\n");
    }
    if(grid->el[x][y].revealed){
        printf("Erreur dans gridSetFlag, on ne peut placer un drapeau sur une case révélée.\n");
    }
    else{
        grid->el[x][y].flag=1;
    }
}

//fonction qui indique une victoire, une défaite ou ni l'un, ni l'autre
int gridWon(Grid *grid){

    int i,j, control = 1;

    for (j=grid->height-1;j>=0;j--){
        for (i=0;i<grid->width;i++){
            if (!grid->el[i][j].revealed && grid->el[i][j].value != -1)//Non-révélée mais pas de bombe => Pas gagné
                control = 0;
            else if (grid->el[i][j].revealed && grid->el[i][j].value != -1)//Révélée mais pas de bombe => ok
                continue;
            else if (!grid->el[i][j].revealed && grid->el[i][j].value == -1)//Non-révélée et Bombe! => OK
                continue;
            else if (grid->el[i][j].revealed && grid->el[i][j].value == -1)//Révélée et Bombe => Perdu
                return -1;
        }
    }
    return control;

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
