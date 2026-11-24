// NodoVertice.hpp - Nodo de la lista de vertices del grafo
#ifndef NODOVERTICE_HPP
#define NODOVERTICE_HPP

#include <cstddef>
#include "NodoAdy.hpp"

class NodoVertice {
private:
    Vertice info;
    NodoVertice* prox;
    NodoAdy* listaAdy;

public:
    NodoVertice() : info(0), prox(NULL), listaAdy(NULL) {}
    NodoVertice(Vertice v, NodoVertice* p, NodoAdy* lista)
        : info(v), prox(p), listaAdy(lista) {}

    Vertice getInfo() const { return info; }
    NodoVertice* getProx() const { return prox; }
    NodoAdy* getListaAdy() const { return listaAdy; }

    void setInfo(Vertice v) { info = v; }
    void setProx(NodoVertice* p) { prox = p; }
    void setListaAdy(NodoAdy* lista) { listaAdy = lista; }
};

#endif
