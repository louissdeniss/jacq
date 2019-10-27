#include <stdio.h>
#include <stdlib.h>
#include "solver.h"

Move human(Grid *grid){
    Move action;

    printf("X: ");
    scanf("%d", &(action.x));

    printf("Y: ");
    scanf("%d", &(action.y));

    printf("Action: ");
    scanf("%d", &(action.flag));

    return action;
}
