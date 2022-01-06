#include "abb.h"
#include "pila.h"

#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct nodo {
    char* clave;
    void* dato;
    struct nodo* izq;
    struct nodo* der;
} nodo_t;

typedef struct abb {
    nodo_t* raiz;
    size_t cantidad;
    abb_comparar_clave_t funcion_de_comparacion;
    abb_destruir_dato_t funcion_de_destruccion;
} abb_t;

typedef int (*abb_comparar_clave_t) (const char *, const char *);
typedef void (*abb_destruir_dato_t) (void *);

nodo_t *crear_nodo(const char *clave, void *dato) {
    nodo_t *nodo = malloc(sizeof(nodo_t));
    if (!nodo) {
        return NULL;
    }
    nodo->clave = strdup(clave);
    nodo->dato = dato;
    nodo->izq = NULL;
    nodo->der = NULL;
    return nodo;   
}

abb_t* abb_crear(abb_comparar_clave_t cmp, abb_destruir_dato_t destruir_dato) {
    abb_t *arbol = malloc(sizeof(abb_t));
    if (arbol == NULL) {
        return NULL;
    }
    if (cmp != NULL) {
        arbol->funcion_de_comparacion = cmp;
    } else {
        arbol->funcion_de_comparacion = NULL;    
    }
    if (destruir_dato != NULL) {
        arbol->funcion_de_destruccion = destruir_dato;
    } else {
        arbol->funcion_de_destruccion = NULL;    
    }
    arbol->raiz = NULL;
    arbol->cantidad = 0;
    return arbol;
}

nodo_t *buscar(const abb_t *arbol, nodo_t *nodo, const char *clave) {
    if (!nodo) {
        return NULL;
    } else if (arbol->funcion_de_comparacion(clave, nodo->clave) > 0) {
        return buscar(arbol, nodo->der, clave);
    } else if (arbol->funcion_de_comparacion(clave, nodo->clave) < 0) {
        return buscar(arbol, nodo->izq, clave);
    }
    return nodo;
}

bool buscar_lugar(const abb_t *arbol, nodo_t *nodo, const char *clave, nodo_t **nodo_aux) {
    *nodo_aux = nodo;
    if (arbol->funcion_de_comparacion(clave, nodo->clave) > 0) {
        if (!nodo->der) {
            return true;
        }
        return buscar_lugar(arbol, nodo->der, clave, nodo_aux);
    }
    if (!nodo->izq) {
        return false;
    }
    return buscar_lugar(arbol, nodo->izq, clave, nodo_aux);
}

bool abb_guardar(abb_t *arbol, const char *clave, void *dato) {
    nodo_t *nodo;
    if (!arbol->raiz) {
        nodo = crear_nodo(clave, dato);
        if (!nodo) {
            return false;
        }
        arbol->raiz = nodo;
        arbol->cantidad++;
        return true;
    }
    
    nodo = buscar(arbol, arbol->raiz, clave);
    if (nodo != NULL) {
        if (arbol->funcion_de_destruccion != NULL) {
            arbol->funcion_de_destruccion(nodo->dato);
        }
        nodo->dato = dato;
        return true;
    }
    
    nodo = crear_nodo(clave, dato);
    if (!nodo) {
        return false;
    }
    nodo_t *nodo_ant = arbol->raiz;
    if (buscar_lugar(arbol, arbol->raiz, clave, &nodo_ant)) {
        nodo_ant->der = nodo;
    } else {
        nodo_ant->izq = nodo;
    }
    arbol->cantidad++;
    return true;
}
    
bool buscar_padre(const abb_t *arbol, nodo_t *nodo, const char *clave, nodo_t **padre) {
    *padre = nodo;
    if (arbol->funcion_de_comparacion(clave, nodo->clave) > 0) {
        if (arbol->funcion_de_comparacion(clave, nodo->der->clave) == 0) {
            return true;
        }
        return buscar_padre(arbol, nodo->der, clave, padre);
    }
    if (arbol->funcion_de_comparacion(clave, nodo->izq->clave) == 0) {
        return false;
    }
    return buscar_padre(arbol, nodo->izq, clave, padre);
}

nodo_t *buscar_remplazo(nodo_t *nodo) {
    nodo_t *remplazo = nodo->izq;
    while (remplazo->der != NULL) {
        remplazo = remplazo->der;
    }
    return remplazo;
}

void *abb_borrar(abb_t *arbol, const char *clave) {
    nodo_t *nodo = buscar(arbol, arbol->raiz, clave);
    if (!nodo) {
        return NULL;
    }
    void *dato = nodo->dato;
    nodo_t *padre = arbol->raiz;
    
    // borrado no tiene hijos //
    if (!nodo->der && !nodo->izq) {
        if (arbol->raiz == nodo) {
            arbol->raiz = NULL;
        } else if (buscar_padre(arbol, arbol->raiz, clave, &padre)) {
            padre->der = NULL;
        } else {
            padre->izq = NULL;
        }
        arbol->cantidad--;
        free(nodo->clave);
        free(nodo);
    // borrado tiene 1 hijo // 
    } else if ((!nodo->der && nodo->izq != NULL) || (nodo->der != NULL && !nodo->izq)) {
        if (arbol->raiz == nodo) {
            if (!nodo->izq) {
                arbol->raiz = nodo->der;
            } else {
                arbol->raiz = nodo->izq;
            }
        } else if (buscar_padre(arbol, arbol->raiz, clave, &padre)) {
            if (!nodo->izq) {
                padre->der = nodo->der;
            } else {
                padre->der = nodo->izq;
            }
        } else {
            if (!nodo->izq) {
                padre->izq = nodo->der;
            } else {
                padre->izq = nodo->izq;
            }
        }
        arbol->cantidad--;
        free(nodo->clave);
        free(nodo);
    // borrado tiene 2 hijos //
    } else {
        nodo_t *remplazo = buscar_remplazo(nodo);
        char* clave_aux = strdup(remplazo->clave);
        void* dato_aux = abb_borrar(arbol, remplazo->clave);
        free(nodo->clave);
        nodo->dato = dato_aux;
        nodo->clave = clave_aux;
    }
    return dato;
}

void *abb_obtener(const abb_t *arbol, const char *clave) {
    nodo_t *nodo = buscar(arbol, arbol->raiz, clave);
    if (!nodo) {
        return NULL;
    }
    return nodo->dato;
} 

bool abb_pertenece(const abb_t *arbol, const char *clave) {
    return buscar(arbol, arbol->raiz, clave) != NULL;
}

size_t abb_cantidad(abb_t *arbol) {
    return arbol->cantidad;
}

void nodos_destruir(abb_t *arbol, nodo_t *nodo) {
    if (nodo->izq != NULL) {
        nodos_destruir(arbol, nodo->izq);
    }
    if (nodo->der != NULL) {
        nodos_destruir(arbol, nodo->der);
    }
    if (arbol->funcion_de_destruccion != NULL) {
        arbol->funcion_de_destruccion(nodo->dato);
    }
    free(nodo->clave);
    free(nodo);
}

void abb_destruir(abb_t *arbol) {
    if (arbol->raiz != NULL) {
        nodos_destruir(arbol, arbol->raiz);    
    }
    free(arbol);   
}

//------------------------------------------------------------

void nodos_in_order(nodo_t *nodo, bool visitar(const char *, void *, void *), void *extra, bool *continuar) {
    if (nodo->izq != NULL) {
        nodos_in_order(nodo->izq, visitar, extra, continuar);
    }
    if (!*continuar) {
        return;
    }
    *continuar = visitar(nodo->clave, nodo->dato, extra);
    if (*continuar && nodo->der != NULL) {
        nodos_in_order(nodo->der, visitar, extra, continuar);
    }
}

void abb_in_order(abb_t *arbol, bool visitar(const char *, void *, void *), void *extra) {
    if (arbol->raiz != NULL) {
        bool continuar = true;
        nodos_in_order(arbol->raiz, visitar, extra, &continuar);    
    }
}

//------------------------------------------------------------

typedef pila_t pilanodos_t;

pilanodos_t *pilanodos_crear(void) {
    return pila_crear();
}

void apilar_nodo(pilanodos_t *pila, nodo_t *nodo) {
    pila_apilar(pila, nodo);
}

bool desapilar_nodo(pilanodos_t *pila) {
    if (pila_esta_vacia(pila)) {
        return false;
    }
    pila_desapilar(pila);
    return true;
}

bool pilanodo_esta_vacia(pilanodos_t *pila) {
    return pila_esta_vacia(pila);
}

void pilanodo_destruir(pilanodos_t *pila) {
    pila_destruir(pila);
}

nodo_t *pilanodo_ver_tope(const pilanodos_t *pila) {
    nodo_t *nodo = (nodo_t *)pila_ver_tope(pila);
    return nodo;
}

typedef struct abb_iter {
    pilanodos_t *pila;   
} abb_iter_t;

abb_iter_t *abb_iter_in_crear(const abb_t *arbol) {
    abb_iter_t *iter = malloc(sizeof(abb_iter_t));
    if (!iter) {
        return NULL;
    }
    iter->pila = pilanodos_crear();
    if (arbol->raiz != NULL) {
        nodo_t *nodo = arbol->raiz;
        apilar_nodo(iter->pila, nodo);
        while (nodo->izq != NULL) {
            nodo = nodo->izq;
            apilar_nodo(iter->pila, nodo);
        }
    }
    return iter;
}

bool abb_iter_in_avanzar(abb_iter_t *iter) {
    if (abb_iter_in_al_final(iter)) {
        return false;
    }
    nodo_t *nodo = pilanodo_ver_tope(iter->pila)->der;
    desapilar_nodo(iter->pila);
    if (nodo != NULL) {
        apilar_nodo(iter->pila, nodo);
        while (nodo->izq != NULL) {
            nodo = nodo->izq;
            apilar_nodo(iter->pila, nodo);
        }
    }
    return true;
}

const char *abb_iter_in_ver_actual(const abb_iter_t *iter) {
    nodo_t *nodo = pilanodo_ver_tope(iter->pila);
    if (nodo != NULL) {
        return nodo->clave;
    }
    return NULL;
}

bool abb_iter_in_al_final(const abb_iter_t *iter) {
    return pilanodo_esta_vacia(iter->pila);
}

void abb_iter_in_destruir(abb_iter_t* iter) {
    pilanodo_destruir(iter->pila);
    free(iter);
}


