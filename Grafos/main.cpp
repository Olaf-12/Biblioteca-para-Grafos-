#include <iostream>
#include <list>
#include <string>
#include <vector>
#include "Grafo.hpp"
#include "GrafoEtiquetado.hpp"

using namespace std;

void imprimir(const string& titulo, const list<Vertice>& l) {
    cout << titulo << ": ";
    for (list<Vertice>::const_iterator it = l.begin(); it != l.end(); ++it)
        cout << *it << " ";
    cout << endl;
}

void imprimirCiclos(const string& titulo, const list<list<Vertice> >& cs) {
    cout << titulo << " (" << cs.size() << "):" << endl;
    for (list<list<Vertice> >::const_iterator it = cs.begin(); it != cs.end(); ++it) {
        cout << "  ";
        for (list<Vertice>::const_iterator jt = it->begin(); jt != it->end(); ++jt)
            cout << *jt << " ";
        cout << endl;
    }
}

template <typename T>
void imprimirEt(const string& titulo, const list<T>& l) {
    cout << titulo << ": ";
    for (typename list<T>::const_iterator it = l.begin(); it != l.end(); ++it)
        cout << *it << " | ";
    cout << endl;
}

int main() {
    Grafo G;
    for (int i = 1; i <= 6; i++) G.agregarVertice(i);

    // DAG de la Figura 2 del tema
    G.agregarArco(1, 3, 2.0);
    G.agregarArco(1, 4, 4.5);
    G.agregarArco(3, 4, 1.2);
    G.agregarArco(4, 5, 3.0);
    G.agregarArco(5, 6, 5.1);
    G.agregarArco(6, 2, 1.8);

    cout << "orden = " << G.orden() << endl;
    cout << "existeArco(1,3) = " << G.existeArco(1, 3) << endl;
    cout << "existeArco(3,1) = " << G.existeArco(3, 1) << endl;
    cout << "costoArco(4,5) = " << G.costoArco(4, 5) << endl;
    cout << "grado(4): ext = " << G.gradoExterior(4)
         << ", int = " << G.gradoInterior(4)
         << ", total = " << G.grado(4) << endl;
    cout << "grado(2) = " << G.grado(2) << ", grado(99) = " << G.grado(99) << endl;
    imprimir("sucesores(1)", G.sucesores(1));
    imprimir("predecesores(4)", G.predecesores(4));
    imprimir("fuentes", G.obtenerVerticesFuente());
    imprimir("topologico BFS", G.ordenamientoTopologico());
    imprimir("topologico DFS", G.ordenamientoTopologicoDFS());

    imprimir("recorridoBFS(1)", G.recorridoBFS(1));
    imprimir("recorridoBFS(3)", G.recorridoBFS(3));
    imprimir("alcanzables(3)", G.alcanzables(3));
    imprimir("alcanzables(2)", G.alcanzables(2));
    imprimir("alcanzables(99) (no existe)", G.alcanzables(99));

    // Dijkstra
    std::vector<double> d1 = G.dijkstra(1);
    cout << "Dijkstra desde 1:" << endl;
    for (int i = 1; i < (int)d1.size(); i++) {
        cout << "  v" << i << " = ";
        if (d1[i] == DIST_INFINITA) cout << "inalcanzable" << endl;
        else cout << d1[i] << endl;
    }
    std::vector<double> d3 = G.dijkstra(3);
    cout << "Dijkstra desde 3: dist[1] = "
         << (d3[1] == DIST_INFINITA ? "inalcanzable" : "alcanzable")
         << ", dist[2] = " << d3[2] << endl;
    cout << "Dijkstra desde 99 (no existe): tam = " << G.dijkstra(99).size() << endl;

    // Camino minimo con alternativa: 1->3->4 (3.2) vs 1->4 directo (4.5)
    std::vector<double> dd; std::vector<int> pp;
    G.dijkstra(1, dd, pp);
    cout << "pred[4] = " << pp[4] << " (camino 1->3->4 con costo " << dd[4] << ")" << endl;

    // Arco negativo: Dijkstra se rechaza
    Grafo N;
    N.agregarVertice(1); N.agregarVertice(2);
    N.agregarArco(1, 2, -1.0);
    cout << "Dijkstra con arco negativo: tam = " << N.dijkstra(1).size() << endl;

    // Grafo no dirigido
    Grafo U;
    for (int i = 1; i <= 4; i++) U.agregarVertice(i);
    U.agregarArcoNoDirigido(1, 2, 1.0);
    U.agregarArcoNoDirigido(2, 3, 2.0);
    U.agregarArcoNoDirigido(3, 4, 3.0);
    cout << "no dirigido: existeArco(2,1) = " << U.existeArco(2, 1) << endl;
    imprimir("no dirigido recorridoBFS(4)", U.recorridoBFS(4));
    U.eliminarArcoNoDirigido(2, 3);
    cout << "tras eliminar (2,3): existeArco(2,3) = " << U.existeArco(2, 3)
         << ", existeArco(3,2) = " << U.existeArco(3, 2) << endl;

    // Deteccion de ciclos
    cout << "tieneCiclos (DAG de prueba): " << G.tieneCiclos() << endl;
    Grafo D;   // diamante 1->2, 1->3, 2->4, 3->4: aciclico
    for (int i = 1; i <= 4; i++) D.agregarVertice(i);
    D.agregarArco(1, 2, 1.0); D.agregarArco(1, 3, 1.0);
    D.agregarArco(2, 4, 1.0); D.agregarArco(3, 4, 1.0);
    cout << "tieneCiclos (diamante): " << D.tieneCiclos() << endl;
    D.agregarArco(4, 1, 1.0);
    cout << "tieneCiclos (diamante + 4->1): " << D.tieneCiclos() << endl;
    Grafo B;   // bucle
    B.agregarVertice(1); B.agregarArco(1, 1, 1.0);
    cout << "tieneCiclos (bucle): " << B.tieneCiclos() << endl;
    Grafo V;
    cout << "tieneCiclos (vacio): " << V.tieneCiclos() << endl;

    Grafo P;   // no dirigido: camino 1-2-3-4, luego se cierra 4-1
    for (int i = 1; i <= 4; i++) P.agregarVertice(i);
    P.agregarArcoNoDirigido(1, 2, 1.0);
    P.agregarArcoNoDirigido(2, 3, 1.0);
    P.agregarArcoNoDirigido(3, 4, 1.0);
    cout << "tieneCiclosNoDirigido (camino): " << P.tieneCiclosNoDirigido() << endl;
    P.agregarArcoNoDirigido(4, 1, 1.0);
    cout << "tieneCiclosNoDirigido (camino + 4-1): " << P.tieneCiclosNoDirigido() << endl;
    Grafo Q;   // una sola arista no es ciclo
    Q.agregarVertice(1); Q.agregarVertice(2);
    Q.agregarArcoNoDirigido(1, 2, 1.0);
    cout << "tieneCiclosNoDirigido (una arista): " << Q.tieneCiclosNoDirigido() << endl;
    cout << "(tieneCiclos sobre esa misma arista no dirigida: " << Q.tieneCiclos()
         << " -> por eso hay dos metodos)" << endl;

    // Enumeracion de ciclos
    Grafo E;   // ciclos: [3] (bucle), [1,2,3] y [2,4]
    for (int i = 1; i <= 4; i++) E.agregarVertice(i);
    E.agregarArco(1, 2, 1.0); E.agregarArco(2, 3, 1.0); E.agregarArco(3, 1, 1.0);
    E.agregarArco(2, 4, 1.0); E.agregarArco(4, 2, 1.0); E.agregarArco(3, 3, 1.0);
    bool cortado;
    imprimirCiclos("ciclos dirigidos", E.enumerarCiclos(0, cortado));
    cout << "contarCiclos(sin tope) = " << E.contarCiclos(0, cortado)
         << ", cortado = " << cortado << endl;
    cout << "contarCiclos(tope 2) = " << E.contarCiclos(2, cortado)
         << ", cortado = " << cortado << endl;
    cout << "contarCiclos(tope 3) = " << E.contarCiclos(3, cortado)
         << ", cortado = " << cortado << endl;
    cout << "ciclos del DAG de prueba = " << G.contarCiclos(0, cortado) << endl;

    Grafo S;   // cuadrado 1-2-3-4 con diagonal 1-3: ciclos [1,2,3], [1,2,3,4], [1,3,4]
    for (int i = 1; i <= 4; i++) S.agregarVertice(i);
    S.agregarArcoNoDirigido(1, 2, 1.0); S.agregarArcoNoDirigido(2, 3, 1.0);
    S.agregarArcoNoDirigido(3, 4, 1.0); S.agregarArcoNoDirigido(4, 1, 1.0);
    S.agregarArcoNoDirigido(1, 3, 1.0);
    imprimirCiclos("ciclos no dirigidos", S.enumerarCiclosNoDirigido(0, cortado));
    cout << "contarCiclosNoDirigido(tope 2) = " << S.contarCiclosNoDirigido(2, cortado)
         << ", cortado = " << cortado << endl;
    cout << "ciclos no dirigidos del grafo U (sin ciclos) = "
         << U.contarCiclosNoDirigido(0, cortado) << endl;

    // Kruskal: arbol de expansion minima
    Grafo W;
    for (int i = 1; i <= 5; i++) W.agregarVertice(i);
    W.agregarArcoNoDirigido(1, 2, 1.0); W.agregarArcoNoDirigido(3, 4, 2.0);
    W.agregarArcoNoDirigido(2, 3, 3.0); W.agregarArcoNoDirigido(4, 5, 4.0);
    W.agregarArcoNoDirigido(1, 3, 5.0); W.agregarArcoNoDirigido(2, 5, 6.0);
    W.agregarArcoNoDirigido(1, 5, 7.0);
    double pesoMST; bool conexoMST;
    list<Arista> mst = W.kruskal(pesoMST, conexoMST);
    cout << "Kruskal: " << mst.size() << " aristas, peso total = " << pesoMST
         << ", conexo = " << conexoMST << endl;
    for (list<Arista>::const_iterator it = mst.begin(); it != mst.end(); ++it)
        cout << "  (" << it->origen << "," << it->destino << ") costo " << it->costo << endl;

    Grafo T;   // grafo nuevo con solo las aristas del MST
    T.construirDesdeAristas(mst, true);
    cout << "grafo MST: orden = " << T.orden() << ", existeArco(2,1) = " << T.existeArco(2, 1)
         << ", existeArco(1,3) = " << T.existeArco(1, 3)
         << ", tieneCiclosNoDirigido = " << T.tieneCiclosNoDirigido() << endl;
    imprimir("grafo MST recorridoBFS(1)", T.recorridoBFS(1));

    Grafo X;   // no conexo: bosque de expansion minima
    for (int i = 1; i <= 4; i++) X.agregarVertice(i);
    X.agregarArcoNoDirigido(1, 2, 1.0); X.agregarArcoNoDirigido(3, 4, 2.0);
    list<Arista> bosque = X.kruskal(pesoMST, conexoMST);
    cout << "Kruskal (no conexo): " << bosque.size() << " aristas, peso total = "
         << pesoMST << ", conexo = " << conexoMST << endl;

    // Componentes conexas
    cout << "componentesConexas: DAG de prueba = " << G.componentesConexas()
         << ", MST = " << T.componentesConexas()
         << ", bosque (1-2, 3-4) = " << X.componentesConexas()
         << ", vacio = " << V.componentesConexas() << endl;
    Grafo I;   // 3 vertices aislados + 1 arista
    for (int i = 1; i <= 5; i++) I.agregarVertice(i);
    I.agregarArcoNoDirigido(1, 2, 1.0);
    cout << "componentesConexas (5 vertices, una arista) = " << I.componentesConexas() << endl;

    // Bipartito
    list<Vertice> conjA, conjB;
    cout << "esBipartito: camino U = " << U.esBipartito()
         << ", cuadrado P = " << P.esBipartito(conjA, conjB) << endl;
    imprimir("  conjunto A de P", conjA);
    imprimir("  conjunto B de P", conjB);
    cout << "esBipartito: cuadrado con diagonal S = " << S.esBipartito()
         << ", DAG de prueba G (triangulo 1,3,4) = " << G.esBipartito()
         << ", bucle = " << B.esBipartito()
         << ", vacio = " << V.esBipartito() << endl;
    Grafo H;   // dirigido: 1->3, 2->4, 5->3, 5->2 forma el camino 1-3-5-2-4
    for (int i = 1; i <= 5; i++) H.agregarVertice(i);
    H.agregarArco(1, 3, 1.0); H.agregarArco(2, 4, 1.0);
    H.agregarArco(5, 3, 1.0); H.agregarArco(5, 2, 1.0);
    cout << "esBipartito (dirigido, sentidos mezclados) = " << H.esBipartito() << endl;

    // Ciclo euleriano no dirigido
    Grafo M;   // dos triangulos que comparten el vertice 1
    for (int i = 1; i <= 5; i++) M.agregarVertice(i);
    M.agregarArcoNoDirigido(1, 2, 1.0); M.agregarArcoNoDirigido(2, 3, 1.0);
    M.agregarArcoNoDirigido(3, 1, 1.0); M.agregarArcoNoDirigido(1, 4, 1.0);
    M.agregarArcoNoDirigido(4, 5, 1.0); M.agregarArcoNoDirigido(5, 1, 1.0);
    Grafo M2;  // dos triangulos disjuntos: grados pares pero no conexo
    for (int i = 1; i <= 6; i++) M2.agregarVertice(i);
    M2.agregarArcoNoDirigido(1, 2, 1.0); M2.agregarArcoNoDirigido(2, 3, 1.0);
    M2.agregarArcoNoDirigido(3, 1, 1.0); M2.agregarArcoNoDirigido(4, 5, 1.0);
    M2.agregarArcoNoDirigido(5, 6, 1.0); M2.agregarArcoNoDirigido(6, 4, 1.0);
    Grafo M3;  // triangulo + vertice aislado: los aislados se ignoran
    for (int i = 1; i <= 4; i++) M3.agregarVertice(i);
    M3.agregarArcoNoDirigido(1, 2, 1.0); M3.agregarArcoNoDirigido(2, 3, 1.0);
    M3.agregarArcoNoDirigido(3, 1, 1.0);
    cout << "euleriano no dirigido: cuadrado P = " << P.tieneCicloEulerianoNoDirigido()
         << ", dos triangulos unidos M = " << M.tieneCicloEulerianoNoDirigido()
         << ", dos triangulos disjuntos = " << M2.tieneCicloEulerianoNoDirigido()
         << ", triangulo + aislado = " << M3.tieneCicloEulerianoNoDirigido()
         << ", camino U = " << U.tieneCicloEulerianoNoDirigido()
         << ", cuadrado con diagonal S = " << S.tieneCicloEulerianoNoDirigido()
         << ", vacio = " << V.tieneCicloEulerianoNoDirigido() << endl;
    // Ciclo euleriano dirigido
    cout << "euleriano dirigido: E (ciclos con bucle) = " << E.tieneCicloEuleriano()
         << ", DAG de prueba G = " << G.tieneCicloEuleriano()
         << ", diamante + 4->1 = " << D.tieneCicloEuleriano()
         << ", bucle = " << B.tieneCicloEuleriano() << endl;

    // Ciclo hamiltoniano
    {
        list<Vertice> ham;
        cout << "hamiltoniano no dirigido: cuadrado P = "
             << P.tieneCicloHamiltonianoNoDirigido(ham);
        imprimir(" ->", ham);
        cout << "hamiltoniano no dirigido: cuadrado con diagonal S = "
             << S.tieneCicloHamiltonianoNoDirigido(ham);
        imprimir(" ->", ham);
        Grafo PE;   // Petersen: 10 vertices, 3-regular, NO es hamiltoniano
        for (int i = 1; i <= 10; i++) PE.agregarVertice(i);
        for (int i = 1; i <= 5; i++) {
            PE.agregarArcoNoDirigido(i, i % 5 + 1, 1.0);             // exterior
            PE.agregarArcoNoDirigido(i, i + 5, 1.0);                 // radios
            PE.agregarArcoNoDirigido(i + 5, (i + 1) % 5 + 6, 1.0);   // pentagrama
        }
        Grafo K;    // completo K5: hamiltoniano
        for (int i = 1; i <= 5; i++) K.agregarVertice(i);
        for (int i = 1; i <= 5; i++)
            for (int j = i + 1; j <= 5; j++) K.agregarArcoNoDirigido(i, j, 1.0);
        Grafo K2;   // una sola arista: con 2 vertices no hay ciclo no dirigido
        K2.agregarVertice(1); K2.agregarVertice(2);
        K2.agregarArcoNoDirigido(1, 2, 1.0);
        cout << "hamiltoniano no dirigido: dos triangulos unidos M = "
             << M.tieneCicloHamiltonianoNoDirigido()
             << ", Petersen = " << PE.tieneCicloHamiltonianoNoDirigido()
             << ", K5 = " << K.tieneCicloHamiltonianoNoDirigido(ham)
             << ", una arista = " << K2.tieneCicloHamiltonianoNoDirigido()
             << ", vacio = " << V.tieneCicloHamiltonianoNoDirigido() << endl;
        imprimir("  ciclo de K5", ham);
        Grafo DH;   // dirigido: 1->2->3->4->1
        for (int i = 1; i <= 4; i++) DH.agregarVertice(i);
        DH.agregarArco(1, 2, 1.0); DH.agregarArco(2, 3, 1.0);
        DH.agregarArco(3, 4, 1.0); DH.agregarArco(4, 1, 1.0);
        cout << "hamiltoniano dirigido: 1->2->3->4->1 = " << DH.tieneCicloHamiltoniano(ham);
        imprimir(" ->", ham);
        cout << "hamiltoniano dirigido: E = " << E.tieneCicloHamiltoniano()
             << ", DAG de prueba G = " << G.tieneCicloHamiltoniano()
             << ", diamante + 4->1 = " << D.tieneCicloHamiltoniano()
             << ", bucle = " << B.tieneCicloHamiltoniano()
             << ", vacio = " << V.tieneCicloHamiltoniano() << endl;
    }

    // Excentricidad y radio
    {
        Grafo R;    // camino 1-2-3-4 con pesos 1, 2, 3
        for (int i = 1; i <= 4; i++) R.agregarVertice(i);
        R.agregarArcoNoDirigido(1, 2, 1.0); R.agregarArcoNoDirigido(2, 3, 2.0);
        R.agregarArcoNoDirigido(3, 4, 3.0);
        cout << "excentricidad (camino 1-2-3-4, pesos 1,2,3): e(1) = " << R.excentricidad(1)
             << ", e(2) = " << R.excentricidad(2) << ", e(3) = " << R.excentricidad(3)
             << ", e(4) = " << R.excentricidad(4) << ", radio = " << R.radio() << endl;
        cout << "excentricidad(99) (no existe) = " << R.excentricidad(99) << endl;
        cout << "radio del bosque X (no conexo) = "
             << (X.radio() == DIST_INFINITA ? "infinito" : "finito")
             << ", e(1) = " << (X.excentricidad(1) == DIST_INFINITA ? "infinita" : "finita") << endl;
        cout << "dirigido G: e(1) = " << G.excentricidad(1)
             << ", e(2) = " << (G.excentricidad(2) == DIST_INFINITA ? "infinita" : "finita")
             << ", radio = " << G.radio() << endl;
        cout << "arco negativo N: e(1) = " << N.excentricidad(1)
             << ", radio = " << N.radio() << endl;
        cout << "un solo vertice: radio = " << B.radio() << ", vacio: radio = " << V.radio() << endl;
    }

    // Bellman-Ford (costos negativos)
    {
        std::vector<double> bd; std::vector<int> bp;
        cout << "bellmanFord arco negativo N (Dijkstra lo rechaza): ok = "
             << N.bellmanFord(1, bd, bp) << ", dist[2] = " << bd[2] << endl;
        Grafo NG;   // 1->2 (4), 1->3 (5), 3->2 (-3), 2->4 (2): mejor 1->3->2 = 2
        for (int i = 1; i <= 5; i++) NG.agregarVertice(i);
        NG.agregarArco(1, 2, 4.0); NG.agregarArco(1, 3, 5.0);
        NG.agregarArco(3, 2, -3.0); NG.agregarArco(2, 4, 2.0);
        NG.bellmanFord(1, bd, bp);
        cout << "bellmanFord: dist[2] = " << bd[2] << " (pred " << bp[2]
             << "), dist[4] = " << bd[4] << ", vertice 5 "
             << (bd[5] == DIST_INFINITA ? "inalcanzable" : "alcanzable") << endl;
        Grafo NC;   // ciclo negativo 1->2->3->1 (1 - 2 + 0 = -1)
        for (int i = 1; i <= 3; i++) NC.agregarVertice(i);
        NC.agregarArco(1, 2, 1.0); NC.agregarArco(2, 3, -2.0); NC.agregarArco(3, 1, 0.0);
        cout << "bellmanFord con ciclo negativo = " << NC.bellmanFord(1, bd, bp)
             << ", version simple: tam = " << NC.bellmanFord(1).size()
             << ", origen inexistente: " << NC.bellmanFord(99, bd, bp) << endl;
        Grafo NI;   // ciclo negativo NO alcanzable desde 1: no afecta
        for (int i = 1; i <= 4; i++) NI.agregarVertice(i);
        NI.agregarArco(1, 2, 3.0);
        NI.agregarArco(3, 4, -1.0); NI.agregarArco(4, 3, -1.0);
        cout << "bellmanFord con ciclo negativo inalcanzable = " << NI.bellmanFord(1, bd, bp)
             << ", dist[2] = " << bd[2] << endl;
        std::vector<double> dg = G.bellmanFord(1);
        cout << "bellmanFord = dijkstra en el DAG de prueba (v6): " << dg[6]
             << " vs " << G.dijkstra(1)[6] << endl;
    }

    // Floyd-Warshall (todos los pares)
    {
        std::vector<std::vector<double> > fd;
        std::vector<std::vector<int> > fs;
        cout << "floydWarshall DAG de prueba G: ok = " << G.floydWarshall(fd, fs)
             << ", d(1,6) = " << fd[1][6] << ", d(3,2) = " << fd[3][2]
             << ", d(2,1) " << (fd[2][1] == DIST_INFINITA ? "inalcanzable" : "alcanzable")
             << ", d(4,4) = " << fd[4][4] << endl;
        imprimir("camino minimo 1 -> 2", G.reconstruirCamino(fs, 1, 2));
        imprimir("camino minimo 2 -> 1 (inalcanzable)", G.reconstruirCamino(fs, 2, 1));
        imprimir("camino minimo 4 -> 4", G.reconstruirCamino(fs, 4, 4));
        std::vector<double> fdij = G.dijkstra(1);
        bool igual = true;
        for (int v = 1; v <= G.maxVertice(); v++)
            if (fdij[v] != fd[1][v]) igual = false;
        cout << "fila 1 de Floyd-Warshall = dijkstra(1): " << igual << endl;

        Grafo FN;   // 1->2 (4), 1->3 (5), 3->2 (-3), 2->4 (2): mejor 1->3->2->4 = 4
        for (int i = 1; i <= 4; i++) FN.agregarVertice(i);
        FN.agregarArco(1, 2, 4.0); FN.agregarArco(1, 3, 5.0);
        FN.agregarArco(3, 2, -3.0); FN.agregarArco(2, 4, 2.0);
        FN.floydWarshall(fd, fs);
        cout << "floydWarshall costos negativos: d(1,2) = " << fd[1][2]
             << ", d(1,4) = " << fd[1][4];
        imprimir(" -> camino 1 a 4", FN.reconstruirCamino(fs, 1, 4));
        Grafo FC;   // ciclo negativo 1->2->3->1 (1 - 2 + 0 = -1)
        for (int i = 1; i <= 3; i++) FC.agregarVertice(i);
        FC.agregarArco(1, 2, 1.0); FC.agregarArco(2, 3, -2.0); FC.agregarArco(3, 1, 0.0);
        cout << "floydWarshall con ciclo negativo = " << FC.floydWarshall(fd)
             << ", vacio = " << V.floydWarshall(fd) << ", tam = " << fd.size() << endl;
    }

    // Ruta de mayor costo
    {
        std::vector<double> rd;
        std::vector<std::list<Vertice> > rr;
        cout << "rutaMayorCosto DAG de prueba G (origen 1): ok = " << G.rutaMayorCosto(1, rd, rr)
             << ", dist[4] = " << rd[4] << ", dist[2] = " << rd[2] << endl;
        imprimir("  ruta de mayor costo 1 -> 4 (directo 4.5 > 1->3->4 = 3.2)", rr[4]);
        imprimir("  ruta de mayor costo 1 -> 2", rr[2]);
        cout << "  desde 1: dist[1] = " << rd[1] << ", ruta[1] = " << rr[1].size() << " vertice" << endl;
        G.rutaMayorCosto(3, rd, rr);
        cout << "  desde 3: vertice 1 " << (rd[1] == -DIST_INFINITA ? "inalcanzable" : "alcanzable")
             << " (ruta vacia: " << rr[1].empty() << ")" << endl;
        cout << "  origen inexistente: " << G.rutaMayorCosto(99, rd, rr) << endl;

        Grafo RN;   // DAG con costos negativos: 1->2 (-1), 1->3 (-5), 2->4 (-1), 3->4 (10)
        for (int i = 1; i <= 4; i++) RN.agregarVertice(i);
        RN.agregarArco(1, 2, -1.0); RN.agregarArco(1, 3, -5.0);
        RN.agregarArco(2, 4, -1.0); RN.agregarArco(3, 4, 10.0);
        RN.rutaMayorCosto(1, rd, rr);
        cout << "rutaMayorCosto con costos negativos: dist[4] = " << rd[4];
        imprimir(" -> ruta", rr[4]);

        Grafo RC;   // con ciclo: 1->2 (1), 2->3 (2), 3->1 (4), 3->4 (1), 1->4 (3)
        for (int i = 1; i <= 4; i++) RC.agregarVertice(i);
        RC.agregarArco(1, 2, 1.0); RC.agregarArco(2, 3, 2.0); RC.agregarArco(3, 1, 4.0);
        RC.agregarArco(3, 4, 1.0); RC.agregarArco(1, 4, 3.0);
        RC.rutaMayorCosto(1, rd, rr);
        cout << "rutaMayorCosto con ciclo (camino simple): dist[4] = " << rd[4]
             << ", dist[1] = " << rd[1];
        imprimir(" -> ruta a 4", rr[4]);

        Grafo RU;   // no dirigido: triangulo 1-2 (5), 2-3 (1), 1-3 (2) + 3-4 (4)
        for (int i = 1; i <= 4; i++) RU.agregarVertice(i);
        RU.agregarArcoNoDirigido(1, 2, 5.0); RU.agregarArcoNoDirigido(2, 3, 1.0);
        RU.agregarArcoNoDirigido(1, 3, 2.0); RU.agregarArcoNoDirigido(3, 4, 4.0);
        RU.rutaMayorCosto(1, rd, rr);
        cout << "rutaMayorCosto no dirigido: dist[3] = " << rd[3];
        imprimir(" -> ruta a 3", rr[3]);
        cout << "  dist[4] = " << rd[4];
        imprimir(" -> ruta a 4", rr[4]);
    }

    // Planaridad
    {
        Grafo K5, K33, Q3, KP;
        for (int i = 1; i <= 5; i++) K5.agregarVertice(i);
        for (int i = 1; i <= 5; i++)
            for (int j = i + 1; j <= 5; j++) K5.agregarArcoNoDirigido(i, j, 1.0);
        for (int i = 1; i <= 6; i++) K33.agregarVertice(i);
        for (int i = 1; i <= 3; i++)
            for (int j = 4; j <= 6; j++) K33.agregarArcoNoDirigido(i, j, 1.0);
        for (int i = 0; i < 8; i++) Q3.agregarVertice(i + 1);   // cubo: planar
        for (int i = 0; i < 8; i++)
            for (int j = i + 1; j < 8; j++) {
                int x = i ^ j;
                if (x == 1 || x == 2 || x == 4) Q3.agregarArcoNoDirigido(i + 1, j + 1, 1.0);
            }
        for (int i = 1; i <= 6; i++) KP.agregarVertice(i);      // K5 + vertice colgante
        for (int i = 1; i <= 5; i++)
            for (int j = i + 1; j <= 5; j++) KP.agregarArcoNoDirigido(i, j, 1.0);
        KP.agregarArcoNoDirigido(5, 6, 1.0);
        Grafo K5m;  // K5 sin la arista (1,2): planar
        for (int i = 1; i <= 5; i++) K5m.agregarVertice(i);
        for (int i = 1; i <= 5; i++)
            for (int j = i + 1; j <= 5; j++)
                if (!(i == 1 && j == 2)) K5m.agregarArcoNoDirigido(i, j, 1.0);
        Grafo K5d;  // K5 con arcos en un solo sentido: el sentido se ignora
        for (int i = 1; i <= 5; i++) K5d.agregarVertice(i);
        for (int i = 1; i <= 5; i++)
            for (int j = i + 1; j <= 5; j++) K5d.agregarArco(i, j, 1.0);
        cout << "esPlanar: K5 = " << K5.esPlanar()
             << ", K5 dirigido = " << K5d.esPlanar()
             << ", K5 + colgante = " << KP.esPlanar()
             << ", K3,3 = " << K33.esPlanar()
             << ", K5 sin una arista = " << K5m.esPlanar()
             << ", cubo = " << Q3.esPlanar() << endl;
        cout << "esPlanar: cuadrado con diagonal S = " << S.esPlanar()
             << ", dos triangulos unidos M = " << M.esPlanar()
             << ", DAG de prueba G = " << G.esPlanar()
             << ", bucle = " << B.esPlanar()
             << ", vacio = " << V.esPlanar() << endl;
    }

    // Puntos de articulacion
    {
        imprimir("articulacion: dos triangulos unidos M", M.puntosArticulacion());
        imprimir("articulacion: cuadrado P (biconexo)", P.puntosArticulacion());
        Grafo CA;   // camino 1-2-3-4: 2 y 3 son de corte
        for (int i = 1; i <= 4; i++) CA.agregarVertice(i);
        CA.agregarArcoNoDirigido(1, 2, 1.0); CA.agregarArcoNoDirigido(2, 3, 1.0);
        CA.agregarArcoNoDirigido(3, 4, 1.0);
        imprimir("articulacion: camino 1-2-3-4", CA.puntosArticulacion());
        imprimir("articulacion: bosque X (1-2, 3-4)", X.puntosArticulacion());
        imprimir("articulacion: DAG de prueba G (dirigido)", G.puntosArticulacion());
        imprimir("articulacion: vacio", V.puntosArticulacion());
    }

    std::vector<int> dist, pred, td, tf;
    int n = G.maxVertice() + 1;

    G.BFS(1, dist, pred);
    cout << "BFS desde 1:" << endl;
    for (int i = 1; i < n; i++)
        cout << "  v" << i << " dist=" << dist[i] << " pred=" << pred[i] << endl;

    G.DFS(pred, td, tf);
    cout << "DFS:" << endl;
    for (int i = 1; i < n; i++)
        cout << "  v" << i << " [" << td[i] << "," << tf[i] << "] pred=" << pred[i] << endl;

    // Exportacion a SVG: los archivos se crean en el directorio de ejecucion
    cout << "exportarSVG grafo_dirigido.svg: " << G.exportarSVG("grafo_dirigido.svg", false) << endl;
    cout << "exportarSVG ciclos_dirigido.svg: " << E.exportarSVG("ciclos_dirigido.svg", false) << endl;
    cout << "exportarSVG grafo_no_dirigido.svg: " << S.exportarSVG("grafo_no_dirigido.svg", true) << endl;

    G.eliminarArco(1, 4);
    cout << "tras eliminarArco(1,4): existeArco = " << G.existeArco(1, 4) << endl;
    G.eliminarVertice(4);
    cout << "tras eliminarVertice(4): orden = " << G.orden()
         << ", existeVertice(4) = " << G.existeVertice(4)
         << ", existeArco(3,4) = " << G.existeArco(3, 4) << endl;
    imprimir("sucesores(3)", G.sucesores(3));

    // Capa de traduccion: vertices de tipo string
    GrafoEtiquetado<string> C;
    C.agregarVertice("Caracas"); C.agregarVertice("Valencia");
    C.agregarVertice("Maracay"); C.agregarVertice("Barquisimeto");
    C.agregarVertice("Merida");
    C.agregarArcoNoDirigido("Caracas", "Maracay", 110.0);
    C.agregarArcoNoDirigido("Maracay", "Valencia", 50.0);
    C.agregarArcoNoDirigido("Caracas", "Valencia", 160.0);
    C.agregarArcoNoDirigido("Valencia", "Barquisimeto", 250.0);
    cout << "etiquetado: orden = " << C.orden() << ", componentes = "
         << C.componentesConexas() << endl;
    cout << "exportarSVG ciudades.svg: " << C.exportarSVG("ciudades.svg", true) << endl;
    imprimirEt("sucesores(Valencia)", C.sucesores("Valencia"));
    imprimirEt("recorridoBFS(Caracas)", C.recorridoBFS("Caracas"));
    imprimirEt("alcanzables(Merida)", C.alcanzables("Merida"));
    imprimirEt("alcanzables(Caracas)", C.alcanzables("Caracas"));
    map<string, double> dc = C.dijkstra("Caracas");
    cout << "dijkstra(Caracas):";
    for (map<string, double>::const_iterator it = dc.begin(); it != dc.end(); ++it)
        cout << " " << it->first << "=" << it->second;
    cout << endl;
    double pc; bool cc;
    list<AristaEtiquetada<string> > mc = C.kruskal(pc, cc);
    cout << "kruskal: " << mc.size() << " aristas, peso = " << pc << ", conexo = " << cc << endl;
    for (list<AristaEtiquetada<string> >::const_iterator it = mc.begin(); it != mc.end(); ++it)
        cout << "  " << it->origen << " - " << it->destino << " (" << it->costo << ")" << endl;
    bool trunc2;
    list<list<string> > ciclosC = C.enumerarCiclosNoDirigido(0, trunc2);
    cout << "ciclos no dirigidos: " << ciclosC.size() << endl;
    C.eliminarVertice("Maracay");
    C.agregarVertice("Coro");   // reutiliza el identificador interno liberado
    C.agregarArcoNoDirigido("Coro", "Barquisimeto", 200.0);
    cout << "tras eliminar Maracay y agregar Coro: orden = " << C.orden()
         << ", existeVertice(Maracay) = " << C.existeVertice("Maracay")
         << ", existeArco(Coro,Barquisimeto) = " << C.existeArco("Coro", "Barquisimeto") << endl;
    imprimirEt("vertices", C.vertices());
    GrafoEtiquetado<string> C2;
    C2.construirDesdeAristas(mc, true);
    cout << "grafo etiquetado desde el MST: orden = " << C2.orden()
         << ", ciclos = " << C2.tieneCiclosNoDirigido() << endl;
    cout << "existeArco(Fantasma, X) = " << C.existeArco("Fantasma", "X")
         << ", existeVertice(Fantasma) = " << C.existeVertice("Fantasma") << endl;

    GrafoEtiquetado<string> Z;   // cuadrado de ciudades: bipartito y euleriano
    Z.agregarVertice("Caracas"); Z.agregarVertice("Valencia");
    Z.agregarVertice("Maracay"); Z.agregarVertice("Puerto Cabello");
    Z.agregarArcoNoDirigido("Caracas", "Maracay", 1.0);
    Z.agregarArcoNoDirigido("Maracay", "Valencia", 1.0);
    Z.agregarArcoNoDirigido("Valencia", "Puerto Cabello", 1.0);
    Z.agregarArcoNoDirigido("Puerto Cabello", "Caracas", 1.0);
    list<string> zA, zB;
    cout << "etiquetado: esBipartito(cuadrado) = " << Z.esBipartito(zA, zB) << endl;
    imprimirEt("  conjunto A", zA);
    imprimirEt("  conjunto B", zB);
    cout << "etiquetado: tieneCicloEulerianoNoDirigido(cuadrado) = "
         << Z.tieneCicloEulerianoNoDirigido()
         << ", ciudades C (esBipartito = " << C.esBipartito()
         << ", euleriano = " << C.tieneCicloEulerianoNoDirigido() << ")" << endl;
    GrafoEtiquetado<string> Y;   // dirigido: A->B->C->A
    Y.agregarVertice("A"); Y.agregarVertice("B"); Y.agregarVertice("C");
    Y.agregarArco("A", "B", 1.0); Y.agregarArco("B", "C", 1.0); Y.agregarArco("C", "A", 1.0);
    cout << "etiquetado dirigido: tieneCicloEuleriano(A->B->C->A) = " << Y.tieneCicloEuleriano()
         << ", esBipartito (triangulo) = " << Y.esBipartito() << endl;
    list<string> hamZ;
    cout << "etiquetado: tieneCicloHamiltonianoNoDirigido(cuadrado) = "
         << Z.tieneCicloHamiltonianoNoDirigido(hamZ) << endl;
    imprimirEt("  ciclo", hamZ);
    cout << "etiquetado: excentricidad(Caracas) = " << Z.excentricidad("Caracas")
         << ", excentricidad(Fantasma) = " << Z.excentricidad("Fantasma")
         << ", radio = " << Z.radio() << endl;
    cout << "etiquetado dirigido: tieneCicloHamiltoniano(A->B->C->A) = "
         << Y.tieneCicloHamiltoniano() << ", excentricidad(A) = " << Y.excentricidad("A")
         << ", radio = " << Y.radio() << endl;
    GrafoEtiquetado<string> K5e;   // 5 ciudades todas conectadas: no planar
    const char* ciud[5] = {"Caracas", "Valencia", "Maracay", "Merida", "Coro"};
    for (int i = 0; i < 5; i++) K5e.agregarVertice(ciud[i]);
    cout << "etiquetado: esPlanar(cuadrado) = " << Z.esPlanar();
    for (int i = 0; i < 5; i++)
        for (int j = i + 1; j < 5; j++) K5e.agregarArcoNoDirigido(ciud[i], ciud[j], 1.0);
    cout << ", esPlanar(5 ciudades todas conectadas) = " << K5e.esPlanar();
    K5e.eliminarArcoNoDirigido("Merida", "Coro");
    cout << ", sin la arista Merida-Coro = " << K5e.esPlanar() << endl;
    GrafoEtiquetado<string> Ar;   // Caracas-Maracay-Valencia en fila, Valencia-Coro
    Ar.agregarVertice("Caracas"); Ar.agregarVertice("Maracay");
    Ar.agregarVertice("Valencia"); Ar.agregarVertice("Coro");
    Ar.agregarArcoNoDirigido("Caracas", "Maracay", 1.0);
    Ar.agregarArcoNoDirigido("Maracay", "Valencia", 1.0);
    Ar.agregarArcoNoDirigido("Valencia", "Coro", 1.0);
    imprimirEt("etiquetado: puntosArticulacion", Ar.puntosArticulacion());
    GrafoEtiquetado<string> Bf;   // costo negativo: Caracas->Coro->Maracay
    Bf.agregarVertice("Caracas"); Bf.agregarVertice("Maracay"); Bf.agregarVertice("Coro");
    Bf.agregarArco("Caracas", "Maracay", 10.0);
    Bf.agregarArco("Caracas", "Coro", 4.0);
    Bf.agregarArco("Coro", "Maracay", -2.0);
    map<string, double> bfd; map<string, string> bfp;
    cout << "etiquetado bellmanFord(Caracas): ok = " << Bf.bellmanFord("Caracas", bfd, bfp);
    for (map<string, double>::const_iterator it = bfd.begin(); it != bfd.end(); ++it)
        cout << " " << it->first << "=" << it->second;
    cout << ", pred[Maracay] = " << bfp["Maracay"] << endl;
    map<string, map<string, double> > fwd; map<string, map<string, string> > fws;
    cout << "etiquetado floydWarshall: ok = " << Bf.floydWarshall(fwd, fws)
         << ", d(Caracas,Maracay) = " << fwd["Caracas"]["Maracay"]
         << ", Maracay llega a " << fwd["Maracay"].size() << " vertice(s)" << endl;
    imprimirEt("  camino Caracas -> Maracay", Bf.reconstruirCamino(fws, "Caracas", "Maracay"));
    imprimirEt("  camino Maracay -> Caracas", Bf.reconstruirCamino(fws, "Maracay", "Caracas"));
    map<string, double> rmd; map<string, list<string> > rmr;
    cout << "etiquetado rutaMayorCosto(Caracas): ok = " << Bf.rutaMayorCosto("Caracas", rmd, rmr)
         << ", d(Maracay) = " << rmd["Maracay"] << endl;
    imprimirEt("  ruta Caracas -> Maracay", rmr["Maracay"]);
    cout << "  origen inexistente: " << Bf.rutaMayorCosto("Fantasma", rmd, rmr)
         << ", mapas vacios: " << (rmd.empty() && rmr.empty()) << endl;

    // ---- caminoSimple
    {
        Grafo D;   // DAG con dos caminos de 1 a 4: 1->4 y 1->3->4
        for (int i = 1; i <= 6; i++) D.agregarVertice(i);
        D.agregarArco(1, 3, 2.0); D.agregarArco(1, 4, 4.5); D.agregarArco(3, 4, 1.2);
        D.agregarArco(4, 5, 3.0); D.agregarArco(5, 6, 5.1); D.agregarArco(6, 2, 1.8);
        imprimir("caminoSimple DAG 1 -> 6", D.caminoSimple(1, 6));
        imprimir("caminoSimple DAG 6 -> 1 (inalcanzable)", D.caminoSimple(6, 1));
        imprimir("caminoSimple DAG 4 -> 4", D.caminoSimple(4, 4));
        cout << "  vertice inexistente: vacio = " << D.caminoSimple(1, 99).empty() << endl;

        Grafo C;   // no dirigido: ciclo 1-2-3-4-5-1 con cuerda 1-3
        for (int i = 1; i <= 5; i++) C.agregarVertice(i);
        for (int i = 1; i <= 5; i++) C.agregarArcoNoDirigido(i, i % 5 + 1, 1.0);
        C.agregarArcoNoDirigido(1, 3, 1.0);
        imprimir("caminoSimple no dirigido 1 -> 4", C.caminoSimple(1, 4));

        GrafoEtiquetado<string> E;
        E.agregarVertice("Caracas"); E.agregarVertice("Valencia");
        E.agregarVertice("Maracay"); E.agregarVertice("Merida");
        E.agregarArco("Caracas", "Maracay", 1.0);
        E.agregarArco("Maracay", "Valencia", 1.0);
        imprimirEt("caminoSimple etiquetado Caracas -> Valencia",
                   E.caminoSimple("Caracas", "Valencia"));
        cout << "  Caracas -> Merida (inalcanzable): vacio = "
             << E.caminoSimple("Caracas", "Merida").empty() << endl;
    }

    // ---- caminoMenorCosto
    {
        Grafo M;   // 1->4 directo cuesta 10; 1->2->3->4 cuesta 1+1+1 = 3
        for (int i = 1; i <= 5; i++) M.agregarVertice(i);
        M.agregarArco(1, 4, 10.0); M.agregarArco(1, 2, 1.0);
        M.agregarArco(2, 3, 1.0);  M.agregarArco(3, 4, 1.0);
        double costo;
        imprimir("caminoMenorCosto 1 -> 4", M.caminoMenorCosto(1, 4, costo));
        cout << "  costo = " << costo << endl;
        imprimir("caminoMenorCosto 1 -> 1", M.caminoMenorCosto(1, 1, costo));
        cout << "  costo = " << costo << endl;
        cout << "  4 -> 1 (inalcanzable): vacio = " << M.caminoMenorCosto(4, 1, costo).empty()
             << ", costo = " << costo << endl;
        cout << "  1 -> 5 (aislado): vacio = " << M.caminoMenorCosto(1, 5).empty() << endl;
        cout << "  vertice inexistente: vacio = " << M.caminoMenorCosto(1, 99).empty() << endl;
        M.agregarArco(2, 3, -1.0);
        cout << "  con un costo negativo: vacio = " << M.caminoMenorCosto(1, 4).empty() << endl;

        GrafoEtiquetado<string> W;
        W.agregarVertice("Caracas"); W.agregarVertice("Valencia");
        W.agregarVertice("Maracay"); W.agregarVertice("Merida");
        W.agregarArco("Caracas", "Valencia", 9.0);
        W.agregarArco("Caracas", "Maracay", 4.0);
        W.agregarArco("Maracay", "Valencia", 3.0);
        imprimirEt("caminoMenorCosto etiquetado Caracas -> Valencia",
                   W.caminoMenorCosto("Caracas", "Valencia", costo));
        cout << "  costo = " << costo << endl;
        cout << "  Caracas -> Merida (inalcanzable): vacio = "
             << W.caminoMenorCosto("Caracas", "Merida").empty() << endl;
    }

    // ---- esIsomorfo
    {
        Grafo A, B;   // ciclo dirigido 1->2->3->1 contra 7->9->4->7
        int va[3] = {1, 2, 3}, vb[3] = {7, 9, 4};
        for (int i = 0; i < 3; i++) { A.agregarVertice(va[i]); B.agregarVertice(vb[i]); }
        for (int i = 0; i < 3; i++) {
            A.agregarArco(va[i], va[(i + 1) % 3], 1.0);
            B.agregarArco(vb[i], vb[(i + 1) % 3], 5.0);
        }
        cout << "esIsomorfo ciclo 3 renumerado = " << A.esIsomorfo(B)
             << ", con costos = " << A.esIsomorfo(B, true) << endl;

        Grafo C6, T2;   // C6 contra dos triangulos: mismos grados, no isomorfos
        for (int i = 1; i <= 6; i++) { C6.agregarVertice(i); T2.agregarVertice(i); }
        for (int i = 1; i <= 6; i++) C6.agregarArcoNoDirigido(i, i % 6 + 1, 1.0);
        for (int t = 0; t < 2; t++)
            for (int i = 0; i < 3; i++)
                T2.agregarArcoNoDirigido(3 * t + i + 1, 3 * t + (i + 1) % 3 + 1, 1.0);
        cout << "esIsomorfo C6 vs dos triangulos = " << C6.esIsomorfo(T2) << endl;

        Grafo P, Q;   // Petersen contra prisma pentagonal: 3-regulares, 10 vertices
        for (int i = 1; i <= 10; i++) { P.agregarVertice(i); Q.agregarVertice(i); }
        for (int i = 0; i < 5; i++) {
            P.agregarArcoNoDirigido(i + 1, (i + 1) % 5 + 1, 1.0);          // exterior
            P.agregarArcoNoDirigido(i + 6, (i + 2) % 5 + 6, 1.0);          // estrella
            P.agregarArcoNoDirigido(i + 1, i + 6, 1.0);                    // radios
            Q.agregarArcoNoDirigido(i + 1, (i + 1) % 5 + 1, 1.0);
            Q.agregarArcoNoDirigido(i + 6, (i + 1) % 5 + 6, 1.0);
            Q.agregarArcoNoDirigido(i + 1, i + 6, 1.0);
        }
        cout << "esIsomorfo Petersen vs prisma pentagonal = " << P.esIsomorfo(Q)
             << ", Petersen vs si mismo = " << P.esIsomorfo(P) << endl;

        Grafo D1, D2;   // 1->2 contra 2->1 (isomorfos) y vertice+arco distinto
        D1.agregarVertice(1); D1.agregarVertice(2); D1.agregarArco(1, 2, 1.0);
        D2.agregarVertice(5); D2.agregarVertice(9); D2.agregarArco(9, 5, 1.0);
        cout << "esIsomorfo 1->2 vs 9->5 = " << D1.esIsomorfo(D2);
        D2.agregarArco(5, 5, 1.0);
        cout << ", con un bucle de mas = " << D1.esIsomorfo(D2) << endl;

        Grafo V1, V2;
        cout << "esIsomorfo dos grafos vacios = " << V1.esIsomorfo(V2) << endl;

        GrafoEtiquetado<string> L1;
        GrafoEtiquetado<char> L2;
        L1.agregarVertice("a"); L1.agregarVertice("b"); L1.agregarArco("a", "b", 1.0);
        L2.agregarVertice('x'); L2.agregarVertice('y'); L2.agregarArco('y', 'x', 1.0);
        GrafoEtiquetado<char> L3;
        L3.agregarVertice('p'); L3.agregarVertice('q'); L3.agregarArco('p', 'q', 1.0);
        cout << "esIsomorfo etiquetado (char) x<-y vs p->q = " << L2.esIsomorfo(L3) << endl;
    }

    G.destruir();
    cout << "esVacio tras destruir = " << G.esVacio() << endl;
    return 0;
}
