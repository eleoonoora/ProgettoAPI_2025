#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define INF 2000000000
#define MAX_CACHE 8
#define MAX_TILE 100

/*Funzionamento della cache: la cache ha grandezze fisse, ovvero salva le prime MAX_TILE di tiles diverse e per ogni tile
* salva al massimo MAX_CACHE destinazioni diverse. Questi valore sono stati scelti arbitrariamente.
 */

//================= CACHE =================//
typedef struct destinazione {
	int32_t idd;
	int32_t distanza;
} Destinazioni;

typedef struct tileCache {
	int32_t idp;
	int32_t size;
	Destinazioni destinazioni[MAX_CACHE];
} TileCache;

typedef struct cache {
	int32_t size;
	TileCache tiles[MAX_TILE];
} Cache;
//=========================================//

/* Struttura per la mappa esagonale: la mappa della specifica è stata salvata su una matrice row * col. La gestione
 * non è immediatamente logica per via degli sfasamenti, per ogni tile vengono salvate le coordinate cubiche perchè
 * permettono un calcolo delle distanze semplice. Ogni tile contiene un array dinamico delle air route, fatto dinamicamente.
 */

//================= TILE =================//
typedef struct airRoute {
	int32_t xDest;
	int32_t yDest;
	int32_t costAirRoute;
	struct airRoute *next;
} AirRoute;

typedef struct tile {
	int32_t x;
	int32_t y;
	int32_t z;
	int32_t cost;
	int32_t numAirRoute;
	AirRoute *array;
} Tile;
//========================================//

// Struttura dello heap per Dijkstra: heap molto semplice e statico visto che la funzione viene chiamata molte volte.

//================= HEAP =================//
typedef struct node {
	int32_t distanza;
	int32_t indice;
} Node;
//========================================//

static inline int32_t AirRouteCost(Tile **map, int32_t col, int32_t row);
static inline int32_t Incremento(Tile **map, int32_t col, int32_t row, int32_t v, int32_t raggio, int32_t distanza);
static inline int32_t DistanzaEsagoni(Tile **map, int32_t cola, int32_t rowa, int32_t colb, int32_t rowb);
static inline int32_t DijkstraShortestPath(Tile **G, int32_t idp, int32_t idd, int32_t col, int32_t row);

// static Node ExtractMin (Queue *Q);
// static void MinHeapify (Queue *Q, int32_t i);
// int32_t Left(int32_t i);
// int32_t Right(int32_t i);
// void Swap (Queue *Q, int32_t i, int32_t j);
// static void DecreaseKey (Queue *Q, int32_t i, int32_t key);
// int32_t Parent(int32_t i);
// static void HeapInsert (Queue *Q, int32_t id, int32_t key);

//! le matrici sono map[row][col]

int32_t main() {
	char command[64];
	int32_t res = 0, row = 0, col = 0, flag_reset_cache = 0;
	Tile **map = NULL;
	Cache cache = {0};

	while ((res = scanf("%s", command)) != EOF) {
		if (strcmp(command, "init") == 0) {
			//se la mappa è già presente la si libera
			if (map != NULL) {
				flag_reset_cache = 1;
				for (int32_t i = 0; i < row; i++) {
					for (int32_t j = 0; j < col; j++) {
						AirRoute *head = map[i][j].array;
						while (head != NULL) {
							AirRoute *tmp = head->next;
							free(head);
							head = tmp;
						}
						map[i][j].array = NULL;
					}
					free(map[i]);
				}
				free(map);
				map = NULL;
			}

			//poi si procede nella lettura dei valori e alla creazione di una nuova mappa
			res = scanf("%d", &col);
			res = scanf("%d", &row);

			map = malloc(row * sizeof(Tile *));
			for (int32_t i = 0; i < row; i++) {
				map[i] = (Tile *) malloc(col * sizeof(Tile));
				for (int32_t j = 0; j < col; j++) {
					map[i][j].x = j - ((i - (i & 1)) / 2);
					map[i][j].z = i;
					map[i][j].y = -map[i][j].x - map[i][j].z;
					map[i][j].cost = 1;
					map[i][j].numAirRoute = 0;
					map[i][j].array = NULL;
				}
			}

			//creazione della cache
			cache.size = 0;

			printf("OK\n");
		} else if (strcmp(command, "change_cost") == 0) {
			int32_t colStart, rowStart, v, raggio, hColo, val;
			res = scanf("%d" "%d" "%d" "%d", &colStart, &rowStart, &v, &raggio);

			if (rowStart < row && rowStart >= 0 && colStart < col && colStart >= 0 && raggio > 1 && -10 <= v && v <= 10 && map != NULL) {
				hColo = 1;

				//loop per le colonne da modificare (asse x)
				for (int32_t i = -raggio + 1; i <= raggio - 1; i++) {
					//loop per le righe da modificare (asse y)
					if ((colStart + i) >= 0 && (colStart + i) < col) { //solo se esiste la colonna
						for (int32_t j = -hColo; j <= hColo; j++) {
							if ((rowStart + j) >= 0 && (rowStart + j) < row) {
								// solo se esiste la riga
								val = Incremento(map, colStart + i, rowStart + j, v, raggio, DistanzaEsagoni(map, colStart, rowStart, colStart + i, rowStart + j));
								if (val > 0 && val <= 100) {
									//lo aggiorna solo se il nuovo costo (finale) è tra 0 e 100
									map[rowStart + j][colStart + i].cost = val;
									for (int32_t k = 0; k < map[rowStart + j][colStart + i].numAirRoute; k++) {
										map[rowStart + j][colStart + i].array[k].costAirRoute = val;
									}
								} else {
									if (val <= 0) {
										val = 0;
									}else {
										val = 100;
									}

									map[rowStart + j][colStart + i].cost = val;
									for (int32_t k = 0; k < map[rowStart + j][colStart + i].numAirRoute; k++) {
										map[rowStart + j][colStart + i].array[k].costAirRoute = val;
									}
								}
							}
						}
					}
					//gestione incremento altezza delle colonne
					if (i <= (raggio / 2)) {
						if (hColo < (raggio - 1)) {
							hColo = hColo + 2;
						} else if (hColo < raggio) {
							hColo = hColo + 1;
						}
					} else {
						if (hColo == raggio && raggio & 1 != 0) {
							hColo = hColo - 1;
						} else if (hColo > 0) {
							hColo = hColo - 2;
						}
					}
				}

				flag_reset_cache = 1;
				printf("OK\n");

			} else if (rowStart < row && rowStart >= 0 && colStart < col && colStart >= 0 && raggio == 1 && -10 <= v && v <= 10  && map != NULL) {
				val = map[rowStart][colStart].cost + v;
				if (val > 0 && val <= 100) {
					map[rowStart][colStart].cost = val;
					for (int32_t k = 0; k < map[rowStart][colStart].numAirRoute; k++) {
						map[rowStart][colStart].array[k].costAirRoute = v;
					}
				} else {
					if (val <= 0) {
						val = 0;
					}else {
						val = 100;
					}

					map[rowStart][colStart].cost = val;
					for (int32_t k = 0; k < map[rowStart][colStart].numAirRoute; k++) {
						map[rowStart][colStart].array[k].costAirRoute = val;
					}

				}

				flag_reset_cache =1;
				printf("OK\n");

			} else {
				printf("KO\n");
			}
		} else if (strcmp(command, "toggle_air_route") == 0) {
			int32_t flag = 0; //flag per la ricerca della rotta da inserire
			int32_t col_start = 0, row_start = 0, col_end = 0, row_end = 0;
			res = scanf("%d" "%d" "%d" "%d", &col_start, &row_start, &col_end, &row_end);

			if (col_start < col && col_end < col && row_start < row && row_end < row && map != NULL) {
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
				if (flag == 1) {
					if (airRoutePrev == NULL) {
						//Caso 1: è il primo -> prev è null
						if (airRouteHead != NULL) {
							//Caso 1.1: ci sono altri nodi -> head non è null
							map[row_start][col_start].array = airRouteHead->next;
							map[row_start][col_start].numAirRoute--;
							free(airRouteHead);
							printf("OK\n");
						} else {
							//Caso 1.2: è l'unico nodo -> head è null
							map[row_start][col_start].array = NULL;
							map[row_start][col_start].numAirRoute = 0;
							free(airRouteHead);
							printf("OK\n");
						}
					} else {
						if (airRouteHead == NULL) {
							//Caso 2: è l'ultimo -> prev non è null e head è null
							airRoutePrev->next = NULL;
							map[row_start][col_start].numAirRoute--;
							printf("OK\n");
						} else {
							airRoutePrev->next = airRouteHead->next;
							map[row_start][col_start].numAirRoute--;
							free(airRouteHead);
							printf("OK\n");
						}
					}

					flag_reset_cache = 1;
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

					flag_reset_cache = 1;
					printf("OK\n");

				} else {
					printf("KO\n");
				}
			} else {
				printf("KO\n");
			}
		} else if (strcmp(command, "travel_cost") == 0) {
			int32_t colp, rowp, cold, rowd;
			res = scanf("%d" "%d" "%d" "%d", &colp, &rowp, &cold, &rowd);

			if (colp < 0 || rowp < 0 || cold < 0 || rowd < 0 || colp >= col || rowp >= row || cold >= col || rowd >= row && map == NULL) {
				printf("-1\n");
			} else {
				if (map[rowp][colp].cost == 0) {
					printf("-1\n");
				} else if (colp == cold && rowp == rowd) {
					printf("0\n");
				} else {
					int32_t risultato, idd, idp, flag_idp_found = 0, tileStart = 0, flag_idd_found = 0, posizione_destinazione = 0;
					idp = rowp * col + colp;
					idd = rowd * col + cold;

					//Se tra l'ultimo travel e il corrente è stato modificato qualcosa nella mappa, resetto la cache
					if (flag_reset_cache == 1) {
						//reset di tutta la cache per cambiamento costi
						for (int i = 0; i < cache.size; i++) {
							cache.tiles[i].size = 0;
						}
						cache.size = 0;
						flag_reset_cache = 0;

						risultato = DijkstraShortestPath(map, rowp * col + colp, rowd * col + cold, col, row);
						printf("%d\n", risultato);

						//Va inserita sia la tile, sia la destinazione della tile
						TileCache *new = &cache.tiles[cache.size];
						new->idp = idp;
						new->size = 0;

						//Singola destinazione
						Destinazioni *new2 = &cache.tiles[cache.size].destinazioni[cache.tiles[cache.size].size];
						new2->idd = idd;
						new2->distanza = risultato;
						cache.tiles[cache.size].size++;

						cache.size++;

					}else {
						//Cerco se c'è la tile di partenza nella cache
						while (flag_idp_found != 1 && tileStart < cache.size) {
							if (cache.tiles[tileStart].idp == idp) {
								flag_idp_found = 1;
							} else {
								tileStart++;
							}
						}

						//Se c'è, cerco se c'è quella destinazione
						if (flag_idp_found == 1) {
							while (flag_idd_found != 1 && posizione_destinazione < cache.tiles[tileStart].size) {
								if (cache.tiles[tileStart].destinazioni[posizione_destinazione].idd == idd) {
									flag_idd_found = 1;
								} else {
									posizione_destinazione++;
								}
							}
						}

						if (flag_idp_found == 1 && flag_idd_found == 1) {
							printf("%d\n", cache.tiles[tileStart].destinazioni[posizione_destinazione].distanza);
						} else {
							risultato = DijkstraShortestPath(map, rowp * col + colp, rowd * col + cold, col, row);
							printf("%d\n", risultato);

							//Se c'è la tile ma non la destinazione aggiungo solo quella
							if (flag_idp_found == 1 && flag_idd_found == 0) {
								if (cache.tiles[tileStart].size < MAX_CACHE) {
									Destinazioni *new = &cache.tiles[tileStart].destinazioni[cache.tiles[tileStart].size];
									new->idd = idd;
									new->distanza = risultato;
									cache.tiles[tileStart].size++;
								}
							} else {
								//Altrimenti devo aggiungere entrambe
								if (cache.size < MAX_TILE) {
									TileCache *new = &cache.tiles[cache.size];
									new->idp = idp;
									new->size = 0;

									//Singola destinazione
									Destinazioni *new2 = &cache.tiles[cache.size].destinazioni[cache.tiles[tileStart].size];
									new2->idd = idd;
									new2->distanza = risultato;
									cache.tiles[cache.size].size++;

									cache.size++;
								}


							}
						}
					}
				}
			}
		}
	}

	for (int32_t i = 0; i < row; i++) {
		for (int32_t j = 0; j < col; j++) {
			AirRoute *head = map[i][j].array;
			while (head != NULL) {
				AirRoute *tmp = head->next;
				free(head);
				head = tmp;
			}
			map[i][j].array = NULL;
		}
		free(map[i]);
	}
	free(map);
	map = NULL;
}

static inline int32_t AirRouteCost(Tile **map, int32_t col, int32_t row) {
	int32_t sum = 0;
	for (int32_t i = 0; i < map[row][col].numAirRoute; i++) {
		sum = sum + map[row][col].array[i].costAirRoute;
	}
	sum = (sum + map[row][col].cost) / (map[row][col].numAirRoute + 1);
	return sum;
}

static inline int32_t Incremento(Tile **map, int32_t col, int32_t row, int32_t v, int32_t raggio, int32_t distanza) {
	int32_t costo;
	float temp;

	temp = (float) (raggio - distanza) / (float) raggio;
	costo = map[row][col].cost;

	if (temp > 0) {
		costo = costo + (int32_t) floor(v * temp);
	}

	return costo;
}

static inline int32_t DistanzaEsagoni(Tile **map, int32_t cola, int32_t rowa, int32_t colb, int32_t rowb) {
	//visto che uso le coordinate cubiche la formula è veloce
	int32_t dx, dy, dz, val;
	dx = abs(map[rowa][cola].x - map[rowb][colb].x);
	dy = abs(map[rowa][cola].y - map[rowb][colb].y);
	dz = abs(map[rowa][cola].z - map[rowb][colb].z);

	val = (dx + dy + dz) / 2;

	return val;
}

// static Node ExtractMin (Queue *Q) {
// 		Node min = Q->minHeap[0];
// 		Q->minHeap[0] = Q->minHeap[( Q->size) - 1];
// 		Q->posizione[Q->minHeap[0].indice] = 0;
// 		( Q->size)--;
// 		MinHeapify(Q, 0);
// 		return min;
//
// }

// static void MinHeapify (Queue *Q, int32_t i) {
// 	int32_t l, r, min, flag = 0;
// 	while (flag == 0) {
// 		l = 2 * i + 1;
// 		r = 2 * i + 2;
// 		min = i;
//
// 		if (l < Q->size && Q->minHeap[l].distanza < Q->minHeap[min].distanza) {
// 			min = l;
// 		}
//
// 		if (r < Q->size && Q->minHeap[r].distanza < Q->minHeap[min].distanza) {
// 			min = r;
// 		}
//
// 		if (min != i) {
// 			//scambio
// 			Node temp = Q->minHeap[i];
// 			Q->minHeap[i] = Q->minHeap[min];
// 			Q->minHeap[min] = temp;
//
// 			Q->posizione[Q->minHeap[i].indice] = i;
// 			Q->posizione[Q->minHeap[min].indice] = min;
//
// 			i = min;
// 		}else {
// 			flag = 1;
// 		}
// 	}
// }

// int32_t Left(int32_t i) {
// 	return 2 * i + 1;
// }

// int32_t Right(int32_t i) {
// 	return 2 * i + 2;
// }

// void Swap (Queue *Q, int32_t i, int32_t j) {
// 	Node temp;
// 	temp = Q->minHeap[i];
// 	Q->minHeap[i] = Q->minHeap[j];
// 	Q->minHeap[j] = temp;
//
// 	Q->posizione[Q->minHeap[i].indice] = i;
// 	Q->posizione[Q->minHeap[j].indice] = j;
//
//
// 		// printf("Scambio %d con %d:\n", Q->posizione[Q->minHeap[j].indice], Q->posizione[Q->minHeap[i].indice]);
// 		// printf("0	1	2	3	4	5	6	7	8	9\n");
// 		// for (int k=0; k<10; k++) {
// 		// 	printf("%d	", Q->posizione[k]);
// 		// }
// 		// printf("\n\n");
// }

// static void DecreaseKey (Queue *Q, int32_t i, int32_t key) {
// 	if (key < Q->minHeap[i].distanza) {
// 		Q->minHeap[i].distanza = key;
// 		int32_t parent = (i - 1) / 2;
//
// 		while (i > 0 && Q->minHeap[parent].distanza > Q->minHeap[i].distanza ) {
//
// 			//scambio
// 			Node temp = Q->minHeap[i];
// 			Q->minHeap[i] = Q->minHeap[parent];
// 			Q->minHeap[parent] = temp;
//
// 			Q->posizione[Q->minHeap[i].indice] = i;
// 			Q->posizione[Q->minHeap[parent].indice] = parent;
//
// 			i = parent;
// 			parent = (i - 1) / 2;
// 		}
// 	}
// }

// int32_t Parent(int32_t i) {
// 	return (i - 1) / 2;
// }

//Gestione dell'inserimento nello heap
// static void HeapInsert (Queue *Q, int32_t id, int32_t key) {
// 	Q->minHeap[Q->size].distanza = key;
// 	Q->minHeap[Q->size].indice = id;
// 	DecreaseKey(Q, Q->size,key);
//
// 	Q->size += 1;
// }

static inline int32_t DijkstraShortestPath(Tile **G, int32_t idp, int32_t idd, int32_t col, int32_t row) {
	int32_t distance[row * col];

	int32_t size = 0;
	Node minHeap[row * col];
	int32_t posizione[row * col];

	distance[idp] = 0;

	//inserisco nello heap
	minHeap[size].distanza = 0;
	minHeap[size].indice = idp;
	size += 1;
	posizione[idp] = 0;

	for (int32_t i = 0; i < row * col; i++) {
		if (i != idp) {
			posizione[i] = size;
			distance[i] = INF;

			//inserisco nello heap
			minHeap[size].distanza = INF;
			minHeap[size].indice = i;

			size += 1;
		}
	}

	// printf("ARRAY POSIZIONI - INIZIALE:\n");
	// printf("0	1	2	3	4	5	6	7	8	9\n");
	// for (int k=0; k<10; k++) {
	// 	printf("%d	", Q->posizione[k]);
	// }
	// printf("\n\n");

	int32_t flagInf = 0;

	while (size > 0 && flagInf == 0) {
		//estraggo il nodo a distanza minore
		Node u = minHeap[0];

		if (u.indice != INF && u.indice != idd) {
			posizione[u.indice] = -1;
			minHeap[0] = minHeap[(size) - 1];
			posizione[minHeap[0].indice] = 0;
			size--;

			//MinHeapify
			int32_t l, r, min, flag = 0, nodoHeap;
			nodoHeap = 0;
			while (flag == 0) {
				l = 2 * nodoHeap + 1;
				r = 2 * nodoHeap + 2;
				min = nodoHeap;

				if (l < size && minHeap[l].distanza < minHeap[min].distanza) {
					min = l;
				}

				if (r < size && minHeap[r].distanza < minHeap[min].distanza) {
					min = r;
				}

				if (min != nodoHeap) {
					//scambio
					Node temp = minHeap[nodoHeap];
					minHeap[nodoHeap] = minHeap[min];
					minHeap[min] = temp;

					posizione[minHeap[nodoHeap].indice] = nodoHeap;
					posizione[minHeap[min].indice] = min;

					nodoHeap = min;
				} else {
					flag = 1;
				}
			}

			int32_t y = u.indice / col;
			int32_t x = u.indice % col;
			int32_t w = G[y][x].cost;

			if (G[y][x].cost != 0) {
				int32_t xp, zp;
				xp = x - (y - (y & 1)) / 2;
				zp = y;

				int32_t xd, zd, idFinale, col_finale, row_finale;

				//1 0 -1 (alto destra)
				xd = xp + 1;
				zd = zp - 1;
				row_finale = zd;
				col_finale = xd + (zd - (zd & 1)) / 2;
				idFinale = row_finale * col + col_finale;
				if (row_finale >= 0 && row_finale < row && col_finale >= 0 && col_finale < col && idFinale >= 0 &&
				    idFinale < row * col && distance[idFinale] > u.distanza + w && posizione[idFinale] != -1) {
					distance[idFinale] = u.distanza + w;

					//DecreaseKey
					int32_t k = posizione[idFinale];
					if (distance[idFinale] < minHeap[k].distanza) {
						minHeap[k].distanza = distance[idFinale];
						int32_t parent = (k - 1) / 2;

						while (k > 0 && minHeap[parent].distanza > minHeap[k].distanza) {
							//scambio
							Node temp = minHeap[k];
							minHeap[k] = minHeap[parent];
							minHeap[parent] = temp;

							posizione[minHeap[k].indice] = k;
							posizione[minHeap[parent].indice] = parent;

							k = parent;
							parent = (k - 1) / 2;
						}
					}
				}

				//1 -1 0 (destra)
				xd = xp + 1;
				zd = zp;

				row_finale = zd;
				col_finale = xd + (zd - (zd & 1)) / 2;
				idFinale = row_finale * col + col_finale;
				if (row_finale >= 0 && row_finale < row && col_finale >= 0 && col_finale < col && idFinale < row * col
				    && distance[idFinale] > u.distanza + w && posizione[idFinale] != -1) {
					distance[idFinale] = u.distanza + w;

					//DecreaseKey
					int32_t k = posizione[idFinale];
					if (distance[idFinale] < minHeap[k].distanza) {
						minHeap[k].distanza = distance[idFinale];
						int32_t parent = (k - 1) / 2;

						while (k > 0 && minHeap[parent].distanza > minHeap[k].distanza) {
							//scambio
							Node temp = minHeap[k];
							minHeap[k] = minHeap[parent];
							minHeap[parent] = temp;

							posizione[minHeap[k].indice] = k;
							posizione[minHeap[parent].indice] = parent;

							k = parent;
							parent = (k - 1) / 2;
						}
					}
				}

				//0 -1 1 (basso destra)
				xd = xp;
				zd = zp + 1;

				row_finale = zd;
				col_finale = xd + (zd - (zd & 1)) / 2;
				idFinale = row_finale * col + col_finale;
				if (row_finale >= 0 && row_finale < row && col_finale >= 0 && col_finale < col && idFinale >= 0 &&
				    idFinale < row * col && distance[idFinale] > u.distanza + w && posizione[idFinale] != -1) {
					distance[idFinale] = u.distanza + w;

					//DecreaseKey
					int32_t k = posizione[idFinale];
					if (distance[idFinale] < minHeap[k].distanza) {
						minHeap[k].distanza = distance[idFinale];
						int32_t parent = (k - 1) / 2;

						while (k > 0 && minHeap[parent].distanza > minHeap[k].distanza) {
							//scambio
							Node temp = minHeap[k];
							minHeap[k] = minHeap[parent];
							minHeap[parent] = temp;

							posizione[minHeap[k].indice] = k;
							posizione[minHeap[parent].indice] = parent;

							k = parent;
							parent = (k - 1) / 2;
						}
					}
				}

				//-1 0 1 (basso sinistra)
				xd = xp - 1;
				zd = zp + 1;

				row_finale = zd;
				col_finale = xd + (zd - (zd & 1)) / 2;
				idFinale = row_finale * col + col_finale;
				if (row_finale >= 0 && row_finale < row && col_finale >= 0 && col_finale < col && idFinale >= 0 &&
				    idFinale < row * col && distance[idFinale] > u.distanza + w && posizione[idFinale] != -1) {
					distance[idFinale] = u.distanza + w;

					//DecreaseKey
					int32_t k = posizione[idFinale];
					if (distance[idFinale] < minHeap[k].distanza) {
						minHeap[k].distanza = distance[idFinale];
						int32_t parent = (k - 1) / 2;

						while (k > 0 && minHeap[parent].distanza > minHeap[k].distanza) {
							//scambio
							Node temp = minHeap[k];
							minHeap[k] = minHeap[parent];
							minHeap[parent] = temp;

							posizione[minHeap[k].indice] = k;
							posizione[minHeap[parent].indice] = parent;

							k = parent;
							parent = (k - 1) / 2;
						}
					}
				}

				//-1 1 0 (sinistra)
				xd = xp - 1;
				zd = zp;

				row_finale = zd;
				col_finale = xd + (zd - (zd & 1)) / 2;
				idFinale = (row_finale * col) + col_finale;
				if (row_finale >= 0 && row_finale < row && col_finale >= 0 && col_finale < col && idFinale >= 0 &&
				    idFinale < row * col && distance[idFinale] > u.distanza + w && posizione[idFinale] != -1) {
					distance[idFinale] = u.distanza + w;

					//DecreaseKey
					int32_t k = posizione[idFinale];
					if (distance[idFinale] < minHeap[k].distanza) {
						minHeap[k].distanza = distance[idFinale];
						int32_t parent = (k - 1) / 2;

						while (k > 0 && minHeap[parent].distanza > minHeap[k].distanza) {
							//scambio
							Node temp = minHeap[k];
							minHeap[k] = minHeap[parent];
							minHeap[parent] = temp;

							posizione[minHeap[k].indice] = k;
							posizione[minHeap[parent].indice] = parent;

							k = parent;
							parent = (k - 1) / 2;
						}
					}
				}

				//0 1 -1 (alto sinistra)
				xd = xp;
				zd = zp - 1;

				row_finale = zd;
				col_finale = xd + (zd - (zd & 1)) / 2;
				idFinale = row_finale * col + col_finale;
				if (row_finale >= 0 && row_finale < row && col_finale >= 0 && col_finale < col && idFinale >= 0 &&
				    idFinale < row * col && distance[idFinale] > u.distanza + w && posizione[idFinale] != -1) {
					distance[idFinale] = u.distanza + w;

					//DecreaseKey
					int32_t k = posizione[idFinale];
					if (distance[idFinale] < minHeap[k].distanza) {
						minHeap[k].distanza = distance[idFinale];
						int32_t parent = (k - 1) / 2;

						while (k > 0 && minHeap[parent].distanza > minHeap[k].distanza) {
							//scambio
							Node temp = minHeap[k];
							minHeap[k] = minHeap[parent];
							minHeap[parent] = temp;

							posizione[minHeap[k].indice] = k;
							posizione[minHeap[parent].indice] = parent;

							k = parent;
							parent = (k - 1) / 2;
						}
					}
				}

				//air route
				for (int32_t i = 0; i < G[y][x].numAirRoute; i++) {
					idFinale = G[y][x].array[i].yDest * col + G[y][x].array[i].xDest;
					if (idFinale >= 0 && idFinale < row * col && distance[idFinale] > u.distanza + G[y][x].array[i].costAirRoute && posizione[idFinale] != -1) {
						distance[idFinale] = u.distanza + G[y][x].array[i].costAirRoute;

						//DecreaseKey
						int32_t k = posizione[idFinale];
						if (distance[idFinale] < minHeap[k].distanza) {
							minHeap[k].distanza = distance[idFinale];
							int32_t parent = (k - 1) / 2;

							while (k > 0 && minHeap[parent].distanza > minHeap[k].distanza) {
								//scambio
								Node temp = minHeap[k];
								minHeap[k] = minHeap[parent];
								minHeap[parent] = temp;

								posizione[minHeap[k].indice] = k;
								posizione[minHeap[parent].indice] = parent;

								k = parent;
								parent = (k - 1) / 2;
							}
						}
					}
				}
			}
		} else {
			flagInf = 1;
		}
	}

	// printf("MATRICE POSIZIONI:\n");
	// for (int i = 0; i < row; i++) {
	// 	for (int j = 0; j < col; j++) {
	// 		if (distance[i][j] < INF) {
	// 			printf("%d	", distance[i][j]);
	// 		}
	// 		else {
	// 			printf("INF	");
	// 		}
	// 	}
	// 	printf("\n");
	// }
	//
	// printf("\n\n");


	if (distance[idd] >= INF) {
		return -1;
	}

	return distance[idd];
}
