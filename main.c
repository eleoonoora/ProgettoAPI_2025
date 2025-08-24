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
typedef struct heap {
	int distanza;
	int indice;
} Heap;
//========================================//

int AirRouteCost(Tile **map, int x, int y);
int Incremento(Tile **map, int x, int y, int v, int raggio, int distanza);
int DistanzaEsagoni (Tile** map, int xa, int ya, int xb, int yb);
Heap ExtractMin (Heap *Q, int *HeapSize);
void MinHeapify (Heap *Q, int HeapSize, int i);
int Left(int i);
int Right(int i);
void Swap (Heap *a, Heap *b);
void DecreaseKey (Heap *Q, int i, int key);
int Parent(int i);
void HeapInsert (Heap *Q, int *heapSize, int key);
void DijkstraShortestPath(Tile **G, int w, int id, int col, int row);

int main() {
	char command[64];
	int res = 0, row = 0, col = 0;
	Tile **map = NULL;

	while ((res = scanf("%s", command)) != EOF) {
		if (strcmp(command, "init") == 0) {
			//se la mappa è già presente la si libera
			if (map != NULL) {
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
	else if (strcmp(command, "change_cost") == 0) { //! controlla perchè raggio 1 è solo la cella inserita
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
							val = Incremento(map, x+i, y+j, v, raggio, DistanzaEsagoni(map, x+i, y+j, x, y));
							if (val > 0 && val <= 100) { //lo aggiorna solo se il nuovo costo (finale) è tra 0 e 100
								map[x+i][y+j].cost = val;
								for (int k = 0; k < map[x+i][y+j].numAirRoute; k++) {
									map[x+i][y+j].array[k].costAirRoute = val;
								}
							}else {
								if (val <= 0) { // se è negativo lo poni a zero
									map[x+i][y+j].cost = 0;
									for (int k = 0; k < map[x+i][y+j].numAirRoute; k++) {
										map[x+i][y+j].array[k].costAirRoute = 0;
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
		else if (raggio == 1) {
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

		if (x_start <= row && x_end <= row && y_end <= col && y_end <= col) {
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

		if (xp < 0 || yp < 0 || xd < 0 || yd < 0 || xp > row || yp > col || xd > row || yd > col) {
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
				DijkstraShortestPath(map, map[xp][yp].cost, xd * col + yd, col, row);
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

// Gestione dell'estrazione del valore minimo dell'heap (pop)
Heap ExtractMin (Heap *Q, int *HeapSize) {
	if (*HeapSize > 0) {
		Heap min;
		min = Q[0];
		Q[0] = Q[(*HeapSize) - 1];
		(*HeapSize)--;
		MinHeapify(Q, *HeapSize, 0);
		return min;
	}

}

void MinHeapify (Heap *Q, int HeapSize, int i) {
	int l = Left(i);
	int r = Right(i);
	int min = i;

	if (l < HeapSize && Q[l].distanza < Q[i].distanza) {
		min = l;
	}
	else {
		if (r < HeapSize && Q[r].distanza < Q[min].distanza) {
			min = r;
		}
	}

	if (min != i) {
		Swap(&Q[i], &Q[min]);
		MinHeapify(Q, HeapSize, min);
	}
}

int Left(int i) {
	return 2 * i + 1;
}

int Right(int i) {
	return 2 * i + 2;
}

void Swap (Heap *a, Heap *b) {
	Heap temp;
	temp = *a;
	*a = *b;
	*b = temp;
}

//gestione per "alzare" un nodo dopo una modifica
void DecreaseKey (Heap *Q, int i, int key) {
	if (key < Q[i].distanza) {
		Q[i].distanza = key;
		while (i > 0 && Q[Parent(i)].distanza > Q[i].distanza ) {
			Swap(&Q[i], &Q[Parent(i)]);
			i = Parent(i);
		}
	}
}

int Parent(int i) {
	return (i - 1) / 2;
}

//Gestione dell'inserimento nello heap
void HeapInsert (Heap *Q, int *heapSize, int key) {
	//uso -1 al posto di infinito perchè i pesi vanno da 0 a 100,usare un numero come 101 sarebbe errato perchè
	//procedenndo con i passi potrei avere la somma maggiore. Ma mai negativa
	Q[*heapSize].distanza = key;
	(*heapSize) ++;
	DecreaseKey(Q, *heapSize, key);
}

void DijkstraShortestPath(Tile **G, int w, int id, int col, int row) {
	int heapSize = 0, distance[row][col];
	Heap Q[row*col];
	Heap s;
	s.indice = id;
	s.distanza = 0;

	HeapInsert (&Q, &heapSize, s.distanza);

	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			if ((i*col)+j != s.indice) {
				Q[heapSize].distanza = INF;
				Q[heapSize].indice = i * col + j;
				HeapInsert (&Q, &heapSize, Q[heapSize].distanza);
			}
		}
	}

	//! Qui c'è il problema, non è gestitto correttamente l'heap e il salvataggio delle distanze
	//In pratica provo ad accedere ad un nodo dello heap con l'id ma la posizione dei nodi cambia ad ogni inserimento/rimozione!
	//Bisognerebbe guardare (e quindi passare) la matrice delle distanze per questo
	while (heapSize > 0) {
		Heap u;
		u = ExtractMin (Q, &heapSize);
		int x = u.indice / col;
		int y = u.indice % col;

		int xp, yp, zp;
		xp = y - ((x - (x & 1)) / 2);
		zp = x;
		yp = xp- zp;

		int xd, yd, zd, idFinale;

		//1 0 -1
		xd = xp + 1;
		yd = yp;
		zd = zp - 1;
		idFinale = (xd * col) + (zp -(zp & 1)) /2;
		if (idFinale >=0 && idFinale < row*col && Q[idFinale].distanza > u.distanza + w) {
			Q[idFinale].distanza = u.distanza + w;
			distance[zd][(xp -(zp & 1)) /2] = u.distanza + w;
			DecreaseKey (&Q, idFinale, Q[idFinale].distanza);
		}

		//1 -1 0
		xd = xp + 1;
		yd = yp -1;
		zd = zp;
		idFinale = (zd * col) + xp + (zp -(zp & 1)) /2;
		if (idFinale >=0 && idFinale < row*col && Q[idFinale].distanza > u.distanza + w) {
			Q[idFinale].distanza = u.distanza + w;
			distance[zd][xp + (zp -(zp & 1)) /2] = u.distanza + w;
			DecreaseKey (&Q, idFinale, Q[idFinale].distanza);
		}

		//0 -1 1
		xd = xp;
		yd = yp - 1;
		zd = zp + 1;
		idFinale = (zd * col) + xp + (zp -(zp & 1)) /2;
		if (idFinale >=0 && idFinale < row*col && Q[idFinale].distanza > u.distanza + w) {
			Q[idFinale].distanza = u.distanza + w;
			distance[zd][xp + (zp -(zp & 1)) /2] = u.distanza + w;

			
			DecreaseKey (&Q, idFinale, Q[idFinale].distanza);
		}

		//-1 0 1
		xd = xp - 1;
		yd = yp;
		zd = zp + 1;
		idFinale = (zd * col) + xp + (zp -(zp & 1)) /2;
		if (idFinale >=0 && idFinale < row*col && Q[idFinale].distanza > u.distanza + w) {
			Q[idFinale].distanza = u.distanza + w;
			distance[zd][xp + (zp -(zp & 1)) /2] = u.distanza + w;
			DecreaseKey (&Q, idFinale, Q[idFinale].distanza);
		}

		//-1 1 0
		xd = xp - 1;
		yd = yp + 1;
		zd = zp;
		idFinale = (zd * col) + xp + (zp -(zp & 1)) /2;
		if (idFinale >=0 && idFinale < row*col && Q[idFinale].distanza > u.distanza + w) {
			Q[idFinale].distanza = u.distanza + w;
			distance[zd][xp + (zp -(zp & 1)) /2] = u.distanza + w;
			DecreaseKey (&Q, idFinale, Q[idFinale].distanza);
		}

		//0 1 -1
		xd = xp;
		yd = yp + 1;
		zd = zp - 1;
		idFinale = (zd * col) + xp + (zp -(zp & 1)) /2;
		if (idFinale >=0 && idFinale < row*col && Q[idFinale].distanza > u.distanza + w) {
			Q[idFinale].distanza = u.distanza + w;
			distance[zd][xp + (zp -(zp & 1)) /2] = u.distanza + w;
			DecreaseKey (&Q, idFinale, Q[idFinale].distanza);
		}

		//air route
		for (int i = 0; i < G[x][y].numAirRoute; i++) {
			idFinale = G[x][y].array->xDest * col + G[x][y].array->yDest;
			if (idFinale >=0 && idFinale < row*col && Q[idFinale].distanza > u.distanza + w) {
				Q[idFinale].distanza = u.distanza + w;
				distance[zd][xp + (zp -(zp & 1)) /2] = u.distanza + w;
				DecreaseKey (&Q, idFinale, Q[idFinale].distanza);
			}
		}

	}

	if (distance[id/col][id%col] >= INF) {
		printf("-1\n");
	}
	else {
		printf("%d\n", distance[id/col][id%col]);
	}

}