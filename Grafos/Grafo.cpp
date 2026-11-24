#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <algorithm>
#include <functional>
#include <utility>
#include <vector>
#include "Grafo.hpp"
#include "NodoAdy.hpp"
#include "NodoVertice.hpp"

// ============================================================================
// ORGANIZACION DEL ARCHIVO
// Los metodos estan agrupados por proposito. Las clases y funciones auxiliares
// internas (namespace anonimo / static) aparecen justo antes del primer metodo
// que las usa. Los metodos marcados "privado" estan declarados en la seccion
// private de Grafo.hpp.
//
//   1. CONSTRUCCION, DESTRUCCION Y COPIA
//   2. MODIFICACION DE VERTICES Y ARCOS
//   3. CONSULTAS BASICAS
//   4. RECORRIDOS
//   5. ORDEN TOPOLOGICO
//   6. CAMINOS Y DISTANCIAS
//   7. CICLOS
//   8. CAMINOS Y CICLOS EULERIANOS / HAMILTONIANOS
//   9. CONECTIVIDAD Y PROPIEDADES ESTRUCTURALES
//   10. ARBOLES DE EXPANSION
//   11. FLUJO, EMPAREJAMIENTO Y PROBLEMAS NP
//   12. OPERACIONES ENTRE GRAFOS
//   13. ENTRADA, SALIDA Y CONVERSION
// ============================================================================


// ============================================================================
// 1. CONSTRUCCION, DESTRUCCION Y COPIA
// ============================================================================

Grafo::Grafo() : g(NULL), cantVertices(0) {}

Grafo::~Grafo() { destruir(); }

void Grafo::construir() { destruir(); }

void Grafo::destruir() {
    while (g != NULL) {
        NodoVertice* t = g;
        g = g->getProx();
        NodoAdy* a = t->getListaAdy();
        while (a != NULL) {
            NodoAdy* ta = a;
            a = a->getProx();
            delete ta;
        }
        delete t;
    }
    cantVertices = 0;
}

void Grafo::copiar(const Grafo& otro) {
    if (&otro == this) return;
    otro.copiarA(*this, false);
}

void Grafo::intercambiar(Grafo& otro) {
    std::swap(g, otro.g);
    std::swap(cantVertices, otro.cantVertices);
}

void Grafo::tomar(NodoVertice* cabeza, int cantidad) {
    destruir();
    g = cabeza;
    cantVertices = cantidad;
}

// Auxiliar interno (no es metodo de Grafo). Usado por: copiarA, cantidadAristas, esSimetrico, unir, interseccion, ...
namespace {

// Construye una cadena de NodoVertice/NodoAdy en orden ascendente agregando
// al final en O(1). El cliente debe dar los vertices y, dentro de cada uno,
// los arcos, ya ordenados.
class Armador {
public:
    Armador() : cab(NULL), ult(NULL), ultAdy(NULL), cant(0) {}
    ~Armador() { liberar(); }
    void vertice(Vertice v) {
        NodoVertice* n = new NodoVertice(v, NULL, NULL);
        if (ult == NULL) cab = n; else ult->setProx(n);
        ult = n;
        ultAdy = NULL;
        cant++;
    }
    void arco(Vertice w, double c) {
        NodoAdy* n = new NodoAdy(w, c, NULL);
        if (ultAdy == NULL) ult->setListaAdy(n); else ultAdy->setProx(n);
        ultAdy = n;
    }
    int cantidad() const { return cant; }
    // Entrega la cadena (el Armador deja de ser duenio de ella)
    NodoVertice* soltar() {
        NodoVertice* r = cab;
        cab = ult = NULL;
        ultAdy = NULL;
        cant = 0;
        return r;
    }
private:
    void liberar() {
        while (cab != NULL) {
            NodoVertice* v = cab;
            cab = cab->getProx();
            NodoAdy* a = v->getListaAdy();
            while (a != NULL) { NodoAdy* t = a; a = a->getProx(); delete t; }
            delete v;
        }
    }
    Armador(const Armador&);
    Armador& operator=(const Armador&);
    NodoVertice* cab;
    NodoVertice* ult;
    NodoAdy* ultAdy;
    int cant;
};

void copiarVertice(Armador& A, const NodoVertice* p) {
    A.vertice(p->getInfo());
    for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
        A.arco(a->getInfo(), a->getCosto());
}

// Busca el arco u -> w en la lista (ordenada) de u. idx[u] debe existir.
bool buscarArcoIdx(const std::vector<NodoVertice*>& idx, int u, int w, double& costo) {
    for (NodoAdy* a = idx[u]->getListaAdy(); a != NULL && a->getInfo() <= w; a = a->getProx())
        if (a->getInfo() == w) { costo = a->getCosto(); return true; }
    return false;
}

// Pares {u, v} distintos con arco en algun sentido, y numero de bucles.
void aristasSubyacentes(NodoVertice* g, int& pares, int& bucles) {
    std::set<std::pair<int, int> > s;
    bucles = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            int u = p->getInfo(), w = a->getInfo();
            if (u == w) bucles++;
            else s.insert(std::make_pair(std::min(u, w), std::max(u, w)));
        }
    pares = (int)s.size();
}

// Grafo simple subyacente por POSICION: ids[i] = numero del vertice i
// (ascendente) y ady[i] = vecinos (posiciones, ascendentes), sin bucles.
void grafoSubyacente(NodoVertice* g, int tam, std::vector<int>& ids,
                     std::vector<std::vector<int> >& ady) {
    std::vector<int> pos(tam, -1);
    ids.clear();
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        pos[p->getInfo()] = (int)ids.size();
        ids.push_back(p->getInfo());
    }
    std::set<std::pair<int, int> > aristas;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            int u = pos[p->getInfo()], v = pos[a->getInfo()];
            if (u != v) aristas.insert(std::make_pair(std::min(u, v), std::max(u, v)));
        }
    ady.assign(ids.size(), std::vector<int>());
    for (std::set<std::pair<int, int> >::const_iterator it = aristas.begin();
         it != aristas.end(); ++it) {
        ady[it->first].push_back(it->second);
        ady[it->second].push_back(it->first);
    }
}

} // namespace

void Grafo::copiarA(Grafo& destino, bool costosUnitarios) const {
    Armador A;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        A.vertice(p->getInfo());
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            A.arco(a->getInfo(), costosUnitarios ? 1.0 : a->getCosto());
    }
    int c = A.cantidad();
    destino.tomar(A.soltar(), c);
}


// ============================================================================
// 2. MODIFICACION DE VERTICES Y ARCOS
// ============================================================================


// ---- Vertices --------------------------------------------------------------

// Pre: v >= 1 y v no existe en G
bool Grafo::agregarVertice(Vertice v) {
    if (v < 1) return false;
    NodoVertice* ant = NULL;
    NodoVertice* sig = g;
    while (sig != NULL && sig->getInfo() < v) {
        ant = sig;
        sig = sig->getProx();
    }
    if (sig != NULL && sig->getInfo() == v) return false;  // ya existe

    NodoVertice* nuevo = new NodoVertice(v, sig, NULL);
    if (ant == NULL) g = nuevo;
    else ant->setProx(nuevo);
    cantVertices++;
    return true;
}

// Elimina el vertice, sus arcos salientes y todos los arcos que llegan a el.
bool Grafo::eliminarVertice(Vertice v) {
    NodoVertice* ant = NULL;
    NodoVertice* act = g;
    while (act != NULL && act->getInfo() < v) {
        ant = act;
        act = act->getProx();
    }
    if (act == NULL || act->getInfo() != v) return false;

    // Desenlazar el vertice
    if (ant == NULL) g = act->getProx();
    else ant->setProx(act->getProx());

    // Liberar su lista de adyacencia
    NodoAdy* a = act->getListaAdy();
    while (a != NULL) {
        NodoAdy* t = a;
        a = a->getProx();
        delete t;
    }
    delete act;
    cantVertices--;

    // Eliminar los arcos que llegaban a v
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        quitarDeAdyacencia(p, v);
    return true;
}

// Auxiliar interno (no es metodo de Grafo). Usado por: renombrarVertice, contraerArista
namespace {
typedef std::pair<int, double> ArcoCosto;

// Salientes y entrantes de v (sin contar el bucle, que se devuelve aparte)
void arcosDe(const Grafo& G, Vertice v, std::vector<ArcoCosto>& salientes,
             std::vector<ArcoCosto>& entrantes, bool& hayBucle, double& costoBucle) {
    hayBucle = false;
    costoBucle = 0.0;
    std::list<Vertice> suc = G.sucesores(v);
    for (std::list<Vertice>::const_iterator it = suc.begin(); it != suc.end(); ++it) {
        if (*it == v) { hayBucle = true; costoBucle = G.costoArco(v, v); }
        else salientes.push_back(ArcoCosto(*it, G.costoArco(v, *it)));
    }
    std::list<Vertice> pre = G.predecesores(v);
    for (std::list<Vertice>::const_iterator it = pre.begin(); it != pre.end(); ++it)
        if (*it != v) entrantes.push_back(ArcoCosto(*it, G.costoArco(*it, v)));
}
} // namespace

bool Grafo::renombrarVertice(Vertice v, Vertice w) {
    if (!existeVertice(v) || w < 1) return false;
    if (v == w) return true;
    if (existeVertice(w)) return false;
    std::vector<ArcoCosto> sal, ent;
    bool bucle;
    double cb;
    arcosDe(*this, v, sal, ent, bucle, cb);
    eliminarVertice(v);
    agregarVertice(w);
    for (size_t i = 0; i < sal.size(); i++) agregarArco(w, sal[i].first, sal[i].second);
    for (size_t i = 0; i < ent.size(); i++) agregarArco(ent[i].first, w, ent[i].second);
    if (bucle) agregarArco(w, w, cb);
    return true;
}

int Grafo::aislarVertice(Vertice v) {
    int a = eliminarArcosDe(v);
    if (a < 0) return -1;
    return a + eliminarArcosHacia(v);
}


// ---- Arcos dirigidos -------------------------------------------------------

// Pre: v, w existen. Si el arco ya existe se actualiza su costo.
bool Grafo::agregarArco(Vertice v, Vertice w, double c) {
    NodoVertice* origen = buscarVertice(v);
    if (origen == NULL || !existeVertice(w)) return false;

    NodoAdy* ant = NULL;
    NodoAdy* sig = origen->getListaAdy();
    while (sig != NULL && sig->getInfo() < w) {
        ant = sig;
        sig = sig->getProx();
    }
    if (sig != NULL && sig->getInfo() == w) {
        sig->setCosto(c);
        return true;
    }
    NodoAdy* nuevo = new NodoAdy(w, c, sig);
    if (ant == NULL) origen->setListaAdy(nuevo);
    else ant->setProx(nuevo);
    return true;
}

bool Grafo::agregarArcoSiNoExiste(Vertice v, Vertice w, double c) {
    if (existeArco(v, w)) return false;
    return agregarArco(v, w, c);
}

// Pre: existe el arco (v, w)
bool Grafo::eliminarArco(Vertice v, Vertice w) {
    NodoVertice* origen = buscarVertice(v);
    if (origen == NULL) return false;
    return quitarDeAdyacencia(origen, w);
}

bool Grafo::actualizarCosto(Vertice v, Vertice w, double c) {
    if (!existeArco(v, w)) return false;
    return agregarArco(v, w, c);          // agregarArco actualiza si ya existe
}

int Grafo::eliminarArcosDe(Vertice v) {
    NodoVertice* p = buscarVertice(v);
    if (p == NULL) return -1;
    int c = 0;
    NodoAdy* a = p->getListaAdy();
    p->setListaAdy(NULL);
    while (a != NULL) {
        NodoAdy* t = a;
        a = a->getProx();
        delete t;
        c++;
    }
    return c;
}

int Grafo::eliminarArcosHacia(Vertice v) {
    if (!existeVertice(v)) return -1;
    int c = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        if (quitarDeAdyacencia(p, v)) c++;
    return c;
}


// ---- Arcos no dirigidos ----------------------------------------------------

// Agrega (v, w) y (w, v) con el mismo costo. Pre: v y w existen.
bool Grafo::agregarArcoNoDirigido(Vertice v, Vertice w, double c) {
    if (!existeVertice(v) || !existeVertice(w)) return false;
    agregarArco(v, w, c);
    agregarArco(w, v, c);
    return true;
}

// Elimina (v, w) y (w, v). Devuelve true si se elimino al menos uno.
bool Grafo::eliminarArcoNoDirigido(Vertice v, Vertice w) {
    bool a = eliminarArco(v, w);
    bool b = eliminarArco(w, v);
    return a || b;
}

bool Grafo::actualizarCostoNoDirigido(Vertice v, Vertice w, double c) {
    if (!existeArco(v, w) || !existeArco(w, v)) return false;
    agregarArco(v, w, c);
    agregarArco(w, v, c);
    return true;
}


// ---- Transformaciones ------------------------------------------------------

bool Grafo::contraerArista(Vertice u, Vertice v) {
    if (u == v || !existeVertice(u) || !existeVertice(v)) return false;
    std::vector<ArcoCosto> sal, ent;
    bool bucle;
    double cb;
    arcosDe(*this, v, sal, ent, bucle, cb);
    eliminarVertice(v);
    // Fusion: si el arco ya existe se queda el de menor costo
    for (size_t i = 0; i < sal.size(); i++) {
        int w = sal[i].first;
        if (w == u) continue;                       // arco v -> u: se descarta
        if (!existeArco(u, w) || sal[i].second < costoArco(u, w))
            agregarArco(u, w, sal[i].second);
    }
    for (size_t i = 0; i < ent.size(); i++) {
        int x = ent[i].first;
        if (x == u) continue;                       // arco u -> v: ya no existe
        if (!existeArco(x, u) || ent[i].second < costoArco(x, u))
            agregarArco(x, u, ent[i].second);
    }
    if (bucle && (!existeArco(u, u) || cb < costoArco(u, u))) agregarArco(u, u, cb);
    return true;
}

// Recorre los vertices en orden ascendente y mueve cada nodo de adyacencia
// (u -> v) a la cola de la nueva lista de v, cambiando su info a u. Como u
// crece, cada lista nueva queda ordenada sin tener que ordenarla. Las
// listas viejas se consumen nodo a nodo, asi que no hay copias ni new/delete.
void Grafo::invertirArcos() {
    int tam = maxVertice() + 1;
    std::vector<NodoAdy*> cab(tam, (NodoAdy*)NULL);
    std::vector<NodoAdy*> cola(tam, (NodoAdy*)NULL);

    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        NodoAdy* a = p->getListaAdy();
        p->setListaAdy(NULL);
        while (a != NULL) {
            NodoAdy* sig = a->getProx();
            Vertice destino = a->getInfo();       // el arco era p -> destino
            a->setInfo(p->getInfo());             // ahora es destino -> p
            a->setProx(NULL);
            if (cab[destino] == NULL) cab[destino] = a;
            else cola[destino]->setProx(a);
            cola[destino] = a;
            a = sig;
        }
    }
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        p->setListaAdy(cab[p->getInfo()]);
}

void Grafo::reverseGraph() { invertirArcos(); }

bool Grafo::construirDesdeAristas(const std::list<Arista>& aristas, bool noDirigido) {
    destruir();
    bool ok = true;
    for (std::list<Arista>::const_iterator it = aristas.begin(); it != aristas.end(); ++it) {
        if (it->origen < 1 || it->destino < 1) { ok = false; continue; }
        agregarVertice(it->origen);     // ignorado si ya existe
        agregarVertice(it->destino);
        if (noDirigido) agregarArcoNoDirigido(it->origen, it->destino, it->costo);
        else agregarArco(it->origen, it->destino, it->costo);
    }
    return ok;
}


// ---- Auxiliar privado ------------------------------------------------------

// Elimina el arco (origen, w) de la lista de adyacencia de origen.
bool Grafo::quitarDeAdyacencia(NodoVertice* origen, Vertice w) {
    NodoAdy* ant = NULL;
    NodoAdy* act = origen->getListaAdy();
    while (act != NULL && act->getInfo() < w) {
        ant = act;
        act = act->getProx();
    }
    if (act == NULL || act->getInfo() != w) return false;

    if (ant == NULL) origen->setListaAdy(act->getProx());
    else ant->setProx(act->getProx());
    delete act;
    return true;
}


// ============================================================================
// 3. CONSULTAS BASICAS
// ============================================================================


// ---- Existencia y costo ----------------------------------------------------

bool Grafo::esVacio() const { return g == NULL; }

bool Grafo::existeVertice(Vertice v) const {
    return buscarVertice(v) != NULL;
}

bool Grafo::existeArco(Vertice v, Vertice w) const {
    NodoVertice* origen = buscarVertice(v);
    if (origen == NULL) return false;
    NodoAdy* a = origen->getListaAdy();
    while (a != NULL && a->getInfo() < w)
        a = a->getProx();
    return (a != NULL && a->getInfo() == w);
}

double Grafo::costoArco(Vertice v, Vertice w) const {
    NodoVertice* origen = buscarVertice(v);
    if (origen == NULL) return -1.0;
    NodoAdy* a = origen->getListaAdy();
    while (a != NULL && a->getInfo() < w)
        a = a->getProx();
    if (a != NULL && a->getInfo() == w) return a->getCosto();
    return -1.0;
}

// Busca un vertice aprovechando que la lista esta ordenada. NULL si no existe.
NodoVertice* Grafo::buscarVertice(Vertice v) const {
    NodoVertice* p = g;
    while (p != NULL && p->getInfo() < v)
        p = p->getProx();
    if (p != NULL && p->getInfo() == v) return p;
    return NULL;
}


// ---- Tamano ----------------------------------------------------------------

int Grafo::orden() const { return cantVertices; }

Vertice Grafo::maxVertice() const {
    NodoVertice* p = g;
    if (p == NULL) return 0;
    while (p->getProx() != NULL) p = p->getProx();
    return p->getInfo();
}

int Grafo::cantidadArcos() const {
    int m = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            m++;
    return m;
}

int Grafo::cantidadAristas() const {
    int pares, bucles;
    aristasSubyacentes(g, pares, bucles);
    return pares + bucles;
}

double Grafo::densidad() const {
    int n = cantVertices;
    if (n < 2) return 0.0;
    int m = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            if (a->getInfo() != p->getInfo()) m++;
    return (double)m / ((double)n * (double)(n - 1));
}


// ---- Grados ----------------------------------------------------------------

int Grafo::grado(Vertice v) const {
    if (!existeVertice(v)) return -1;
    return gradoInterior(v) + gradoExterior(v);
}

// Cuenta cuantas listas de adyacencia contienen a v. O(n + m).
// Un bucle (v, v) cuenta aqui y tambien en gradoExterior.
int Grafo::gradoInterior(Vertice v) const {
    if (!existeVertice(v)) return -1;
    int cont = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        NodoAdy* a = p->getListaAdy();
        while (a != NULL && a->getInfo() < v)
            a = a->getProx();
        if (a != NULL && a->getInfo() == v)
            cont++;
    }
    return cont;
}

// Cuenta los nodos de la lista de adyacencia de v. O(n + grad+(v)).
int Grafo::gradoExterior(Vertice v) const {
    NodoVertice* origen = buscarVertice(v);
    if (origen == NULL) return -1;
    int cont = 0;
    for (NodoAdy* a = origen->getListaAdy(); a != NULL; a = a->getProx())
        cont++;
    return cont;
}

// O(n + m): recorre cada lista de adyacencia una sola vez.
void Grafo::obtenerGradosDeEntrada(std::vector<int>& gradoDeEntrada) const {
    gradoDeEntrada.assign(maxVertice() + 1, 0);

    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            gradoDeEntrada[a->getInfo()]++;
}


// ---- Vecindad --------------------------------------------------------------

std::list<Vertice> Grafo::sucesores(Vertice v) const {
    std::list<Vertice> L;
    NodoVertice* origen = buscarVertice(v);
    if (origen == NULL) return L;
    for (NodoAdy* a = origen->getListaAdy(); a != NULL; a = a->getProx())
        L.push_back(a->getInfo());
    return L;
}

// Recorre todas las listas de adyacencia buscando a v.
// (Corrige el PDF, donde se asignaba "band" en lugar de "encontrado".)
std::list<Vertice> Grafo::predecesores(Vertice v) const {
    std::list<Vertice> L;
    for (NodoVertice* vact = g; vact != NULL; vact = vact->getProx()) {
        NodoAdy* act = vact->getListaAdy();
        bool encontrado = false;
        while (act != NULL && !encontrado && act->getInfo() <= v) {
            if (act->getInfo() == v) {
                L.push_back(vact->getInfo());
                encontrado = true;
            }
            act = act->getProx();
        }
    }
    return L;
}

// O(n + m): vertices con grado de entrada 0.
std::list<Vertice> Grafo::obtenerVerticesFuente() const {
    std::list<Vertice> fuentes;
    std::vector<int> grado;
    obtenerGradosDeEntrada(grado);

    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        if (grado[p->getInfo()] == 0)
            fuentes.push_back(p->getInfo());
    return fuentes;
}


// ============================================================================
// 4. RECORRIDOS
// ============================================================================

// dist y pred se redimensionan a maxVertice()+1.
// dist[v] = INFINITO si v no es alcanzable desde s; pred[v] = 0 si no tiene.
void Grafo::BFS(Vertice s, std::vector<int>& dist, std::vector<int>& pred) const {
    int tam = maxVertice() + 1;
    dist.assign(tam, INFINITO);
    pred.assign(tam, 0);
    if (!existeVertice(s)) return;

    std::vector<int> color(tam, 0);  // 0 = blanco, 1 = gris, 2 = negro

    color[s] = 1;
    dist[s] = 0;
    std::queue<Vertice> C;
    C.push(s);

    while (!C.empty()) {
        Vertice u = C.front();
        C.pop();
        std::list<Vertice> L = sucesores(u);
        while (!L.empty()) {
            Vertice v = L.front();
            L.pop_front();
            if (color[v] == 0) {
                color[v] = 1;
                dist[v] = dist[u] + 1;
                pred[v] = u;
                C.push(v);
            }
        }
        color[u] = 2;
    }
}

// Recorrido en amplitud: devuelve los vertices alcanzables desde s en el
// orden en que son visitados (s primero). Lista vacia si s no existe.
std::list<Vertice> Grafo::recorridoBFS(Vertice s) const {
    std::list<Vertice> orden;
    if (!existeVertice(s)) return orden;

    std::vector<int> visitado(maxVertice() + 1, 0);
    std::queue<Vertice> C;
    visitado[s] = 1;
    C.push(s);

    while (!C.empty()) {
        Vertice u = C.front();
        C.pop();
        orden.push_back(u);

        std::list<Vertice> L = sucesores(u);
        while (!L.empty()) {
            Vertice v = L.front();
            L.pop_front();
            if (!visitado[v]) {
                visitado[v] = 1;
                C.push(v);
            }
        }
    }
    return orden;
}

void Grafo::DFS(std::vector<int>& pred, std::vector<int>& tdesc,
                std::vector<int>& tfinal) const {
    int tam = maxVertice() + 1;
    std::vector<int> color(tam, 0);
    pred.assign(tam, 0);
    tdesc.assign(tam, 0);
    tfinal.assign(tam, 0);

    int tiempo = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        Vertice u = p->getInfo();
        if (color[u] == 0)
            DFS_Visitar(u, pred, tdesc, tfinal, color, tiempo);
    }
}

// Recorrido en profundidad: devuelve los vertices alcanzables desde s en el
// orden en que son descubiertos (s primero). Lista vacia si s no existe.
std::list<Vertice> Grafo::recorridoDFS(Vertice s) const {
    std::list<Vertice> orden;
    if (!existeVertice(s)) return orden;

    std::vector<int> visitado(maxVertice() + 1, 0);
    recorridoDFS_Visitar(s, visitado, orden);
    return orden;
}

void Grafo::DFS_Visitar(Vertice u, std::vector<int>& pred, std::vector<int>& tdesc,
                        std::vector<int>& tfinal, std::vector<int>& color,
                        int& tiempo) const {
    color[u] = 1;
    tiempo++;
    tdesc[u] = tiempo;

    std::list<Vertice> L = sucesores(u);
    while (!L.empty()) {
        Vertice v = L.front();
        L.pop_front();
        if (color[v] == 0) {
            pred[v] = u;
            DFS_Visitar(v, pred, tdesc, tfinal, color, tiempo);
        }
    }
    color[u] = 2;
    tiempo++;
    tfinal[u] = tiempo;
}

void Grafo::recorridoDFS_Visitar(Vertice u, std::vector<int>& visitado,
                                 std::list<Vertice>& orden) const {
    visitado[u] = 1;
    orden.push_back(u);

    std::list<Vertice> L = sucesores(u);
    while (!L.empty()) {
        Vertice v = L.front();
        L.pop_front();
        if (!visitado[v])
            recorridoDFS_Visitar(v, visitado, orden);
    }
}

// Auxiliar interno (no es metodo de Grafo). Usado por: tieneCiclos, tieneCiclosNoDirigido, alcanzables, rutaMayorCosto, buscarCaminoBFS, ...
// Indice vertice -> nodo para acceder a cada lista de adyacencia en O(1).
static void construirIndice(NodoVertice* g, int tam, std::vector<NodoVertice*>& idx) {
    idx.assign(tam, (NodoVertice*)NULL);
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        idx[p->getInfo()] = p;
}

// Busqueda iterativa (pila explicita) con indice vertice -> nodo: O(n + m).
std::list<Vertice> Grafo::alcanzables(Vertice s) const {
    std::list<Vertice> res;
    if (!existeVertice(s)) return res;

    int tam = maxVertice() + 1;
    std::vector<NodoVertice*> idx;
    construirIndice(g, tam, idx);
    std::vector<char> visitado(tam, 0);
    std::vector<int> pila(1, s);
    visitado[s] = 1;

    while (!pila.empty()) {
        int u = pila.back();
        pila.pop_back();
        for (NodoAdy* a = idx[u]->getListaAdy(); a != NULL; a = a->getProx()) {
            Vertice v = a->getInfo();
            if (!visitado[v]) {
                visitado[v] = 1;
                pila.push_back(v);
            }
        }
    }
    // La lista de vertices esta ordenada: el resultado sale ascendente.
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        if (visitado[p->getInfo()])
            res.push_back(p->getInfo());
    return res;
}

void Grafo::clausuraTransitiva(std::vector<std::vector<char> >& alcanza) const {
    int tam = maxVertice() + 1;
    alcanza.assign(tam, std::vector<char>(tam, 0));
    std::vector<int> ids;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        ids.push_back(p->getInfo());
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            alcanza[p->getInfo()][a->getInfo()] = 1;
    }
    for (size_t kk = 0; kk < ids.size(); kk++) {
        int k = ids[kk];
        for (size_t ii = 0; ii < ids.size(); ii++) {
            int i = ids[ii];
            if (!alcanza[i][k]) continue;
            for (size_t jj = 0; jj < ids.size(); jj++) {
                int j = ids[jj];
                if (alcanza[k][j]) alcanza[i][j] = 1;
            }
        }
    }
}


// ============================================================================
// 5. ORDEN TOPOLOGICO
// ============================================================================

// Version BFS modificado (algoritmo de Kahn).
std::list<Vertice> Grafo::ordenamientoTopologico() const {
    std::list<Vertice> ord;
    std::vector<int> grado;
    std::queue<Vertice> C;

    obtenerGradosDeEntrada(grado);
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        if (grado[p->getInfo()] == 0)
            C.push(p->getInfo());

    while (!C.empty()) {
        Vertice u = C.front();
        C.pop();
        ord.push_back(u);

        std::list<Vertice> L = sucesores(u);
        while (!L.empty()) {
            Vertice v = L.front();
            L.pop_front();
            grado[v]--;
            if (grado[v] == 0) C.push(v);
        }
    }
    return ord;
}

// Version DFS: cada vertice se inserta al principio al terminar de visitarlo.
std::list<Vertice> Grafo::ordenamientoTopologicoDFS() const {
    std::list<Vertice> ord;
    std::list<Vertice> fuentes = obtenerVerticesFuente();
    std::vector<int> color(maxVertice() + 1, 0);

    while (!fuentes.empty()) {
        Vertice u = fuentes.front();
        fuentes.pop_front();
        if (color[u] == 0)
            DFS_VisitarTopo(u, color, ord);
    }
    return ord;
}

// (Corrige el PDF, donde se usaba "pred" sin declararlo.)
void Grafo::DFS_VisitarTopo(Vertice u, std::vector<int>& color,
                            std::list<Vertice>& ord) const {
    color[u] = 1;
    std::list<Vertice> L = sucesores(u);
    while (!L.empty()) {
        Vertice v = L.front();
        L.pop_front();
        if (color[v] == 0)
            DFS_VisitarTopo(v, color, ord);
    }
    color[u] = 2;
    ord.push_front(u);
}


// ============================================================================
// 6. CAMINOS Y DISTANCIAS
// ============================================================================


// ---- Desde un origen -------------------------------------------------------

// O((n + m) log n): cola de prioridad (min-heap) con eliminacion perezosa.
// Se construye un indice vertice -> nodo para acceder a la lista de
// adyacencia de cada vertice en O(1) en lugar de buscarlo en la lista.
bool Grafo::dijkstra(Vertice s, std::vector<double>& dist,
                     std::vector<int>& pred) const {
    int tam = maxVertice() + 1;
    dist.assign(tam, DIST_INFINITA);
    pred.assign(tam, 0);
    if (!existeVertice(s)) return false;

    std::vector<NodoVertice*> idx(tam, (NodoVertice*)NULL);
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        idx[p->getInfo()] = p;
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            if (a->getCosto() < 0) return false;   // Dijkstra exige pesos >= 0
    }

    typedef std::pair<double, Vertice> Par;        // (distancia, vertice)
    std::priority_queue<Par, std::vector<Par>, std::greater<Par> > Q;

    dist[s] = 0.0;
    Q.push(Par(0.0, s));

    while (!Q.empty()) {
        Par top = Q.top();
        Q.pop();
        double d = top.first;
        Vertice u = top.second;
        if (d > dist[u]) continue;                 // entrada desactualizada

        for (NodoAdy* a = idx[u]->getListaAdy(); a != NULL; a = a->getProx()) {
            Vertice v = a->getInfo();
            double nd = d + a->getCosto();
            if (nd < dist[v]) {
                dist[v] = nd;
                pred[v] = u;
                Q.push(Par(nd, v));
            }
        }
    }
    return true;
}

std::vector<double> Grafo::dijkstra(Vertice s) const {
    std::vector<double> dist;
    std::vector<int> pred;
    if (!dijkstra(s, dist, pred)) dist.clear();
    return dist;
}

bool Grafo::bellmanFord(Vertice s, std::vector<double>& dist, std::vector<int>& pred) const {
    if (!existeVertice(s)) return false;
    int tam = maxVertice() + 1;
    dist.assign(tam, DIST_INFINITA);
    pred.assign(tam, 0);
    dist[s] = 0.0;

    std::vector<Arista> arcos;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            arcos.push_back(Arista(p->getInfo(), a->getInfo(), a->getCosto()));

    // n - 1 rondas bastan; si una ronda no cambia nada se termina antes
    for (int ronda = 1; ronda < cantVertices; ronda++) {
        bool cambio = false;
        for (size_t i = 0; i < arcos.size(); i++) {
            const Arista& e = arcos[i];
            if (dist[e.origen] == DIST_INFINITA) continue;
            if (dist[e.origen] + e.costo < dist[e.destino]) {
                dist[e.destino] = dist[e.origen] + e.costo;
                pred[e.destino] = e.origen;
                cambio = true;
            }
        }
        if (!cambio) return true;
    }
    // Una ronda mas: si aun se puede mejorar, hay un ciclo negativo alcanzable
    for (size_t i = 0; i < arcos.size(); i++) {
        const Arista& e = arcos[i];
        if (dist[e.origen] != DIST_INFINITA &&
            dist[e.origen] + e.costo < dist[e.destino])
            return false;
    }
    return true;
}

std::vector<double> Grafo::bellmanFord(Vertice s) const {
    std::vector<double> d;
    std::vector<int> p;
    if (!bellmanFord(s, d, p)) return std::vector<double>();
    return d;
}

// Auxiliar interno (no es metodo de Grafo). Usado por: rutaMayorCosto
namespace {

// DFS desde u sobre lo alcanzable. Devuelve false si encuentra un ciclo
// (arco a un vertice gris; un bucle tambien lo es). orden = postorden.
bool mayorCostoDAG(int u, const std::vector<NodoVertice*>& idx,
                   std::vector<int>& color, std::vector<int>& orden) {
    color[u] = 1;
    for (NodoAdy* a = idx[u]->getListaAdy(); a != NULL; a = a->getProx()) {
        int v = a->getInfo();
        if (color[v] == 1) return false;
        if (color[v] == 0 && !mayorCostoDAG(v, idx, color, orden)) return false;
    }
    color[u] = 2;
    orden.push_back(u);
    return true;
}

// Backtracking: recorre todos los caminos simples desde s y se queda con el
// mayor costo hacia cada vertice. camino = vertices del camino actual.
void mayorCostoSimple(int u, double acum, const std::vector<NodoVertice*>& idx,
                      std::vector<char>& vis, std::vector<int>& camino,
                      std::vector<double>& dist,
                      std::vector<std::list<Vertice> >& ruta) {
    for (NodoAdy* a = idx[u]->getListaAdy(); a != NULL; a = a->getProx()) {
        int w = a->getInfo();
        if (vis[w]) continue;                     // camino simple (y sin bucles)
        double costo = acum + a->getCosto();
        vis[w] = 1;
        camino.push_back(w);
        if (costo > dist[w]) {
            dist[w] = costo;
            ruta[w].assign(camino.begin(), camino.end());
        }
        mayorCostoSimple(w, costo, idx, vis, camino, dist, ruta);
        camino.pop_back();
        vis[w] = 0;
    }
}

} // namespace

bool Grafo::rutaMayorCosto(Vertice s, std::vector<double>& dist,
                           std::vector<std::list<Vertice> >& ruta) const {
    if (!existeVertice(s)) return false;
    int tam = maxVertice() + 1;
    dist.assign(tam, -DIST_INFINITA);
    ruta.assign(tam, std::list<Vertice>());
    dist[s] = 0.0;
    ruta[s].push_back(s);

    std::vector<NodoVertice*> idx;
    construirIndice(g, tam, idx);

    std::vector<int> color(tam, 0), orden;
    if (mayorCostoDAG(s, idx, color, orden)) {
        // Sin ciclos: programacion dinamica en orden topologico (el inverso
        // del postorden). Aqui el prefijo de una mejor ruta si es mejor ruta.
        std::vector<int> pred(tam, 0);
        for (size_t k = orden.size(); k-- > 0; ) {
            int u = orden[k];
            if (dist[u] == -DIST_INFINITA) continue;
            for (NodoAdy* a = idx[u]->getListaAdy(); a != NULL; a = a->getProx()) {
                int v = a->getInfo();
                if (dist[u] + a->getCosto() > dist[v]) {
                    dist[v] = dist[u] + a->getCosto();
                    pred[v] = u;
                }
            }
        }
        for (size_t k = 0; k < orden.size(); k++) {
            int v = orden[k];
            if (v == s) continue;
            for (int x = v; x != 0; x = pred[x]) ruta[v].push_front(x);
        }
        return true;
    }

    std::vector<char> vis(tam, 0);
    std::vector<int> camino;
    vis[s] = 1;
    camino.push_back(s);
    mayorCostoSimple(s, 0.0, idx, vis, camino, dist, ruta);
    return true;
}


// ---- Todos los pares -------------------------------------------------------

bool Grafo::floydWarshall(std::vector<std::vector<double> >& dist) const {
    std::vector<std::vector<int> > sig;
    return floydWarshall(dist, sig);
}

// Se calcula sobre indices compactos 0..n-1 (solo los vertices que existen)
// y al final se vuelca a matrices indexadas por numero de vertice.
bool Grafo::floydWarshall(std::vector<std::vector<double> >& dist,
                          std::vector<std::vector<int> >& sig) const {
    int n = cantVertices;
    int tam = maxVertice() + 1;
    dist.assign(tam, std::vector<double>(tam, DIST_INFINITA));
    sig.assign(tam, std::vector<int>(tam, 0));

    std::vector<int> ids, pos(tam, -1);
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        pos[p->getInfo()] = (int)ids.size();
        ids.push_back(p->getInfo());
    }

    std::vector<std::vector<double> > D(n, std::vector<double>(n, DIST_INFINITA));
    std::vector<std::vector<int> > S(n, std::vector<int>(n, -1));   // indice compacto
    for (int i = 0; i < n; i++) D[i][i] = 0.0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        int u = pos[p->getInfo()];
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            int v = pos[a->getInfo()];
            if (a->getCosto() < D[u][v]) {        // un bucle solo cuenta si es negativo
                D[u][v] = a->getCosto();
                S[u][v] = v;
            }
        }
    }
    for (int i = 0; i < n; i++)
        if (D[i][i] < 0.0) return false;          // bucle negativo

    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            if (D[i][k] == DIST_INFINITA) continue;
            for (int j = 0; j < n; j++) {
                if (D[k][j] == DIST_INFINITA) continue;
                double nuevo = D[i][k] + D[k][j];
                if (nuevo < D[i][j]) {
                    D[i][j] = nuevo;
                    S[i][j] = S[i][k];
                }
            }
        }
        // Un ciclo negativo deja un valor negativo en la diagonal; se corta
        // aqui para que las distancias no sigan decreciendo sin control.
        for (int i = 0; i < n; i++)
            if (D[i][i] < 0.0) return false;
    }

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            dist[ids[i]][ids[j]] = D[i][j];
            if (i != j && S[i][j] != -1) sig[ids[i]][ids[j]] = ids[S[i][j]];
        }
    return true;
}

std::list<Vertice> Grafo::reconstruirCamino(const std::vector<std::vector<int> >& sig,
                                            Vertice u, Vertice v) const {
    std::list<Vertice> camino;
    if (!existeVertice(u) || !existeVertice(v)) return camino;
    if ((int)sig.size() <= maxVertice()) return camino;      // sig no es de este grafo
    camino.push_back(u);
    int pasos = 0;
    Vertice act = u;
    while (act != v) {
        if ((int)sig[act].size() <= maxVertice()) return std::list<Vertice>();
        Vertice prox = sig[act][v];
        if (prox == 0 || ++pasos > cantVertices)             // inalcanzable o sig corrupta
            return std::list<Vertice>();
        camino.push_back(prox);
        act = prox;
    }
    return camino;
}


// ---- Entre dos vertices ----------------------------------------------------

bool Grafo::existeCamino(Vertice u, Vertice v) const {
    if (!existeVertice(u) || !existeVertice(v)) return false;
    if (u == v) return true;
    std::vector<int> pred;
    return buscarCaminoBFS(u, v, pred);
}

std::list<Vertice> Grafo::caminoSimple(Vertice u, Vertice v) const {
    std::list<Vertice> camino;
    if (!existeVertice(u) || !existeVertice(v)) return camino;
    std::vector<int> pred;
    if (!buscarCaminoBFS(u, v, pred)) return camino;
    for (Vertice x = v; x != u; x = pred[x])      // si u == v no entra: [u]
        camino.push_front(x);
    camino.push_front(u);
    return camino;
}

// Usa solo metodos publicos (dijkstra): no toca ningun puntero.
std::list<Vertice> Grafo::caminoMenorCosto(Vertice u, Vertice v, double& costo) const {
    std::list<Vertice> camino;
    costo = -1.0;
    if (!existeVertice(u) || !existeVertice(v)) return camino;
    std::vector<double> dist;
    std::vector<int> pred;
    if (!dijkstra(u, dist, pred)) return camino;          // costo negativo
    if (dist[v] == DIST_INFINITA) return camino;          // v inalcanzable

    costo = dist[v];
    for (Vertice x = v; x != u; x = pred[x])              // si u == v no entra: [u]
        camino.push_front(x);
    camino.push_front(u);
    return camino;
}

std::list<Vertice> Grafo::caminoMenorCosto(Vertice u, Vertice v) const {
    double costo;
    return caminoMenorCosto(u, v, costo);
}

// Usa solo metodos publicos (caminoSimple = BFS con indice, O(n + m)).
std::list<Vertice> Grafo::getCaminoMasCortoArcos(Vertice v, Vertice w, Vertice intermedio) const {
    std::list<Vertice> camino = caminoSimple(v, intermedio);
    if (camino.empty()) return camino;
    std::list<Vertice> resto = caminoSimple(intermedio, w);
    if (resto.empty()) return resto;
    resto.pop_front();                             // intermedio ya esta en camino
    camino.splice(camino.end(), resto);
    return camino;
}

std::list<Vertice> Grafo::caminoMasLargoArcos(Vertice u, Vertice v) const {
    if (!existeVertice(u) || !existeVertice(v)) return std::list<Vertice>();
    Grafo h;
    copiarA(h, true);                              // mismos arcos, costo 1
    std::vector<double> dist;
    std::vector<std::list<Vertice> > ruta;
    h.rutaMayorCosto(u, dist, ruta);
    return ruta[v];                                // vacia si es inalcanzable
}

std::list<Vertice> Grafo::caminoMasLargoArcos() const {
    std::list<Vertice> mejor;
    if (g == NULL) return mejor;
    Grafo h;
    copiarA(h, true);
    double mejorD = -1.0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        std::vector<double> dist;
        std::vector<std::list<Vertice> > ruta;
        h.rutaMayorCosto(p->getInfo(), dist, ruta);
        for (NodoVertice* q = g; q != NULL; q = q->getProx()) {
            int v = q->getInfo();
            if (dist[v] > mejorD) {
                mejorD = dist[v];
                mejor = ruta[v];
            }
        }
    }
    return mejor;
}

std::list<std::list<Vertice> > Grafo::kCaminosMasCortos(Vertice u, Vertice v, int k) const {
    std::list<double> costos;
    return kCaminosMasCortos(u, v, k, costos);
}

// Auxiliar interno (no es metodo de Grafo). Usado por: kCaminosMasCortos
namespace {

typedef std::vector<std::vector<std::pair<int, double> > > AdyCosto;

// Dijkstra de s a t que ignora los vertices vetados y los arcos vetados.
bool dijkstraVetado(int s, int t, const AdyCosto& ady, const std::vector<char>& vetado,
                    const std::set<std::pair<int, int> >& arcosVetados,
                    std::vector<int>& camino, double& costo) {
    int tam = (int)ady.size();
    std::vector<double> dist(tam, DIST_INFINITA);
    std::vector<int> pred(tam, -1);
    std::vector<char> hecho(tam, 0);
    typedef std::pair<double, int> Par;
    std::priority_queue<Par, std::vector<Par>, std::greater<Par> > cola;
    dist[s] = 0.0;
    cola.push(Par(0.0, s));
    while (!cola.empty()) {
        Par top = cola.top();
        cola.pop();
        int u = top.second;
        if (hecho[u]) continue;
        hecho[u] = 1;
        if (u == t) break;
        for (size_t i = 0; i < ady[u].size(); i++) {
            int w = ady[u][i].first;
            if (vetado[w] || hecho[w]) continue;
            if (!arcosVetados.empty() && arcosVetados.count(std::make_pair(u, w))) continue;
            double nd = dist[u] + ady[u][i].second;
            if (nd < dist[w]) {
                dist[w] = nd;
                pred[w] = u;
                cola.push(Par(nd, w));
            }
        }
    }
    if (dist[t] == DIST_INFINITA) return false;
    camino.clear();
    for (int x = t; x != -1; x = pred[x]) camino.push_back(x);
    std::reverse(camino.begin(), camino.end());
    costo = dist[t];
    return true;
}

double costoEntre(const AdyCosto& ady, int u, int w) {
    for (size_t i = 0; i < ady[u].size(); i++)
        if (ady[u][i].first == w) return ady[u][i].second;
    return 0.0;
}

} // namespace

std::list<std::list<Vertice> > Grafo::kCaminosMasCortos(Vertice u, Vertice v, int k,
                                                        std::list<double>& costos) const {
    std::list<std::list<Vertice> > res;
    costos.clear();
    if (k <= 0 || !existeVertice(u) || !existeVertice(v)) return res;
    int tam = maxVertice() + 1;
    AdyCosto ady(tam);
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            if (a->getCosto() < 0.0) return res;
            if (a->getInfo() != p->getInfo())
                ady[p->getInfo()].push_back(std::make_pair(a->getInfo(), a->getCosto()));
        }
    if (u == v) {
        res.push_back(std::list<Vertice>(1, u));
        costos.push_back(0.0);
        return res;
    }

    std::vector<std::vector<int> > A;              // caminos aceptados
    std::vector<double> costoA;
    std::set<std::pair<double, std::vector<int> > > B;   // candidatos
    std::vector<char> ninguno(tam, 0);
    std::set<std::pair<int, int> > sinArcos;
    std::vector<int> primero;
    double c0;
    if (!dijkstraVetado(u, v, ady, ninguno, sinArcos, primero, c0)) return res;
    A.push_back(primero);
    costoA.push_back(c0);

    for (int kk = 1; kk < k; kk++) {
        const std::vector<int> ultimo = A[kk - 1];
        std::vector<double> prefijo(ultimo.size(), 0.0);   // costo de ultimo[0..i]
        for (size_t i = 1; i < ultimo.size(); i++)
            prefijo[i] = prefijo[i - 1] + costoEntre(ady, ultimo[i - 1], ultimo[i]);
        for (size_t i = 0; i + 1 < ultimo.size(); i++) {
            int spur = ultimo[i];
            std::set<std::pair<int, int> > arcosVet;
            for (size_t a = 0; a < A.size(); a++) {
                if (A[a].size() <= i + 1) continue;
                bool mismaRaiz = true;
                for (size_t j = 0; j <= i && mismaRaiz; j++)
                    if (A[a][j] != ultimo[j]) mismaRaiz = false;
                if (mismaRaiz) arcosVet.insert(std::make_pair(A[a][i], A[a][i + 1]));
            }
            std::vector<char> vetado(tam, 0);
            for (size_t j = 0; j < i; j++) vetado[ultimo[j]] = 1;
            std::vector<int> desvio;
            double cd;
            if (!dijkstraVetado(spur, v, ady, vetado, arcosVet, desvio, cd)) continue;
            std::vector<int> total(ultimo.begin(), ultimo.begin() + i);
            total.insert(total.end(), desvio.begin(), desvio.end());
            bool repetido = false;
            for (size_t a = 0; a < A.size() && !repetido; a++)
                if (A[a] == total) repetido = true;
            if (!repetido) B.insert(std::make_pair(prefijo[i] + cd, total));
        }
        if (B.empty()) break;
        A.push_back(B.begin()->second);
        costoA.push_back(B.begin()->first);
        B.erase(B.begin());
    }
    for (size_t a = 0; a < A.size(); a++) {
        res.push_back(std::list<Vertice>(A[a].begin(), A[a].end()));
        costos.push_back(costoA[a]);
    }
    return res;
}


// ---- Metricas --------------------------------------------------------------

double Grafo::excentricidad(Vertice v) const {
    if (!existeVertice(v)) return -1.0;
    std::vector<double> dist;
    std::vector<int> pred;
    if (!dijkstra(v, dist, pred)) return -1.0;    // costo negativo
    double ecc = 0.0;
    // Solo los vertices que existen: dist tambien tiene posiciones de
    // numeros de vertice que no estan en el grafo.
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        if (dist[p->getInfo()] > ecc) ecc = dist[p->getInfo()];
    return ecc;
}

double Grafo::radio() const {
    if (g == NULL) return -1.0;
    double r = DIST_INFINITA;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        double e = excentricidad(p->getInfo());
        if (e < 0.0) return -1.0;                 // costo negativo
        if (e < r) r = e;
    }
    return r;
}

double Grafo::diametro() const {
    if (g == NULL) return -1.0;
    double d = 0.0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        double e = excentricidad(p->getInfo());
        if (e < 0.0) return -1.0;
        if (e > d) d = e;
    }
    return d;
}

std::list<Vertice> Grafo::centro() const {
    std::list<Vertice> res;
    if (g == NULL) return res;
    std::vector<double> ecc;
    double r = DIST_INFINITA;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        double e = excentricidad(p->getInfo());
        if (e < 0.0) return res;                  // costo negativo
        ecc.push_back(e);
        if (e < r) r = e;
    }
    if (r == DIST_INFINITA) return res;
    size_t i = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx(), i++)
        if (ecc[i] == r) res.push_back(p->getInfo());
    return res;
}


// ---- Auxiliar privado ------------------------------------------------------

// BFS con punteros (unico lugar de caminoSimple que toca los nodos). Un camino
// de BFS nunca repite vertices: cada vertice se descubre una sola vez.
bool Grafo::buscarCaminoBFS(Vertice u, Vertice v, std::vector<int>& pred) const {
    int tam = maxVertice() + 1;
    std::vector<NodoVertice*> idx;
    construirIndice(g, tam, idx);
    pred.assign(tam, 0);

    std::vector<char> visto(tam, 0);
    std::queue<Vertice> C;
    visto[u] = 1;
    C.push(u);
    while (!C.empty()) {
        Vertice x = C.front();
        C.pop();
        if (x == v) return true;
        for (NodoAdy* a = idx[x]->getListaAdy(); a != NULL; a = a->getProx()) {
            Vertice y = a->getInfo();
            if (!visto[y]) {
                visto[y] = 1;
                pred[y] = x;
                C.push(y);
            }
        }
    }
    return false;
}


// ============================================================================
// 7. CICLOS
// ============================================================================


// ---- Deteccion -------------------------------------------------------------

bool Grafo::tieneCiclos() const {
    int tam = maxVertice() + 1;
    std::vector<NodoVertice*> idx;
    construirIndice(g, tam, idx);
    std::vector<int> color(tam, 0);

    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        if (color[p->getInfo()] == 0 &&
            cicloDirigidoVisitar(p->getInfo(), idx, color))
            return true;
    return false;
}

bool Grafo::tieneCiclosNoDirigido() const {
    int tam = maxVertice() + 1;
    std::vector<NodoVertice*> idx;
    construirIndice(g, tam, idx);
    std::vector<int> visitado(tam, 0);

    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        if (!visitado[p->getInfo()] &&
            cicloNoDirigidoVisitar(p->getInfo(), 0, idx, visitado))
            return true;
    return false;
}

int Grafo::cintura() const {
    int tam = maxVertice() + 1;
    std::vector<NodoVertice*> idx;
    construirIndice(g, tam, idx);
    int mejor = INFINITO;
    std::vector<int> dist(tam);
    std::vector<int> cola;
    for (NodoVertice* s = g; s != NULL; s = s->getProx()) {
        std::fill(dist.begin(), dist.end(), -1);
        cola.clear();
        int src = s->getInfo();
        dist[src] = 0;
        cola.push_back(src);
        for (size_t h = 0; h < cola.size(); h++) {
            int u = cola[h];
            if (dist[u] + 1 >= mejor) break;
            for (NodoAdy* a = idx[u]->getListaAdy(); a != NULL; a = a->getProx()) {
                int w = a->getInfo();
                if (w == src) {                       // cierra un ciclo por s
                    if (dist[u] + 1 < mejor) mejor = dist[u] + 1;
                } else if (dist[w] == -1) {
                    dist[w] = dist[u] + 1;
                    cola.push_back(w);
                }
            }
        }
    }
    return mejor;
}

int Grafo::cinturaNoDirigido() const {
    int tam = maxVertice() + 1;
    std::vector<NodoVertice*> idx;
    construirIndice(g, tam, idx);
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        if (existeArco(p->getInfo(), p->getInfo())) return 1;      // bucle
    int mejor = INFINITO;
    std::vector<int> dist(tam), padre(tam);
    std::vector<int> cola;
    for (NodoVertice* s = g; s != NULL; s = s->getProx()) {
        std::fill(dist.begin(), dist.end(), -1);
        cola.clear();
        int src = s->getInfo();
        dist[src] = 0;
        padre[src] = 0;
        cola.push_back(src);
        for (size_t h = 0; h < cola.size(); h++) {
            int u = cola[h];
            for (NodoAdy* a = idx[u]->getListaAdy(); a != NULL; a = a->getProx()) {
                int w = a->getInfo();
                if (dist[w] == -1) {
                    dist[w] = dist[u] + 1;
                    padre[w] = u;
                    cola.push_back(w);
                } else if (padre[u] != w) {
                    int c = dist[u] + dist[w] + 1;
                    if (c < mejor) mejor = c;
                }
            }
        }
    }
    return mejor;
}


// ---- Enumeracion y conteo --------------------------------------------------

std::list<std::list<Vertice> > Grafo::enumerarCiclos(int maxCiclos, bool& truncado) const {
    std::list<std::list<Vertice> > res;
    int cant;
    buscarCiclos(false, false, maxCiclos, truncado, &res, cant);
    return res;
}

int Grafo::contarCiclos(int maxCiclos, bool& truncado) const {
    int cant;
    buscarCiclos(false, true, maxCiclos, truncado, NULL, cant);
    return cant;
}

std::list<std::list<Vertice> > Grafo::enumerarCiclosNoDirigido(int maxCiclos, bool& truncado) const {
    std::list<std::list<Vertice> > res;
    int cant;
    buscarCiclos(true, false, maxCiclos, truncado, &res, cant);
    return res;
}

int Grafo::contarCiclosNoDirigido(int maxCiclos, bool& truncado) const {
    int cant;
    buscarCiclos(true, true, maxCiclos, truncado, NULL, cant);
    return cant;
}


// ---- Auxiliares privados ---------------------------------------------------

// Dirigido. color: 0 = blanco, 1 = gris (abierto), 2 = negro (terminado).
bool Grafo::cicloDirigidoVisitar(Vertice u, const std::vector<NodoVertice*>& idx,
                                 std::vector<int>& color) const {
    color[u] = 1;
    for (NodoAdy* a = idx[u]->getListaAdy(); a != NULL; a = a->getProx()) {
        Vertice v = a->getInfo();
        if (color[v] == 1) return true;   // arco hacia un ancestro abierto
        if (color[v] == 0 && cicloDirigidoVisitar(v, idx, color)) return true;
    }
    color[u] = 2;
    return false;
}

// No dirigido: el arco de vuelta al padre no cuenta como ciclo.
bool Grafo::cicloNoDirigidoVisitar(Vertice u, Vertice padre,
                                   const std::vector<NodoVertice*>& idx,
                                   std::vector<int>& visitado) const {
    visitado[u] = 1;
    for (NodoAdy* a = idx[u]->getListaAdy(); a != NULL; a = a->getProx()) {
        Vertice v = a->getInfo();
        if (!visitado[v]) {
            if (cicloNoDirigidoVisitar(v, u, idx, visitado)) return true;
        } else if (v != padre) {
            return true;                  // visitado y no es el padre (o bucle)
        }
    }
    return false;
}

// Auxiliar interno (no es metodo de Grafo). Usado por: buscarCiclos
// Algoritmo de Johnson (1975). Para cada vertice inicial s (en orden
// ascendente) se toma la componente fuertemente conexa del subgrafo inducido
// por los vertices >= s que contiene al menor vertice con ciclos, y se buscan
// los circuitos que empiezan en s con bloqueo de vertices.
namespace {

class Johnson {
public:
    int cantidad;
    bool truncado;

    Johnson(const std::vector<std::vector<int> >& adyacencia, bool noDir,
            bool soloCont, int maximo, std::list<std::list<Vertice> >* sal)
        : cantidad(0), truncado(false), adj(adyacencia),
          tam((int)adyacencia.size()), noDirigido(noDir), soloContar(soloCont),
          maxCiclos(maximo), salida(sal), detener(false), s(0), contador(0) {}

    void registrarBucle(int v) {
        if (detener) return;
        pila.push_back(v);
        registrar();
        pila.pop_back();
    }

    void ejecutar() {
        bloqueado.assign(tam, 0);
        B.assign(tam, std::list<int>());
        enComp.assign(tam, 0);
        s = 1;
        while (s < tam && !detener) {
            // 1. Componentes fuertemente conexas del subgrafo de vertices >= s
            indice.assign(tam, 0);
            bajo.assign(tam, 0);
            enTarjan.assign(tam, 0);
            comp.assign(tam, -1);
            tamComp.clear();
            tpila.clear();
            contador = 0;
            for (int v = s; v < tam; v++)
                if (indice[v] == 0) tarjan(v);

            // 2. Menor vertice que pertenece a una componente con >= 2 vertices
            int ini = -1;
            for (int v = s; v < tam && ini < 0; v++)
                if (tamComp[comp[v]] >= 2) ini = v;
            if (ini < 0) break;            // no quedan ciclos
            s = ini;

            for (int v = 0; v < tam; v++) {
                enComp[v] = (v >= s && comp[v] == comp[s]) ? 1 : 0;
                bloqueado[v] = 0;
                B[v].clear();
            }
            circuito(s);
            s++;
        }
    }

private:
    const std::vector<std::vector<int> >& adj;
    int tam;
    bool noDirigido;
    bool soloContar;
    int maxCiclos;
    std::list<std::list<Vertice> >* salida;
    bool detener;
    int s;
    int contador;
    std::vector<int> pila;
    std::vector<char> bloqueado;
    std::vector<std::list<int> > B;
    std::vector<char> enComp;
    std::vector<int> indice, bajo, comp, tamComp, tpila;
    std::vector<char> enTarjan;

    // Registra el ciclo que esta en la pila (si pasa el filtro y hay cupo).
    void registrar() {
        size_t k = pila.size();
        if (noDirigido && k >= 2) {
            if (k == 2) return;                    // ida y vuelta por una arista
            if (pila[1] > pila[k - 1]) return;     // mismo ciclo en sentido inverso
        }
        if (maxCiclos > 0 && cantidad >= maxCiclos) {
            truncado = true;                       // existe al menos uno mas
            detener = true;
            return;
        }
        cantidad++;
        if (!soloContar)
            salida->push_back(std::list<Vertice>(pila.begin(), pila.end()));
    }

    void desbloquear(int u) {
        bloqueado[u] = 0;
        while (!B[u].empty()) {
            int w = B[u].front();
            B[u].pop_front();
            if (bloqueado[w]) desbloquear(w);
        }
    }

    bool circuito(int v) {
        bool f = false;
        pila.push_back(v);
        bloqueado[v] = 1;
        for (size_t i = 0; i < adj[v].size() && !detener; i++) {
            int w = adj[v][i];
            if (!enComp[w]) continue;
            if (w == s) {
                registrar();
                f = true;
            } else if (!bloqueado[w]) {
                if (circuito(w)) f = true;
            }
        }
        if (!detener) {
            if (f) {
                desbloquear(v);
            } else {
                for (size_t i = 0; i < adj[v].size(); i++) {
                    int w = adj[v][i];
                    if (enComp[w] &&
                        std::find(B[w].begin(), B[w].end(), v) == B[w].end())
                        B[w].push_back(v);
                }
            }
        }
        pila.pop_back();
        return f;
    }

    // Tarjan sobre el subgrafo inducido por los vertices >= s
    void tarjan(int v) {
        indice[v] = bajo[v] = ++contador;
        tpila.push_back(v);
        enTarjan[v] = 1;
        for (size_t i = 0; i < adj[v].size(); i++) {
            int w = adj[v][i];
            if (w < s) continue;
            if (indice[w] == 0) {
                tarjan(w);
                if (bajo[w] < bajo[v]) bajo[v] = bajo[w];
            } else if (enTarjan[w] && indice[w] < bajo[v]) {
                bajo[v] = indice[w];
            }
        }
        if (bajo[v] == indice[v]) {
            int id = (int)tamComp.size();
            int cuenta = 0;
            int w;
            do {
                w = tpila.back();
                tpila.pop_back();
                enTarjan[w] = 0;
                comp[w] = id;
                cuenta++;
            } while (w != v);
            tamComp.push_back(cuenta);
        }
    }
};

} // namespace

void Grafo::buscarCiclos(bool noDirigido, bool soloContar, int maxCiclos,
                         bool& truncado, std::list<std::list<Vertice> >* salida,
                         int& cantidad) const {
    truncado = false;
    cantidad = 0;
    if (g == NULL) return;
    if (noDirigido && !tieneCiclosNoDirigido()) return;   // bosque: O(n + m)

    int tam = maxVertice() + 1;
    std::vector<std::vector<int> > adj(tam);
    std::vector<int> bucles;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        Vertice u = p->getInfo();
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            if (a->getInfo() == u) bucles.push_back(u);
            else adj[u].push_back(a->getInfo());
        }
    }

    Johnson J(adj, noDirigido, soloContar, maxCiclos, salida);
    for (size_t i = 0; i < bucles.size(); i++)
        J.registrarBucle(bucles[i]);
    J.ejecutar();
    cantidad = J.cantidad;
    truncado = J.truncado;
}


// ============================================================================
// 8. CAMINOS Y CICLOS EULERIANOS / HAMILTONIANOS
// ============================================================================


// ---- Eulerianos ------------------------------------------------------------

// Auxiliar interno (no es metodo de Grafo). Usado por: kruskalGeneral, componentesConexas, tieneCicloEuleriano, tieneCicloEulerianoNoDirigido
namespace {

class UnionFind {
public:
    explicit UnionFind(int n) : padre(n), rango(n, 0) {
        for (int i = 0; i < n; i++) padre[i] = i;
    }
    int buscar(int x) {                       // compresion de caminos
        while (padre[x] != x) {
            padre[x] = padre[padre[x]];
            x = padre[x];
        }
        return x;
    }
    bool unir(int a, int b) {                 // union por rango
        int ra = buscar(a), rb = buscar(b);
        if (ra == rb) return false;           // ya conectados: cerraria un ciclo
        if (rango[ra] < rango[rb]) std::swap(ra, rb);
        padre[rb] = ra;
        if (rango[ra] == rango[rb]) rango[ra]++;
        return true;
    }
private:
    std::vector<int> padre;
    std::vector<int> rango;
};

bool menorCosto(const Arista& a, const Arista& b) { return a.costo < b.costo; }
bool mayorCosto(const Arista& a, const Arista& b) { return a.costo > b.costo; }

} // namespace

bool Grafo::tieneCicloEuleriano() const {
    std::vector<int> gradoEnt;
    obtenerGradosDeEntrada(gradoEnt);
    UnionFind uf(maxVertice() + 1);
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        int salida = 0;
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            salida++;
            uf.unir(p->getInfo(), a->getInfo());
        }
        if (salida != gradoEnt[p->getInfo()]) return false;   // desbalanceado
    }
    // Con todos los grados balanceados, basta que los vertices con arcos
    // esten en una sola componente (entonces es fuertemente conexa).
    int raiz = -1;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        if (p->getListaAdy() == NULL) continue;               // aislado
        int r = uf.buscar(p->getInfo());
        if (raiz == -1) raiz = r;
        else if (r != raiz) return false;
    }
    return true;
}

bool Grafo::tieneCicloEulerianoNoDirigido() const {
    UnionFind uf(maxVertice() + 1);
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        int gr = 0;
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            gr++;
            if (a->getInfo() == p->getInfo()) gr++;   // el bucle se guarda una vez y suma 2
            uf.unir(p->getInfo(), a->getInfo());
        }
        if (gr % 2 != 0) return false;                // grado impar
    }
    int raiz = -1;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        if (p->getListaAdy() == NULL) continue;       // aislado
        int r = uf.buscar(p->getInfo());
        if (raiz == -1) raiz = r;
        else if (r != raiz) return false;
    }
    return true;
}

// Auxiliar interno (no es metodo de Grafo). Usado por: caminoEuleriano, tieneCaminoEuleriano, cicloEuleriano, caminoEulerianoNoDirigido, tieneCaminoEulerianoNoDirigido, ...
namespace {

// Hierholzer. noDirigido: grafo simetrico (cada arista una vez, bucle = 2 al
// grado). soloCiclo: exige recorrido cerrado. Devuelve false si no existe.
// Sin arcos devuelve true con res vacio.
bool recorridoEuleriano(NodoVertice* g, int tam, bool noDirigido, bool soloCiclo,
                        std::list<Vertice>& res) {
    res.clear();
    typedef std::pair<int, int> Vec;                 // (vecino, id de arista)
    std::vector<std::vector<Vec> > adj(tam);
    std::vector<int> entrada(tam, 0);
    int m = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            int u = p->getInfo(), w = a->getInfo();
            if (noDirigido) {
                if (u > w) continue;                 // cada arista una vez
                adj[u].push_back(Vec(w, m));
                adj[w].push_back(Vec(u, m));         // bucle: queda dos veces
            } else {
                adj[u].push_back(Vec(w, m));
                entrada[w]++;
            }
            m++;
        }
    if (m == 0) return true;

    int inicio = 0, inicios = 0, finales = 0, impares = 0, menorImpar = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        int v = p->getInfo();
        if (noDirigido) {
            if (adj[v].size() % 2 == 1) {
                impares++;
                if (menorImpar == 0) menorImpar = v;
            }
        } else {
            int d = (int)adj[v].size() - entrada[v];
            if (d == 1) { inicios++; inicio = v; }
            else if (d == -1) finales++;
            else if (d != 0) return false;
        }
    }
    if (noDirigido) {
        if (impares != 0 && (soloCiclo || impares != 2)) return false;
        if (impares == 2) inicio = menorImpar;
    } else {
        if (soloCiclo ? (inicios != 0 || finales != 0)
                      : !((inicios == 0 && finales == 0) || (inicios == 1 && finales == 1)))
            return false;
    }
    if (inicio == 0)                                 // circuito: menor vertice con arcos
        for (NodoVertice* p = g; p != NULL; p = p->getProx())
            if (!adj[p->getInfo()].empty()) { inicio = p->getInfo(); break; }

    std::vector<size_t> ptr(tam, 0);
    std::vector<char> usada(m, 0);
    std::vector<int> pila, circuito;
    pila.push_back(inicio);
    while (!pila.empty()) {
        int u = pila.back();
        if (noDirigido)
            while (ptr[u] < adj[u].size() && usada[adj[u][ptr[u]].second]) ptr[u]++;
        if (ptr[u] < adj[u].size()) {
            Vec e = adj[u][ptr[u]++];
            if (noDirigido) usada[e.second] = 1;
            pila.push_back(e.first);
        } else {
            circuito.push_back(u);
            pila.pop_back();
        }
    }
    if ((int)circuito.size() != m + 1) return false;  // arcos en otra componente
    for (size_t i = circuito.size(); i-- > 0; ) res.push_back(circuito[i]);
    return true;
}

} // namespace

bool Grafo::tieneCaminoEuleriano() const {
    std::list<Vertice> r;
    return recorridoEuleriano(g, maxVertice() + 1, false, false, r);
}

bool Grafo::tieneCaminoEulerianoNoDirigido() const {
    std::list<Vertice> r;
    return recorridoEuleriano(g, maxVertice() + 1, true, false, r);
}

std::list<Vertice> Grafo::caminoEuleriano() const {
    std::list<Vertice> r;
    if (!recorridoEuleriano(g, maxVertice() + 1, false, false, r)) r.clear();
    return r;
}

std::list<Vertice> Grafo::cicloEuleriano() const {
    std::list<Vertice> r;
    if (!recorridoEuleriano(g, maxVertice() + 1, false, true, r)) r.clear();
    return r;
}

std::list<Vertice> Grafo::caminoEulerianoNoDirigido() const {
    std::list<Vertice> r;
    if (!recorridoEuleriano(g, maxVertice() + 1, true, false, r)) r.clear();
    return r;
}

std::list<Vertice> Grafo::cicloEulerianoNoDirigido() const {
    std::list<Vertice> r;
    if (!recorridoEuleriano(g, maxVertice() + 1, true, true, r)) r.clear();
    return r;
}

std::list<Vertice> Grafo::caminoEulerianoDesde(Vertice s, bool costoEsCantidad) const {
    std::list<Vertice> res;
    if (!existeVertice(s)) return res;
    int tam = maxVertice() + 1;
    std::vector<std::vector<std::pair<int, int> > > ady;
    if (!cargarMultiadyacencia(costoEsCantidad, tam, ady)) return res;

    std::vector<long> salida(tam, 0), entrada(tam, 0);
    long m = 0;
    for (int u = 1; u < tam; u++)
        for (size_t i = 0; i < ady[u].size(); i++) {
            salida[u] += ady[u][i].second;
            entrada[ady[u][i].first] += ady[u][i].second;
            m += ady[u][i].second;
        }
    if (m == 0) return res;

    int inicios = 0, finales = 0, inicioObligado = 0;
    for (int v = 1; v < tam; v++) {
        long d = salida[v] - entrada[v];
        if (d == 1) { inicios++; inicioObligado = v; }
        else if (d == -1) finales++;
        else if (d != 0) return res;
    }
    if (inicios != finales || inicios > 1) return res;
    if (inicios == 1 && inicioObligado != s) return res;
    if (salida[s] == 0) return res;

    std::vector<size_t> ptr(tam, 0);
    std::vector<int> pila, circuito;
    pila.push_back(s);
    while (!pila.empty()) {
        int u = pila.back();
        while (ptr[u] < ady[u].size() && ady[u][ptr[u]].second == 0) ptr[u]++;
        if (ptr[u] < ady[u].size()) {
            ady[u][ptr[u]].second--;
            pila.push_back(ady[u][ptr[u]].first);
        } else {
            circuito.push_back(u);
            pila.pop_back();
        }
    }
    if ((long)circuito.size() != m + 1) return res;     // arcos en otra componente
    for (size_t i = circuito.size(); i-- > 0; ) res.push_back(circuito[i]);
    return res;
}

std::list<Vertice> Grafo::caminoEulerianoDesde(Vertice s) const {
    return caminoEulerianoDesde(s, false);
}


// ---- Hamiltonianos ---------------------------------------------------------

bool Grafo::tieneCicloHamiltoniano() const {
    return buscarCicloHamiltoniano(false, NULL);
}

bool Grafo::tieneCicloHamiltoniano(std::list<Vertice>& ciclo) const {
    return buscarCicloHamiltoniano(false, &ciclo);
}

bool Grafo::tieneCicloHamiltonianoNoDirigido() const {
    return buscarCicloHamiltoniano(true, NULL);
}

bool Grafo::tieneCicloHamiltonianoNoDirigido(std::list<Vertice>& ciclo) const {
    return buscarCicloHamiltoniano(true, &ciclo);
}


// ---- Auxiliares privados ---------------------------------------------------

// Unico lugar de caminoEulerianoDesde que toca los nodos.
bool Grafo::cargarMultiadyacencia(bool costoEsCantidad, int tam,
                                  std::vector<std::vector<std::pair<int, int> > >& ady) const {
    ady.assign(tam, std::vector<std::pair<int, int> >());
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            int cantidad = 1;
            if (costoEsCantidad) {
                double c = a->getCosto();
                if (c < 1.0 || c > 1000000.0 || c != std::floor(c)) return false;
                cantidad = (int)c;
            }
            ady[p->getInfo()].push_back(std::make_pair(a->getInfo(), cantidad));
        }
    return true;
}

// Auxiliar interno (no es metodo de Grafo). Usado por: buscarCicloHamiltoniano
namespace {

// Backtracking desde u. cont = vertices ya en el camino. sal[u] = sucesores
// de u (indices compactos, sin bucles). vuelve[u] = existe el arco u -> 0.
bool hamiltonVisitar(int u, int cont, int n,
                     const std::vector<std::vector<int> >& sal,
                     const std::vector<char>& vuelve,
                     std::vector<char>& vis, std::vector<int>& camino) {
    if (cont == n) return vuelve[u] != 0;         // cerrar el ciclo con 0
    for (size_t i = 0; i < sal[u].size(); i++) {
        int v = sal[u][i];
        if (vis[v]) continue;
        vis[v] = 1;
        camino.push_back(v);
        if (hamiltonVisitar(v, cont + 1, n, sal, vuelve, vis, camino))
            return true;
        camino.pop_back();
        vis[v] = 0;
    }
    return false;
}

} // namespace

// El ciclo se fija para empezar en el vertice menor (todo ciclo hamiltoniano
// pasa por el, asi que no se pierde ninguno).
bool Grafo::buscarCicloHamiltoniano(bool noDirigido, std::list<Vertice>* ciclo) const {
    if (ciclo != NULL) ciclo->clear();
    int n = cantVertices;
    if (n == 0) return false;
    if (noDirigido && n < 3) return false;

    if (n == 1) {                                 // solo vale con un bucle
        for (NodoAdy* a = g->getListaAdy(); a != NULL; a = a->getProx())
            if (a->getInfo() == g->getInfo()) {
                if (ciclo != NULL) ciclo->push_back(g->getInfo());
                return true;
            }
        return false;
    }

    // Indices compactos 0..n-1 (en orden ascendente de vertice)
    std::vector<int> ids;
    std::vector<int> pos(maxVertice() + 1, -1);
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        pos[p->getInfo()] = (int)ids.size();
        ids.push_back(p->getInfo());
    }
    std::vector<std::vector<int> > sal(n);
    std::vector<int> ent(n, 0);
    std::vector<char> vuelve(n, 0);
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        int i = pos[p->getInfo()];
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            int j = pos[a->getInfo()];
            if (j == i) continue;                 // los bucles no sirven
            sal[i].push_back(j);
            ent[j]++;
            if (j == 0) vuelve[i] = 1;
        }
    }
    // Poda: todo vertice necesita al menos un arco que entre y uno que salga
    for (int i = 0; i < n; i++)
        if (sal[i].empty() || ent[i] == 0) return false;

    std::vector<char> vis(n, 0);
    std::vector<int> camino;
    vis[0] = 1;
    camino.push_back(0);
    if (!hamiltonVisitar(0, 1, n, sal, vuelve, vis, camino)) return false;

    if (ciclo != NULL)
        for (size_t i = 0; i < camino.size(); i++)
            ciclo->push_back(ids[camino[i]]);
    return true;
}


// ============================================================================
// 9. CONECTIVIDAD Y PROPIEDADES ESTRUCTURALES
// ============================================================================


// ---- Conectividad ----------------------------------------------------------

int Grafo::componentesConexas() const {
    int comps = cantVertices;
    UnionFind uf(maxVertice() + 1);
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            if (uf.unir(p->getInfo(), a->getInfo()))
                comps--;               // dos componentes se fusionan en una
    return comps;
}

bool Grafo::esConexo() const { return componentesConexas() == 1; }

// Auxiliar interno (no es metodo de Grafo). Usado por: componentesFuertementeConexas, puentes
namespace {

// Tarjan para componentes fuertemente conexas
void fuerteVisitar(int u, const std::vector<NodoVertice*>& idx, std::vector<int>& disc,
                   std::vector<int>& low, std::vector<char>& enPila, std::vector<int>& pila,
                   int& tiempo, std::list<std::list<Vertice> >& comps) {
    disc[u] = low[u] = ++tiempo;
    pila.push_back(u);
    enPila[u] = 1;
    for (NodoAdy* a = idx[u]->getListaAdy(); a != NULL; a = a->getProx()) {
        int w = a->getInfo();
        if (disc[w] == 0) {
            fuerteVisitar(w, idx, disc, low, enPila, pila, tiempo, comps);
            if (low[w] < low[u]) low[u] = low[w];
        } else if (enPila[w] && disc[w] < low[u]) {
            low[u] = disc[w];
        }
    }
    if (low[u] == disc[u]) {
        std::list<Vertice> c;
        int w;
        do {
            w = pila.back();
            pila.pop_back();
            enPila[w] = 0;
            c.push_back(w);
        } while (w != u);
        c.sort();
        comps.push_back(c);
    }
}

bool menorPrimero(const std::list<Vertice>& a, const std::list<Vertice>& b) {
    return a.front() < b.front();
}

// DFS de puentes (Tarjan). Trabaja por posicion sobre el grafo simple.
void puenteVisitar(int u, int padre, const std::vector<std::vector<int> >& ady,
                   std::vector<int>& disc, std::vector<int>& low, int& tiempo,
                   std::vector<std::pair<int, int> >& res) {
    disc[u] = low[u] = ++tiempo;
    for (size_t i = 0; i < ady[u].size(); i++) {
        int v = ady[u][i];
        if (v == padre) continue;                 // grafo simple: una sola arista al padre
        if (disc[v] == 0) {
            puenteVisitar(v, u, ady, disc, low, tiempo, res);
            if (low[v] < low[u]) low[u] = low[v];
            if (low[v] > disc[u]) res.push_back(std::make_pair(u, v));
        } else if (disc[v] < low[u]) {
            low[u] = disc[v];
        }
    }
}

} // namespace

std::list<std::list<Vertice> > Grafo::componentesFuertementeConexas() const {
    std::list<std::list<Vertice> > comps;
    int tam = maxVertice() + 1;
    std::vector<NodoVertice*> idx;
    construirIndice(g, tam, idx);
    std::vector<int> disc(tam, 0), low(tam, 0), pila;
    std::vector<char> enPila(tam, 0);
    int tiempo = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        if (disc[p->getInfo()] == 0)
            fuerteVisitar(p->getInfo(), idx, disc, low, enPila, pila, tiempo, comps);
    comps.sort(menorPrimero);
    return comps;
}

bool Grafo::esFuertementeConexo() const {
    return cantVertices >= 1 && componentesFuertementeConexas().size() == 1;
}

bool Grafo::esBiconexo() const {
    return cantVertices >= 2 && componentesConexas() == 1 && puntosArticulacion().empty();
}


// ---- Cortes ----------------------------------------------------------------

std::list<Arista> Grafo::puentes() const {
    std::list<Arista> res;
    int tam = maxVertice() + 1;
    std::vector<int> ids;
    std::vector<std::vector<int> > ady;
    grafoSubyacente(g, tam, ids, ady);
    int n = (int)ids.size();
    std::vector<int> disc(n, 0), low(n, 0);
    std::vector<std::pair<int, int> > pares;
    int tiempo = 0;
    for (int s = 0; s < n; s++)
        if (disc[s] == 0) puenteVisitar(s, -1, ady, disc, low, tiempo, pares);
    std::vector<std::pair<int, int> > vs;
    for (size_t i = 0; i < pares.size(); i++) {
        int a = ids[pares[i].first], b = ids[pares[i].second];
        vs.push_back(std::make_pair(std::min(a, b), std::max(a, b)));
    }
    std::sort(vs.begin(), vs.end());
    std::vector<NodoVertice*> idx;
    construirIndice(g, tam, idx);
    for (size_t i = 0; i < vs.size(); i++) {
        double c = 0.0;
        if (!buscarArcoIdx(idx, vs[i].first, vs[i].second, c))
            buscarArcoIdx(idx, vs[i].second, vs[i].first, c);
        res.push_back(Arista(vs[i].first, vs[i].second, c));
    }
    return res;
}

// Auxiliar interno (no es metodo de Grafo). Usado por: puntosArticulacion
namespace {

// DFS de Tarjan. disc = orden de descubrimiento, low = menor disc alcanzable
// desde el subarbol de u con a lo sumo una arista de retroceso.
void articulacionVisitar(int u, int padre, const std::vector<std::vector<int> >& ady,
                         std::vector<int>& disc, std::vector<int>& low, int& tiempo,
                         std::vector<char>& esArt) {
    disc[u] = low[u] = ++tiempo;
    int hijos = 0;
    for (size_t i = 0; i < ady[u].size(); i++) {
        int v = ady[u][i];
        if (v == padre) continue;                 // grafo simple: unica arista al padre
        if (disc[v] == 0) {
            hijos++;
            articulacionVisitar(v, u, ady, disc, low, tiempo, esArt);
            if (low[v] < low[u]) low[u] = low[v];
            if (padre != -1 && low[v] >= disc[u]) esArt[u] = 1;
        } else if (disc[v] < low[u]) {
            low[u] = disc[v];                     // arista de retroceso
        }
    }
    if (padre == -1 && hijos >= 2) esArt[u] = 1;  // raiz con dos o mas hijos
}

} // namespace

std::list<Vertice> Grafo::puntosArticulacion() const {
    std::list<Vertice> res;
    int n = cantVertices;
    if (n < 3) return res;

    std::vector<int> pos(maxVertice() + 1, -1);
    std::vector<int> ids;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        pos[p->getInfo()] = (int)ids.size();
        ids.push_back(p->getInfo());
    }

    // Grafo simple subyacente (sin bucles ni arcos repetidos)
    std::set<std::pair<int, int> > aristas;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            int u = pos[p->getInfo()], v = pos[a->getInfo()];
            if (u != v) aristas.insert(std::make_pair(std::min(u, v), std::max(u, v)));
        }
    std::vector<std::vector<int> > ady(n);
    for (std::set<std::pair<int, int> >::const_iterator it = aristas.begin();
         it != aristas.end(); ++it) {
        ady[it->first].push_back(it->second);
        ady[it->second].push_back(it->first);
    }

    std::vector<int> disc(n, 0), low(n, 0);
    std::vector<char> esArt(n, 0);
    int tiempo = 0;
    for (int s = 0; s < n; s++)
        if (disc[s] == 0)
            articulacionVisitar(s, -1, ady, disc, low, tiempo, esArt);

    for (int i = 0; i < n; i++)                   // ids ya esta en orden ascendente
        if (esArt[i]) res.push_back(ids[i]);
    return res;
}


// ---- Tipo de grafo ---------------------------------------------------------

bool Grafo::esArbol() const {
    return cantVertices >= 1 && componentesConexas() == 1 && esBosque();
}

// Arborescencia (arbol dirigido): un unico vertice sin arcos entrantes (la raiz)
// y todos los demas con exactamente uno, es decir, los arcos van del padre al
// hijo. Se busca la raiz por grados de entrada y se delega en la sobrecarga.
// O(n + m).
bool Grafo::esArbolDirigido() const {
    if (cantVertices == 0) return false;
    std::vector<int> entrada;
    obtenerGradosDeEntrada(entrada);
    Vertice raiz = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        if (entrada[p->getInfo()] == 0) {
            if (raiz != 0) return false;          // dos raices: es un bosque
            raiz = p->getInfo();
        }
    }
    if (raiz == 0) return false;                  // todos tienen padre: hay ciclo
    return esArbolDirigido(raiz);
}

// Con raiz fija: la raiz no recibe arcos, los demas reciben exactamente uno y
// todos son alcanzables desde la raiz (esto descarta los ciclos aislados que
// cumplen la regla de grados). O(n + m).
bool Grafo::esArbolDirigido(Vertice raiz) const {
    if (!existeVertice(raiz)) return false;
    std::vector<int> entrada;
    obtenerGradosDeEntrada(entrada);
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        int esperado = (p->getInfo() == raiz) ? 0 : 1;
        if (entrada[p->getInfo()] != esperado) return false;
    }
    return (int)alcanzables(raiz).size() == cantVertices;
}

bool Grafo::esBosque() const {
    int pares, bucles;
    aristasSubyacentes(g, pares, bucles);
    if (bucles > 0) return false;
    return pares == cantVertices - componentesConexas();
}

bool Grafo::esDAG() const { return !tieneCiclos(); }

bool Grafo::esBipartito() const {
    std::list<Vertice> A, B;
    return esBipartito(A, B);
}

// Se trabaja sobre el grafo subyacente: cada arco (u, v) se guarda en ambas
// listas de vecinos. Colorear solo siguiendo los sucesores daria falsos
// negativos en grafos dirigidos (un arco que "entra" a una zona ya coloreada).
bool Grafo::esBipartito(std::list<Vertice>& A, std::list<Vertice>& B) const {
    A.clear();
    B.clear();
    int tam = maxVertice() + 1;
    std::vector<std::vector<int> > vec(tam);
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            Vertice u = p->getInfo(), v = a->getInfo();
            if (u == v) return false;             // un bucle es un ciclo impar
            vec[u].push_back(v);
            vec[v].push_back(u);
        }

    std::vector<int> color(tam, -1);              // -1 sin color, 0 o 1
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        Vertice s = p->getInfo();
        if (color[s] != -1) continue;
        color[s] = 0;
        std::queue<int> Q;
        Q.push(s);
        while (!Q.empty()) {
            int u = Q.front();
            Q.pop();
            for (size_t i = 0; i < vec[u].size(); i++) {
                int v = vec[u][i];
                if (color[v] == -1) {
                    color[v] = 1 - color[u];
                    Q.push(v);
                } else if (color[v] == color[u]) {
                    return false;                 // arista dentro de un conjunto
                }
            }
        }
    }
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        if (color[p->getInfo()] == 0) A.push_back(p->getInfo());
        else B.push_back(p->getInfo());
    }
    return true;
}

bool Grafo::esCompleto() const {
    int n = cantVertices;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        int k = 0;
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            if (a->getInfo() != p->getInfo()) k++;
        if (k != n - 1) return false;
    }
    return true;
}

bool Grafo::esRegular() const {
    int k;
    return esRegular(k);
}

bool Grafo::esRegular(int& k) const {
    k = -1;
    if (g == NULL) return true;
    std::vector<int> entrada;
    obtenerGradosDeEntrada(entrada);
    int ext = -1, ent = -1;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        int e = 0;
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) e++;
        if (ext == -1) { ext = e; ent = entrada[p->getInfo()]; }
        else if (e != ext || entrada[p->getInfo()] != ent) return false;
    }
    k = ext;
    return true;
}

// Auxiliar interno (no es metodo de Grafo). Usado por: esPlanar
namespace {

typedef std::pair<int, int> ParInt;

// Bloques (componentes biconexas) por Tarjan con pila de aristas. El grafo
// es simple, asi que "v == padre" identifica la arista del arbol.
void bloquesVisitar(int u, int padre, const std::vector<std::vector<int> >& ady,
                    std::vector<int>& disc, std::vector<int>& low, int& tiempo,
                    std::vector<ParInt>& pila,
                    std::vector<std::vector<ParInt> >& bloques) {
    disc[u] = low[u] = ++tiempo;
    for (size_t i = 0; i < ady[u].size(); i++) {
        int v = ady[u][i];
        if (v == padre) continue;
        if (disc[v] == 0) {
            pila.push_back(ParInt(u, v));
            bloquesVisitar(v, u, ady, disc, low, tiempo, pila, bloques);
            if (low[v] < low[u]) low[u] = low[v];
            if (low[v] >= disc[u]) {              // u separa el bloque de v
                std::vector<ParInt> bloque;
                ParInt tope;
                do {
                    tope = pila.back();
                    pila.pop_back();
                    bloque.push_back(tope);
                } while (tope != ParInt(u, v));
                bloques.push_back(bloque);
            }
        } else if (disc[v] < disc[u]) {           // arista de retroceso
            pila.push_back(ParInt(u, v));
            if (disc[v] < low[u]) low[u] = disc[v];
        }
    }
}

// Busca un ciclo con DFS: al llegar a un vertice gris que no es el padre.
bool hallarCiclo(int u, int padre, const std::vector<std::vector<int> >& ady,
                 std::vector<int>& color, std::vector<int>& par,
                 std::vector<int>& ciclo) {
    color[u] = 1;
    par[u] = padre;
    for (size_t i = 0; i < ady[u].size(); i++) {
        int v = ady[u][i];
        if (v == padre) continue;
        if (color[v] == 1) {                      // v es ancestro de u
            for (int x = u; x != v; x = par[x]) ciclo.push_back(x);
            ciclo.push_back(v);
            return true;
        }
        if (color[v] == 0 && hallarCiclo(v, u, ady, color, par, ciclo))
            return true;
    }
    color[u] = 2;
    return false;
}

struct Fragmento {
    std::vector<int> verts;    // vertices fuera de H (vacio si es una arista)
    std::vector<int> att;      // vertices de H donde se une
};

// DMP sobre un bloque (grafo simple y biconexo). Vertices con cualquier
// numero; se renumeran a 0..k-1.
bool bloquePlanar(const std::vector<ParInt>& aristas) {
    std::map<int, int> num;
    for (size_t i = 0; i < aristas.size(); i++) {
        num.insert(ParInt(aristas[i].first, 0));
        num.insert(ParInt(aristas[i].second, 0));
    }
    int k = 0;
    for (std::map<int, int>::iterator it = num.begin(); it != num.end(); ++it)
        it->second = k++;
    int m = (int)aristas.size();
    if (k < 3) return true;
    if (m > 3 * k - 6) return false;              // cota de Euler

    std::vector<std::vector<int> > ady(k);
    for (int i = 0; i < m; i++) {
        int a = num[aristas[i].first], b = num[aristas[i].second];
        ady[a].push_back(b);
        ady[b].push_back(a);
    }

    // H inicial: un ciclo, con sus dos caras (interior y exterior)
    std::vector<char> enH(k, 0);
    std::set<ParInt> aristasH;
    int cuentaH = 0;
    std::vector<std::vector<int> > caras;
    {
        std::vector<int> color(k, 0), par(k, -1), ciclo;
        hallarCiclo(0, -1, ady, color, par, ciclo);   // un bloque con k >= 3 tiene ciclo
        int L = (int)ciclo.size();
        for (int i = 0; i < L; i++) {
            int a = ciclo[i], b = ciclo[(i + 1) % L];
            enH[a] = 1;
            aristasH.insert(ParInt(std::min(a, b), std::max(a, b)));
            cuentaH++;
        }
        caras.push_back(ciclo);
        caras.push_back(ciclo);
    }

    while (cuentaH < m) {
        // ---- fragmentos de G respecto a H
        std::vector<Fragmento> frag;
        for (int u = 0; u < k; u++) {
            if (!enH[u]) continue;
            for (size_t i = 0; i < ady[u].size(); i++) {
                int v = ady[u][i];
                if (u < v && enH[v] && aristasH.count(ParInt(u, v)) == 0) {
                    Fragmento f;
                    f.att.push_back(u);
                    f.att.push_back(v);
                    frag.push_back(f);
                }
            }
        }
        std::vector<int> compVisto(k, 0);
        std::vector<int> marca(k, 0);
        for (int s = 0; s < k; s++) {
            if (enH[s] || compVisto[s]) continue;
            Fragmento f;
            std::vector<int> cola;
            cola.push_back(s);
            compVisto[s] = 1;
            for (size_t q = 0; q < cola.size(); q++) {
                int y = cola[q];
                f.verts.push_back(y);
                for (size_t i = 0; i < ady[y].size(); i++) {
                    int w = ady[y][i];
                    if (enH[w]) {
                        if (!marca[w]) { marca[w] = 1; f.att.push_back(w); }
                    } else if (!compVisto[w]) {
                        compVisto[w] = 1;
                        cola.push_back(w);
                    }
                }
            }
            for (size_t i = 0; i < f.att.size(); i++) marca[f.att[i]] = 0;
            frag.push_back(f);
        }

        // ---- caras admisibles de cada fragmento
        std::vector<std::vector<char> > enCara(caras.size(), std::vector<char>(k, 0));
        for (size_t c2 = 0; c2 < caras.size(); c2++)
            for (size_t i = 0; i < caras[c2].size(); i++)
                enCara[c2][caras[c2][i]] = 1;

        int mejor = -1, mejorCara = -1, mejorCant = 0;
        for (size_t fi = 0; fi < frag.size(); fi++) {
            int cant = 0, primera = -1;
            for (size_t c2 = 0; c2 < caras.size(); c2++) {
                bool ok = true;
                for (size_t i = 0; i < frag[fi].att.size() && ok; i++)
                    if (!enCara[c2][frag[fi].att[i]]) ok = false;
                if (ok) {
                    cant++;
                    if (primera == -1) primera = (int)c2;
                }
            }
            if (cant == 0) return false;          // no cabe en ninguna cara
            if (mejor == -1 || cant < mejorCant) {
                mejor = (int)fi;
                mejorCara = primera;
                mejorCant = cant;
            }
        }

        // ---- camino del fragmento elegido entre dos de sus attachments
        const Fragmento& F = frag[mejor];
        std::vector<int> camino;
        if (F.verts.empty()) {
            camino.push_back(F.att[0]);
            camino.push_back(F.att[1]);
        } else {
            int u = F.att[0];
            std::vector<char> dentro(k, 0);
            for (size_t i = 0; i < F.verts.size(); i++) dentro[F.verts[i]] = 1;
            std::vector<int> par(k, -1), cola;
            for (size_t i = 0; i < ady[u].size(); i++) {
                int x = ady[u][i];
                if (dentro[x]) { par[x] = u; cola.push_back(x); }
            }
            int fin = -1, destino = -1;
            for (size_t q = 0; q < cola.size() && fin == -1; q++) {
                int y = cola[q];
                for (size_t i = 0; i < ady[y].size(); i++) {
                    int w = ady[y][i];
                    if (enH[w] && w != u) { fin = y; destino = w; break; }
                    if (dentro[w] && par[w] == -1) { par[w] = y; cola.push_back(w); }
                }
            }
            camino.push_back(destino);
            for (int x = fin; x != u; x = par[x]) camino.push_back(x);
            camino.push_back(u);
            std::reverse(camino.begin(), camino.end());      // u ... destino
        }

        // ---- incrustar el camino y partir la cara
        int u = camino.front(), v = camino.back();
        for (size_t i = 0; i + 1 < camino.size(); i++) {
            int a = camino[i], b = camino[i + 1];
            aristasH.insert(ParInt(std::min(a, b), std::max(a, b)));
            cuentaH++;
        }
        for (size_t i = 0; i < camino.size(); i++) enH[camino[i]] = 1;

        const std::vector<int> cara = caras[mejorCara];
        int L = (int)cara.size();
        int pu = 0, pv = 0;
        for (int i = 0; i < L; i++) {
            if (cara[i] == u) pu = i;
            if (cara[i] == v) pv = i;
        }
        std::vector<int> cara1, cara2;
        for (int i = pu; ; i = (i + 1) % L) {                 // u ... v
            cara1.push_back(cara[i]);
            if (i == pv) break;
        }
        for (size_t i = camino.size() - 1; i-- > 1; )         // p_k ... p_1
            cara1.push_back(camino[i]);
        for (int i = pv; ; i = (i + 1) % L) {                 // v ... u
            cara2.push_back(cara[i]);
            if (i == pu) break;
        }
        for (size_t i = 1; i + 1 < camino.size(); i++)        // p_1 ... p_k
            cara2.push_back(camino[i]);
        caras[mejorCara] = cara1;
        caras.push_back(cara2);
    }
    return true;
}

} // namespace

bool Grafo::esPlanar() const {
    int n = cantVertices;
    if (n < 5) return true;                       // todo grafo con menos de 5 vertices es planar
    std::vector<int> pos(maxVertice() + 1, -1);
    int k = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        pos[p->getInfo()] = k++;

    // Grafo simple subyacente: sin bucles ni arcos repetidos
    std::set<ParInt> aristas;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            int u = pos[p->getInfo()], v = pos[a->getInfo()];
            if (u != v) aristas.insert(ParInt(std::min(u, v), std::max(u, v)));
        }
    if (n >= 3 && (int)aristas.size() > 3 * n - 6) return false;

    std::vector<std::vector<int> > ady(n);
    for (std::set<ParInt>::const_iterator it = aristas.begin(); it != aristas.end(); ++it) {
        ady[it->first].push_back(it->second);
        ady[it->second].push_back(it->first);
    }

    std::vector<int> disc(n, 0), low(n, 0);
    int tiempo = 0;
    std::vector<ParInt> pila;
    std::vector<std::vector<ParInt> > bloques;
    for (int s = 0; s < n; s++)
        if (disc[s] == 0)
            bloquesVisitar(s, -1, ady, disc, low, tiempo, pila, bloques);

    for (size_t i = 0; i < bloques.size(); i++)
        if (!bloquePlanar(bloques[i])) return false;
    return true;
}


// ---- Simetria --------------------------------------------------------------

bool Grafo::esSimetrico() const { return esSimetrico(false); }

bool Grafo::esSimetrico(bool compararCostos) const {
    std::vector<NodoVertice*> idx;
    construirIndice(g, maxVertice() + 1, idx);
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            int u = p->getInfo(), w = a->getInfo();
            if (u == w) continue;
            double c;
            if (!buscarArcoIdx(idx, w, u, c)) return false;
            if (compararCostos && c != a->getCosto()) return false;
        }
    return true;
}

bool Grafo::esDirigido() const { return !esSimetrico(false); }


// ---- Isomorfismo -----------------------------------------------------------

bool Grafo::esIsomorfo(const Grafo& otro) const {
    return esIsomorfo(otro, false);
}

// Auxiliar interno (no es metodo de Grafo). Usado por: esIsomorfo
namespace {

typedef std::vector<std::vector<char> > MatAdy;
typedef std::vector<std::vector<double> > MatCosto;

// Firma del vertice i: su color, y los colores (y costos) de sus vecinos de
// salida y de entrada, ordenados. Dos vertices que se correspondan en un
// isomorfismo tienen la misma firma.
void firmaVertice(const MatAdy& M, const MatCosto& C, const std::vector<int>& col,
                  int i, bool conCostos, std::vector<double>& sig) {
    int n = (int)M.size();
    std::vector<std::pair<int, double> > sal, ent;
    for (int j = 0; j < n; j++) {
        if (M[i][j]) sal.push_back(std::make_pair(col[j], conCostos ? C[i][j] : 0.0));
        if (M[j][i]) ent.push_back(std::make_pair(col[j], conCostos ? C[j][i] : 0.0));
    }
    std::sort(sal.begin(), sal.end());
    std::sort(ent.begin(), ent.end());

    sig.clear();
    sig.push_back((double)col[i]);
    sig.push_back((double)sal.size());
    for (size_t k = 0; k < sal.size(); k++) {
        sig.push_back((double)sal[k].first);
        sig.push_back(sal[k].second);
    }
    sig.push_back((double)ent.size());
    for (size_t k = 0; k < ent.size(); k++) {
        sig.push_back((double)ent[k].first);
        sig.push_back(ent[k].second);
    }
}

// Refinamiento de colores sobre los dos grafos a la vez (los colores usan
// un mismo mapa de firmas, asi que son comparables entre ambos). Se repite
// hasta que la particion deja de dividirse. Es una condicion NECESARIA del
// isomorfismo: sirve para descartar y para podar el backtracking.
void refinarColores(const MatAdy& a, const MatCosto& ca,
                    const MatAdy& b, const MatCosto& cb, bool conCostos,
                    std::vector<int>& colA, std::vector<int>& colB) {
    int n = (int)a.size();
    colA.assign(n, 0);
    colB.assign(n, 0);
    for (int i = 0; i < n; i++) {                 // semilla: tiene bucle o no
        colA[i] = a[i][i] ? 1 : 0;
        colB[i] = b[i][i] ? 1 : 0;
    }
    std::set<int> iniciales(colA.begin(), colA.end());
    iniciales.insert(colB.begin(), colB.end());
    size_t clases = iniciales.size();

    for (int ronda = 0; ronda <= n + 1; ronda++) {
        std::map<std::vector<double>, int> ids;
        std::vector<int> nA(n), nB(n);
        std::vector<double> sig;
        for (int i = 0; i < n; i++) {
            firmaVertice(a, ca, colA, i, conCostos, sig);
            nA[i] = ids.insert(std::make_pair(sig, (int)ids.size())).first->second;
        }
        for (int i = 0; i < n; i++) {
            firmaVertice(b, cb, colB, i, conCostos, sig);
            nB[i] = ids.insert(std::make_pair(sig, (int)ids.size())).first->second;
        }
        colA = nA;
        colB = nB;
        if (ids.size() == clases) break;          // particion estable
        clases = ids.size();
    }
}

// Orden en que se asignan los vertices de a: primero los de clase mas rara y
// luego los mas conectados con los ya colocados, para detectar conflictos
// cuanto antes.
void ordenarParaIsomorfismo(const MatAdy& a, const std::vector<int>& colA,
                            std::vector<int>& orden) {
    int n = (int)a.size();
    std::vector<int> tamClase(2 * n + 2, 0);
    for (int i = 0; i < n; i++) tamClase[colA[i]]++;
    std::vector<int> conex(n, 0);
    std::vector<char> puesto(n, 0);
    orden.clear();
    for (int k = 0; k < n; k++) {
        int mejor = -1;
        for (int i = 0; i < n; i++) {
            if (puesto[i]) continue;
            if (mejor == -1 || conex[i] > conex[mejor] ||
                (conex[i] == conex[mejor] && tamClase[colA[i]] < tamClase[colA[mejor]]))
                mejor = i;
        }
        puesto[mejor] = 1;
        orden.push_back(mejor);
        for (int j = 0; j < n; j++)
            if (!puesto[j] && (a[mejor][j] || a[j][mejor])) conex[j]++;
    }
}

struct EstadoIso {
    const MatAdy& a;
    const MatCosto& ca;
    const MatAdy& b;
    const MatCosto& cb;
    bool conCostos;
    const std::vector<int>& colA;
    const std::vector<int>& colB;
    std::vector<int> orden;     // orden de asignacion de los vertices de a
    std::vector<int> f;         // f[u] = imagen de u en b (-1 si sin asignar)
    std::vector<char> usado;    // vertices de b ya usados

    EstadoIso(const MatAdy& A, const MatCosto& CA, const MatAdy& B,
              const MatCosto& CB, bool cc, const std::vector<int>& cA,
              const std::vector<int>& cB)
        : a(A), ca(CA), b(B), cb(CB), conCostos(cc), colA(cA), colB(cB),
          f(A.size(), -1), usado(A.size(), 0) {}

private:
    EstadoIso& operator=(const EstadoIso&);
};

// Asignar u -> x respeta los arcos con el bucle propio y con los k vertices
// ya asignados.
bool compatibleIso(const EstadoIso& e, int u, int x, int k) {
    if (e.a[u][u] != e.b[x][x]) return false;
    if (e.conCostos && e.a[u][u] && e.ca[u][u] != e.cb[x][x]) return false;
    for (int t = 0; t < k; t++) {
        int w = e.orden[t], y = e.f[w];
        if (e.a[u][w] != e.b[x][y] || e.a[w][u] != e.b[y][x]) return false;
        if (e.conCostos) {
            if (e.a[u][w] && e.ca[u][w] != e.cb[x][y]) return false;
            if (e.a[w][u] && e.ca[w][u] != e.cb[y][x]) return false;
        }
    }
    return true;
}

bool buscarIsomorfismo(EstadoIso& e, int k) {
    int n = (int)e.orden.size();
    if (k == n) return true;
    int u = e.orden[k];
    for (int x = 0; x < n; x++) {
        if (e.usado[x] || e.colB[x] != e.colA[u]) continue;
        if (!compatibleIso(e, u, x, k)) continue;
        e.f[u] = x;
        e.usado[x] = 1;
        if (buscarIsomorfismo(e, k + 1)) return true;
        e.f[u] = -1;
        e.usado[x] = 0;
    }
    return false;
}

bool matricesIsomorfas(const MatAdy& a, const MatCosto& ca,
                       const MatAdy& b, const MatCosto& cb, bool conCostos) {
    int n = (int)a.size();
    if (n != (int)b.size()) return false;
    if (n == 0) return true;

    int arcosA = 0, arcosB = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            arcosA += a[i][j];
            arcosB += b[i][j];
        }
    if (arcosA != arcosB) return false;

    std::vector<int> colA, colB;
    refinarColores(a, ca, b, cb, conCostos, colA, colB);
    std::vector<int> cuentaA(2 * n + 2, 0), cuentaB(2 * n + 2, 0);
    for (int i = 0; i < n; i++) {
        cuentaA[colA[i]]++;
        cuentaB[colB[i]]++;
    }
    if (cuentaA != cuentaB) return false;         // distinto reparto de clases

    EstadoIso e(a, ca, b, cb, conCostos, colA, colB);
    ordenarParaIsomorfismo(a, colA, e.orden);
    return buscarIsomorfismo(e, 0);
}

} // namespace

bool Grafo::esIsomorfo(const Grafo& otro, bool compararCostos) const {
    if (cantVertices != otro.cantVertices) return false;
    MatAdy a, b;
    MatCosto ca, cb;
    matrizAdyacencia(a, ca);
    otro.matrizAdyacencia(b, cb);
    return matricesIsomorfas(a, ca, b, cb, compararCostos);
}


// ---- Auxiliar privado ------------------------------------------------------

// Unico metodo de esIsomorfo que toca los nodos: copia el grafo a una matriz
// para que el resto del algoritmo trabaje solo con vectores.
void Grafo::matrizAdyacencia(std::vector<std::vector<char> >& ady,
                             std::vector<std::vector<double> >& costo) const {
    int n = cantVertices;
    std::vector<int> pos(maxVertice() + 1, -1);
    int k = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        pos[p->getInfo()] = k++;

    ady.assign(n, std::vector<char>(n, 0));
    costo.assign(n, std::vector<double>(n, 0.0));
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            int i = pos[p->getInfo()], j = pos[a->getInfo()];
            ady[i][j] = 1;
            costo[i][j] = a->getCosto();
        }
}


// ============================================================================
// 10. ARBOLES DE EXPANSION
// ============================================================================

std::list<Arista> Grafo::kruskal(double& pesoTotal, bool& conexo) const {
    return kruskalGeneral(false, pesoTotal, conexo);
}

std::list<Arista> Grafo::kruskalMaximo(double& pesoTotal, bool& conexo) const {
    return kruskalGeneral(true, pesoTotal, conexo);
}

// Auxiliar interno (no es metodo de Grafo). Usado por: prim, emparejamientoMaximo
namespace {

struct CandidatoPrim {
    double costo;
    int u;
    int v;
    CandidatoPrim(double c, int a, int b) : costo(c), u(a), v(b) {}
    bool operator>(const CandidatoPrim& o) const {
        if (costo != o.costo) return costo > o.costo;
        if (u != o.u) return u > o.u;
        return v > o.v;
    }
};

bool menorAristaExt(const Arista& a, const Arista& b) {
    if (a.origen != b.origen) return a.origen < b.origen;
    return a.destino < b.destino;
}

} // namespace

std::list<Arista> Grafo::prim(Vertice s, double& pesoTotal, bool& conexo) const {
    std::list<Arista> arbol;
    pesoTotal = 0.0;
    conexo = false;
    if (!existeVertice(s)) return arbol;
    int tam = maxVertice() + 1;
    std::vector<NodoVertice*> idx;
    construirIndice(g, tam, idx);
    std::vector<char> visto(tam, 0);
    std::priority_queue<CandidatoPrim, std::vector<CandidatoPrim>,
                        std::greater<CandidatoPrim> > cola;
    int alcanzados = 1;
    visto[s] = 1;
    for (NodoAdy* a = idx[s]->getListaAdy(); a != NULL; a = a->getProx())
        cola.push(CandidatoPrim(a->getCosto(), s, a->getInfo()));
    while (!cola.empty()) {
        CandidatoPrim c = cola.top();
        cola.pop();
        if (visto[c.v]) continue;                    // incluye los bucles
        visto[c.v] = 1;
        alcanzados++;
        arbol.push_back(Arista(std::min(c.u, c.v), std::max(c.u, c.v), c.costo));
        pesoTotal += c.costo;
        for (NodoAdy* a = idx[c.v]->getListaAdy(); a != NULL; a = a->getProx())
            if (!visto[a->getInfo()])
                cola.push(CandidatoPrim(a->getCosto(), c.v, a->getInfo()));
    }
    conexo = (alcanzados == cantVertices);
    return arbol;
}

std::list<Arista> Grafo::kruskalGeneral(bool maximo, double& pesoTotal, bool& conexo) const {
    std::list<Arista> mst;
    pesoTotal = 0.0;

    // Cada arista no dirigida se toma una sola vez (origen < destino).
    std::vector<Arista> aristas;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        Vertice u = p->getInfo();
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            if (u < a->getInfo())
                aristas.push_back(Arista(u, a->getInfo(), a->getCosto()));
    }
    // stable_sort: ante costos iguales se conserva el orden (origen, destino)
    std::stable_sort(aristas.begin(), aristas.end(), maximo ? mayorCosto : menorCosto);

    UnionFind uf(maxVertice() + 1);
    int n = orden();
    int aceptadas = 0;
    for (size_t i = 0; i < aristas.size() && aceptadas < n - 1; i++) {
        if (uf.unir(aristas[i].origen, aristas[i].destino)) {
            mst.push_back(aristas[i]);
            pesoTotal += aristas[i].costo;
            aceptadas++;
        }
    }
    conexo = (n <= 1 || aceptadas == n - 1);
    return mst;
}


// ============================================================================
// 11. FLUJO, EMPAREJAMIENTO Y PROBLEMAS NP
// ============================================================================

double Grafo::flujoMaximo(Vertice s, Vertice t) const {
    std::list<Arista> ignorado;
    return flujoMaximo(s, t, ignorado);
}

double Grafo::flujoMaximo(Vertice s, Vertice t, std::list<Arista>& flujo) const {
    flujo.clear();
    if (s == t || !existeVertice(s) || !existeVertice(t)) return -1.0;
    int n = cantVertices;
    std::vector<std::vector<char> > ady;
    std::vector<std::vector<double> > cap;
    matrizAdyacencia(ady, cap);                       // por posicion 0..n-1
    std::vector<int> ids;
    int si = -1, ti = -1;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        if (p->getInfo() == s) si = (int)ids.size();
        if (p->getInfo() == t) ti = (int)ids.size();
        ids.push_back(p->getInfo());
    }
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            if (!ady[i][j] || i == j) { cap[i][j] = 0.0; continue; }
            if (cap[i][j] < 0.0) return -1.0;
        }
    const double EPS = 1e-12;
    std::vector<std::vector<double> > res = cap;
    double total = 0.0;
    for (;;) {
        std::vector<int> pred(n, -1);
        std::vector<int> cola;
        cola.push_back(si);
        pred[si] = si;
        for (size_t h = 0; h < cola.size() && pred[ti] == -1; h++) {
            int u = cola[h];
            for (int w = 0; w < n; w++)
                if (pred[w] == -1 && res[u][w] > EPS) {
                    pred[w] = u;
                    cola.push_back(w);
                }
        }
        if (pred[ti] == -1) break;
        double cuello = DIST_INFINITA;
        for (int x = ti; x != si; x = pred[x])
            if (res[pred[x]][x] < cuello) cuello = res[pred[x]][x];
        if (cuello == DIST_INFINITA) return DIST_INFINITA;   // capacidad infinita
        for (int x = ti; x != si; x = pred[x]) {
            res[pred[x]][x] -= cuello;
            res[x][pred[x]] += cuello;
        }
        total += cuello;
    }
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            if (!ady[i][j] || i == j) continue;
            double neto = cap[i][j] - res[i][j];
            if (neto > EPS) flujo.push_back(Arista(ids[i], ids[j], neto));
        }
    return total;
}

// Auxiliar interno (no es metodo de Grafo). Usado por: emparejamientoMaximo
namespace {

bool aumentarKuhn(int u, const std::vector<std::vector<int> >& ady, std::vector<char>& visto,
                  std::vector<int>& parejaDe) {
    for (size_t i = 0; i < ady[u].size(); i++) {
        int w = ady[u][i];
        if (visto[w]) continue;
        visto[w] = 1;
        if (parejaDe[w] == 0 || aumentarKuhn(parejaDe[w], ady, visto, parejaDe)) {
            parejaDe[w] = u;
            return true;
        }
    }
    return false;
}

} // namespace

int Grafo::emparejamientoMaximo(std::list<Arista>& pares) const {
    pares.clear();
    std::list<Vertice> A, B;
    if (!esBipartito(A, B)) return -1;
    int tam = maxVertice() + 1;
    std::vector<char> enA(tam, 0);
    for (std::list<Vertice>::const_iterator it = A.begin(); it != A.end(); ++it) enA[*it] = 1;
    std::vector<std::vector<int> > ady(tam);
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            int u = p->getInfo(), w = a->getInfo();
            if (enA[u]) ady[u].push_back(w);
            else ady[w].push_back(u);
        }
    for (int u = 0; u < tam; u++) {
        std::sort(ady[u].begin(), ady[u].end());
        ady[u].erase(std::unique(ady[u].begin(), ady[u].end()), ady[u].end());
    }
    std::vector<int> parejaDe(tam, 0);                // parejaDe[w en B] = u en A
    int tamano = 0;
    for (std::list<Vertice>::const_iterator it = A.begin(); it != A.end(); ++it) {
        std::vector<char> visto(tam, 0);
        if (aumentarKuhn(*it, ady, visto, parejaDe)) tamano++;
    }
    std::vector<Arista> v;
    for (std::list<Vertice>::const_iterator it = B.begin(); it != B.end(); ++it) {
        int w = *it, u = parejaDe[w];
        if (u == 0) continue;
        double c = existeArco(u, w) ? costoArco(u, w) : costoArco(w, u);
        v.push_back(Arista(std::min(u, w), std::max(u, w), c));
    }
    std::sort(v.begin(), v.end(), menorAristaExt);
    for (size_t i = 0; i < v.size(); i++) pares.push_back(v[i]);
    return tamano;
}

// Auxiliar interno (no es metodo de Grafo). Usado por: coloreadoVoraz, conjuntoDominanteMinimo, cliqueMaxima, conjuntoIndependienteMaximo
namespace {

// Matriz de adyacencia del grafo subyacente (por posicion), sin bucles.
void matrizSubyacente(const std::vector<std::vector<int> >& ady,
                      std::vector<std::vector<char> >& M) {
    int n = (int)ady.size();
    M.assign(n, std::vector<char>(n, 0));
    for (int i = 0; i < n; i++)
        for (size_t j = 0; j < ady[i].size(); j++) M[i][ady[i][j]] = 1;
}

// Bron-Kerbosch con pivote; guarda en mejor la clique mas grande.
void bronKerbosch(const std::vector<std::vector<char> >& M, std::vector<int>& R,
                  const std::vector<int>& P, const std::vector<int>& X,
                  std::vector<int>& mejor) {
    if (P.empty() && X.empty()) {
        if (R.size() > mejor.size()) mejor = R;
        return;
    }
    if (R.size() + P.size() <= mejor.size()) return;     // no puede mejorar
    int piv = -1, cuenta = -1;
    for (int pass = 0; pass < 2; pass++) {
        const std::vector<int>& S = (pass == 0) ? P : X;
        for (size_t i = 0; i < S.size(); i++) {
            int c = 0;
            for (size_t j = 0; j < P.size(); j++) if (M[S[i]][P[j]]) c++;
            if (c > cuenta) { cuenta = c; piv = S[i]; }
        }
    }
    std::vector<int> candidatos;
    for (size_t i = 0; i < P.size(); i++) if (!M[piv][P[i]]) candidatos.push_back(P[i]);
    std::vector<int> Pact = P, Xact = X;
    for (size_t i = 0; i < candidatos.size(); i++) {
        int v = candidatos[i];
        std::vector<int> nP, nX;
        for (size_t j = 0; j < Pact.size(); j++) if (M[v][Pact[j]]) nP.push_back(Pact[j]);
        for (size_t j = 0; j < Xact.size(); j++) if (M[v][Xact[j]]) nX.push_back(Xact[j]);
        R.push_back(v);
        bronKerbosch(M, R, nP, nX, mejor);
        R.pop_back();
        Pact.erase(std::find(Pact.begin(), Pact.end(), v));
        Xact.push_back(v);
    }
}

std::vector<int> cliqueMayor(const std::vector<std::vector<char> >& M) {
    std::vector<int> R, P, X, mejor;
    for (size_t i = 0; i < M.size(); i++) P.push_back((int)i);
    bronKerbosch(M, R, P, X, mejor);
    std::sort(mejor.begin(), mejor.end());
    return mejor;
}

// Busqueda de un conjunto dominante de tamano <= resto (profundizacion iterativa)
struct Dominante {
    std::vector<std::vector<int> > cubre;     // cubre[c] = {c} + sucesores de c
    std::vector<std::vector<int> > cands;     // cands[w] = {w} + predecesores de w
    std::vector<int> cubierto;
    std::vector<int> sol;
    size_t maxCubre;

    bool buscar(int resto) {
        int n = (int)cubierto.size();
        int mejorV = -1, sinCubrir = 0;
        for (int w = 0; w < n; w++)
            if (cubierto[w] == 0) {
                sinCubrir++;
                if (mejorV == -1 || cands[w].size() < cands[mejorV].size()) mejorV = w;
            }
        if (sinCubrir == 0) return true;
        if (resto == 0) return false;
        if ((size_t)sinCubrir > (size_t)resto * maxCubre) return false;
        for (size_t i = 0; i < cands[mejorV].size(); i++) {
            int c = cands[mejorV][i];
            sol.push_back(c);
            for (size_t j = 0; j < cubre[c].size(); j++) cubierto[cubre[c][j]]++;
            if (buscar(resto - 1)) return true;
            for (size_t j = 0; j < cubre[c].size(); j++) cubierto[cubre[c][j]]--;
            sol.pop_back();
        }
        return false;
    }
};

bool mayorGrado(const std::pair<int, int>& a, const std::pair<int, int>& b) {
    if (a.first != b.first) return a.first > b.first;
    return a.second < b.second;
}

} // namespace

int Grafo::coloreadoVoraz(std::vector<int>& color) const {
    int tam = maxVertice() + 1;
    color.assign(tam, 0);
    std::vector<int> ids;
    std::vector<std::vector<int> > ady;
    grafoSubyacente(g, tam, ids, ady);
    int n = (int)ids.size();
    std::vector<std::pair<int, int> > orden;          // (grado, posicion)
    for (int i = 0; i < n; i++) orden.push_back(std::make_pair((int)ady[i].size(), i));
    std::sort(orden.begin(), orden.end(), mayorGrado);
    int usados = 0;
    for (int k = 0; k < n; k++) {
        int i = orden[k].second;
        std::vector<char> ocupado(n + 2, 0);
        for (size_t j = 0; j < ady[i].size(); j++) {
            int c = color[ids[ady[i][j]]];
            if (c > 0) ocupado[c] = 1;
        }
        int c = 1;
        while (ocupado[c]) c++;
        color[ids[i]] = c;
        if (c > usados) usados = c;
    }
    return usados;
}

std::list<Vertice> Grafo::conjuntoDominanteMinimo() const {
    std::list<Vertice> res;
    int n = cantVertices;
    if (n == 0) return res;
    int tam = maxVertice() + 1;
    std::vector<int> pos(tam, -1), ids;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        pos[p->getInfo()] = (int)ids.size();
        ids.push_back(p->getInfo());
    }
    Dominante D;
    D.cubre.assign(n, std::vector<int>());
    D.cands.assign(n, std::vector<int>());
    for (int i = 0; i < n; i++) { D.cubre[i].push_back(i); D.cands[i].push_back(i); }
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            int u = pos[p->getInfo()], w = pos[a->getInfo()];
            if (u == w) continue;
            D.cubre[u].push_back(w);                   // u domina a w
            D.cands[w].push_back(u);
        }
    D.maxCubre = 0;
    for (int i = 0; i < n; i++) D.maxCubre = std::max(D.maxCubre, D.cubre[i].size());
    D.cubierto.assign(n, 0);
    for (int k = 1; k <= n; k++) {
        D.sol.clear();
        std::fill(D.cubierto.begin(), D.cubierto.end(), 0);
        if (D.buscar(k)) break;
    }
    std::vector<int> r;
    for (size_t i = 0; i < D.sol.size(); i++) r.push_back(ids[D.sol[i]]);
    std::sort(r.begin(), r.end());
    res.assign(r.begin(), r.end());
    return res;
}

std::list<Vertice> Grafo::cliqueMaxima() const {
    std::vector<int> ids;
    std::vector<std::vector<int> > ady;
    grafoSubyacente(g, maxVertice() + 1, ids, ady);
    std::vector<std::vector<char> > M;
    matrizSubyacente(ady, M);
    std::vector<int> c = cliqueMayor(M);
    std::list<Vertice> res;
    for (size_t i = 0; i < c.size(); i++) res.push_back(ids[c[i]]);
    return res;
}

std::list<Vertice> Grafo::conjuntoIndependienteMaximo() const {
    std::vector<int> ids;
    std::vector<std::vector<int> > ady;
    grafoSubyacente(g, maxVertice() + 1, ids, ady);
    std::vector<std::vector<char> > M;
    matrizSubyacente(ady, M);
    int n = (int)M.size();
    for (int i = 0; i < n; i++)                         // complemento
        for (int j = 0; j < n; j++) M[i][j] = (i != j && !M[i][j]) ? 1 : 0;
    std::vector<int> c = cliqueMayor(M);
    std::list<Vertice> res;
    for (size_t i = 0; i < c.size(); i++) res.push_back(ids[c[i]]);
    return res;
}


// ============================================================================
// 12. OPERACIONES ENTRE GRAFOS
// ============================================================================

void Grafo::unir(const Grafo& otro) {
    if (&otro == this) return;
    Armador A;
    NodoVertice* p = g;
    NodoVertice* q = otro.g;
    while (p != NULL || q != NULL) {
        if (q == NULL || (p != NULL && p->getInfo() < q->getInfo())) {
            copiarVertice(A, p);
            p = p->getProx();
        } else if (p == NULL || q->getInfo() < p->getInfo()) {
            copiarVertice(A, q);
            q = q->getProx();
        } else {                                    // mismo vertice: mezclar arcos
            A.vertice(p->getInfo());
            NodoAdy* a = p->getListaAdy();
            NodoAdy* b = q->getListaAdy();
            while (a != NULL || b != NULL) {
                if (b == NULL || (a != NULL && a->getInfo() < b->getInfo())) {
                    A.arco(a->getInfo(), a->getCosto());
                    a = a->getProx();
                } else if (a == NULL || b->getInfo() < a->getInfo()) {
                    A.arco(b->getInfo(), b->getCosto());
                    b = b->getProx();
                } else {                            // en ambos: costo de este grafo
                    A.arco(a->getInfo(), a->getCosto());
                    a = a->getProx();
                    b = b->getProx();
                }
            }
            p = p->getProx();
            q = q->getProx();
        }
    }
    int c = A.cantidad();
    tomar(A.soltar(), c);
}

void Grafo::interseccion(const Grafo& otro) {
    if (&otro == this) return;
    Armador A;
    NodoVertice* p = g;
    NodoVertice* q = otro.g;
    while (p != NULL && q != NULL) {
        if (p->getInfo() < q->getInfo()) p = p->getProx();
        else if (q->getInfo() < p->getInfo()) q = q->getProx();
        else {
            A.vertice(p->getInfo());
            NodoAdy* a = p->getListaAdy();
            NodoAdy* b = q->getListaAdy();
            while (a != NULL && b != NULL) {
                if (a->getInfo() < b->getInfo()) a = a->getProx();
                else if (b->getInfo() < a->getInfo()) b = b->getProx();
                else {
                    A.arco(a->getInfo(), a->getCosto());
                    a = a->getProx();
                    b = b->getProx();
                }
            }
            p = p->getProx();
            q = q->getProx();
        }
    }
    int c = A.cantidad();
    tomar(A.soltar(), c);
}

void Grafo::complemento(Grafo& resultado) const {
    Armador A;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        int u = p->getInfo();
        A.vertice(u);
        NodoAdy* a = p->getListaAdy();
        for (NodoVertice* q = g; q != NULL; q = q->getProx()) {
            int w = q->getInfo();
            while (a != NULL && a->getInfo() < w) a = a->getProx();
            if (w == u) continue;
            if (a != NULL && a->getInfo() == w) continue;
            A.arco(w, 1.0);
        }
    }
    int c = A.cantidad();
    resultado.tomar(A.soltar(), c);
}

bool Grafo::subgrafo(const std::list<Vertice>& vs, Grafo& resultado) const {
    int tam = maxVertice() + 1;
    std::vector<NodoVertice*> idx;
    construirIndice(g, tam, idx);
    std::vector<char> sel(tam, 0);
    for (std::list<Vertice>::const_iterator it = vs.begin(); it != vs.end(); ++it) {
        if (*it < 1 || *it >= tam || idx[*it] == NULL) return false;
        sel[*it] = 1;
    }
    Armador A;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        if (!sel[p->getInfo()]) continue;
        A.vertice(p->getInfo());
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            if (sel[a->getInfo()]) A.arco(a->getInfo(), a->getCosto());
    }
    int c = A.cantidad();
    resultado.tomar(A.soltar(), c);
    return true;
}


// ============================================================================
// 13. ENTRADA, SALIDA Y CONVERSION
// ============================================================================


// ---- Impresion -------------------------------------------------------------

void Grafo::imprimir(std::ostream& os) const {
    if (g == NULL) {
        os << "(grafo vacio)\n";
        return;
    }
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        os << p->getInfo() << " ->";
        if (p->getListaAdy() == NULL) os << " (sin arcos)";
        bool primero = true;
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            os << (primero ? " " : ", ") << a->getInfo() << "(" << a->getCosto() << ")";
            primero = false;
        }
        os << "\n";
    }
}

std::ostream& operator<<(std::ostream& os, const Grafo& G) {
    G.imprimir(os);
    return os;
}


// ---- Archivos --------------------------------------------------------------

bool Grafo::guardarEnArchivo(const std::string& archivo, bool noDirigido) const {
    std::ofstream f(archivo.c_str());
    if (!f) return false;
    f.imbue(std::locale::classic());
    f << std::setprecision(15);
    f << "# Grafo " << (noDirigido ? "no dirigido" : "dirigido") << ": "
      << cantVertices << " vertices\n"
      << "# formato: 'origen destino costo' por arco; 'v id' = vertice aislado\n";
    std::vector<int> entrada;
    obtenerGradosDeEntrada(entrada);
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        if (p->getListaAdy() == NULL && entrada[p->getInfo()] == 0)
            f << "v " << p->getInfo() << "\n";
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            if (!noDirigido || p->getInfo() <= a->getInfo())
                f << p->getInfo() << " " << a->getInfo() << " " << a->getCosto() << "\n";
    f.flush();
    return f.good();
}

// Auxiliar interno (no es metodo de Grafo). Usado por: leerDesdeArchivo
namespace {

// Convierte un token completo en numero; false si sobra algo o no es numero.
bool leerEntero(const std::string& t, long& r) {
    std::istringstream s(t);
    s.imbue(std::locale::classic());
    char resto;
    return (s >> r) && !(s >> resto);
}

bool leerReal(const std::string& t, double& r) {
    std::istringstream s(t);
    s.imbue(std::locale::classic());
    char resto;
    return (s >> r) && !(s >> resto);
}

} // namespace

bool Grafo::leerDesdeArchivo(const std::string& archivo, bool noDirigido) {
    std::ifstream f(archivo.c_str());
    if (!f) return false;
    f.imbue(std::locale::classic());
    Grafo tmp;
    std::string linea;
    while (std::getline(f, linea)) {
        std::string::size_type h = linea.find('#');
        if (h != std::string::npos) linea.erase(h);
        std::istringstream ss(linea);
        std::string t1, t2, t3, t4;
        if (!(ss >> t1)) continue;                         // linea vacia
        long u, w;
        double c = 1.0;
        if (t1 == "v" || t1 == "V") {
            if (!(ss >> t2) || (ss >> t3)) return false;
            if (!leerEntero(t2, u) || u < 1 || u > (long)INT_MAX) return false;
            tmp.agregarVertice((Vertice)u);
            continue;
        }
        if (!(ss >> t2)) return false;
        if (ss >> t3) {
            if (ss >> t4) return false;
            if (!leerReal(t3, c)) return false;
        }
        if (!leerEntero(t1, u) || !leerEntero(t2, w)) return false;
        if (u < 1 || w < 1 || u > (long)INT_MAX || w > (long)INT_MAX) return false;
        tmp.agregarVertice((Vertice)u);
        tmp.agregarVertice((Vertice)w);
        if (noDirigido) tmp.agregarArcoNoDirigido((Vertice)u, (Vertice)w, c);
        else tmp.agregarArco((Vertice)u, (Vertice)w, c);
    }
    intercambiar(tmp);                                     // tmp libera el contenido viejo
    return true;
}


// ---- Exportacion -----------------------------------------------------------

bool Grafo::exportarDOT(const std::string& archivo, bool noDirigido) const {
    return exportarDOT(archivo, noDirigido, std::vector<std::string>());
}

// Auxiliar interno (no es metodo de Grafo). Usado por: exportarDOT
namespace {

std::string escaparDOT(const std::string& s) {
    std::string r;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '"' || s[i] == '\\') r += '\\';
        r += s[i];
    }
    return r;
}

} // namespace

bool Grafo::exportarDOT(const std::string& archivo, bool noDirigido,
                        const std::vector<std::string>& etiquetas) const {
    std::ofstream f(archivo.c_str());
    if (!f) return false;
    f.imbue(std::locale::classic());
    f << std::setprecision(15);
    f << (noDirigido ? "graph" : "digraph") << " G {\n";
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        int v = p->getInfo();
        f << "  " << v << " [label=\"";
        if (v < (int)etiquetas.size() && !etiquetas[v].empty()) f << escaparDOT(etiquetas[v]);
        else f << v;
        f << "\"];\n";
    }
    const char* op = noDirigido ? " -- " : " -> ";
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            if (!noDirigido || p->getInfo() <= a->getInfo())
                f << "  " << p->getInfo() << op << a->getInfo()
                  << " [label=\"" << a->getCosto() << "\"];\n";
    f << "}\n";
    f.flush();
    return f.good();
}

bool Grafo::exportarSVG(const std::string& archivo, bool noDirigido) const {
    return exportarSVG(archivo, noDirigido, std::vector<std::string>());
}

// Auxiliar interno (no es metodo de Grafo). Usado por: exportarSVG
namespace {

// Numeros siempre con punto decimal, sin depender de la configuracion
// regional del sistema (Linux / Windows).
std::string numSVG(double v) {
    std::ostringstream s;
    s.imbue(std::locale::classic());
    s << std::fixed << std::setprecision(2) << v;
    return s.str();
}

std::string pesoSVG(double v) {
    std::ostringstream s;
    s.imbue(std::locale::classic());
    s << std::setprecision(6) << v;
    return s.str();
}

std::string enteroSVG(int v) {
    std::ostringstream s;
    s.imbue(std::locale::classic());
    s << v;
    return s.str();
}

std::string escaparXML(const std::string& t) {
    std::string r;
    for (size_t i = 0; i < t.size(); i++) {
        switch (t[i]) {
            case '&': r += "&amp;"; break;
            case '<': r += "&lt;"; break;
            case '>': r += "&gt;"; break;
            case '"': r += "&quot;"; break;
            case '\'': r += "&apos;"; break;
            default: r += t[i];
        }
    }
    return r;
}

// Costo centrado en (x, y) sobre un rectangulo blanco que tapa la linea que
// pasa por detras (un rectangulo funciona igual en todos los visores).
void escribirPeso(std::ostream& out, double x, double y, const std::string& peso) {
    double w = 6.6 * peso.size() + 6.0, h = 16.0;
    out << "<rect x=\"" << numSVG(x - w / 2) << "\" y=\"" << numSVG(y - h / 2)
        << "\" width=\"" << numSVG(w) << "\" height=\"" << numSVG(h)
        << "\" rx=\"3\" fill=\"white\"/>\n";
    out << "<text x=\"" << numSVG(x) << "\" y=\"" << numSVG(y + 4.0)
        << "\" font-size=\"12\" text-anchor=\"middle\" fill=\"#1a5fb4\">"
        << escaparXML(peso) << "</text>\n";
}

// Dibuja un arco entre dos centros de vertices (radio r). Recto o curvo
// (cuando existe tambien el arco inverso). La curva se desvia hacia la
// izquierda del sentido de recorrido, asi las dos quedan a lados opuestos.
void dibujarArco(std::ostream& out, std::ostream& etq, double x1, double y1,
                 double x2, double y2, double r, bool flecha, bool curva,
                 const std::string& peso) {
    double dx = x2 - x1, dy = y2 - y1;
    double len = std::sqrt(dx * dx + dy * dy);
    if (len < 1e-9) return;
    double ux = dx / len, uy = dy / len;
    double nx = -uy, ny = ux;
    std::string marca = flecha ? " marker-end=\"url(#flecha)\"" : "";

    if (!curva) {
        double sx = x1 + ux * r, sy = y1 + uy * r;
        double ex = x2 - ux * r, ey = y2 - uy * r;
        out << "<line x1=\"" << numSVG(sx) << "\" y1=\"" << numSVG(sy)
            << "\" x2=\"" << numSVG(ex) << "\" y2=\"" << numSVG(ey)
            << "\" stroke=\"#555\" stroke-width=\"1.5\"" << marca << "/>\n";
        // al 40% del recorrido (no en el centro): evita que se encimen los
        // costos de arcos que pasan por el centro del circulo
        escribirPeso(etq, sx + 0.4 * (ex - sx), sy + 0.4 * (ey - sy), peso);
    } else {
        double d = len * 0.18;
        if (d < 14) d = 14;
        if (d > 38) d = 38;
        double qx = (x1 + x2) / 2 + nx * d, qy = (y1 + y2) / 2 + ny * d;
        double ax = qx - x1, ay = qy - y1, al = std::sqrt(ax * ax + ay * ay);
        double bx = qx - x2, by = qy - y2, bl = std::sqrt(bx * bx + by * by);
        double sx = x1 + ax / al * r, sy = y1 + ay / al * r;
        double ex = x2 + bx / bl * r, ey = y2 + by / bl * r;
        out << "<path d=\"M " << numSVG(sx) << " " << numSVG(sy) << " Q "
            << numSVG(qx) << " " << numSVG(qy) << " " << numSVG(ex) << " " << numSVG(ey)
            << "\" fill=\"none\" stroke=\"#555\" stroke-width=\"1.5\"" << marca << "/>\n";
        // curva de Bezier en t = 0.4
        escribirPeso(etq, 0.36 * sx + 0.48 * qx + 0.16 * ex,
                          0.36 * sy + 0.48 * qy + 0.16 * ey, peso);
    }
}

} // namespace

bool Grafo::exportarSVG(const std::string& archivo, bool noDirigido,
                        const std::vector<std::string>& etiquetas) const {
    std::ofstream out(archivo.c_str());
    if (!out) return false;
    out.imbue(std::locale::classic());

    const double PI = 3.14159265358979323846;
    int n = cantVertices;
    int tam = maxVertice() + 1;

    out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    if (n == 0) {
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"240\" height=\"100\">\n"
            << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n"
            << "<text x=\"120\" y=\"55\" font-size=\"14\" text-anchor=\"middle\""
            << " font-family=\"sans-serif\">Grafo vacio</text>\n</svg>\n";
        out.close();
        return !out.fail();
    }

    // Texto de cada vertice (etiqueta o numero)
    std::vector<std::string> texto(tam);
    size_t maxLen = 1;
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        Vertice v = p->getInfo();
        if ((size_t)v < etiquetas.size() && !etiquetas[v].empty()) texto[v] = etiquetas[v];
        else texto[v] = enteroSVG(v);
        if (texto[v].size() > maxLen) maxLen = texto[v].size();
    }

    // Tamano de los vertices segun el texto mas largo
    double fs = 14.0;
    double r = 0.6 * fs * maxLen / 2 + 6;
    if (r > 40) {
        fs = 68.0 / (0.6 * maxLen);
        if (fs < 7) fs = 7;
        r = 0.6 * fs * maxLen / 2 + 6;
        if (r > 40) r = 40;
    }
    if (r < 18) r = 18;

    // Disposicion circular: entre vertices consecutivos quedan >= 48 px libres
    double R = 0;
    if (n >= 2) {
        R = (2 * r + 48) / (2 * std::sin(PI / n));
        if (R < 90) R = 90;
    }
    double lado = 2 * (R + r + 50);        // margen para bucles y costos
    double cx = lado / 2, cy = lado / 2;

    std::vector<double> px(tam, 0.0), py(tam, 0.0);
    int i = 0;
    for (NodoVertice* p = g; p != NULL; p = p->getProx(), i++) {
        double ang = -PI / 2 + 2 * PI * i / n;
        px[p->getInfo()] = cx + R * std::cos(ang);
        py[p->getInfo()] = cy + R * std::sin(ang);
    }

    // Arcos
    std::vector<Arista> arcos;
    std::set<std::pair<int, int> > hay;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx()) {
            arcos.push_back(Arista(p->getInfo(), a->getInfo(), a->getCosto()));
            hay.insert(std::make_pair(p->getInfo(), a->getInfo()));
        }

    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"" << numSVG(lado)
        << "\" height=\"" << numSVG(lado) << "\" viewBox=\"0 0 " << numSVG(lado)
        << " " << numSVG(lado) << "\" font-family=\"sans-serif\">\n";
    out << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";
    if (!noDirigido)
        out << "<defs><marker id=\"flecha\" viewBox=\"0 0 10 10\" refX=\"10\" refY=\"5\""
            << " markerWidth=\"8\" markerHeight=\"8\" orient=\"auto\">"
            << "<path d=\"M 0 0 L 10 5 L 0 10 z\" fill=\"#555\"/></marker></defs>\n";

    // Primero todas las lineas y despues todos los costos, para que ninguna
    // linea tape el texto de un costo.
    std::ostringstream etq;
    etq.imbue(std::locale::classic());
    for (size_t k = 0; k < arcos.size(); k++) {
        Vertice v = arcos[k].origen, w = arcos[k].destino;
        std::string peso = pesoSVG(arcos[k].costo);
        if (v == w) {                                   // bucle
            double ux = px[v] - cx, uy = py[v] - cy;
            double l = std::sqrt(ux * ux + uy * uy);
            if (l < 1e-9) { ux = 0; uy = -1; } else { ux /= l; uy /= l; }
            double lr = 13;
            out << "<circle cx=\"" << numSVG(px[v] + ux * (r + lr - 2))
                << "\" cy=\"" << numSVG(py[v] + uy * (r + lr - 2)) << "\" r=\"" << numSVG(lr)
                << "\" fill=\"none\" stroke=\"#555\" stroke-width=\"1.5\"/>\n";
            escribirPeso(etq, px[v] + ux * (r + 2 * lr + 8), py[v] + uy * (r + 2 * lr + 8), peso);
        } else if (noDirigido) {
            if (v > w && hay.count(std::make_pair(w, v))) continue;   // ya dibujado
            dibujarArco(out, etq, px[v], py[v], px[w], py[w], r, false, false, peso);
        } else {
            bool inverso = hay.count(std::make_pair(w, v)) > 0;
            dibujarArco(out, etq, px[v], py[v], px[w], py[w], r, true, inverso, peso);
        }
    }

    out << etq.str();

    // Vertices (despues de los arcos, para tapar sus extremos)
    for (NodoVertice* p = g; p != NULL; p = p->getProx()) {
        Vertice v = p->getInfo();
        out << "<circle cx=\"" << numSVG(px[v]) << "\" cy=\"" << numSVG(py[v])
            << "\" r=\"" << numSVG(r) << "\" fill=\"white\" stroke=\"#222\" stroke-width=\"1.5\"/>\n";
        out << "<text x=\"" << numSVG(px[v]) << "\" y=\"" << numSVG(py[v] + fs * 0.35)
            << "\" font-size=\"" << numSVG(fs) << "\" text-anchor=\"middle\" fill=\"#111\">"
            << escaparXML(texto[v]) << "</text>\n";
    }
    out << "</svg>\n";
    out.close();
    return !out.fail();
}


// ---- Estructuras equivalentes ----------------------------------------------

std::list<Arista> Grafo::listaDeAristas(bool noDirigido) const {
    std::list<Arista> res;
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            if (!noDirigido || p->getInfo() <= a->getInfo())
                res.push_back(Arista(p->getInfo(), a->getInfo(), a->getCosto()));
    return res;
}

void Grafo::obtenerMatrizAdyacencia(std::vector<std::vector<char> >& ady) const {
    int tam = maxVertice() + 1;
    ady.assign(tam, std::vector<char>(tam, 0));
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            ady[p->getInfo()][a->getInfo()] = 1;
}

void Grafo::obtenerMatrizCostos(std::vector<std::vector<double> >& costo) const {
    int tam = maxVertice() + 1;
    costo.assign(tam, std::vector<double>(tam, DIST_INFINITA));
    for (NodoVertice* p = g; p != NULL; p = p->getProx())
        for (NodoAdy* a = p->getListaAdy(); a != NULL; a = a->getProx())
            costo[p->getInfo()][a->getInfo()] = a->getCosto();
}
