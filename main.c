#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INF 10000

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

int AirRouteCost(Tile **map, int x, int y);
int Incremento(Tile **map, int x, int y, int v, int raggio, int distanza);
int DistanzaEsagoni (Tile** map, int xa, int ya, int xb, int yb);
Node ExtractMin (Queue *Q, int *distanza);
void MinHeapify (Queue *Q, int i, int *distanza);
int Left(int i);
int Right(int i);
void Swap (Queue *Q, int i, int j);
void DecreaseKey (Queue *Q, int i, int key);
int Parent(int i);
void HeapInsert (Queue *Q, int id, int key);
void DijkstraShortestPath(Tile **G, int w, int idp, int idd, int col, int row);

int main() {
	char command[64];
	int res = 0, row = 0, col = 0;
	Tile **map = NULL;

	while ((res = scanf("%s", command)) != EOF) {
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
			scanf("%d", &col);
			scanf("%d", &row);

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
		int x, y, v, raggio, hColo, val;
		scanf("%d" "%d" "%d" "%d", &x, &y, &v, &raggio);
		if (x < row && x >= 0 && y < col && y >= 0 && raggio > 1 && -10 <= v && v <= 10) {
			hColo = 1;
			//loop per le colonne da modificare (asse x)
			for (int i = -raggio + 1; i <= raggio - 1; i++) {
				//loop per le righe da modificare (asse y)
				if ((x+i) >= 0 && (x+i) < col) { //solo se esiste la colonna, risparimo tempo
					for (int j = -hColo; j <= hColo; j++) {
						if ((y+j) >= 0 && (y+j) < row) { // solo se esiste la riga
							val = Incremento(map,y+j, x+i, v, raggio, DistanzaEsagoni(map, y+j, x+i, x, y));
							if (val > 0 && val <= 100) { //lo aggiorna solo se il nuovo costo (finale) è tra 0 e 100
								map[y+j][x+i].cost = val;
								for (int k = 0; k < map[y+j][x+i].numAirRoute; k++) {
									map[y+j][x+i].array[k].costAirRoute = val;
								}
							}else {
								if (val <= 0) { // se è negativo lo poni a zero
									map[y+j][x+i].cost = 0;
									for (int k = 0; k < map[y+j][x+i].numAirRoute; k++) {
										map[y+j][x+i].array[k].costAirRoute = 0;
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
		else if (x < row && x >= 0 && y < col && y >= 0 && raggio == 1 && -10 <= v && v <= 10) {
			map[x][y].cost += v;
			printf("OK\n");
		}
		else {
			printf("KO\n");
		}
	}
	else if (strcmp(command, "toggle_air_route") == 0) {
		int flag = 0; //flag per la ricerca della rotta da inserire
		int x_start = 0, y_start = 0, x_end = 0, y_end = 0;
		scanf("%d" "%d" "%d" "%d", &x_start, &y_start, &x_end, &y_end);

		if (x_start < row && x_end < row && y_end < col && y_end < col) {
			AirRoute *airRouteHead = map[x_start][y_start].array;
			AirRoute *airRoutePrev = NULL;

			while (airRouteHead != NULL && flag != 1) {
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
	}
	else if (strcmp(command, "travel_cost") == 0) {
		int xp, yp, xd, yd;
		res = scanf("%d" "%d" "%d" "%d", &xp, &yp, &xd, &yd);

		if (xp < 0 || yp < 0 || xd < 0 || yd < 0 || xp >= row || yp >= col || xd >= row || yd >= col) {
			printf("-1\n");
		}else {
			if (map[xp][yp].cost == 0) {
				printf("-1\n");
			}
			else if (xp == xd && yp == yd) {
				printf("0\n");
			}
			else {
				//dijkstra
				DijkstraShortestPath(map, map[xp][yp].cost, xp * col + yp, xd * col + yd, col, row);
			}

		}
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
	dx = abs(map[xa][ya].x - map[xb][yb].x);
	dy = abs(map[xa][ya].y - map[xb][yb].y);
	dz = abs(map[xa][ya].z - map[xb][yb].z);

	max = dx;
	if (dy > max) {
		max = dy;
	}
	if (dz > max) {
		max = dz;
	}

	return max;
}

//Gestione dell'estrazione del valore minimo dell'heap (pop)
Node ExtractMin (Queue *Q, int *distanza) {
	if (Q->size > 0) {
		Node min = Q->minHeap[0];
		Q->minHeap[0] = Q->minHeap[( Q->size) - 1];
		( Q->size)--;
		MinHeapify(Q, 0, distanza);
		return min;
	}

}

void MinHeapify (Queue *Q, int i, int *distanza) {
	int l = Left(i);
	int r = Right(i);
	int min = i;

	if (l < Q->size && Q->minHeap[l].distanza < Q->minHeap[min].distanza) {
		min = l;
	}
	else {
		if (r < Q->size && Q->minHeap[r].distanza < Q->minHeap[min].distanza) {
			min = r;
		}
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

	Q->posizione[Q->minHeap[i].indice] = j;
	Q->posizione[Q->minHeap[j].indice] = i;

	printf("Scambio %d con %d:\n", i, j);
	printf("0	1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29\n");
	for (int k=0; k<30; k++) {
		printf("%d	", Q->posizione[k]);
	}
	printf("\n\n");
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
	Q->size += 1;
	DecreaseKey(Q, Q->size,key);
}

void DijkstraShortestPath(Tile **G, int w, int idp, int idd, int col, int row) {
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
		int x = u.indice / col;
		int y = u.indice % col;

		if (G[x][y].cost != 0) {
			int xp, yp, zp;
			xp = y - ((x - (x & 1)) / 2);
			zp = x;
			yp = -xp - zp;

			int xd, yd, zd, idFinale;

			//1 0 -1 (alto destra)
			xd = xp + 1;
			yd = yp;
			zd = zp - 1;
			idFinale = (zd * col) + xd + (zd -(zd & 1)) /2;
			if (x > 0 && (xd + (zd -(zd & 1))) < col && idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + w) {
				distance[idFinale] = u.distanza + w;
				DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
			}

			//1 -1 0 (destra)
			xd = xp + 1;
			yd = yp -1;
			zd = zp;
			idFinale = (zd * col) + xd + (zd -(zd & 1)) /2;
			if (y < (col - 1) && idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + w) {
				distance[idFinale] = u.distanza + w;
				DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
			}

			//0 -1 1 (basso destra)
			xd = xp;
			yd = yp - 1;
			zd = zp + 1;
			idFinale = (zd * col) + xd + (zd -(zd & 1)) /2;
			if (x < (row -1) && (xd + (zd -(zd & 1))) < col && idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + w) {
				distance[idFinale] = u.distanza + w;
				DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
			}

			//-1 0 1 (basso sinistra)
			xd = xp - 1;
			yd = yp;
			zd = zp + 1;
			idFinale = (zd * col) + xd + (zd -(zd & 1)) /2;
			if (x < (row -1) && (xd + (zd -(zd & 1))) > 0 && idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + w) {
				distance[idFinale] = u.distanza + w;
				DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
			}

			//-1 1 0 (sinistra)
			xd = xp - 1;
			yd = yp + 1;
			zd = zp;
			idFinale = (zd * col) + xd + (zd -(zd & 1)) /2;
			if (y > 0 && idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + w) {
				distance[idFinale] = u.distanza + w;
				DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
			}

			//0 1 -1 (alto sinistra = alto sinistra in matrice)
			xd = xp;
			yd = yp + 1;
			zd = zp - 1;
			idFinale = (zd * col) + xd + (zd -(zd & 1)) /2;
			if (x > 0 && (xd + (zd -(zd & 1))) > 0 && idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + w) {
				distance[idFinale] = u.distanza + w;
				DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
			}

			//air route
			for (int i = 0; i < G[x][y].numAirRoute; i++) {
				idFinale = G[x][y].array->xDest * col + G[x][y].array->yDest;
				if (idFinale >=0 && idFinale < row*col && distance[idFinale] > u.distanza + w) {
					distance[idFinale] = u.distanza + w;
					DecreaseKey (Q, Q->posizione[idFinale], distance[idFinale]);
				}
			}
		}

		for (int i = 0; i < row; i++) {
			for (int j = 0; j < col; j++) {
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

	}

	if (distance[idd] >= INF) {
		printf("-1\n");
	}
	else {
		printf("%d\n", distance[idd]);
	}

}