#include <stdio.h>
#include <stdlib.h>
#include "Solver.h"
#include "Grid.h"
#include "Engine.h"

static int verifyAppartenance(Grid *grid, int x, int y);
static int countFlags(Grid *grid, int x, int y);
static int countRevealed(Grid *grid, int x, int y);
static int countAdjacent(Grid *grid, int x, int y);

Move human(Grid *grid){
    
    Move action;

    printf("X: ");
    scanf("%d", &(action.x));
    //QUESTION:ENLEVER LE BUG QUAND ON ENTRE UNE LETTRE?

    printf("Y: ");
    scanf("%d", &(action.y));

    printf("Action: ");
    scanf("%d", &(action.flag));

    //gestion du cas où les coordonnées entrées n'appartiennent pas au tableau
    if(!verifyAppartenance(grid, action.x, action.y)){
        printf("Veuillez entrer des coordonnées valables. \n");
        return human(grid);
    }

    //gestion du cas où on veut révéler une case qui l'est déjà ou lui assigner un drapeau
    if(gridIsExplored(grid, action.x, action.y) && action.flag != -1){
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

Move heuristic(Grid *grid){
    int width = gridWidth(grid);
    int height = gridHeight(grid);

    Move action;

    //on vérifie si une case a déjà été révélée, si oui revealed vaudra 1 à la fin de la boucle
    int i,j;
    int revealed=0;
    for(i = 0; i < width; i++){
        for(j = 0;j < height; j++){
            if(gridIsExplored(grid,i,j)){
                revealed = 1;
                break;
            }
        }
        if(revealed)
            break;
    }

    //dans le cas où aucune case n'est révélée, on révèle une case au-hasard
    if(!revealed){
        i=rand() % width;
        j=rand() % height;
        action.x = i;
        action.y = j;
        action.flag = 0;
        return action;
    }

    int revealedAdj; //nombre de cases révélées adjacentes
    int flagAdj; //nombre de cases marquées adjacentes
    int adjacent; //nombre de cases ajdacentes
    int value; //valeur de le case
    int stopboucle = 0; //quand il vaudra 1, on aura trouvé une instruction jouable et on sortira de la boucle.
    //on parcourt le tableau à la recherche de cases révélées.
    int newx,newy; //on stockera la case où on peut éffectuer une action, si on en trouve une
    for(i = 0; i < width; i++){

        for(j = 0;j < height; j++){

            if(gridIsExplored(grid,i,j)){
                
                //on se trouve sur une case révélée
                revealedAdj = countRevealed(grid, i, j);
                adjacent = countAdjacent(grid, i, j);
                flagAdj = countFlags(grid, i, j);

                if(revealedAdj + flagAdj == adjacent)
                    continue;  //OK?        //si toutes les adjacentes sont révélées ou marquées, on passe

                value = gridValue(grid, i, j);

                
                //si une case révélée est de valeur = nbr de flags adjacents, on révèle une case adjacente
                //sauf si le nombre de cases marquées d'un drapeau est justement égal au nombre de case non-révélées
                if(value == flagAdj && flagAdj + revealedAdj != adjacent){
                    action.flag = 0;
                    newx = i;
                    newy = j;
                    stopboucle = 1; //on a trouvé un coup a joué, on arrête de parcourir le tableau
                }
                
                //si une case révélée est de valeur = nbr flags adjacents + cases non-révélées ajdacentes
                //cases non révélées = cases adjacentes -cases révélées -cases flag
                //--> flags adj + non-revel = flagAdj + adjacent - revealedAdj -flagAdj = adjacent -revealedAdj
                //alors on flag une case non-révélée
                if(value == adjacent - revealedAdj){
                    action.flag = 1;
                    newx = i;
                    newy = j;
                    stopboucle = 1;
                }
            }

            if(stopboucle)
                break;

        }

        if(stopboucle)
            break;

    }

    //on a arrêté de parcourir le tableau: 3cas

    //on a trouvé une case qu'on pouvait révéler
    if(action.flag == 0){
        // a nous de trouver une case non-révélée adjacente
        for(i = newx - 1; i <= newx + 1; i++){

            for(j = newy - 1; j <= newy + 1; j++){
                if(verifyAppartenance(grid, i, j) && !gridIsExplored(grid, i, j) && !gridIsFlagged(grid, i, j)){
                    action.x = i;
                    action.y = j;
                    return action;
                }
            }
        }
    }

    //on a trouvé une case qu'on pouvait marquer d'un drapeau
    if(action.flag == 1){
        // a nous de trouver une case non-révélée adjacente
        for(i = newx - 1; i <= newx + 1; i++){
            for(j = newy - 1; j <= newy + 1; j++){
                if(verifyAppartenance(grid, i, j) && !gridIsExplored(grid, i, j) && !gridIsFlagged(grid, i, j)){
                    action.x = i;
                    action.y = j;
                    return action;
                }
            }
        }
    }

    //on a parcouru tout le tableau et trouvé aucun coup jouable
    action.x = 0;
    action.y = 0;
    action.flag = -1;
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

//fonction qui compte le nombre de cases adjacentes à (x,y) marquées
static int countFlags(Grid *grid, int x, int y){
    //PEUT-ON ENLEVER?
    if(!verifyAppartenance(grid, x, y)){
        printf("Erreur dans countFlags, coordonnées erronées. \n");
        return -1;
    }
    int i,j;
    int Flags = 0;

    //cette fonction ne sera appelée que sur une case révélée, donc pas flaggée, donc pas besoin 
    //d'exclure la case en question de la boucle
    for(i = x - 1; i <= x + 1; i++){

        for(j = y - 1; j <= y + 1; j++){

            if(verifyAppartenance(grid, i, j) && gridIsFlagged(grid, i, j))
                Flags++;

        }

    }
    return Flags;
}

//fonction calculant le nombre de cases adjacentes révélées
static int countRevealed(Grid *grid, int x, int y){
    //PEUT-ON ENLEVER?
    if(!verifyAppartenance(grid, x, y)){
        printf("Erreur dans countRevealed, coordonnées erronées. \n");
        return -1;
    }
    int i,j;
    int RevealedCases = 0;

    //on parcourt les cases ajdacentes ET la case (x,y) -> on s'en occupe à la sortie de boucle
    for(i = x - 1; i <= x + 1; i++){

        for(j = y - 1; j <= y + 1; j++){

            if(verifyAppartenance(grid, i, j) && gridIsExplored(grid, i, j))
                RevealedCases++;

        }

    }

    //si la case (x,y) a été révélée, on doit l'enlever du nombre de cases adjacentes révélées
    //car la boucle est passée dessus
    if(gridIsExplored(grid,x,y))
        RevealedCases--;

    return RevealedCases;
}

//fonction qui compte le nombre de cases adjacentes
static int countAdjacent(Grid *grid, int x, int y){
    //PEUT-ON ENLEVER?
    if(!verifyAppartenance(grid, x, y)){
        printf("Erreur dans countAdjacent, coordonnées erronées. \n");
        return -1;
    }

    int i,j;
    int Adj=-1; //nombre de cases adjacentes, on retire direct la case (x,y) qui sera comptée dans la boucle
    for(i = x - 1; i <= x + 1; i++){
        for(j = y - 1; j <= y + 1; j++){
            if(verifyAppartenance(grid, i, j))
                Adj++;
        }
    }

    return Adj;
}
