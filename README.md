# Progetto Finale di Algoritmi e Strutture Dati - A.A. 2024/2025

## Informazioni
Progetto finale del corso di Algoritmi e Strutture Dati del Politecnico di Milano per l'anno accademico 2024/2025.<br/>
**Studentessa:** Eleonora Rezza<br/><br/>

**Docente:** Davide Martinenghi<br/>
**Valutazione:** 24/30

## Obiettivo del progetto
Lo Scopo del progetto è realizzare in C, utilizzando le competenze apprese nel corso, un programma che simuli il software per il calcolo delle distanze tra due punti per un'azienda di trasporti. <br>
La mappa, che simula la Terra, è suddivisa in zone esagonali con diversi costi l'uno. La specifica richiede quattro comandi:
* **init ⟨_n. colonne_⟩ ⟨_n. righe_⟩:** inizializzare la mappa;
* **change_cost ⟨_x_⟩ ⟨_y_⟩ ⟨_v_⟩ ⟨_raggio_⟩:** cambiare il costo degli esagoni, in modo radiale, a partire da una posizione data e fornendo un raggio;
* **toggle_air_route ⟨_x1_⟩ ⟨_y1_⟩ ⟨_x2_⟩ ⟨_y2_⟩:** creare rotte aeree per raggiungere determinati esagoni. Se usato per una rotta già esistente, la rimuove;
* **travel_cost ⟨_xp_⟩ ⟨_yp_⟩ ⟨_xd_⟩ ⟨_yd_⟩:** richiedere il costo da un punto di partenza a uno di arrivo.
<br>
Per ulteriori informazioni [qui](https://github.com/eleoonoora/ProgettoAPI_2025/blob/master/Documents/Specifica_API_2025.pdf) è possibile trovare l'intera specifica.

## Scelte implementative

### Implementazione della mappa
Per gestire il movimento, il codice esegue una conversione tra Offset Coordinates e Axial/Cube Coordinates: viene estratta la componente x e y per calcolare la coordinata assiale xp e zp. <br>

      xp = x - (y - (y \& 1)) / 2 <br>
      zp = y <br>

Dopo aver calcolato la coordinata del vicino in formato assiale, il codice la riconverte in un indice di matrice.

      idFinale = row_finale * col + col_finale;

### Calcolo delle distanze
Per il calcolo delle distanze tra due esagoni, il programma utilizza l'algolirtmo di Dijkstra. A differenza dell'algoritmo per una matrice, in questo caso vengono, ogni volta, verificati gli esagoni nelle sei direzioni, più tutte le eventuali rotte aeree create fino a quel momento. 

### Cache
Visto che, similmente alla realà, alcune aree ed esagoni vengono visitate ripetutamente mentre altre mai, a simulare gli oceani, l'utilizzo di una cache con le ultime rotte calcolate aiuta a ridurre notevolmente i tempi di esecuzione. Vengono salvate, arbitrariamente, solamente 8 esagoni come partenze. Per ognuno degli 8 esagoni, sempre arbitrariamente, vengono salvate un massimo di 100 destinazioni con il relativo costo. <br>
Ogni volta che si inizializza la mappa o si modificano i tragitti con rotte aeree o cambiando i costi, la cache viene svuotata e ripopolata con i successivi calcoli delle rotte.<br>

