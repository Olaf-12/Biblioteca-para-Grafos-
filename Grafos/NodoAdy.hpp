// NodoAdy.hpp - Nodo de la lista de adyacencia de un vertice
#ifndef NODOADY_HPP
#define NODOADY_HPP

#include <cstddef>
#include "Vertice.hpp"


class NodoAdy {
private:
    Vertice info;
    double costo;
    NodoAdy* prox;

public:
    NodoAdy() : info(0), costo(0.0), prox(NULL) {}
    NodoAdy(Vertice v, double c, NodoAdy* p) : info(v), costo(c), prox(p) {}

    Vertice getInfo() const { return info; }
    double getCosto() const { return costo; }
    NodoAdy* getProx() const { return prox; }

    void setInfo(Vertice v) { info = v; }
    void setCosto(double c) { costo = c; }
    void setProx(NodoAdy* p) { prox = p; }
};

#endif
