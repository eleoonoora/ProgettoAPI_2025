#include <stdio.h>
#include <stdlib.h>

/*
 *	Struttura delle tile: per ogni esagono della mappa vengono salvate le cordinate delle x, delle y, il costo di
 *	transito e una lista concatenata per gestire le rotte aeree.
 *	Nella tile ho deciso di salvare le cordinate cubiche della tessera. dalla matrice map si accede alla tile con
 *	le cordinate con la convenzione dell'utente, queste verranno trasformate in cubiche per i calcoli e le ricerche.
 */

//================= TILE =================//
typedef struct airRoute {
	int xDest;
	int yDest;
	int costAirRoute;
	struct airRoute *next;
} AirRoute;

typedef struct tile {
	int x;
	int y;
	int z;
	int cost;
	int numAirRoute;
	AirRoute *array;
	int routeAdded;
} Tile;

//========================================//

int AirRouteCost(Tile **map, int x, int y);
int Incremento(Tile **map, int x, int y, int v, int raggio, int distanza);
int DistanzaEsagoni (Tile** map, int xa, int ya, int xb, int yb);

int main() {
	char command[64];
	int res = 0, row = 0, col = 0;
	Tile **map = NULL;

	while (res != EOF) {
		res = scanf("%s", command);
		switch (command) {
			case 'init':
				//se la mappa è già presente la si libera
				if (map != NULL) {
					for (int i = 0; i < row; ++i) {
						free(map[i]);
					}
					free(map);
					map = NULL;
				}

				//poi si procede nella lettura dei valori e alla crazione di una nuova mappa
				res = scanf("%d", &col);
				res = scanf("%d", &row);

				map = malloc(row * sizeof(Tile *));
				for (int i = 0; i < row; i++) {
					map[i] = (Tile *) malloc(col * sizeof(Tile));
					for (int j = 0; j < col; j++) {
						map[i][j].x = j - ((i - ( i & 1 )) / 2);
						map[i][j].z = i;
						map[i][j].y = - map[i][j].x - map[i][j].z;
						map[i][j].cost = 1;
						map[i][j].numAirRoute = 0;
						map[i][j].array = NULL;
					}
				}
				printf("OK\n");
				break;
			case 'change_cost':
				int x, y, v, raggio, hColo, val;
				res = scanf("%d" "%d" "%d" "%d", &x, &y, &v, &raggio);
				if (x < row && x >= 0 && y < col && y >= 0 && raggio > 0 && -10 <= v && v >= 10) {
					hColo = 1;
					//loop per le colonne da modificare (asse x)
					for (int i = -raggio; i <= raggio; i++) {
						//loop per le righe da modificare (asse y)
						if ((x+i) >= 0 && (x+i) < col) { //solo se esiste la colonna, risparimo tempo
							for (int j = -hColo; j <= hColo; j++) {
								if ((y+j) >= 0 && (y+j) < row) { // solo se esiste la riga
									val = Incremento(map, x+i, y+j, v, raggio, DistanzaEsagoni(map, x+i, y+j, x, y));
									map[x+i][y+j].cost = val;
									for (int k = 0; k < map[x+i][y+j].numAirRoute; k++) {
										map[x+i][y+j].array[k].costAirRoute = val;
									}
								}
							}
						}
						//gestione incremento altezza delle colonne
						if (i <= (raggio/2)) {
							if (hColo < (raggio - 1)) {
								hColo = hColo + 2;
							}else if (hColo < raggio){
								hColo = hColo + 1;
							}
						}else {
							if (hColo == raggio && raggio % 2 != 0) {
								hColo = hColo - 1;
							}else if (hColo > 0) {
								hColo = hColo - 2;
							}
						}
					}
				}
				else {
					printf("KO\n");
				}
				break;
			case 'toggle_air_route':
				int flag = 0; //flag per la ricerca della rotta da inserire
				int x_start = 0, y_start = 0, x_end = 0, y_end = 0;
				res = scanf("%d" "%d" "%d" "%d", x_start, y_start, &x_end, &y_end);

				if (x_start <= row && x_end <= row && y_end <= col && y_end <= col) {
					AirRoute *airRouteHead = map[x_start][y_start].array;
					AirRoute *airRoutePrev = NULL;

					while (airRouteHead != NULL || flag == 1) {
						if (airRouteHead->xDest == x_end && airRouteHead->yDest == y_end) {
							flag = 1;
						} else {
							airRoutePrev = airRouteHead;
							airRouteHead = airRouteHead->next;
						}
					}

					//se lo trova, lo rimuove
					if (flag == 1 && airRoutePrev != NULL) {
						airRoutePrev->next = airRouteHead->next;
						map[x_start][y_start].numAirRoute--;
						printf("OK\n");
					} else if (flag == 1 && airRoutePrev == NULL) {
						map[x_start][y_start].array = airRouteHead->next;
						map[x_start][y_start].numAirRoute--;
						printf("OK\n");
					}
					//altrimenti lo aggiunge solo se ce ne sono meno di 5
					else if (map[x_start][y_start].numAirRoute < 5) {
						AirRoute *airRouteNew = malloc(sizeof(AirRoute));
						airRouteNew->xDest = x_end;
						airRouteNew->yDest = y_end;
						airRouteNew->costAirRoute = AirRouteCost(map, x_start, y_start);
						map[x_start][y_start].numAirRoute++;
						airRouteNew->next = map[x_start][y_start].array;
						map[x_start][y_start].array = airRouteNew;
						printf("OK\n");
					} else {
						printf("KO\n");
					}

					free(airRoutePrev);
					free(airRouteHead);
				} else {
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

int AirRouteCost(Tile **map, int x, int y) {
	int sum = 0;
	for (int i = 0; i < map[x][y].numAirRoute; i++) {
		sum = sum + map[x][y].array[i].costAirRoute;
	}
	sum = (sum + map[x][y].cost) / (map[x][y].numAirRoute + 1);
	return sum;
}

int Incremento(Tile **map, int x, int y, int v, int raggio, int distanza) {
	int costo, temp;

	temp = (raggio - distanza)/raggio;
	costo = map[x][y].cost;

	if (temp > 0) {
		costo = costo + (v * temp);
	}

	return costo;
}

int DistanzaEsagoni (Tile** map, int xa, int ya, int xb, int yb) {
	//visto che uso le cordinate cubiche la formula è veloce, ovvero il massimo tra le differenze dei 3 assi
	int dx, dy, dz, max;
	dx = abs(map[xa][ya].x - map[xb][yb].x), dy = abs(map[xa][ya].y - map[xb][yb].y), dz = abs(map[xa][ya].z - map[xb][yb].z);

	max = dx;
	if (dy > max) {
		max = dy;
	}
	if (dz > max) {
		max = dz;
	}

	return max;
}