#include <stdio.h>
#include <stdlib.h>
#include "solver.h"

static int verifyAppartenance(Grid *grid, int x, int y);

Move human(Grid *grid){
    Move action;

    printf("X: ");
    scanf("%d", &(action.x));

    printf("Y: ");
    scanf("%d", &(action.y));

    printf("Action: ");
    scanf("%d", &(action.flag));
    
    //MODIF LOUIS
    //gestion du cas où les coordonnées entrées n'appartiennent pas au tableau
    if(!verifyAppartenance(grid, action.x, action.y)){
        printf("Veuillez entrer des coordonnées valables. \n");
        return human(grid);
    }

    //gestion du cas où on veut révéler une case qui l'est déjà
    if(gridIsExplored(grid, action.x, action.y) && !action.flag){
        printf("Cette case a déjà été révélée! \n");
        return human(grid);
    }

    //gestion du cas où l'action demandée n'existe pas
    if(action.flag < -1 || action.flag > 1){
        printf("Veuillez entrer une action existante. \n");
        return human(grid);
    }

    return action;
}

//fonction qui vérifie si la case de cooordonnées x,y appartient bien à la grille
static int verifyAppartenance(Grid *grid, int x, int y){
    int width=gridWidth(grid);
    int height=gridHeight(grid);
    if(x >= 0 && x < width && y >= 0 && y < height)
        return 1;
    else
    {
        return 0;
    }
    
}
