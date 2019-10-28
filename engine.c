#include <stdio.h>
#include <stdlib.h>
#include "Engine.h"

/*Moteur du jeu*/

int play(int width, int height,int nbrBombs,
         Move (*computeMove)(Grid *), int show)
{
    int WonOrLost = 0; //Détermine si la partie est gagnée ou perdue.
    Grid *mine_field = gridInit(width, height, nbrBombs);
    Move action;

    while (!WonOrLost){ //Tant qu'on a pas perdu ou gagné, on continue!

        if (show)
            gridPrint(mine_field);
        action = computeMove(mine_field);
        if (action.flag == 0){ //On révèle
            if (gridReveal(mine_field, action.x, action.y) == 1)//gridreveal retourne 1 si la case est une bombe => PERDDU => sort de la boucle.
                break;
        }
        else if (action.flag == 1) //Pose un flag
            gridSetFlag(mine_field, action.x, action.y);
        else if(action.flag == -1){ //Abandon
            printf("Thanks for playing with us!");
            return 0;
        }
        else{ //Commande non connue, on recommence la boucle sans afficher la grille.
            printf("Unknown action.\n");
            continue;
        }
        WonOrLost = gridWon(mine_field);

    }
    gridPrint(mine_field); //Quelque soit la valeur de show, on affiche la grille de fin!
    gridFree(mine_field);

    return WonOrLost;
}
