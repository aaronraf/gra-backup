# Cache Simulation und Analyse (A11)

## Beitrag jedes Gruppenmitglieds
- **Lie Aditya Bryan**:
- **Jovan Rio Tjandra**:
- **Aaron Rafael Thamin**:

## Übliche Größen und Latenzen

### Größen
- **Direkt-assoziativ**
    - **L1-Cache**: 64 KB - 128 KB
    - **L2-Cache**: 256 KB bis 512 KB
    - **L3-Cache**: 4 MB bis 8 MB
- **4-fach-assoziativ** (z.B. Macbook Air M1)
    - **L1-Cache**: 32 KB bis 64 KB
    - **L2-Cache**: 512 KB bis 1 MB
    - **L3-Cache**: 8 MB bis 64 MB

### Latenzen
| Komponent                 | Latenzen in Zyklen| 1 GHz (ns)       | 2 GHz (ns)       | 3 GHz (ns)       | 4 GHz (ns)       |  5 GHz (ns)      |
|---------------------------|-------------------|------------------|------------------|------------------|------------------|------------------|
| **L1-Cache**              | 4                 | 1 - 4            | 0,5 - 2          | 0,333 - 1,333    | 0.25 - 1         | 0.2 - 0.8        |
| **L2-Cache**              | 14                | 5 - 12           | 2,5 - 6          | 1,667 - 4        | 1,25 - 3         | 1 - 2,4          |
| **L3-Cache**              | 50 - 70           | 20 - 40          | 10 - 20          | 6,667 - 13,333   | 5 - 10           | 4 - 8            |
| **Hauptspeicher (RAM)**   | 50 - 150          | 50 - 150         | 25 - 75          | 16,667 - 50      | 12,5 - 37,5      | 10 - 30          |



## Implementierung
Die verwendeten Structs:
1. `Request` und `Result`: in der Aufgabenstellung definiert
2. `CacheConfig`: definieren Anzahl von Tag-, Offset- und Index Bits (abhängig von `cachelines` und `cacheline-size`)
3. `CacheAddress`: parsen `request.addr` zu Tag, Index, und Offset (abhängig vom übergebenen `CacheConfig`)

### Direkt-abgebildet
`DirectMappedCache` hat ein Array von `CacheEntry`-Objekten mit Größe `cachelines`.
Dieses `CacheEntry` verhält sich als das Index und enthält:
 - Tag, der später beim Zugriff verglichen wird
 - Array von Daten mit Größe von `number_of_offset_bits`$^2$

Speicherzugriffsverhalten:
1. Die Addresse werden anhand von `CacheAddress` geparst und der `current_entry` wird aus dem erhaltenen Index bestimmt
<!-- 2. Der Tag von `current_entry` wird mit dem übergebenen Tag auf Ungleichheit verglichen, oder ob der `current_entry` zum ersten Mal zugegriffen wird(Coldmiss) -->
2. `current_entry` wird mit dem übergebenen Tag auf Tag-Ungleichheit oder Coldmiss-Fall gecheckt
    - Falls ja: die Daten vom `current_entry` wird mit den richtigen Daten vom Hauptspeicher ersetzt und die `result.misses` wird inkrementiert
    - Sonst, die `result.hits` wird inkrementiert
3. Die Daten werden dann aus dem Cache gelesen oder in den Caches geschrieben (durch write-through: schreiben in den Cache $\to$ schreiben in den Hauptspeicher)

### 4-fach-assoziativ
`FourWayLRUCache` hat ein Array von `LRUCache` mit Größe von `number_of_offset_bits` $^2$

- `LRUCache` implementiert:
    <!-- - ein `HashMap` mit KV-Paar von Tag und den entsprechenden Node $\to$ Lesen hat Laufzeit `O(1)` -->
    - ein `HashMap` mit Tag (`Key`) und den entsprechenden Node (`Value`) $\to$ Lesen hat Laufzeit `O(1)`
    - ein `DoublyLinkedList` mit MRU $\harr$ LRU Struktur.
    - LRU als Ersetzungstrategie und bei jedem Schreiben/Lesen, wird LRU zu MRU

Speicherzugriffsverhalten
1. Die Addresse werden anhand von `CacheAddress` geparst und der richtigen `LRUCache` Node wird aus dem erhaltenden Index bestimmt
2. Das Tag wird auf seine Existenz in der Map geprüft
    - Falls nicht: die Daten vom `LRUCache` Node wird mit den richtigen Daten vom Hauptspeicher ersetzt und die `result.misses` wird inkrementiert
    - Falls ja: Überprüfen von Cold Miss
        - Falls ja: die Daten vom `LRUCache` Node wird mit den richtigen Daten vom Hauptspeicher ersetzt und die `result.misses` wird inkrementiert
        - Sonst, die `result.hits` wird inkrementiert
3. Die Daten werden dann aus dem Cache gelesen oder in den Cache geschrieben, und dieser Node wird zu MRU


Ersetzungsverhalten:
Wir nehmen auf einmal einen Block mit Größe von `number_of_offset_bits`$^2$ vom Hauptspeicher mit der Formel:
$$start\; address =  (address\; / \;total\; offset) * total\; offset$$
$$end\; address = start\; address * total\; offset- 1$$

get appropriate 4 (number_of_offset) byte data from ram

| Adresse | Formel                                                  | Fetched Address |
|--------|----------------------------------------------------------|-----------------|
| 1      | 1 / 4 = 0 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0 * 4 = 0 | 0 - 3           |
| 2      | 2 / 4 = 0 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0 * 4 = 0 | 0 - 3           |
| 5      | 5 / 4 = 1 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 1 * 4 = 4 | 4 - 7           |
| 9      | 9 / 4 = 2 &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 2 * 4 = 8 | 8 - 11          |

## Simulation
Matrixmultiplikation $C=A\times B \in \N^{4\times 4}$

### Cache Module
Diese `CacheModule` verwenden `Cache`, der polymorph ist und sich zur Laufzeit je nach Eingabe in `FourWayLRUCache` oder `DirectMappedCache` verwandelt

### CSV: inputs.csv
1. Initialisierung des Hauptspeichers.
2. Für jeden Eintrag $c\in C$ werden Einträge $a\in A$ und $b\in B$ gelesen und miteinander multipliziert.
3. Lesen von Zwischenwert in $c$.
5. Addieren den Zwischenwert und Produkt von entsprechenden $a$ und $b$.
## sc_main
Für jedes Zyklus, werden die Signale im `CacheModule` mit dem aktuellen `request` aktualisiert

Die Matrizen werden manuell gerechnet fürs Vergleichen
1. Initialisierung des Hauptspeichers.
2. Für jeden Eintrag $c\in C$ werden Einträge $a\in A$ und $b\in B$ gelesen und miteinander multipliziert.
3. Lesen von Zwischenwert in $c$.
4. Write to c the `request.data` and compare the `request.data` with c + a*b, if it's correct then the cache implementation stores data correctly.
4. `request.data` wird mit dem $c + a \times b$ verglichen. Sind beide gleich, dann funktioniert unsere Cacheimplemenitierung wie gemeint.

## Primitive Gate
Halbaddierer : 2 Gatter<br>
Volladdierer : 5 Gatter<br>

n-Bit Addition:<br>
- VA: (n-1)<br>
- HA: 1
 
n-Bit Multiplication: <br>
- AND Gates = n^2 <br>
- VA = n(n-2)<br>
- HA = n 

Eine 32-Bit Addition braucht VA * (32 - 1) + HA = 157<br>
Eine 32-Bit Multiplikation braucht 32 $^2$ + 32 * HA + 32(30)*FA = 5888

    Dieser Simulation enthält 16 Elemente in Matrix C und jedes Element besteht aus<br>
    4 Zwischenadditionen, also 64 * 157 = 10048<br>
    4 Multiplicationen, also 64 * 5888 = 376832<br>
    Insgesamt 386880 Gatter

### Simulationsergebnis
Aus der Simulation geht hervor, dass `result.misses` in `DirectMappedCache` > `FourWayLRUCache`, deswegen verwenden moderne Prozessoren n-fach-assoziativen Cache statt direct-abgebildeten Cache, da n-fach-assoziativer Cache eine ausgewogene Alternative zwischen direct-abgebildeter Cache und voll-assoziativer Cache, was ziemlich schnelle Suche und weniger Conflict-Misses anbietet.


<!-- da n-fach-assoziativer Cache ziemlich schnelle Suche(aufgrund der Existenz von Indizes) und weniger Misses im Vergleich zu direct-abgebildetem Cache anbietet. -->


 <!-- that's why modern processor use n-way set assoc than  because it offers balance between DM and fully associative which is pretty fast search and not as much trashing as in DM -->





<!-- ## Speicherzugriffsverhalten

- **Raumliche Lokalität (Zugriff auf nahegelegene Speicherplätze)**: Beide Caches profitieren davon, aber der 4-fach assoziative Cache nutzt die raumliche Lokalität besser aus, indem er mehrere nahegelegene Blöcke im selben Satz speichert.
- **Zeitliche Lokalität (erneuter Zugriff auf dieselben Speicherplätze kurz nach dem ersten Zugriff)**: Beide Cache-Speicher profitieren in ähnlicher Weise davon, obwohl der 4-fach-assoziative Cache potenziell mehrere Blöcke, auf die kürzlich zugegriffen wurde, im selben Satz behalten kann.
- **Sequentieller Zugriff**: Beide Caches bewältigen sequentielle Zugriffsmuster gut, aber der 4-fach-assoziative Cache kann aufgrund seiner größeren Flexibilität und geringeren Konfliktwahrscheinlichkeit besser mit Bursts sequentieller Zugriffe umgehen.
- **Zufälliger Zugriff**: Direkt assoziative Caches können stärker unter zufälligen Zugriffsmustern leiden, wenn verschiedene Speicherblöcke auf dieselbe Cache-Zeile abgebildet werden, was zu häufigen Cache-Misses führt. Der 4-fach assoziative Cache kann dieses Problem entschärfen, indem er mehr Möglichkeiten für die Zwischenspeicherung verschiedener Blöcke im gleichen Satz bietet. -->

<!-- ## Simulation eines speicherintensiven Algorithmus

Als Beispiel verwenden wir das Algorithmus für eine Matrixmultiplikation. -->

<!-- ## Beispiel für ein Speicherzugriffsmuster

```csv
# W/R, Adresse, Data

``` -->
