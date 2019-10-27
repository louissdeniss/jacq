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

//Cr�ation d'un type structur�e repr�sentant les cases du tableau
typedef struct Cases_t{
    int value; //nbr bombes adjacentes ou -1 si bombe.
    int flag; //case marqu�e d'un drapeau:1 non:0
    int revealed; //case r�v�l�e:1 non:0
}cases;
//D�finition concr�te du type grid
struct Grid_t {
cases **el;
int width, height, nbrBombs;
int played; // grille pas encore jou�e:0 ,sinon:1
int exploded; // grille a explos�:1, sinon:0
};

Grid *gridInit(int width, int height, int nbrBombs){
    if(width<1 || height<1 || nbrBombs<0){
        printf("Erreur, param�tres non-valables.\n");
        exit(-1);
    }
    // V�rification de la contrainte des bombes; QUESTION: PEUT-ON ALLOUER UN TABLEAU <5  ???
    if( height * width < (min(5,height) * min(5,width) +nbrBombs) ){
        printf("Erreur, la contrainte initiale li�e aux bombes n'est pas respect�e.\n");
        exit(-1);
    }

    Grid *new;
    new = malloc(sizeof(Grid));
    if(!new){
        printf("Erreur d'allocation de m�moire dans GridInit.\n");
        exit(-1);
    }
    new->el = malloc(width * sizeof(cases*));
    if(!new->el){ //allocation dynamique d'une ligne de pointeurs vers des cases
        printf("Erreur d'allocation de m�moire dans GridInit.\n");
        free(new);
        exit(-1);
    }
    int i;
    int j;
    for(i=0;i<width;i++){
        new->el[i]=malloc(height * sizeof(cases));
        if(!new->el[i]){ //allocation dynamique de chaque colonne de cases
            printf("Erreur d'allocation de m�moire dans GridInit.\n");
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
    new->exploded = 0; //MODIF LOUIS: INITIALISATION DU CHAMP EXPLODED


    return new;
}

//Comment impl�menter la programmation d�fensive?? Typeof??
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
    //gestion du cas o� la case � r�v�ler n'appartient pas au tableau
    if(!(verifyAppartenance(grid,x,y))){
        printf("Erreur dans gridReveal: cette case n'appartient pas � la grille.\n");
        return -2;
    }

    //gestion du cas o� la grille est jou�e pour la premi�re fois (grid->played=0) -> on doit initialiser le tableau
    if(!grid->played){
        placeBombs(grid, x, y);

        //bombes plac�es, il faut maintenant calculer les valeurs de chaque case
        int i,j;
        for(i = 0; i < width ;i++){
            for(j = 0; j < height; j++){
                //si la case est une bombe (value=-1), il ne faut pas calculer sa valeur
                if(grid->el[i][j].value + 1){
                    grid->el[i][j].value = calculValue(grid, i, j);
                }
            }
        }
        // la grille est maintenant initialis�e, on peut commencer � r�v�ler des cases
        grid->played = 1;
        return gridReveal(grid, x, y);
    }

    //gestion du cas o� la case � r�v�ler est d�ja r�v�l�e
    if(grid->el[x][y].revealed){
        printf("Erreur dans gridReveal: cette case est d�ja r�v�l�e. \n");
        return -1;
    }

    //R�v�lation de la case
    grid->el[x][y].revealed = 1;

    //pas de drapeau si r�v�l�e

    grid->el[x][y].flag = 0;

    //cas o� la case est une bombe
    if(grid->el[x][y].value == -1){
        grid->exploded = 1; //MODIF LOUIS: INDICATION QUE LA GRILLE EXPLOSE
        return 1;
    }

    //cas o� aucune bombe dans les cases adjacentes
    if(!grid->el[x][y].value){
        //on r�v�le les cases adjacentes, si celles-ci appartiennent � la grille et ne sont pas encore r�v�l�es
        int i,j;
        for(i = x - 1; i <= x + 1; i++){
            for(j = y - 1; j <= y + 1; j++){
                if(verifyAppartenance(grid,i,j) && grid->el[i][j].revealed == 0) //MODIICTAION JULIEN
                    gridReveal(grid, i, j);
            }
        }
    }

    return 0;
}            //DEBUT MODIFICATION !!!!!

//fonction retournant le nombre de bombes adjacentes si la case est r�v�l�e, sinon -1.
int gridValue(Grid *grid, int x, int y){
    //test d'appartenance � la grille
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

//fonction indiquant si une case (x,y) est r�v�l�e ou non
int gridIsExplored(Grid *grid, int x, int y){
    //test d'appartenance � la grille
    if(!verifyAppartenance(grid, x, y)){
        printf("Erreur dans gridIsExplored, la case n'appartient pas au tableau.\n");
        return -2;
    }
    return grid->el[x][y].revealed;
}

//fonction retournant la largeur d'une grille donn�e
int gridWidth(Grid *g){
    return g->width;
}

//fonction retournant la hauteur d'une grille donn�e
int gridHeight(Grid *g){
    return g->height;
}

int gridIsFlagged(Grid *grid, int x, int y){
    //test d'appartenance � la grille
    if(!verifyAppartenance(grid, x, y)){
        printf("Erreur dans gridIsFlagged, la case n'appartient pas au tableau.\n");
        return -2;
    }
    return grid->el[x][y].flag;
}

void gridSetFlag(Grid *grid, int x, int y){
    //test d'appartenance � la grille
    if(!verifyAppartenance(grid, x, y)){
        printf("Erreur dans gridSetFlag, la case n'appartient pas au tableau.\n");
    }
    else if(grid->el[x][y].revealed){ //MODIFICATION JULIEN: Sinon on risque un d�passement de m�moire.
        printf("Erreur dans gridSetFlag, on ne peut pas placer un drapeau sur une case r�v�l�e.\n");
    }
    else{
        grid->el[x][y].flag=1;
    }
}

/* int gridWon(Grid *grid){

}*/   //FIN MODIFICATION !!!!!

//fonction pla�ant les bombes al�atoirement dans la grille
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

        //on s'assure que la case g�n�r�e n'appartienne pas au carr� de longueur 5 centr� en (x,y) et ne soit pas d�j� une bombe
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




//fonction qui v�rifie si la case de cooordonn�es x,y appartient bien � la grille
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
//!DEBUT MODIFICTATIONS!





//Afiche la grille //LOUIS: DEVRAIT-ON PRINT LES AUTRES
void gridPrint(Grid *grid)
{
    int i, j,k;  // MODIF LOUIS: ENLEVER LE UNSIGNED CAR IL GENERAIT DES WARNINGS

    for(j = grid->height-1;j>=0;j--){//Le sens de parcours est un peu sp�cial car non semblable � la r�presentation dans la m�moire.
        printf(" ");                            //Les lignes deviennent colones et vise-versa.
        for(k = 0; k<2*(grid->width)+1; k++)//Affiche '----' � chaque fois qu'on commence une nouvelle ligne.
            printf("-");
        printf("\n%d", j); //Quadrillage
        for (i = 0;i<grid->width;i++){
            printf("|"); //Affiche "|" � chauqe fois qu'on veut afficher un nouveau caract�re.
            if (grid->el[i][j].revealed && grid->el[i][j].value != -1) //Affiche la valeur de la case si celle-ci est r�v�l�e, MODIF LOUIS,sauf si bombe (cas suivant)
                                                           //CETTE MODIF PERMET D AFFICHER L ETAT DES CASES REVELEES AVANT EXPLOSION
                printf("%d", grid->el[i][j].value);
            else if (grid->el[i][j].flag && !grid->exploded)//Affiche un drapeau si cela est demand� et que la grille n'a pas explos�,
                                                            //car dans ce cas il faut afficher 'X'.
                printf("F");
            else if (grid->el[i][j].value == -1 && grid->exploded)//Si la case contient une bombe et que la grille a explos�e.
                printf("X");
            else
                printf(" ");//la grille n'est pas r�v�l�e ou ne contient pas de bombe (dans le cas ou  la grille a explos�e).
        }
        printf("|\n");
    }
    printf(" ");
    for(k = 0; k<2*(grid->width)+1; k++)
            printf("-");
    printf("\n  ");
    for(k = 0; k<grid->width; k++)
            printf("%d ",k);
    printf("\n");

}

//Dit si la partie est gagn�, continue ou est perdue

int gridWon(Grid *grid){

    int i,j, control = 1; //MODIF LOUIS: ENLEVER UNSIGNED,CAR IL GENERE WARNINGS

    for (j=grid->height-1;j>=0;j--){
        for (i=0;i<grid->width;i++){
            if (!grid->el[i][j].revealed && grid->el[i][j].value != -1)//Non-r�v�l�e mais pas de bombe => Pas gagn�
                control = 0;
            else if (grid->el[i][j].revealed && grid->el[i][j].value != -1)//R�v�l�e mais pas de bombe => ok
                continue;
            else if (!grid->el[i][j].revealed && grid->el[i][j].value == -1)//Non-r�v�l�e et Bombe! => OK
                continue;
            else if (grid->el[i][j].revealed && grid->el[i][j].value == -1)//R�v�l�e et Bombe => Perdu
                return -1;
        }
    }
    return control;

}


