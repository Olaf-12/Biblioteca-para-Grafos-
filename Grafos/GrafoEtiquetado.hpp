// GrafoEtiquetado.hpp - Capa de traduccion: grafo cuyos vertices son de
// cualquier tipo T (std::string, char, estructuras propias, ...).
//
// Por dentro mantiene un Grafo de vertices enteros y traduce en la entrada
// (etiqueta -> entero) y en la salida (entero -> etiqueta). Los algoritmos
// son exactamente los de Grafo, con los mismos costos; solo se paga un
// O(log n) por cada etiqueta que entra (std::map).
//
// Requisitos de T: copiable y comparable con operator< (orden estricto
// debil), igual que una clave de std::map. No necesita constructor por
// defecto salvo para usar AristaEtiquetada::AristaEtiquetada().
//
// Notas:
//  - Los identificadores enteros de vertices eliminados se reutilizan, asi
//    la memoria depende del maximo de vertices simultaneos.
//  - Las listas de resultados (sucesores, recorridos, ...) salen en el orden
//    de los identificadores internos, que es el orden en que se agregaron
//    los vertices (salvo identificadores reutilizados tras eliminar).
//  - Una etiqueta que no existe nunca se inserta por accidente: los metodos
//    de consulta devuelven false, -1, -1.0 o un resultado vacio.
//  - Los mapas de resultado (distancias, predecesores, ...) solo incluyen
//    los vertices alcanzables / con valor definido.
#ifndef GRAFOETIQUETADO_HPP
#define GRAFOETIQUETADO_HPP

#include <fstream>
#include <iomanip>
#include <list>
#include <map>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "Grafo.hpp"

template <typename T>
struct AristaEtiquetada {
    T origen;
    T destino;
    double costo;
    AristaEtiquetada() : origen(), destino(), costo(0.0) {}
    AristaEtiquetada(const T& o, const T& d, double c)
        : origen(o), destino(d), costo(c) {}
};

template <typename T>
class GrafoEtiquetado {
private:
    Grafo g;
    std::map<T, int> idDe;          // etiqueta -> identificador (>= 1)
    std::vector<T> etiquetas;       // identificador - 1 -> etiqueta
    std::vector<char> vivos;        // identificador - 1 -> esta en uso
    std::vector<int> libres;        // identificadores reutilizables

    // Copia deshabilitada (idioma C++98)
    GrafoEtiquetado(const GrafoEtiquetado&);
    GrafoEtiquetado& operator=(const GrafoEtiquetado&);

    bool buscarId(const T& e, int& id) const {
        typename std::map<T, int>::const_iterator it = idDe.find(e);
        if (it == idDe.end()) return false;
        id = it->second;
        return true;
    }

    bool vivo(int id) const {
        return id >= 1 && id <= (int)vivos.size() && vivos[id - 1] != 0;
    }

    const T& etiquetaDe(int id) const { return etiquetas[id - 1]; }

    std::list<T> traducir(const std::list<Vertice>& L) const {
        std::list<T> r;
        for (std::list<Vertice>::const_iterator it = L.begin(); it != L.end(); ++it)
            r.push_back(etiquetaDe(*it));
        return r;
    }

    std::list<std::list<T> > traducirCiclos(
            const std::list<std::list<Vertice> >& cs) const {
        std::list<std::list<T> > r;
        for (std::list<std::list<Vertice> >::const_iterator it = cs.begin();
             it != cs.end(); ++it)
            r.push_back(traducir(*it));
        return r;
    }

    std::list<AristaEtiquetada<T> > traducirAristas(const std::list<Arista>& as) const {
        std::list<AristaEtiquetada<T> > r;
        for (std::list<Arista>::const_iterator it = as.begin(); it != as.end(); ++it)
            r.push_back(AristaEtiquetada<T>(etiquetaDe(it->origen),
                                            etiquetaDe(it->destino), it->costo));
        return r;
    }

    static bool leerEtiqueta(const std::string& texto, T& e) {
        std::istringstream s(texto);
        return (s >> e) ? true : false;
    }

public:
    GrafoEtiquetado() {}

    void construir() { destruir(); }

    void destruir() {
        g.destruir();
        idDe.clear();
        etiquetas.clear();
        vivos.clear();
        libres.clear();
    }

    bool esVacio() const { return g.esVacio(); }
    int orden() const { return g.orden(); }

    // Todas las etiquetas, en orden de identificador interno
    std::list<T> vertices() const {
        std::list<T> r;
        for (size_t i = 0; i < etiquetas.size(); i++)
            if (vivos[i]) r.push_back(etiquetas[i]);
        return r;
    }

    // ------------------------------------------------ vertices y arcos
    bool agregarVertice(const T& e) {
        if (idDe.find(e) != idDe.end()) return false;       // ya existe
        int id;
        if (!libres.empty()) {
            id = libres.back();
            libres.pop_back();
            etiquetas[id - 1] = e;
            vivos[id - 1] = 1;
        } else {
            etiquetas.push_back(e);
            vivos.push_back(1);
            id = (int)etiquetas.size();
        }
        idDe.insert(std::make_pair(e, id));
        g.agregarVertice(id);
        return true;
    }

    bool existeVertice(const T& e) const { return idDe.find(e) != idDe.end(); }

    bool eliminarVertice(const T& e) {
        int id;
        if (!buscarId(e, id)) return false;
        g.eliminarVertice(id);
        idDe.erase(e);
        vivos[id - 1] = 0;
        libres.push_back(id);
        return true;
    }

    // Pre: v y w existen (si no, devuelve false y no hace nada)
    bool agregarArco(const T& v, const T& w, double c) {
        int iv, iw;
        if (!buscarId(v, iv) || !buscarId(w, iw)) return false;
        return g.agregarArco(iv, iw, c);
    }

    bool agregarArcoNoDirigido(const T& v, const T& w, double c) {
        int iv, iw;
        if (!buscarId(v, iv) || !buscarId(w, iw)) return false;
        return g.agregarArcoNoDirigido(iv, iw, c);
    }

    bool eliminarArco(const T& v, const T& w) {
        int iv, iw;
        if (!buscarId(v, iv) || !buscarId(w, iw)) return false;
        return g.eliminarArco(iv, iw);
    }

    bool eliminarArcoNoDirigido(const T& v, const T& w) {
        int iv, iw;
        if (!buscarId(v, iv) || !buscarId(w, iw)) return false;
        return g.eliminarArcoNoDirigido(iv, iw);
    }

    bool existeArco(const T& v, const T& w) const {
        int iv, iw;
        if (!buscarId(v, iv) || !buscarId(w, iw)) return false;
        return g.existeArco(iv, iw);
    }

    double costoArco(const T& v, const T& w) const {          // -1.0 si no existe
        int iv, iw;
        if (!buscarId(v, iv) || !buscarId(w, iw)) return -1.0;
        return g.costoArco(iv, iw);
    }

    // ------------------------------------------------ vecindad y grados
    std::list<T> sucesores(const T& v) const {
        int id;
        if (!buscarId(v, id)) return std::list<T>();
        return traducir(g.sucesores(id));
    }

    std::list<T> predecesores(const T& v) const {
        int id;
        if (!buscarId(v, id)) return std::list<T>();
        return traducir(g.predecesores(id));
    }

    int gradoExterior(const T& v) const {                     // -1 si no existe
        int id;
        return buscarId(v, id) ? g.gradoExterior(id) : -1;
    }
    int gradoInterior(const T& v) const {
        int id;
        return buscarId(v, id) ? g.gradoInterior(id) : -1;
    }
    int grado(const T& v) const {
        int id;
        return buscarId(v, id) ? g.grado(id) : -1;
    }

    std::map<T, int> obtenerGradosDeEntrada() const {
        std::map<T, int> r;
        std::vector<int> gr;
        g.obtenerGradosDeEntrada(gr);
        for (int v = 1; v < (int)gr.size(); v++)
            if (vivo(v)) r.insert(std::make_pair(etiquetaDe(v), gr[v]));
        return r;
    }

    std::list<T> obtenerVerticesFuente() const {
        return traducir(g.obtenerVerticesFuente());
    }

    // ------------------------------------------------ recorridos
    std::list<T> recorridoBFS(const T& s) const {
        int id;
        if (!buscarId(s, id)) return std::list<T>();
        return traducir(g.recorridoBFS(id));
    }

    // Vertices alcanzables desde s (s incluido) en el orden en que DFS los
    // descubre. Lista vacia si s no existe.
    std::list<T> recorridoDFS(const T& s) const {
        int id;
        if (!buscarId(s, id)) return std::list<T>();
        return traducir(g.recorridoDFS(id));
    }

    // Vertices alcanzables desde s (s incluido) en orden de identificador
    // interno (ver notas del encabezado). Lista vacia si s no existe.
    std::list<T> alcanzables(const T& s) const {
        int id;
        if (!buscarId(s, id)) return std::list<T>();
        return traducir(g.alcanzables(id));
    }

    // dist: aristas desde s a cada vertice alcanzable; pred: su predecesor
    // en el arbol BFS (s no aparece en pred).
    void BFS(const T& s, std::map<T, int>& dist, std::map<T, T>& pred) const {
        dist.clear();
        pred.clear();
        int id;
        if (!buscarId(s, id)) return;
        std::vector<int> d, p;
        g.BFS(id, d, p);
        for (int v = 1; v < (int)d.size(); v++) {
            if (!vivo(v) || d[v] == INFINITO) continue;
            dist.insert(std::make_pair(etiquetaDe(v), d[v]));
            if (p[v] != 0)
                pred.insert(std::make_pair(etiquetaDe(v), etiquetaDe(p[v])));
        }
    }

    // tdesc / tfinal para todos los vertices; pred solo para los no raiz.
    void DFS(std::map<T, T>& pred, std::map<T, int>& tdesc,
             std::map<T, int>& tfinal) const {
        pred.clear();
        tdesc.clear();
        tfinal.clear();
        std::vector<int> p, td, tf;
        g.DFS(p, td, tf);
        for (int v = 1; v < (int)p.size(); v++) {
            if (!vivo(v)) continue;
            tdesc.insert(std::make_pair(etiquetaDe(v), td[v]));
            tfinal.insert(std::make_pair(etiquetaDe(v), tf[v]));
            if (p[v] != 0)
                pred.insert(std::make_pair(etiquetaDe(v), etiquetaDe(p[v])));
        }
    }

    std::list<T> ordenamientoTopologico() const {
        return traducir(g.ordenamientoTopologico());
    }
    std::list<T> ordenamientoTopologicoDFS() const {
        return traducir(g.ordenamientoTopologicoDFS());
    }

    // ------------------------------------------------ ciclos y componentes
    bool tieneCiclos() const { return g.tieneCiclos(); }
    bool tieneCiclosNoDirigido() const { return g.tieneCiclosNoDirigido(); }
    int componentesConexas() const { return g.componentesConexas(); }

    // Ver Grafo::esBipartito. Con A y B devuelve los dos conjuntos (en orden
    // de identificador interno); si no es bipartito quedan vacios.
    bool esBipartito() const { return g.esBipartito(); }
    bool esBipartito(std::list<T>& A, std::list<T>& B) const {
        std::list<Vertice> a, b;
        bool ok = g.esBipartito(a, b);
        A = traducir(a);
        B = traducir(b);
        return ok;
    }

    // Ver Grafo::tieneCicloEuleriano / tieneCicloEulerianoNoDirigido
    bool tieneCicloEuleriano() const { return g.tieneCicloEuleriano(); }
    bool tieneCicloEulerianoNoDirigido() const { return g.tieneCicloEulerianoNoDirigido(); }

    // Ver Grafo::tieneCicloHamiltoniano. Con ciclo devuelve las etiquetas en
    // orden de recorrido (empieza por el vertice de menor identificador
    // interno); si no existe, la lista queda vacia.
    bool tieneCicloHamiltoniano() const { return g.tieneCicloHamiltoniano(); }
    bool tieneCicloHamiltoniano(std::list<T>& ciclo) const {
        std::list<Vertice> c;
        bool ok = g.tieneCicloHamiltoniano(c);
        ciclo = traducir(c);
        return ok;
    }
    bool tieneCicloHamiltonianoNoDirigido() const {
        return g.tieneCicloHamiltonianoNoDirigido();
    }
    bool tieneCicloHamiltonianoNoDirigido(std::list<T>& ciclo) const {
        std::list<Vertice> c;
        bool ok = g.tieneCicloHamiltonianoNoDirigido(c);
        ciclo = traducir(c);
        return ok;
    }

    // Ver Grafo::excentricidad y Grafo::radio. excentricidad devuelve -1.0 si
    // la etiqueta no existe; DIST_INFINITA si algun vertice es inalcanzable.
    double excentricidad(const T& v) const {
        int id;
        return buscarId(v, id) ? g.excentricidad(id) : -1.0;
    }
    double radio() const { return g.radio(); }

    // Ver Grafo::esPlanar
    bool esPlanar() const { return g.esPlanar(); }

    // Ver Grafo::puntosArticulacion (orden de identificador interno)
    std::list<T> puntosArticulacion() const {
        return traducir(g.puntosArticulacion());
    }

    std::list<std::list<T> > enumerarCiclos(int maxCiclos, bool& truncado) const {
        return traducirCiclos(g.enumerarCiclos(maxCiclos, truncado));
    }
    int contarCiclos(int maxCiclos, bool& truncado) const {
        return g.contarCiclos(maxCiclos, truncado);
    }
    std::list<std::list<T> > enumerarCiclosNoDirigido(int maxCiclos, bool& truncado) const {
        return traducirCiclos(g.enumerarCiclosNoDirigido(maxCiclos, truncado));
    }
    int contarCiclosNoDirigido(int maxCiclos, bool& truncado) const {
        return g.contarCiclosNoDirigido(maxCiclos, truncado);
    }

    // ------------------------------------------------ dibujo
    // Ver Grafo::exportarSVG. Las etiquetas se escriben con operator<<, asi
    // que T debe poder imprimirse con un ostream (solo se exige si se usa
    // este metodo).
    bool exportarSVG(const std::string& archivo, bool noDirigido) const {
        std::vector<std::string> textos(etiquetas.size() + 1);
        for (size_t i = 0; i < etiquetas.size(); i++) {
            if (!vivos[i]) continue;
            std::ostringstream s;
            s << etiquetas[i];
            textos[i + 1] = s.str();
        }
        return g.exportarSVG(archivo, noDirigido, textos);
    }

    // ------------------------------------------------ caminos minimos
    // Dijkstra: dist solo con los vertices alcanzables (incluye s con 0);
    // pred con su predecesor en el camino minimo (s no aparece). Devuelve
    // false (mapas vacios) si s no existe o hay algun costo negativo.
    bool dijkstra(const T& s, std::map<T, double>& dist, std::map<T, T>& pred) const {
        dist.clear();
        pred.clear();
        int id;
        if (!buscarId(s, id)) return false;
        std::vector<double> d;
        std::vector<int> p;
        if (!g.dijkstra(id, d, p)) return false;
        for (int v = 1; v < (int)d.size(); v++) {
            if (!vivo(v) || d[v] == DIST_INFINITA) continue;
            dist.insert(std::make_pair(etiquetaDe(v), d[v]));
            if (p[v] != 0)
                pred.insert(std::make_pair(etiquetaDe(v), etiquetaDe(p[v])));
        }
        return true;
    }

    std::map<T, double> dijkstra(const T& s) const {
        std::map<T, double> d;
        std::map<T, T> p;
        dijkstra(s, d, p);
        return d;
    }

    // Bellman-Ford (admite costos negativos). Igual que dijkstra: dist solo
    // con los vertices alcanzables (incluye s con 0), pred con su predecesor
    // (s no aparece). Devuelve false (mapas vacios) si s no existe o hay un
    // ciclo negativo alcanzable desde s.
    bool bellmanFord(const T& s, std::map<T, double>& dist, std::map<T, T>& pred) const {
        dist.clear();
        pred.clear();
        int id;
        if (!buscarId(s, id)) return false;
        std::vector<double> d;
        std::vector<int> p;
        if (!g.bellmanFord(id, d, p)) return false;
        for (int v = 1; v < (int)d.size(); v++) {
            if (!vivo(v) || d[v] == DIST_INFINITA) continue;
            dist.insert(std::make_pair(etiquetaDe(v), d[v]));
            if (p[v] != 0)
                pred.insert(std::make_pair(etiquetaDe(v), etiquetaDe(p[v])));
        }
        return true;
    }

    std::map<T, double> bellmanFord(const T& s) const {
        std::map<T, double> d;
        std::map<T, T> p;
        bellmanFord(s, d, p);
        return d;
    }

    // Floyd-Warshall (admite costos negativos): dist[u][v] solo para los
    // pares con v alcanzable desde u (dist[u][u] = 0 siempre esta). sig[u][v]
    // = etiqueta que sigue a u en un camino minimo hacia v (solo u != v
    // alcanzables). Devuelve false (mapas vacios) si hay un ciclo negativo.
    bool floydWarshall(std::map<T, std::map<T, double> >& dist,
                       std::map<T, std::map<T, T> >& sig) const {
        dist.clear();
        sig.clear();
        std::vector<std::vector<double> > d;
        std::vector<std::vector<int> > s;
        if (!g.floydWarshall(d, s)) return false;
        for (int u = 1; u < (int)d.size(); u++) {
            if (!vivo(u)) continue;
            std::map<T, double>& filaD = dist[etiquetaDe(u)];
            std::map<T, T>& filaS = sig[etiquetaDe(u)];
            for (int v = 1; v < (int)d.size(); v++) {
                if (!vivo(v) || d[u][v] == DIST_INFINITA) continue;
                filaD.insert(std::make_pair(etiquetaDe(v), d[u][v]));
                if (s[u][v] != 0)
                    filaS.insert(std::make_pair(etiquetaDe(v), etiquetaDe(s[u][v])));
            }
        }
        return true;
    }

    bool floydWarshall(std::map<T, std::map<T, double> >& dist) const {
        std::map<T, std::map<T, T> > sig;
        return floydWarshall(dist, sig);
    }

    // Camino minimo de u a v (ambos incluidos) con la matriz sig de
    // floydWarshall. Lista vacia si no existen, si v es inalcanzable o si
    // sig no corresponde a este grafo. Si u == v: [u].
    std::list<T> reconstruirCamino(const std::map<T, std::map<T, T> >& sig,
                                   const T& u, const T& v) const {
        std::list<T> camino;
        if (!existeVertice(u) || !existeVertice(v)) return camino;
        camino.push_back(u);
        int pasos = 0;
        T act = u;
        while ((act < v) || (v < act)) {                      // act != v (solo con operator<)
            typename std::map<T, std::map<T, T> >::const_iterator fila = sig.find(act);
            if (fila == sig.end()) return std::list<T>();
            typename std::map<T, T>::const_iterator prox = fila->second.find(v);
            if (prox == fila->second.end() || ++pasos > orden())
                return std::list<T>();
            act = prox->second;
            camino.push_back(act);
        }
        return camino;
    }

    // Camino de menor costo de u a v (ver Grafo::caminoMenorCosto, usa
    // Dijkstra): etiquetas de u a v, ambas incluidas; costo recibe el total.
    // Lista vacia (y costo = -1.0) si alguna etiqueta no existe, si v no es
    // alcanzable desde u o si hay algun costo negativo. Si u == v: [u], 0.
    std::list<T> caminoMenorCosto(const T& u, const T& v, double& costo) const {
        costo = -1.0;
        int iu, iv;
        if (!buscarId(u, iu) || !buscarId(v, iv)) return std::list<T>();
        return traducir(g.caminoMenorCosto(iu, iv, costo));
    }
    std::list<T> caminoMenorCosto(const T& u, const T& v) const {
        double costo;
        return caminoMenorCosto(u, v, costo);
    }

    // Camino simple de u a v (ver Grafo::caminoSimple): etiquetas de u a v,
    // ambas incluidas; el camino de menos arcos. Lista vacia si alguna
    // etiqueta no existe o v no es alcanzable desde u. Si u == v: [u].
    std::list<T> caminoSimple(const T& u, const T& v) const {
        int iu, iv;
        if (!buscarId(u, iu) || !buscarId(v, iv)) return std::list<T>();
        return traducir(g.caminoSimple(iu, iv));
    }

    // Invierte todos los arcos (ver Grafo::reverseGraph). O(n + m).
    void reverseGraph() { g.reverseGraph(); }

    // Camino con menos arcos de v a w pasando por intermedio (ver
    // Grafo::getCaminoMasCortoArcos). Lista vacia si alguna etiqueta no existe
    // o no hay camino.
    std::list<T> getCaminoMasCortoArcos(const T& v, const T& w, const T& intermedio) const {
        int iv, iw, ii;
        if (!buscarId(v, iv) || !buscarId(w, iw) || !buscarId(intermedio, ii))
            return std::list<T>();
        return traducir(g.getCaminoMasCortoArcos(iv, iw, ii));
    }

    // Isomorfismo (ver Grafo::esIsomorfo): solo cuenta la estructura del
    // grafo; las etiquetas NO tienen que coincidir ni se comparan.
    bool esIsomorfo(const GrafoEtiquetado<T>& otro) const {
        return g.esIsomorfo(otro.g);
    }
    bool esIsomorfo(const GrafoEtiquetado<T>& otro, bool compararCostos) const {
        return g.esIsomorfo(otro.g, compararCostos);
    }

    // Ruta de mayor costo desde s (ver Grafo::rutaMayorCosto): dist[v] y
    // ruta[v] (etiquetas de s a v, ambos incluidos) solo para los vertices
    // alcanzables, s incluido con costo 0. Devuelve false (mapas vacios) si s
    // no existe. Exponencial si lo alcanzable desde s tiene ciclos.
    bool rutaMayorCosto(const T& s, std::map<T, double>& dist,
                        std::map<T, std::list<T> >& ruta) const {
        dist.clear();
        ruta.clear();
        int id;
        if (!buscarId(s, id)) return false;
        std::vector<double> d;
        std::vector<std::list<Vertice> > r;
        if (!g.rutaMayorCosto(id, d, r)) return false;
        for (int v = 1; v < (int)d.size(); v++) {
            if (!vivo(v) || d[v] == -DIST_INFINITA) continue;
            dist.insert(std::make_pair(etiquetaDe(v), d[v]));
            ruta.insert(std::make_pair(etiquetaDe(v), traducir(r[v])));
        }
        return true;
    }

    // ------------------------------------------------ arbol de expansion minima
    // Ver Grafo::kruskal. En cada arista, origen es el vertice que se agrego
    // primero al grafo.
    std::list<AristaEtiquetada<T> > kruskal(double& pesoTotal, bool& conexo) const {
        std::list<Arista> r = g.kruskal(pesoTotal, conexo);
        std::list<AristaEtiquetada<T> > res;
        for (std::list<Arista>::const_iterator it = r.begin(); it != r.end(); ++it)
            res.push_back(AristaEtiquetada<T>(etiquetaDe(it->origen),
                                              etiquetaDe(it->destino), it->costo));
        return res;
    }

    // Reconstruye ESTE grafo con los vertices de las aristas dadas (descarta
    // su contenido). Es seguro pasarle el resultado de su propio kruskal().
    void construirDesdeAristas(const std::list<AristaEtiquetada<T> >& aristas,
                               bool noDirigido) {
        destruir();
        for (typename std::list<AristaEtiquetada<T> >::const_iterator it = aristas.begin();
             it != aristas.end(); ++it) {
            agregarVertice(it->origen);       // ignorado si ya existe
            agregarVertice(it->destino);
            if (noDirigido) agregarArcoNoDirigido(it->origen, it->destino, it->costo);
            else agregarArco(it->origen, it->destino, it->costo);
        }
    }

    // =====================================================================
    // AMPLIACION: mismos metodos que la ampliacion de Grafo (ver Grafo.hpp
    // para el detalle de cada uno), con etiquetas en lugar de enteros.
    // =====================================================================

    // ---------------------------------------------------- Copia y tamano
    void intercambiar(GrafoEtiquetado<T>& otro) {
        g.intercambiar(otro.g);
        idDe.swap(otro.idDe);
        etiquetas.swap(otro.etiquetas);
        vivos.swap(otro.vivos);
        libres.swap(otro.libres);
    }

    void copiar(const GrafoEtiquetado<T>& otro) {
        if (&otro == this) return;
        g.copiar(otro.g);
        idDe = otro.idDe;
        etiquetas = otro.etiquetas;
        vivos = otro.vivos;
        libres = otro.libres;
    }

    int cantidadArcos() const { return g.cantidadArcos(); }
    int cantidadAristas() const { return g.cantidadAristas(); }
    bool esSimetrico() const { return g.esSimetrico(); }
    bool esSimetrico(bool compararCostos) const { return g.esSimetrico(compararCostos); }
    bool esDirigido() const { return g.esDirigido(); }
    double densidad() const { return g.densidad(); }

    // --------------------------------- Modificacion de arcos y vertices
    bool agregarArcoSiNoExiste(const T& v, const T& w, double c) {
        int iv, iw;
        if (!buscarId(v, iv) || !buscarId(w, iw)) return false;
        return g.agregarArcoSiNoExiste(iv, iw, c);
    }
    bool actualizarCosto(const T& v, const T& w, double c) {
        int iv, iw;
        if (!buscarId(v, iv) || !buscarId(w, iw)) return false;
        return g.actualizarCosto(iv, iw, c);
    }
    bool actualizarCostoNoDirigido(const T& v, const T& w, double c) {
        int iv, iw;
        if (!buscarId(v, iv) || !buscarId(w, iw)) return false;
        return g.actualizarCostoNoDirigido(iv, iw, c);
    }
    int eliminarArcosDe(const T& v) {
        int id;
        return buscarId(v, id) ? g.eliminarArcosDe(id) : -1;
    }
    int eliminarArcosHacia(const T& v) {
        int id;
        return buscarId(v, id) ? g.eliminarArcosHacia(id) : -1;
    }
    int aislarVertice(const T& v) {
        int id;
        return buscarId(v, id) ? g.aislarVertice(id) : -1;
    }

    // Cambia la etiqueta v por w (los arcos no se tocan). false si v no
    // existe o si w ya es OTRA etiqueta existente; si w equivale a v, true.
    bool renombrarVertice(const T& v, const T& w) {
        int id;
        if (!buscarId(v, id)) return false;
        if (!(v < w) && !(w < v)) return true;
        if (existeVertice(w)) return false;
        idDe.erase(v);
        idDe.insert(std::make_pair(w, id));
        etiquetas[id - 1] = w;
        return true;
    }

    bool contraerArista(const T& u, const T& v) {
        int iu, iv;
        if (!buscarId(u, iu) || !buscarId(v, iv)) return false;
        if (!g.contraerArista(iu, iv)) return false;
        idDe.erase(v);
        vivos[iv - 1] = 0;
        libres.push_back(iv);
        return true;
    }

    // Union (este grafo := este U otro): costo de este grafo si el arco esta
    // en ambos.
    void unir(const GrafoEtiquetado<T>& otro) {
        if (&otro == this) return;
        for (size_t i = 0; i < otro.etiquetas.size(); i++)
            if (otro.vivos[i]) agregarVertice(otro.etiquetas[i]);
        std::list<Arista> as = otro.g.listaDeAristas(false);
        for (std::list<Arista>::const_iterator it = as.begin(); it != as.end(); ++it)
            agregarArcoSiNoExiste(otro.etiquetaDe(it->origen), otro.etiquetaDe(it->destino),
                                  it->costo);
    }

    // Interseccion (este grafo := este ^ otro), con los costos de este grafo.
    void interseccion(const GrafoEtiquetado<T>& otro) {
        if (&otro == this) return;
        GrafoEtiquetado<T> r;
        for (size_t i = 0; i < etiquetas.size(); i++)
            if (vivos[i] && otro.existeVertice(etiquetas[i])) r.agregarVertice(etiquetas[i]);
        std::list<Arista> as = g.listaDeAristas(false);
        for (std::list<Arista>::const_iterator it = as.begin(); it != as.end(); ++it) {
            const T& a = etiquetaDe(it->origen);
            const T& b = etiquetaDe(it->destino);
            if (r.existeVertice(a) && r.existeVertice(b) && otro.existeArco(a, b))
                r.agregarArco(a, b, it->costo);
        }
        intercambiar(r);
    }

    // Subgrafo inducido por vs. false (resultado intacto) si falta alguna.
    bool subgrafo(const std::list<T>& vs, GrafoEtiquetado<T>& resultado) const {
        std::set<int> ids;
        for (typename std::list<T>::const_iterator it = vs.begin(); it != vs.end(); ++it) {
            int id;
            if (!buscarId(*it, id)) return false;
            ids.insert(id);
        }
        GrafoEtiquetado<T> r;
        for (std::set<int>::const_iterator it = ids.begin(); it != ids.end(); ++it)
            r.agregarVertice(etiquetaDe(*it));
        std::list<Arista> as = g.listaDeAristas(false);
        for (std::list<Arista>::const_iterator it = as.begin(); it != as.end(); ++it)
            if (ids.count(it->origen) && ids.count(it->destino))
                r.agregarArco(etiquetaDe(it->origen), etiquetaDe(it->destino), it->costo);
        resultado.intercambiar(r);
        return true;
    }

    // Complemento (arcos de costo 1.0 entre los pares sin arco).
    void complemento(GrafoEtiquetado<T>& resultado) const {
        Grafo c;
        g.complemento(c);
        GrafoEtiquetado<T> r;
        for (size_t i = 0; i < etiquetas.size(); i++)
            if (vivos[i]) r.agregarVertice(etiquetas[i]);
        std::list<Arista> as = c.listaDeAristas(false);
        for (std::list<Arista>::const_iterator it = as.begin(); it != as.end(); ++it)
            r.agregarArco(etiquetaDe(it->origen), etiquetaDe(it->destino), it->costo);
        resultado.intercambiar(r);
    }

    // ------------------------------------------------ Estructura del grafo
    std::list<std::list<T> > componentesFuertementeConexas() const {
        return traducirCiclos(g.componentesFuertementeConexas());
    }
    bool esFuertementeConexo() const { return g.esFuertementeConexo(); }
    bool esConexo() const { return g.esConexo(); }

    std::list<AristaEtiquetada<T> > puentes() const {
        return traducirAristas(g.puentes());
    }
    bool esBiconexo() const { return g.esBiconexo(); }
    bool esBosque() const { return g.esBosque(); }
    bool esArbol() const { return g.esArbol(); }
    // Arbol dirigido (arcos del padre al hijo); con raiz fija si se indica.
    bool esArbolDirigido() const { return g.esArbolDirigido(); }
    bool esArbolDirigido(const T& raiz) const {
        int id;
        return buscarId(raiz, id) && g.esArbolDirigido(id);
    }
    bool esDAG() const { return g.esDAG(); }
    bool esCompleto() const { return g.esCompleto(); }
    bool esRegular() const { return g.esRegular(); }
    bool esRegular(int& k) const { return g.esRegular(k); }
    int cintura() const { return g.cintura(); }
    int cinturaNoDirigido() const { return g.cinturaNoDirigido(); }
    double diametro() const { return g.diametro(); }
    std::list<T> centro() const { return traducir(g.centro()); }

    // alcanza[v] = vertices alcanzables desde v con uno o mas arcos
    void clausuraTransitiva(std::map<T, std::list<T> >& alcanza) const {
        alcanza.clear();
        std::vector<std::vector<char> > m;
        g.clausuraTransitiva(m);
        for (int u = 1; u < (int)m.size(); u++) {
            if (!vivo(u)) continue;
            std::list<T>& fila = alcanza[etiquetaDe(u)];
            for (int v = 1; v < (int)m.size(); v++)
                if (vivo(v) && m[u][v]) fila.push_back(etiquetaDe(v));
        }
    }

    // ---------------------------------------------------- Caminos
    bool existeCamino(const T& u, const T& v) const {
        int iu, iv;
        if (!buscarId(u, iu) || !buscarId(v, iv)) return false;
        return g.existeCamino(iu, iv);
    }
    std::list<T> caminoMasLargoArcos(const T& u, const T& v) const {
        int iu, iv;
        if (!buscarId(u, iu) || !buscarId(v, iv)) return std::list<T>();
        return traducir(g.caminoMasLargoArcos(iu, iv));
    }
    std::list<T> caminoMasLargoArcos() const {
        return traducir(g.caminoMasLargoArcos());
    }
    std::list<std::list<T> > kCaminosMasCortos(const T& u, const T& v, int k,
                                               std::list<double>& costos) const {
        costos.clear();
        int iu, iv;
        if (!buscarId(u, iu) || !buscarId(v, iv)) return std::list<std::list<T> >();
        return traducirCiclos(g.kCaminosMasCortos(iu, iv, k, costos));
    }
    std::list<std::list<T> > kCaminosMasCortos(const T& u, const T& v, int k) const {
        std::list<double> costos;
        return kCaminosMasCortos(u, v, k, costos);
    }
    std::list<T> caminoEuleriano() const { return traducir(g.caminoEuleriano()); }
    bool tieneCaminoEuleriano() const { return g.tieneCaminoEuleriano(); }
    std::list<T> cicloEuleriano() const { return traducir(g.cicloEuleriano()); }
    std::list<T> caminoEulerianoNoDirigido() const {
        return traducir(g.caminoEulerianoNoDirigido());
    }
    bool tieneCaminoEulerianoNoDirigido() const { return g.tieneCaminoEulerianoNoDirigido(); }
    std::list<T> cicloEulerianoNoDirigido() const {
        return traducir(g.cicloEulerianoNoDirigido());
    }
    // Ver Grafo::caminoEulerianoDesde. "Menor lexicograficamente" es por el
    // orden de los identificadores internos, o sea el orden en que se agregaron
    // los vertices: para que sea el orden de T, agregarlos en orden ascendente.
    // Lista vacia si s no existe.
    std::list<T> caminoEulerianoDesde(const T& s) const {
        int is;
        if (!buscarId(s, is)) return std::list<T>();
        return traducir(g.caminoEulerianoDesde(is));
    }
    std::list<T> caminoEulerianoDesde(const T& s, bool costoEsCantidad) const {
        int is;
        if (!buscarId(s, is)) return std::list<T>();
        return traducir(g.caminoEulerianoDesde(is, costoEsCantidad));
    }

    // --------------------------- Arboles de expansion, flujos, emparejamientos
    std::list<AristaEtiquetada<T> > prim(const T& s, double& pesoTotal, bool& conexo) const {
        pesoTotal = 0.0;
        conexo = false;
        int id;
        if (!buscarId(s, id)) return std::list<AristaEtiquetada<T> >();
        return traducirAristas(g.prim(id, pesoTotal, conexo));
    }
    std::list<AristaEtiquetada<T> > kruskalMaximo(double& pesoTotal, bool& conexo) const {
        return traducirAristas(g.kruskalMaximo(pesoTotal, conexo));
    }
    double flujoMaximo(const T& s, const T& t) const {
        int is, it;
        if (!buscarId(s, is) || !buscarId(t, it)) return -1.0;
        return g.flujoMaximo(is, it);
    }
    double flujoMaximo(const T& s, const T& t, std::list<AristaEtiquetada<T> >& flujo) const {
        flujo.clear();
        int is, it;
        if (!buscarId(s, is) || !buscarId(t, it)) return -1.0;
        std::list<Arista> f;
        double r = g.flujoMaximo(is, it, f);
        flujo = traducirAristas(f);
        return r;
    }
    int emparejamientoMaximo(std::list<AristaEtiquetada<T> >& pares) const {
        std::list<Arista> p;
        int r = g.emparejamientoMaximo(p);
        pares = traducirAristas(p);
        return r;
    }

    // ------------------------------------------ Problemas NP (exactos)
    // color[etiqueta] = 1..k; devuelve k
    int coloreadoVoraz(std::map<T, int>& color) const {
        color.clear();
        std::vector<int> c;
        int k = g.coloreadoVoraz(c);
        for (int v = 1; v < (int)c.size(); v++)
            if (vivo(v)) color.insert(std::make_pair(etiquetaDe(v), c[v]));
        return k;
    }
    std::list<T> conjuntoDominanteMinimo() const {
        return traducir(g.conjuntoDominanteMinimo());
    }
    std::list<T> conjuntoIndependienteMaximo() const {
        return traducir(g.conjuntoIndependienteMaximo());
    }
    std::list<T> cliqueMaxima() const { return traducir(g.cliqueMaxima()); }

    // ------------------------------------------- Entrada, salida y vistas
    // Una linea por vertice: "A -> B(5), C(7)". Requiere operator<< para T.
    void imprimir(std::ostream& os) const {
        if (esVacio()) {
            os << "(grafo vacio)\n";
            return;
        }
        for (size_t i = 0; i < etiquetas.size(); i++) {
            if (!vivos[i]) continue;
            int id = (int)i + 1;
            os << etiquetas[i] << " ->";
            std::list<Vertice> suc = g.sucesores(id);
            if (suc.empty()) os << " (sin arcos)";
            bool primero = true;
            for (std::list<Vertice>::const_iterator it = suc.begin(); it != suc.end(); ++it) {
                os << (primero ? " " : ", ") << etiquetaDe(*it)
                   << "(" << g.costoArco(id, *it) << ")";
                primero = false;
            }
            os << "\n";
        }
    }

    std::list<AristaEtiquetada<T> > listaDeAristas(bool noDirigido) const {
        // noDirigido: cada par una vez, segun el orden de los identificadores
        return traducirAristas(g.listaDeAristas(noDirigido));
    }

    // Las filas/columnas siguen el orden de "orden" (etiquetas en orden de
    // identificador interno): ady[i][j] = hay arco orden[i] -> orden[j].
    void obtenerMatrizAdyacencia(std::vector<T>& orden,
                                 std::vector<std::vector<char> >& ady) const {
        orden.clear();
        std::vector<int> pos(etiquetas.size() + 1, -1);
        for (size_t i = 0; i < etiquetas.size(); i++)
            if (vivos[i]) { pos[i + 1] = (int)orden.size(); orden.push_back(etiquetas[i]); }
        ady.assign(orden.size(), std::vector<char>(orden.size(), 0));
        std::list<Arista> as = g.listaDeAristas(false);
        for (std::list<Arista>::const_iterator it = as.begin(); it != as.end(); ++it)
            ady[pos[it->origen]][pos[it->destino]] = 1;
    }
    // costo[i][j] = costo del arco o DIST_INFINITA si no existe
    void obtenerMatrizCostos(std::vector<T>& orden,
                             std::vector<std::vector<double> >& costo) const {
        orden.clear();
        std::vector<int> pos(etiquetas.size() + 1, -1);
        for (size_t i = 0; i < etiquetas.size(); i++)
            if (vivos[i]) { pos[i + 1] = (int)orden.size(); orden.push_back(etiquetas[i]); }
        costo.assign(orden.size(), std::vector<double>(orden.size(), DIST_INFINITA));
        std::list<Arista> as = g.listaDeAristas(false);
        for (std::list<Arista>::const_iterator it = as.begin(); it != as.end(); ++it)
            costo[pos[it->origen]][pos[it->destino]] = it->costo;
    }

    // Archivo de texto: "origen destino costo" por arco y "@v etiqueta" por
    // vertice aislado ('#' inicia un comentario). Las etiquetas se escriben
    // con operator<< y se leen con operator>>, asi que no pueden contener
    // espacios (T necesita ademas constructor por defecto para leer).
    // Misma semantica que Grafo::guardarEnArchivo / leerDesdeArchivo.
    bool guardarEnArchivo(const std::string& archivo, bool noDirigido) const {
        std::ofstream f(archivo.c_str());
        if (!f) return false;
        f << std::setprecision(15);
        f << "# Grafo " << (noDirigido ? "no dirigido" : "dirigido") << ": "
          << orden() << " vertices\n"
          << "# formato: 'origen destino costo' por arco; '@v etiqueta' = vertice aislado\n";
        std::vector<int> entrada;
        g.obtenerGradosDeEntrada(entrada);
        for (size_t i = 0; i < etiquetas.size(); i++)
            if (vivos[i] && g.gradoExterior((int)i + 1) == 0 && entrada[i + 1] == 0)
                f << "@v " << etiquetas[i] << "\n";
        std::list<Arista> as = g.listaDeAristas(noDirigido);
        for (std::list<Arista>::const_iterator it = as.begin(); it != as.end(); ++it)
            f << etiquetaDe(it->origen) << " " << etiquetaDe(it->destino)
              << " " << it->costo << "\n";
        f.flush();
        return f.good();
    }

    bool leerDesdeArchivo(const std::string& archivo, bool noDirigido) {
        std::ifstream f(archivo.c_str());
        if (!f) return false;
        GrafoEtiquetado<T> tmp;
        std::string linea;
        while (std::getline(f, linea)) {
            std::string::size_type h = linea.find('#');
            if (h != std::string::npos) linea.erase(h);
            std::istringstream ss(linea);
            std::string t1, t2, t3, t4;
            if (!(ss >> t1)) continue;
            if (t1 == "@v") {
                if (!(ss >> t2) || (ss >> t3)) return false;
                T e;
                if (!leerEtiqueta(t2, e)) return false;
                tmp.agregarVertice(e);
                continue;
            }
            double c = 1.0;
            if (!(ss >> t2)) return false;
            if (ss >> t3) {
                if (ss >> t4) return false;
                std::istringstream sc(t3);
                char resto;
                if (!(sc >> c) || (sc >> resto)) return false;
            }
            T a, b;
            if (!leerEtiqueta(t1, a) || !leerEtiqueta(t2, b)) return false;
            tmp.agregarVertice(a);
            tmp.agregarVertice(b);
            if (noDirigido) tmp.agregarArcoNoDirigido(a, b, c);
            else tmp.agregarArco(a, b, c);
        }
        intercambiar(tmp);
        return true;
    }

    // Graphviz; el texto de cada nodo es su etiqueta (via operator<<).
    bool exportarDOT(const std::string& archivo, bool noDirigido) const {
        std::vector<std::string> textos(etiquetas.size() + 1);
        for (size_t i = 0; i < etiquetas.size(); i++) {
            if (!vivos[i]) continue;
            std::ostringstream s;
            s << etiquetas[i];
            textos[i + 1] = s.str();
        }
        return g.exportarDOT(archivo, noDirigido, textos);
    }
};

template <typename T>
std::ostream& operator<<(std::ostream& os, const GrafoEtiquetado<T>& G) {
    G.imprimir(os);
    return os;
}

#endif
