// Grafo.hpp - Grafo dirigido y pesado con listas de adyacencia (C++98)
//
// Los vertices son enteros >= 1. La lista de vertices y cada lista de
// adyacencia se mantienen ordenadas ascendentemente.
//
// Los metodos que devuelven resultados por vertice (BFS, DFS,
// obtenerGradosDeEntrada) reciben std::vector<int>& y los redimensionan
// ellos mismos a maxVertice() + 1 posiciones; se indexan por numero de
// vertice (la posicion 0 no se usa).
#ifndef GRAFO_HPP
#define GRAFO_HPP

#include <climits>
#include <iosfwd>
#include <list>
#include <string>
#include <vector>
#include <limits>
#include <utility>
#include "Vertice.hpp"

const int INFINITO = INT_MAX;
// Distancia de un vertice inalcanzable en Dijkstra
const double DIST_INFINITA = std::numeric_limits<double>::infinity();

// Los nodos son un detalle interno: el cliente de Grafo nunca los ve.
class NodoVertice;

// Arista de un grafo no dirigido (resultado de Kruskal, entrada de
// construirDesdeAristas). No expone ningun nodo interno.
struct Arista {
    Vertice origen;
    Vertice destino;
    double costo;
    Arista() : origen(0), destino(0), costo(0.0) {}
    Arista(Vertice o, Vertice d, double c) : origen(o), destino(d), costo(c) {}
};

class Grafo {
private:
    NodoVertice* g;
    int cantVertices;

    // Copia deshabilitada (idioma C++98)
    Grafo(const Grafo&);
    Grafo& operator=(const Grafo&);

    NodoVertice* buscarVertice(Vertice v) const;
    bool quitarDeAdyacencia(NodoVertice* origen, Vertice w);
    void DFS_Visitar(Vertice u, std::vector<int>& pred, std::vector<int>& tdesc,
                     std::vector<int>& tfinal, std::vector<int>& color,
                     int& tiempo) const;
    void recorridoDFS_Visitar(Vertice u, std::vector<int>& visitado,
                              std::list<Vertice>& orden) const;
    void DFS_VisitarTopo(Vertice u, std::vector<int>& color,
                         std::list<Vertice>& ord) const;
    bool cicloDirigidoVisitar(Vertice u, const std::vector<NodoVertice*>& idx,
                              std::vector<int>& color) const;
    bool cicloNoDirigidoVisitar(Vertice u, Vertice padre,
                                const std::vector<NodoVertice*>& idx,
                                std::vector<int>& visitado) const;
    void buscarCiclos(bool noDirigido, bool soloContar, int maxCiclos,
                      bool& truncado, std::list<std::list<Vertice> >* salida,
                      int& cantidad) const;
    bool buscarCicloHamiltoniano(bool noDirigido, std::list<Vertice>* ciclo) const;
    // Todo el uso de punteros de caminoSimple y esIsomorfo vive en estos dos
    // metodos privados; los publicos solo trabajan con vectores y listas.
    // BFS desde u que se detiene al llegar a v: pred[x] = vertice anterior a
    // x en el recorrido (0 si no fue alcanzado). Devuelve true si v es
    // alcanzable desde u. Pre: u y v existen.
    bool buscarCaminoBFS(Vertice u, Vertice v, std::vector<int>& pred) const;
    // reverseGraph: todo el uso de punteros vive aqui. Reengancha los nodos
    // de adyacencia existentes (no crea ni borra ninguno). O(n + m).
    void invertirArcos();
    // Matriz de adyacencia indexada por posicion del vertice en la lista
    // (0 .. n-1, en orden ascendente de numero de vertice): ady[i][j] = 1 si
    // existe el arco i -> j (un bucle es ady[i][i]); costo[i][j] = su costo.
    void matrizAdyacencia(std::vector<std::vector<char> >& ady,
                          std::vector<std::vector<double> >& costo) const;
    // caminoEulerianoDesde: todo el uso de punteros vive aqui. Carga los arcos
    // en ady, indexado por numero de vertice (tam posiciones): ady[u] tiene un
    // par (w, cantidad) por cada arco u -> w, en orden ascendente de w. Con
    // costoEsCantidad = true, cantidad es el costo del arco (debe ser un
    // entero >= 1; si algun arco no lo es devuelve false); si no, vale 1.
    bool cargarMultiadyacencia(bool costoEsCantidad, int tam,
                               std::vector<std::vector<std::pair<int, int> > >& ady) const;
    // Ampliacion: reemplaza el contenido de este grafo por la cadena de nodos
    // dada (la toma sin copiarla; el contenido anterior se libera).
    void tomar(NodoVertice* cabeza, int cantidad);
    // Ampliacion: copia este grafo en destino (destino debe ser otro objeto);
    // con costosUnitarios = true todos los arcos copiados cuestan 1.0.
    void copiarA(Grafo& destino, bool costosUnitarios) const;
    // Ampliacion: nucleo comun de kruskal (minimo) y kruskalMaximo.
    std::list<Arista> kruskalGeneral(bool maximo, double& pesoTotal, bool& conexo) const;

public:
    Grafo();
    ~Grafo();

    void construir();
    bool esVacio() const;
    bool agregarVertice(Vertice v);
    bool agregarArco(Vertice v, Vertice w, double c);
    bool eliminarVertice(Vertice v);
    bool eliminarArco(Vertice v, Vertice w);
    // Grafo no dirigido = arco en ambos sentidos
    bool agregarArcoNoDirigido(Vertice v, Vertice w, double c);
    bool eliminarArcoNoDirigido(Vertice v, Vertice w);
    bool existeVertice(Vertice v) const;
    bool existeArco(Vertice v, Vertice w) const;
    double costoArco(Vertice v, Vertice w) const;   // -1.0 si no existe el arco
    int orden() const;
    // Grados de un vertice (-1 si el vertice no existe)
    int gradoExterior(Vertice v) const;   // arcos que salen de v
    int gradoInterior(Vertice v) const;   // arcos que llegan a v
    int grado(Vertice v) const;           // gradoInterior + gradoExterior
    Vertice maxVertice() const;                     // 0 si el grafo es vacio
    std::list<Vertice> predecesores(Vertice v) const;
    std::list<Vertice> sucesores(Vertice v) const;
    void destruir();

    // Recorridos
    void BFS(Vertice s, std::vector<int>& dist, std::vector<int>& pred) const;
    // Deteccion de ciclos, ambas O(n + m).
    // Grafo dirigido: hay ciclo si un DFS llega a un vertice "gris" (aun
    // abierto en la recursion). Un bucle (v, v) es un ciclo.
    bool tieneCiclos() const;
    // Grafo no dirigido (cada arista guardada en ambos sentidos, ver
    // agregarArcoNoDirigido): hay ciclo si un DFS llega a un vertice ya
    // visitado que no es su padre. Un bucle (v, v) es un ciclo.
    // Pre: el grafo es simetrico (solo se uso agregarArcoNoDirigido).
    bool tieneCiclosNoDirigido() const;
    // Enumeracion de ciclos (algoritmo de Johnson). Cada ciclo es una lista
    // de vertices en orden de recorrido, empezando por su vertice menor y
    // sin repetir el primero al final: [1,2,3] es 1->2->3->1.
    // maxCiclos: tope de ciclos a devolver; 0 (o negativo) = sin tope.
    // truncado: queda en true solo si existen mas ciclos que maxCiclos.
    // ADVERTENCIA: el numero de ciclos puede ser exponencial en n; usar un
    // tope en grafos grandes o densos.
    //
    // Dirigidos: O((n + m)(c + 1)). Un bucle (v, v) es un ciclo [v].
    std::list<std::list<Vertice> > enumerarCiclos(int maxCiclos, bool& truncado) const;
    int contarCiclos(int maxCiclos, bool& truncado) const;   // sin guardar los ciclos
    // No dirigidos (grafo simetrico, ver agregarArcoNoDirigido): cada ciclo
    // se devuelve una sola vez (no en ambos sentidos) y la ida y vuelta por
    // una misma arista no cuenta. Un bucle (v, v) es un ciclo [v].
    // Costo proporcional a (n + m)(c + m); un bosque se descarta en O(n + m).
    std::list<std::list<Vertice> > enumerarCiclosNoDirigido(int maxCiclos, bool& truncado) const;
    int contarCiclosNoDirigido(int maxCiclos, bool& truncado) const;
    // Numero de componentes conexas, O(n + m) (Union-Find). Ignora el sentido
    // de los arcos: en un grafo no dirigido (simetrico) son sus componentes
    // conexas; en uno dirigido son las componentes DEBILMENTE conexas. Un
    // vertice aislado es una componente; el grafo vacio tiene 0.
    int componentesConexas() const;
    // Grafo bipartito, O(n + m) (BFS con dos colores). Ignora el sentido de
    // los arcos, igual que componentesConexas: en un grafo no dirigido
    // (simetrico) es el bipartito de siempre; en uno dirigido se estudia el
    // grafo subyacente. Un grafo es bipartito si sus vertices se pueden
    // repartir en dos conjuntos sin ninguna arista dentro de un mismo
    // conjunto (equivale a no tener ciclos de longitud impar). Un bucle
    // (v, v) lo impide. El grafo vacio y los grafos sin arcos son bipartitos.
    bool esBipartito() const;
    // Igual, y si lo es devuelve en A y B los dos conjuntos (cada uno en
    // orden ascendente; los vertices aislados quedan en A). Si no es
    // bipartito, A y B quedan vacios.
    bool esBipartito(std::list<Vertice>& A, std::list<Vertice>& B) const;
    // Ciclo euleriano: recorrido cerrado que usa CADA arco exactamente una
    // vez. Los vertices sin arcos se ignoran. O(n + m). Un grafo sin arcos
    // se considera que lo tiene (el recorrido vacio).
    // Grafo dirigido: gradoInterior(v) == gradoExterior(v) para todo v y
    // todos los arcos en una misma componente (debilmente) conexa.
    bool tieneCicloEuleriano() const;
    // Grafo no dirigido (simetrico, ver agregarArcoNoDirigido): todos los
    // vertices con grado par (un bucle suma 2) y todas las aristas en una
    // misma componente conexa.
    // Pre: el grafo es simetrico (solo se uso agregarArcoNoDirigido).
    bool tieneCicloEulerianoNoDirigido() const;
    // Ciclo hamiltoniano: ciclo simple que pasa por TODOS los vertices
    // exactamente una vez. Problema NP-completo: se resuelve con backtracking
    // (con una poda previa por grados), de costo exponencial en el peor caso;
    // pensado para grafos pequenos (unos 20-25 vertices). Los bucles no
    // sirven y se ignoran. El grafo vacio no tiene ciclo hamiltoniano.
    // Dirigido: sigue el sentido de los arcos. Un grafo de un solo vertice
    // lo tiene solo si tiene un bucle (el ciclo [v]); con dos vertices hacen
    // falta los arcos (v, w) y (w, v).
    bool tieneCicloHamiltoniano() const;
    // Igual, y si existe devuelve en ciclo los vertices en orden de
    // recorrido, empezando por el menor y sin repetirlo al final: [1,2,3]
    // es 1->2->3->1. Si no existe, la lista queda vacia.
    bool tieneCicloHamiltoniano(std::list<Vertice>& ciclo) const;
    // No dirigido (simetrico, ver agregarArcoNoDirigido): igual, pero exige
    // al menos 3 vertices (la ida y vuelta por una arista no es un ciclo).
    // Pre: el grafo es simetrico (solo se uso agregarArcoNoDirigido).
    bool tieneCicloHamiltonianoNoDirigido() const;
    bool tieneCicloHamiltonianoNoDirigido(std::list<Vertice>& ciclo) const;
    // Excentricidad de v: la mayor de las distancias minimas de v a los
    // demas vertices, con distancia = suma de costos del camino (Dijkstra;
    // con costos todos iguales a 1 es el numero de arcos). En un grafo
    // dirigido se miden las distancias que SALEN de v. Si algun vertice es
    // inalcanzable desde v devuelve DIST_INFINITA. Devuelve -1.0 si v no
    // existe o si algun arco tiene costo negativo. Un grafo de un solo
    // vertice da 0. O((n + m) log n).
    double excentricidad(Vertice v) const;
    // Radio: la menor excentricidad entre todos los vertices. DIST_INFINITA
    // si ningun vertice alcanza a todos los demas (grafo no conexo, o no
    // fuertemente conexo si es dirigido). Devuelve -1.0 si el grafo es vacio
    // o si algun arco tiene costo negativo. O(n (n + m) log n).
    double radio() const;
    // Planaridad: true si el grafo se puede dibujar en el plano sin que se
    // crucen sus aristas. Ignora el sentido de los arcos, los bucles y los
    // arcos repetidos en ambos sentidos (se estudia el grafo simple
    // subyacente), igual que componentesConexas. Un grafo se descompone en
    // sus bloques (componentes biconexas) y es planar si y solo si lo es
    // cada bloque; cada bloque se prueba con el algoritmo DMP
    // (Demoucron-Malgrange-Pertuiset), con descarte previo por la cota
    // m <= 3n - 6. Costo O(n^2) por bloque (O(n^3) en el peor caso del
    // codigo actual), adecuado para grafos pequenos o medianos.
    // El grafo vacio y los grafos sin ciclos son planares.
    bool esPlanar() const;
    // Puntos de articulacion (vertices de corte): los vertices cuya
    // eliminacion aumenta el numero de componentes conexas. Ignora el sentido
    // de los arcos, los bucles y los arcos repetidos en ambos sentidos, igual
    // que componentesConexas. Resultado en orden ascendente, sin repetidos;
    // lista vacia si no hay (grafo vacio, biconexo, o con menos de 3
    // vertices). Algoritmo de Tarjan (DFS con low-link), O(n + m).
    std::list<Vertice> puntosArticulacion() const;
    // Arbol de expansion minima (Kruskal con Union-Find), O(m log m).
    // Devuelve las aristas del arbol en el orden en que fueron aceptadas
    // (de menor a mayor costo; NO forman un camino encadenado). Cada arista
    // aparece una sola vez, con origen < destino. pesoTotal = suma de costos.
    // conexo = false si el grafo no es conexo: entonces el resultado es un
    // bosque de expansion minima (n - c aristas, c = componentes). Un grafo
    // vacio o de un solo vertice cuenta como conexo.
    // Pre: grafo no dirigido y simetrico (ver agregarArcoNoDirigido). Admite
    // costos negativos; los bucles nunca forman parte del resultado.
    std::list<Arista> kruskal(double& pesoTotal, bool& conexo) const;
    // Reconstruye ESTE grafo (descarta su contenido) con los vertices que
    // aparecen en las aristas dadas y un arco por arista; si noDirigido es
    // true se agrega tambien el arco inverso. Un vertice sin ninguna arista
    // no aparece. Devuelve false si alguna arista tiene un vertice < 1.
    bool construirDesdeAristas(const std::list<Arista>& aristas, bool noDirigido);
    // Dibujo del grafo en un archivo SVG (se abre con cualquier navegador).
    // Los vertices se colocan en un circulo; cada arco es una linea con su
    // costo (con flecha si el grafo es dirigido; si hay arcos en ambos
    // sentidos se dibujan como curvas). Con noDirigido = true se dibuja una
    // sola linea sin flechas por cada par de vertices. Un bucle es un
    // circulo pequeno junto al vertice.
    // archivo: ruta relativa (se crea en el directorio desde el que se
    // ejecuta el programa) o absoluta; el directorio debe existir y un
    // archivo previo se sobrescribe. Devuelve false si no se pudo escribir.
    // etiquetas (opcional): texto de cada vertice, indexado por numero de
    // vertice; si falta o esta vacio se escribe el numero.
    // Pensado para grafos pequenos (hasta unos 20 vertices): con grafos
    // densos o no planares habra cruces de lineas.
    bool exportarSVG(const std::string& archivo, bool noDirigido) const;
    bool exportarSVG(const std::string& archivo, bool noDirigido,
                     const std::vector<std::string>& etiquetas) const;
    // Dijkstra con std::priority_queue: distancia minima de s a todos los
    // vertices (pesos >= 0). dist y pred se indexan por numero de vertice
    // (maxVertice() + 1 posiciones). Inalcanzable: dist = DIST_INFINITA,
    // pred = 0. Devuelve false (y dist/pred no son validos) si s no existe
    // o si algun arco tiene costo negativo.
    bool dijkstra(Vertice s, std::vector<double>& dist, std::vector<int>& pred) const;
    // Version simple: devuelve solo las distancias (vector vacio si falla)
    std::vector<double> dijkstra(Vertice s) const;
    // Bellman-Ford: distancia minima de s a todos los vertices, admitiendo
    // costos NEGATIVOS (Dijkstra los rechaza). dist y pred se indexan por
    // numero de vertice (maxVertice() + 1 posiciones). Inalcanzable:
    // dist = DIST_INFINITA, pred = 0. Devuelve false (y dist/pred no son
    // validos) si s no existe o si desde s se alcanza un ciclo de costo
    // total negativo (entonces no hay distancia minima). Ojo: una arista no
    // dirigida con costo negativo es un ciclo negativo (ida y vuelta).
    // O(n * m); con costos >= 0 conviene dijkstra, que es mas rapido.
    bool bellmanFord(Vertice s, std::vector<double>& dist, std::vector<int>& pred) const;
    // Version simple: devuelve solo las distancias (vector vacio si falla)
    std::vector<double> bellmanFord(Vertice s) const;
    // Floyd-Warshall: distancia minima entre TODOS los pares de vertices,
    // admitiendo costos negativos. dist es una matriz que se indexa por
    // numero de vertice: dist[u][v] (maxVertice() + 1 filas y columnas; la
    // fila y columna 0 no se usan). dist[v][v] = 0; inalcanzable =
    // DIST_INFINITA (tambien las posiciones de numeros que no son vertices).
    // Devuelve false (y dist/sig no son validos) si el grafo tiene algun
    // ciclo de costo total negativo (no importa desde donde), incluido un
    // bucle negativo o una arista no dirigida con costo negativo (ida y
    // vuelta). O(n^3) en tiempo y O(maxVertice^2) en memoria: pensado para
    // grafos pequenos o medianos.
    bool floydWarshall(std::vector<std::vector<double> >& dist) const;
    // Ademas devuelve sig: sig[u][v] = vertice que sigue a u en un camino
    // minimo de u a v (0 si u == v o v es inalcanzable desde u). Con sig se
    // reconstruye cualquier camino con reconstruirCamino.
    bool floydWarshall(std::vector<std::vector<double> >& dist,
                       std::vector<std::vector<int> >& sig) const;
    // Camino minimo de u a v (u y v incluidos) a partir de la matriz sig de
    // floydWarshall. Lista vacia si u o v no existen, si v es inalcanzable
    // desde u o si sig no corresponde a este grafo. Si u == v: [u]. O(n).
    std::list<Vertice> reconstruirCamino(const std::vector<std::vector<int> >& sig,
                                         Vertice u, Vertice v) const;
    // Ruta de MAYOR costo desde s hacia cada vertice (camino SIMPLE: no repite
    // vertices; el costo es la suma de los costos de sus arcos, de cualquier
    // signo). dist[v] = costo de la mejor ruta s -> v y ruta[v] = sus
    // vertices en orden, s y v incluidos; ambos se indexan por numero de
    // vertice (maxVertice() + 1 posiciones). dist[s] = 0 y ruta[s] = [s]
    // (no se cuentan ciclos de vuelta a s). Inalcanzable: dist = -DIST_INFINITA
    // y ruta vacia. Si hay empate, se devuelve una de las rutas.
    // Devuelve false (y dist/ruta no son validos) si s no existe.
    // Se devuelve la ruta completa y no un arreglo de predecesores porque en
    // un camino de mayor costo el prefijo hacia un vertice intermedio no
    // tiene por que ser su mejor ruta.
    // Costo: si lo alcanzable desde s no tiene ciclos (un DAG; un bucle es un
    // ciclo) se resuelve en O(n + m) con programacion dinamica sobre el orden
    // topologico. Si hay ciclos es el problema del camino simple mas largo
    // (NP-completo) y se resuelve por backtracking, de costo exponencial en
    // el peor caso: pensado para grafos pequenos (unos 20-25 vertices). En un
    // grafo no dirigido (simetrico) siempre hay ciclos, asi que usa el
    // backtracking.
    bool rutaMayorCosto(Vertice s, std::vector<double>& dist,
                        std::vector<std::list<Vertice> >& ruta) const;
    // Camino SIMPLE (sin vertices repetidos) de u a v siguiendo el sentido de
    // los arcos, con u y v incluidos. Es un camino de MENOS arcos (BFS), no
    // el de menor costo (para eso estan dijkstra / bellmanFord / floydWarshall).
    // Lista vacia si u o v no existen o si v no es alcanzable desde u. Si
    // u == v devuelve [u]. En un grafo no dirigido (simetrico) sirve tal cual.
    // O(n + m).
    std::list<Vertice> caminoSimple(Vertice u, Vertice v) const;
    // Parcial IV, problema 1: invierte la direccion de todos los arcos: cada
    // arco (u, v) pasa a ser (v, u) conservando su costo. Los bucles quedan
    // igual. Las listas de adyacencia siguen ordenadas. O(n + m).
    void reverseGraph();
    // Parcial IV, problema 2: camino con menos arcos de v a w que pasa
    // obligatoriamente por intermedio (grafo no dirigido, no ponderado). Es
    // el camino minimo v -> intermedio seguido del minimo intermedio -> w;
    // intermedio aparece una sola vez. Si hace falta puede repetir vertices
    // (es un recorrido, y es el de menos arcos). Lista vacia si algun vertice
    // no existe o no hay tal camino. O(n + m).
    std::list<Vertice> getCaminoMasCortoArcos(Vertice v, Vertice w, Vertice intermedio) const;
    // Isomorfismo: true si existe una biyeccion entre los vertices de este
    // grafo y los de otro que conserva los arcos (u -> v existe en uno si y
    // solo si f(u) -> f(v) existe en el otro; un bucle se corresponde con un
    // bucle). Los numeros de vertice NO importan, solo la estructura: {1,2}
    // con 1->2 es isomorfo a {5,9} con 9->5. Respeta el sentido de los arcos
    // (en un grafo no dirigido simetrico equivale al isomorfismo de siempre).
    // Los costos se ignoran; con compararCostos = true tambien deben coincidir
    // (comparacion exacta) los costos de arcos correspondientes. Dos grafos
    // vacios son isomorfos.
    // Problema sin algoritmo polinomial conocido: se descartan primero los
    // casos faciles (orden, numero de arcos, refinamiento de colores por
    // grados y vecinos) y el resto se resuelve por backtracking, exponencial
    // en el peor caso (grafos muy regulares); sobra para grafos pequenos y
    // medianos.
    bool esIsomorfo(const Grafo& otro) const;
    bool esIsomorfo(const Grafo& otro, bool compararCostos) const;
    // Camino de MENOR COSTO de u a v (u y v incluidos), con Dijkstra: el
    // costo de un camino es la suma de los costos de sus arcos. costo recibe
    // ese total. Lista vacia (y costo = -1.0) si u o v no existen, si v no es
    // alcanzable desde u o si algun arco tiene costo negativo (Dijkstra no lo
    // admite; para costos negativos usar bellmanFord o floydWarshall). Si
    // u == v devuelve [u] con costo 0. Con costos todos iguales a 1 es un
    // camino de menos arcos. En un grafo no dirigido (simetrico) sirve tal
    // cual. O((n + m) log n).
    std::list<Vertice> caminoMenorCosto(Vertice u, Vertice v, double& costo) const;
    std::list<Vertice> caminoMenorCosto(Vertice u, Vertice v) const;
    // Todos los vertices alcanzables desde s siguiendo el sentido de los
    // arcos (s incluido), ordenados de menor a mayor. O(n + m). Lista vacia
    // si s no existe. Es el mismo conjunto que recorridoBFS(s), pero en
    // orden ascendente en lugar de orden de visita.
    std::list<Vertice> alcanzables(Vertice s) const;
    // Vertices alcanzables desde s, en el orden en que BFS los visita
    std::list<Vertice> recorridoBFS(Vertice s) const;
    // Vertices alcanzables desde s, en el orden en que DFS los descubre
    // (s primero; los sucesores se exploran en orden ascendente). Lista vacia
    // si s no existe. O(n + m). Es el orden de tdesc restringido a lo alcanzable.
    std::list<Vertice> recorridoDFS(Vertice s) const;
    void DFS(std::vector<int>& pred, std::vector<int>& tdesc,
             std::vector<int>& tfinal) const;

    // Auxiliares O(n + m)
    void obtenerGradosDeEntrada(std::vector<int>& gradoDeEntrada) const;
    std::list<Vertice> obtenerVerticesFuente() const;

    // Ordenamiento topologico (solo valido en grafos aciclicos; si hay un
    // ciclo, la lista resultante tiene menos de orden() vertices)
    std::list<Vertice> ordenamientoTopologico() const;     // version BFS
    std::list<Vertice> ordenamientoTopologicoDFS() const;  // version DFS

    // =====================================================================
    // AMPLIACION DE LA BIBLIOTECA
    // Salvo que se indique lo contrario, los metodos "NoDirigido" y los que
    // hablan de aristas suponen un grafo simetrico (solo se uso
    // agregarArcoNoDirigido) y los demas trabajan con el sentido de los arcos.
    // =====================================================================

    // ---------------------------------------------------- Copia y tamano
    // Reemplaza el contenido de este grafo por una copia de otro (vertices,
    // arcos y costos). O(n + m). La copia por constructor/asignacion sigue
    // deshabilitada a proposito: asi las copias siempre son explicitas.
    void copiar(const Grafo& otro);
    // Intercambia el contenido de este grafo con el de otro en O(1).
    void intercambiar(Grafo& otro);
    // Numero de arcos (un bucle cuenta como uno). O(n + m).
    int cantidadArcos() const;
    // Numero de aristas del grafo subyacente: pares {u, v} distintos con
    // algun arco entre ellos (en cualquier sentido) mas los bucles. En un
    // grafo simetrico es el numero de aristas de siempre. O(m log m).
    int cantidadAristas() const;
    // true si todo arco (u, v) tiene su inverso (v, u) (los bucles se
    // cumplen solos). Con compararCostos = true tambien deben coincidir los
    // costos de (u, v) y (v, u). El grafo vacio es simetrico.
    bool esSimetrico() const;
    bool esSimetrico(bool compararCostos) const;
    // Contrario de esSimetrico(): hay algun arco sin inverso.
    bool esDirigido() const;
    // Densidad = arcos (sin contar bucles) / (n (n - 1)): 1.0 en un grafo
    // completo. 0.0 si n < 2. En un grafo simetrico coincide con la densidad
    // no dirigida aristas / C(n, 2).
    double densidad() const;

    // --------------------------------- Modificacion de arcos y vertices
    // Agrega el arco solo si no existe (no toca el costo de uno existente;
    // agregarArco, en cambio, lo actualiza). false si ya existia o si v o w
    // no existen.
    bool agregarArcoSiNoExiste(Vertice v, Vertice w, double c);
    // Cambia el costo de un arco EXISTENTE. false si el arco no existe.
    bool actualizarCosto(Vertice v, Vertice w, double c);
    // Igual para los dos sentidos; false (sin cambios) si falta alguno.
    bool actualizarCostoNoDirigido(Vertice v, Vertice w, double c);
    // Borran los arcos que salen de v / llegan a v / ambos, SIN borrar el
    // vertice. Devuelven cuantos arcos quitaron (-1 si v no existe). Un
    // bucle (v, v) cuenta una sola vez.
    int eliminarArcosDe(Vertice v);
    int eliminarArcosHacia(Vertice v);
    int aislarVertice(Vertice v);
    // Cambia el numero del vertice v por w conservando todos sus arcos y
    // costos (un bucle sigue siendo bucle). false si v no existe, si w < 1 o
    // si w ya existe. Si v == w no hace nada y devuelve true. O(n + m).
    bool renombrarVertice(Vertice v, Vertice w);
    // Contrae el vertice v dentro de u: v desaparece y sus arcos pasan a u.
    // Los arcos entre u y v se descartan (no se crea ningun bucle; los que ya
    // existian se conservan como bucle de u). Si dos arcos se funden en uno
    // se queda el de menor costo. false si u == v o alguno no existe.
    bool contraerArista(Vertice u, Vertice v);
    // Union: este grafo pasa a ser G1 U G2 (vertices y arcos de ambos). Si un
    // arco esta en los dos se conserva el costo de ESTE grafo. O(n + m).
    void unir(const Grafo& otro);
    // Interseccion: este grafo pasa a tener solo los vertices que estan en
    // ambos y los arcos que estan en ambos (con el costo de ESTE grafo).
    void interseccion(const Grafo& otro);
    // Subgrafo inducido: resultado recibe los vertices de vs y los arcos de
    // este grafo entre ellos. false (resultado intacto) si algun vertice de
    // vs no existe. resultado puede ser este mismo grafo.
    bool subgrafo(const std::list<Vertice>& vs, Grafo& resultado) const;
    // Complemento: mismos vertices y un arco (u, v) de costo 1.0 por cada par
    // ordenado u != v SIN arco en este grafo (no hay bucles). O(n^2).
    // resultado puede ser este mismo grafo.
    void complemento(Grafo& resultado) const;

    // ------------------------------------------------ Estructura del grafo
    // Componentes FUERTEMENTE conexas (Tarjan, O(n + m)): cada componente es
    // una lista ascendente de vertices y las componentes salen ordenadas por
    // su vertice menor. En un grafo simetrico coinciden con las conexas.
    std::list<std::list<Vertice> > componentesFuertementeConexas() const;
    // true si de todo vertice se llega a todo otro siguiendo los arcos. El
    // grafo vacio NO lo es.
    bool esFuertementeConexo() const;
    // true si el grafo subyacente (sin sentido de arcos) tiene una sola
    // componente. El grafo vacio no es conexo.
    bool esConexo() const;
    // Puentes (aristas de corte): las aristas cuya eliminacion aumenta el
    // numero de componentes. Trabaja sobre el grafo simple subyacente (ignora
    // sentidos, bucles y arcos repetidos), como puntosArticulacion. Cada
    // puente sale una vez con origen < destino y el costo del arco (u, v) (o
    // el de (v, u) si solo existe ese), en orden ascendente. Tarjan, O(n + m).
    std::list<Arista> puentes() const;
    // Biconexo: conexo, con al menos 2 vertices y sin puntos de articulacion
    // (K2 cuenta como biconexo). Grafo subyacente.
    bool esBiconexo() const;
    // Sobre el grafo subyacente: sin ciclos (los bucles cuentan como ciclo).
    // El grafo vacio es un bosque.
    bool esBosque() const;
    // Bosque conexo con al menos un vertice.
    bool esArbol() const;
    // Arbol DIRIGIDO (arborescencia): sigue el sentido de los arcos, que van del
    // padre al hijo. Hay exactamente un vertice sin arcos entrantes (la raiz),
    // todos los demas reciben exactamente uno y todos son alcanzables desde la
    // raiz. Un bucle o un ciclo lo impiden. El grafo vacio no es un arbol; un
    // solo vertice sin arcos si. O(n + m). (esArbol ignora el sentido.)
    bool esArbolDirigido() const;
    // Igual, pero exigiendo que la raiz sea el vertice dado: false si raiz no
    // existe, si recibe algun arco o si no alcanza a todos los vertices. O(n + m).
    bool esArbolDirigido(Vertice raiz) const;
    // Dirigido sin ciclos (un bucle es un ciclo); el grafo vacio es un DAG.
    bool esDAG() const;
    // Todo par de vertices distintos tiene los arcos en ambos sentidos
    // (los bucles no importan). El grafo vacio y el de un vertice lo son.
    bool esCompleto() const;
    // Todos los vertices con el mismo gradoExterior y el mismo gradoInterior
    // (en un grafo simetrico: regular de siempre). El grafo vacio lo es.
    // En la version con k, k recibe ese grado exterior comun (-1 si no es
    // regular o el grafo es vacio).
    bool esRegular() const;
    bool esRegular(int& k) const;
    // Longitud (en arcos) del ciclo mas corto; INFINITO si no hay ciclos.
    // cintura(): ciclos dirigidos (un bucle mide 1; u->v->u mide 2).
    // cinturaNoDirigido(): grafo simetrico, la ida y vuelta por una misma
    // arista no cuenta (un bucle mide 1, el ciclo mas corto de verdad es >=
    // 3). O(n (n + m)).
    int cintura() const;
    int cinturaNoDirigido() const;
    // Diametro: la mayor excentricidad (mayor distancia minima entre dos
    // vertices, con costos como longitud). DIST_INFINITA si algun vertice no
    // alcanza a otro; -1.0 si el grafo es vacio o hay costos negativos.
    double diametro() const;
    // Centro: los vertices cuya excentricidad es igual al radio, ascendente.
    // Lista vacia si el grafo es vacio, hay costos negativos o ningun vertice
    // alcanza a todos los demas.
    std::list<Vertice> centro() const;
    // Clausura transitiva (Warshall, O(n^3)): alcanza[u][v] = 1 si existe un
    // camino de UNO o mas arcos de u a v. Se indexa por numero de vertice
    // (maxVertice() + 1 filas y columnas). alcanza[v][v] = 1 solo si v esta
    // en un ciclo (o tiene bucle).
    void clausuraTransitiva(std::vector<std::vector<char> >& alcanza) const;

    // ---------------------------------------------------- Caminos
    // true si v es alcanzable desde u siguiendo los arcos (u == v: true si
    // existe). false si alguno no existe. O(n + m).
    bool existeCamino(Vertice u, Vertice v) const;
    // Camino SIMPLE con MAS arcos de u a v (u y v incluidos); vacio si no hay
    // camino o alguno no existe. Es rutaMayorCosto con todos los costos en 1:
    // O(n + m) si lo alcanzable desde u no tiene ciclos y exponencial (camino
    // simple mas largo, NP-completo) si los hay, p. ej. en un grafo no
    // dirigido: pensado para unos 20-25 vertices.
    std::list<Vertice> caminoMasLargoArcos(Vertice u, Vertice v) const;
    // Camino simple con mas arcos de TODO el grafo (vacio si no hay
    // vertices; un solo vertice si no hay arcos). Mismo costo que arriba por
    // cada origen posible.
    std::list<Vertice> caminoMasLargoArcos() const;
    // Los k caminos SIMPLES de menor costo de u a v (algoritmo de Yen), de
    // menor a mayor costo (entre caminos de igual costo el orden no esta
    // especificado, ni cual de ellos entra si el empate cae en el corte de
    // k). Puede devolver menos de k si no hay tantos. costos recibe el costo de cada
    // uno. Costos >= 0; con algun costo negativo, o si u o v no existen,
    // devuelve vacio. u == v: un unico camino [u]. Los bucles se ignoran.
    std::list<std::list<Vertice> > kCaminosMasCortos(Vertice u, Vertice v, int k) const;
    std::list<std::list<Vertice> > kCaminosMasCortos(Vertice u, Vertice v, int k,
                                                     std::list<double>& costos) const;
    // Recorrido euleriano (Hierholzer, O(n + m)): recorrido que usa CADA arco
    // exactamente una vez, abierto o cerrado. Devuelve los vertices en orden
    // de recorrido (m + 1 vertices; en un ciclo el primero se repite al
    // final). Lista vacia si no existe o si no hay arcos.
    // Dirigido: a lo sumo un vertice con (salida - entrada) = 1 (el inicio) y
    // uno con -1 (el final), el resto equilibrado, y todos los arcos en una
    // misma componente debilmente conexa.
    std::list<Vertice> caminoEuleriano() const;
    bool tieneCaminoEuleriano() const;      // true tambien si no hay arcos
    // Solo recorridos CERRADOS (ciclo euleriano), empezando por el menor
    // vertice con arcos. Complementa a tieneCicloEuleriano().
    std::list<Vertice> cicloEuleriano() const;
    // No dirigido (grafo simetrico): a lo sumo dos vertices de grado impar
    // (un bucle suma 2) y todas las aristas en una componente conexa.
    std::list<Vertice> caminoEulerianoNoDirigido() const;
    bool tieneCaminoEulerianoNoDirigido() const;
    std::list<Vertice> cicloEulerianoNoDirigido() const;
    // Camino euleriano dirigido que EMPIEZA en s (Hierholzer, O(n + m)): usa
    // cada arco exactamente una vez, y entre todos los caminos posibles
    // devuelve el menor lexicograficamente (comparando los numeros de vertice
    // de izquierda a derecha), porque en cada vertice se prueba primero el
    // sucesor menor y los callejones sin salida quedan al final del recorrido.
    // Devuelve m + 1 vertices (s primero; en un ciclo s se repite al final).
    // Lista vacia si s no existe, si no hay arcos, o si no existe un camino
    // euleriano que empiece en s (p. ej. si el inicio obligado es otro vertice).
    // Diferencia con caminoEuleriano(): alli el inicio lo decide el grafo; aqui
    // lo decide el cliente.
    std::list<Vertice> caminoEulerianoDesde(Vertice s) const;
    // Igual, pero con costoEsCantidad = true el costo de cada arco (u, v) se
    // interpreta como la CANTIDAD de arcos paralelos u -> v, que se recorren
    // todos (el costo debe ser un entero >= 1; si algun arco no lo es devuelve
    // la lista vacia). Sirve para multigrafos: dos arcos iguales no caben en
    // Grafo (agregarArco actualiza el costo del existente), asi que se guardan
    // como un solo arco con costo 2. Con false es la version de arriba.
    std::list<Vertice> caminoEulerianoDesde(Vertice s, bool costoEsCantidad) const;

    // --------------------------- Arboles de expansion, flujos, emparejamientos
    // Prim desde s: arbol de expansion minima de la componente de s (grafo
    // simetrico). Devuelve las aristas en el orden aceptado, cada una con
    // origen < destino; pesoTotal = suma de costos; conexo = true si el
    // arbol abarca todo el grafo. Vacio, pesoTotal = 0 y conexo = false si s
    // no existe. O(m log n). Mejor que kruskal en grafos densos.
    std::list<Arista> prim(Vertice s, double& pesoTotal, bool& conexo) const;
    // Arbol (bosque) de expansion MAXIMA: como kruskal pero eligiendo los
    // costos mas altos. Mismas condiciones y resultado que kruskal.
    std::list<Arista> kruskalMaximo(double& pesoTotal, bool& conexo) const;
    // Flujo maximo de s a t (Edmonds-Karp, BFS). El costo de cada arco es su
    // CAPACIDAD (>= 0; los bucles se ignoran). Devuelve el valor del flujo, o
    // -1.0 si s o t no existen, s == t o hay capacidades negativas. La version
    // con flujo devuelve ademas los arcos con flujo neto positivo (el costo
    // de cada Arista es la cantidad que circula). Memoria O(n^2), tiempo
    // O(n m^2) en el peor caso: grafos pequenos o medianos.
    double flujoMaximo(Vertice s, Vertice t) const;
    double flujoMaximo(Vertice s, Vertice t, std::list<Arista>& flujo) const;
    // Emparejamiento maximo de un grafo bipartito (ignora sentidos, como
    // esBipartito; algoritmo de Kuhn, O(n m)). Devuelve su tamano, o -1 si
    // el grafo no es bipartito. pares recibe las aristas elegidas (origen <
    // destino, con el costo del arco) en orden ascendente.
    int emparejamientoMaximo(std::list<Arista>& pares) const;

    // ------------------------------------------ Problemas NP (exactos)
    // Pensados para grafos pequenos (hasta unos 25-30 vertices; dependen de
    // la estructura). Trabajan sobre el grafo subyacente, salvo el dominante.
    // Coloreado voraz (Welsh-Powell): color[v] en 1..k para cada vertice (0
    // en los numeros que no son vertices; se indexa por numero de vertice).
    // Devuelve k, una cota superior del numero cromatico (no siempre el
    // optimo). Ignora sentidos y bucles. O(n^2 + m).
    int coloreadoVoraz(std::vector<int>& color) const;
    // Conjunto dominante de MENOR tamano: cada vertice esta en el conjunto o
    // recibe un arco desde alguno de ellos (en un grafo simetrico es el
    // dominante de siempre). Resultado ascendente; vacio si el grafo es vacio.
    std::list<Vertice> conjuntoDominanteMinimo() const;
    // Conjunto independiente de MAYOR tamano (sin dos vertices unidos por un
    // arco en ningun sentido; los bucles se ignoran), ascendente.
    std::list<Vertice> conjuntoIndependienteMaximo() const;
    // Clique de MAYOR tamano (todos unidos con todos, en algun sentido; los
    // bucles se ignoran), ascendente (Bron-Kerbosch con pivote).
    std::list<Vertice> cliqueMaxima() const;

    // ------------------------------------------- Entrada, salida y vistas
    // Escribe una linea por vertice: "1 -> 2(5), 3(7)" o "4 -> (sin arcos)".
    void imprimir(std::ostream& os) const;
    // Lista de aristas: todos los arcos o, con noDirigido = true, cada par
    // una sola vez (origen <= destino; para un grafo simetrico). Es la
    // inversa de construirDesdeAristas (salvo los vertices aislados).
    std::list<Arista> listaDeAristas(bool noDirigido) const;
    // Matrices indexadas por numero de vertice (maxVertice() + 1 filas y
    // columnas, la 0 sin usar): adyacencia (1 = hay arco, un bucle es [v][v])
    // y costos (DIST_INFINITA donde no hay arco).
    void obtenerMatrizAdyacencia(std::vector<std::vector<char> >& ady) const;
    void obtenerMatrizCostos(std::vector<std::vector<double> >& costo) const;
    // Archivo de texto: una linea "origen destino costo" por arco y "v id"
    // por vertice aislado; '#' inicia un comentario. Al leer, el costo es
    // opcional (1.0 por defecto) y los vertices se crean solos. Con
    // noDirigido = true, guardar escribe cada arista una sola vez (grafo
    // simetrico) y leer agrega ambos sentidos. leerDesdeArchivo reemplaza el
    // contenido; si falla (archivo ilegible o linea mal formada) devuelve
    // false y el grafo queda como estaba. Los costos se guardan con 15
    // cifras significativas.
    bool guardarEnArchivo(const std::string& archivo, bool noDirigido) const;
    bool leerDesdeArchivo(const std::string& archivo, bool noDirigido);
    // Graphviz (.dot): "digraph" con flechas, o "graph" con noDirigido = true
    // (cada arista una vez). Con etiquetas (opcional, indexadas por numero de
    // vertice) se escribe ese texto en lugar del numero. false si no se pudo
    // escribir. Se dibuja con: dot -Tpng grafo.dot -o grafo.png
    bool exportarDOT(const std::string& archivo, bool noDirigido) const;
    bool exportarDOT(const std::string& archivo, bool noDirigido,
                     const std::vector<std::string>& etiquetas) const;
};

// Permite  std::cout << grafo;  (equivale a grafo.imprimir(std::cout))
std::ostream& operator<<(std::ostream& os, const Grafo& G);

#endif
