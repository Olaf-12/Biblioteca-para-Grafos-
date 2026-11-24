CXX = g++
CXXFLAGS = -std=c++98 -Wall -Wextra -pedantic
OBJS = main.o Grafo.o

# Linux / macOS / MSYS / Git Bash: rm. Windows con cmd.exe: del.
# En Windows g++ agrega la extension .exe al ejecutable.
ifeq ($(OS),Windows_NT)
    EXE = .exe
    ifdef MSYSTEM
        BORRAR = rm -f
    else
        BORRAR = del /Q /F
    endif
else
    EXE =
    BORRAR = rm -f
endif

grafo$(EXE): $(OBJS)
	$(CXX) $(CXXFLAGS) -o grafo$(EXE) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

main.o: main.cpp Grafo.hpp Vertice.hpp GrafoEtiquetado.hpp
Grafo.o: Grafo.cpp Grafo.hpp NodoAdy.hpp NodoVertice.hpp Vertice.hpp

clean:
	-$(BORRAR) *.o grafo$(EXE) pruebas$(EXE)

# Pruebas de los metodos agregados en la ampliacion (compara con fuerza bruta)
pruebas$(EXE): pruebas_extension.o Grafo.o
	$(CXX) $(CXXFLAGS) -o pruebas$(EXE) pruebas_extension.o Grafo.o

pruebas_extension.o: pruebas_extension.cpp Grafo.hpp Vertice.hpp GrafoEtiquetado.hpp
