#include <stdio.h>
#include <stdlib.h>

/*
 *	Struttura delle tile: per ogni esagono della mappa vengono salvate le cordinate delle x, delle y, il costo di
 *	transito e una lista concatenata per gestire le rotte aeree.
 */

//================= TILE =================//
typedef struct airRoute {
	int xDest;
	int yDest;
	int costAirRoute;
	struct airRoute* next;
}AirRoute;

typedef struct tile {
	int x;
	int y;
	int cost;
	int numAirRoute;
	AirRoute* array;
	int routeAdded;
}Tile;
//========================================//

int AirRouteCost (Tile** map, int x, int y);

int main(){
	char command[64];
	int res = 0, row = 0, col = 0;
	Tile** map = NULL;

	while (res != EOF) {
		res = scanf("%s", command);
		switch (command) {
			case 'init':
				res = scanf("%d", &col);
				res = scanf("%d", &row);
				map = malloc(row * sizeof(Tile*));
				for (int i = 0; i < row; i++) {
					map[i] = (Tile*) malloc(col * sizeof(Tile));
					for (int j = 0; j < col; j++) {
						map[i][j].x = i;
						map[i][j].y = j;
						map[i][j].cost = 1;
					}
				}
				printf("OK\n");
				break;
			case 'change_cost':
				break;
			case 'toggle_air_route':
				int flag = 0; //flag per la ricerca della rotta da inserire
				int x_start = 0, y_start = 0, x_end = 0, y_end = 0;
				res = scanf("%d" "%d" "%d" "%d" , x_start, y_start, &x_end, &y_end);

				if (x_start <= row && x_end <= row && y_end <= col && y_end <= col) {
					AirRoute* airRouteHead = map[x_start][y_start].array;
					AirRoute* airRoutePrev = NULL;

					while (airRouteHead != NULL || flag == 1) {
						if (airRouteHead->xDest == x_end && airRouteHead->yDest == y_end) {
							flag = 1;
						}
						else {
							airRoutePrev = airRouteHead;
							airRouteHead = airRouteHead->next;
						}
					}

					//se lo trova, lo rimuove
					if (flag == 1 && airRoutePrev != NULL) {
						airRoutePrev->next = airRouteHead->next;
						map[x_start][y_start].numAirRoute --;
						printf("OK\n");
					}
					else if (flag == 1 && airRoutePrev == NULL) {
						map[x_start][y_start].array = airRouteHead->next;
						map[x_start][y_start].numAirRoute --;
						printf("OK\n");
					}
					//altrimenti lo aggiunge solo se ce ne sono meno di 5
					else if (map[x_start][y_start].numAirRoute < 5) {
						AirRoute* airRouteNew = malloc(sizeof (AirRoute));
						airRouteNew->xDest = x_end;
						airRouteNew->yDest = y_end;
						airRouteNew->costAirRoute = AirRouteCost(map, x_start, y_start);
						airRouteNew->next = map[x_start][y_start].array;
						map[x_start][y_start].array = airRouteNew;
						printf("OK\n");
					}
					else {
						printf("KO\n");
					}

					free (airRoutePrev);
					free(airRouteHead);
				}
				else {
					printf("KO\n");
				}

				break;
			case 'travel_cost':
				break;
			default:
				printf("Wrong command.\n");
		}
	}
	free(map);
}

int AirRouteCost (Tile** map, int x, int y) {
	int sum = 0;
	for (int i = 0; i < map[x][y].numAirRoute; i++) {
		sum = sum + map[x][y].array[i].costAirRoute;
	}
	sum = (sum + map[x][y].cost)/( map[x][y].numAirRoute +1);
	return sum;
}