#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INF 10000

int counter = 0;

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
	//int routeAdded;
} Tile;

//========================================//

//================= HEAP =================//
typedef struct node {
	int distanza;
	int indice;
} Node;

typedef struct queue {
	int size;
	int *posizione;
	Node *minHeap;
} Queue;
//========================================//

int AirRouteCost(Tile **map, int col, int row);
int Incremento(Tile **map, int col, int row, int v, int raggio, int distanza);
int DistanzaEsagoni (Tile** map, int cola, int rowa, int colb, int rowb);
Node ExtractMin (Queue *Q, int *distanza);
void MinHeapify (Queue *Q, int i, int *distanza);
int Left(int i);
int Right(int i);
void Swap (Queue *Q, int i, int j);
void DecreaseKey (Queue *Q, int i, int key);
int Parent(int i);
void HeapInsert (Queue *Q, int id, int key);
void DijkstraShortestPath(Tile **G, int idp, int idd, int col, int row);

//! le matrici sono map[row][col]

int main() {
	char command[64];
	int res = 0, row = 0, col = 0;
	Tile **map = NULL;

	while ((res = scanf("%s", command)) != EOF) {
		counter++;
		if (strcmp(command, "init") == 0) {
			//se la mappa è già presente la si libera
			if (map != NULL) { //! non inizializza bene dopo il primo init
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
		}
	else if (strcmp(command, "change_cost") == 0) {
		int colStart, rowStart, v, raggio, hColo, val;
		res = scanf("%d" "%d" "%d" "%d", &colStart, &rowStart, &v, &raggio);
		if (rowStart < row && rowStart >= 0 && colStart < col && colStart >= 0 && raggio > 1 && -10 <= v && v <= 10) {
			hColo = 1;
			//loop per le colonne da modificare (asse x)
			for (int i = -raggio + 1; i <= raggio - 1; i++) {
				//loop per le righe da modificare (asse y)
				if ((colStart+i) >= 0 && (colStart+i) < col) { //solo se esiste la colonna, risparimo tempo
					for (int j = -hColo; j <= hColo; j++) {
						if ((rowStart+j) >= 0 && (rowStart+j) < row) { // solo se esiste la riga
							val = Incremento(map, colStart+i, rowStart+j, v, raggio, DistanzaEsagoni(map, colStart, rowStart, colStart+i, rowStart+j));
							if (val > 0 && val <= 100) { //lo aggiorna solo se il nuovo costo (finale) è tra 0 e 100
								map[rowStart+j][colStart+i].cost = val;
								for (int k = 0; k < map[rowStart+j][colStart+i].numAirRoute; k++) {
									map[rowStart+j][colStart+i].array[k].costAirRoute = val;
								}
							}else {
								if (val <= 0) { // se è negativo lo poni a zero
									map[rowStart+j][colStart+i].cost = 0;
									for (int k = 0; k <map[rowStart+j][colStart+i].numAirRoute; k++) {
										map[rowStart+j][colStart+i].array[k].costAirRoute = 0;
									}
								}else {
									map[rowStart+j][colStart+i].cost = 100;
									for (int k = 0; k < map[rowStart+j][colStart+i].numAirRoute; k++) {
										map[rowStart+j][colStart+i].array[k].costAirRoute = 100;
									}
								}
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
			printf("OK\n");
		}
		else if (rowStart < row && rowStart >= 0 && colStart < col && colStart >= 0 && raggio == 1 && -10 <= v && v <= 10) {
			map[rowStart][colStart].cost += v;
			printf("OK\n");
		}
		else {
			printf("KO\n");
		}
	}
	else if (strcmp(command, "toggle_air_route") == 0) {
		int flag = 0; //flag per la ricerca della rotta da inserire
		int col_start = 0, row_start = 0, col_end = 0, row_end = 0;
		res = scanf("%d" "%d" "%d" "%d", &col_start, &row_start, &col_end, &row_end);

		if (col_start < col && col_end < col && row_start < row && row_end < row) {
			AirRoute *airRouteHead = map[row_start][col_start].array;
			AirRoute *airRoutePrev = NULL;

			while (airRouteHead != NULL && flag != 1) {
				if (airRouteHead->xDest == col_end && airRouteHead->yDest == row_end) {
					flag = 1;
				} else {
					airRoutePrev = airRouteHead;
					airRouteHead = airRouteHead->next;
				}
			}

			//se lo trova, lo rimuove
			if (flag == 1 && airRoutePrev != NULL) {
				airRoutePrev->next = airRouteHead->next;
				map[row_start][col_start].numAirRoute--;
				printf("OK\n");
			} else if (flag == 1 && airRoutePrev == NULL) {
				map[row_start][col_start].array = airRouteHead->next;
				map[row_start][col_start].numAirRoute--;
				printf("OK\n");
			}
			//altrimenti lo aggiunge solo se ce ne sono meno di 5
			else if (map[row_start][col_start].numAirRoute < 5) {
				AirRoute *airRouteNew = malloc(sizeof(AirRoute));
				airRouteNew->xDest = col_end;
				airRouteNew->yDest = row_end;
				airRouteNew->costAirRoute = AirRouteCost(map, col_start, row_start);
				map[row_start][col_start].numAirRoute++;
				airRouteNew->next = map[row_start][col_start].array;
				map[row_start][col_start].array = airRouteNew;
				printf("OK\n");
			} else {
				printf("KO\n");
			}

			free(airRoutePrev);
			free(airRouteHead);
		} else {
			printf("KO\n");
		}
	}
	else if (strcmp(command, "travel_cost") == 0) {
		int colp, rowp, cold, rowd;
		res = scanf("%d" "%d" "%d" "%d", &colp, &rowp, &cold, &rowd);

		if (colp < 0 || rowp < 0 || cold < 0 || rowd < 0 || colp >= col || rowp >= row || cold >= col || rowd >= row) {
			printf("-1\n");
		}else {
			if (map[rowp][colp].cost == 0) {
				printf("-1\n");
			}
			else if (colp == cold && rowp == rowd) {
				printf("0\n");
			}
			else {
				// if (row > 1 && col > 1) {
					//dijkstra
					DijkstraShortestPath(map, rowp * col + colp, rowd * col + cold, col, row);
				// }
				// else {
				// 	int distanza = 0;
				// 	if (row == 1) {
				// 		if (colp > cold) {
				// 			for (int i = cold; i < colp; i++) {
				// 				distanza += map[rowp][i].cost;
				// 			}
				// 		}else {
				// 			for (int i = colp; i < cold; i++) {
				// 				distanza += map[rowp][i].cost;
				// 			}
				// 		}
				// 	}
				// 	else {
				// 		if (rowp > rowd) {
				// 			for (int i = rowd; i < rowp; i++) {
				// 				distanza += map[i][colp].cost;
				// 			}
				// 		}else {
				// 			for (int i = rowp; i < rowd; i++) {
				// 				distanza += map[i][colp].cost;
				// 			}
				// 		}
				// 	}
				//
				// 	printf("%d\n", distanza);
				// }
			}

		}
	}

	}
	free(map);
}

int AirRouteCost(Tile **map, int col, int row) {
	int sum = 0;
	for (int i = 0; i < map[row][col].numAirRoute; i++) {
		sum = sum + map[row][col].array[i].costAirRoute;
	}
	sum = (sum + map[row][col].cost) / (map[row][col].numAirRoute + 1);
	return sum;
}

int Incremento(Tile **map, int col, int row, int v, int raggio, int distanza) {
	int costo;
	float temp;

	temp = (float)(raggio - distanza)/(float)raggio;
	costo = map[row][col].cost;

	if (temp > 0) {
		costo = costo + (int) (v * temp);
	}

	return costo;
}

int DistanzaEsagoni (Tile** map, int cola, int rowa, int colb, int rowb) {
	//visto che uso le coordinate cubiche la formula è veloce
	int dx, dy, dz, val;
	dx = abs(map[rowa][cola].x - map[rowb][colb].x);
	dy = abs(map[rowa][cola].y - map[rowb][colb].y);
	dz = abs(map[rowa][cola].z - map[rowb][colb].z);

	val = (dx + dy + dz)/2;

	return val;
}

//Gestione dell'estrazione del valore minimo dell'heap (pop)
Node ExtractMin (Queue *Q, int *distanza) {
		Node min = Q->minHeap[0];
		Q->minHeap[0] = Q->minHeap[( Q->size) - 1];
		Q->posizione[Q->minHeap[0].indice] = 0;
		( Q->size)--;
		MinHeapify(Q, 0, distanza);
		return min;

}

void MinHeapify (Queue *Q, int i, int *distanza) {
	int l = Left(i);
	int r = Right(i);
	int min = i;

	if (l < Q->size && Q->minHeap[l].distanza < Q->minHeap[min].distanza) {
		min = l;
	}

	if (r < Q->size && Q->minHeap[r].distanza < Q->minHeap[min].distanza) {
		min = r;
	}

	if (min != i) {
		Swap(Q, i, min);
		MinHeapify(Q, min, distanza);
	}
}

int Left(int i) {
	return 2 * i + 1;
}

int Right(int i) {
	return 2 * i + 2;
}

void Swap (Queue *Q, int i, int j) {
	Node temp;
	temp = Q->minHeap[i];
	Q->minHeap[i] = Q->minHeap[j];
	Q->minHeap[j] = temp;

	Q->posizione[Q->minHeap[i].indice] = i;
	Q->posizione[Q->minHeap[j].indice] = j;

	// printf("Scambio %d con %d:\n", i, j);
	// printf("0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29\n");
	// for (int k=0; k<30; k++) {
	// 	printf("%d	", Q->posizione[k]);
	// }
	// printf("\n\n");
}

//gestione per "alzare" un nodo dopo una modifica
void DecreaseKey (Queue *Q, int i, int key) {
	if (key < Q->minHeap[i].distanza) {
		Q->minHeap[i].distanza = key;
		while (i > 0 && Q->minHeap[Parent(i)].distanza > Q->minHeap[i].distanza ) {
			Swap(Q, i, Parent(i));
			i = Parent(i);
		}
	}
}

int Parent(int i) {
	return (i - 1) / 2;
}

//Gestione dell'inserimento nello heap
void HeapInsert (Queue *Q, int id, int key) {
	Q->minHeap[Q->size].distanza = key;
	Q->minHeap[Q->size].indice = id;
	DecreaseKey(Q, Q->size,key);

	Q->size += 1;
}

void DijkstraShortestPath(Tile **G, int idp, int idd, int col, int row) {
	int distance[row*col];
	Queue *Q = malloc(sizeof(Queue));
	Q->minHeap = malloc(sizeof(Node) * (row*col));
	Q->size = 0;
	Q->posizione = malloc(sizeof(int) * (row*col));

	distance[idp] = 0;
	HeapInsert (Q, idp,0);
	Q->posizione[idp] = idp;

	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			if ((i*col)+j != idp) {
				Q->posizione[(i*col)+j] = (i*col)+j;
				distance[(i*col)+j] = INF;
				HeapInsert (Q, (i*col)+j, INF);
			}
		}
	}

	while (Q->size > 0) {
		Node u;
		u = ExtractMin (Q, distance);
		int y = u.indice / col;
		int x = u.indice % col;
		int w = G[y][x].cost;

		if (G[y][x].cost != 0) {
			int xp, zp;
			xp = x - ((y - (y & 1)) / 2);
			zp = y;

			int xd, zd, idFinale, col_finale, row_finale;

			//1 0 -1 (alto destra)
			xd = xp + 1;
			zd = zp - 1;
			row_finale = zd;
			col_finale = xd + (zd -(zd & 1)) /2;
			idFinale = row_finale*col+col_finale;
			if (row_finale >= 0 && row_finale < row && col_finale >= 0 && col_finale < col && idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + w) {
				distance[idFinale] = u.distanza + w;
				DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
			}

			//1 -1 0 (destra)
			xd = xp + 1;
			zd = zp;
			row_finale = zd;
			col_finale = xd + (zd -(zd & 1)) /2;
			idFinale = row_finale*col+col_finale;
			if (row_finale >= 0 && row_finale < row && col_finale >= 0 && col_finale < col && idFinale < row*col && distance[idFinale] > u.distanza + w) {
				distance[idFinale] = u.distanza + w;
				DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
			}

			//0 -1 1 (basso destra)
			xd = xp;
			zd = zp + 1;
			row_finale = zd;
			col_finale = xd + (zd -(zd & 1)) /2;
			idFinale = row_finale*col+col_finale;
			if (row_finale >= 0 && row_finale < row && col_finale >= 0 && col_finale < col && idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + w) {
				distance[idFinale] = u.distanza + w;
				DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
			}

			//-1 0 1 (basso sinistra)
			xd = xp - 1;
			zd = zp + 1;
			row_finale = zd;
			col_finale = xd + (zd -(zd & 1)) /2;
			idFinale = row_finale*col+col_finale;
			if (row_finale >= 0 && row_finale < row && col_finale >= 0 && col_finale < col && idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + w) {
				distance[idFinale] = u.distanza + w;
				DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
			}

			//-1 1 0 (sinistra)
			xd = xp - 1;
			zd = zp;
			row_finale = zd;
			col_finale = xd + (zd -(zd & 1)) /2;
			idFinale = row_finale*col+col_finale;
			if (row_finale >= 0 && row_finale < row && col_finale >= 0 && col_finale < col && idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + w) {
				distance[idFinale] = u.distanza + w;
				DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
			}

			//0 1 -1 (alto sinistra)
			xd = xp;
			zd = zp - 1;
			row_finale = zd;
			col_finale = xd + (zd -(zd & 1)) /2;
			idFinale = row_finale*col+col_finale;
			if (row_finale >= 0 && row_finale < row && col_finale >= 0 && col_finale < col && idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + w) {
				distance[idFinale] = u.distanza + w;
				DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
			}

			//air route
			for (int i = 0; i < G[y][x].numAirRoute; i++) {
				idFinale = G[y][x].array[i].yDest * col + G[y][x].array[i].xDest;
				if (idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + G[y][x].array[i].costAirRoute) {
					distance[idFinale] = u.distanza + G[y][x].array[i].costAirRoute;
					DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
				}
			}
		}


	}

		for (int i = 0; i < row; i++) {
			for (int j = 70869; j < 70969; j++) {
				if (distance[i*col+j] < INF) {
					printf("%d	", distance[i*col+j]);
				}
				else {
					printf("INF	");
				}
			}
			printf("\n");
		}

		printf("\n\n");


	if (distance[idd] >= INF) {
		printf("-1\n");
	}
	else {
		printf("%d\n", distance[idd]);
	}

}