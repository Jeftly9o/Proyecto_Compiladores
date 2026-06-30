#include <iostream>
#include <string>
#include <vector>
#include <cctype>
using namespace std;

// =========================================================================
// 1. NUEVO STRUCT: Guarda la cadena dividida en tokens mediante punteros
// =========================================================================
struct NodoToken {
    string valor;
    NodoToken* sig; // Puntero al siguiente elemento de la cadena
    NodoToken* ant; // Puntero al elemento anterior

    NodoToken(string v) : valor(v), sig(nullptr), ant(nullptr) {}
};

// =========================================================================
// 2. STRUCT ORIGINAL: Guarda el historial de derivaciones
// =========================================================================
struct NodoDerivacion {
    string derivacion;
    string estado_actual;
    NodoDerivacion* siguiente;

    NodoDerivacion(string d, string e) : derivacion(d), estado_actual(e), siguiente(nullptr) {}
};

// Función auxiliar para agregar un token a nuestra nueva lista doblemente enlazada
void agregarToken(NodoToken*& cabeza, NodoToken*& cola, string valor) {
    NodoToken* nuevo = new NodoToken(valor);
    if (!cabeza) {
        cabeza = nuevo;
        cola = nuevo;
    } else {
        cola->sig = nuevo;
        nuevo->ant = cola;
        cola = nuevo;
    }
}

// Analizador léxico que ahora devuelve un puntero a la cabeza de la lista de tokens
NodoToken* tokenizarLista(string expr) {
    NodoToken* cabeza = nullptr;
    NodoToken* cola = nullptr;
    
    string actual = "";
    bool esperar_unario = true;

    for (size_t i = 0; i < expr.length(); i++) {
        char c = expr[i];
        if (isspace(c)) continue;

        if (c == '=' || c == '*' || c == '/') {
            if (!actual.empty()) { agregarToken(cabeza, cola, actual); actual = ""; }
            agregarToken(cabeza, cola, string(1, c));
            esperar_unario = true;
        } else if (c == '+' || c == '-') {
            if (esperar_unario) {
                actual += c;
                esperar_unario = false; 
            } else {
                if (!actual.empty()) { agregarToken(cabeza, cola, actual); actual = ""; }
                agregarToken(cabeza, cola, string(1, c));
                esperar_unario = true;
            }
        } else if (isdigit(c) || c == '.' || isalpha(c)) {
            actual += c;
            esperar_unario = false;
        }
    }
    if (!actual.empty()) agregarToken(cabeza, cola, actual);
    
    return cabeza;
}

// Función auxiliar para obtener la cadena completa leyendo los punteros
string obtenerCadenaActual(NodoToken* cabeza) {
    string resultado = "";
    NodoToken* temp = cabeza;
    while (temp != nullptr) {
        resultado += temp->valor;
        temp = temp->sig;
    }
    return resultado;
}

// Motor de derivación que manipula los punteros de la lista de tokens
NodoDerivacion* generarDerivaciones(NodoToken* cabeza_tokens) {
    int num_t = 1;
    NodoDerivacion* cabeza_deriv = nullptr;
    NodoDerivacion* actual_deriv = nullptr;

    // Mientras la lista de tokens tenga más de 1 elemento (no sea solo "t4")
    while (cabeza_tokens != nullptr && cabeza_tokens->sig != nullptr) {
        NodoToken* op_nodo = nullptr;
        NodoToken* temp = cabeza_tokens;

        // Búsqueda Prioridad 1: * o /
        while (temp != nullptr) {
            if (temp->valor == "*" || temp->valor == "/") { op_nodo = temp; break; }
            temp = temp->sig;
        }
        
        // Búsqueda Prioridad 2: + o -
        if (!op_nodo) {
            temp = cabeza_tokens;
            while (temp != nullptr) {
                // Buscamos + o - aislados, no signos pegados a números
                if (temp->valor == "+" || temp->valor == "-") { op_nodo = temp; break; }
                temp = temp->sig;
            }
        }
        
        // Búsqueda Prioridad 3: = (Recordando que siempre empieza con "x=")
        if (!op_nodo) {
            temp = cabeza_tokens;
            while (temp != nullptr) {
                if (temp->valor == "=") { op_nodo = temp; break; }
                temp = temp->sig;
            }
        }

        // Si por alguna razón no hay operador, terminamos
        if (!op_nodo) break;

        // Extraer los valores usando los punteros vecinos
        NodoToken* nodo_izq = op_nodo->ant;
        NodoToken* nodo_der = op_nodo->sig;

        string izq = nodo_izq->valor;
        string op = op_nodo->valor;
        string der = nodo_der->valor;

        string t_name = "t" + to_string(num_t++);
        string paso_derivacion = t_name + "=>" + izq + op + der;

        // Crear el nuevo nodo temporal 'tX'
        NodoToken* nuevo_t = new NodoToken(t_name);

        // ==========================================================
        // MAGIA DE PUNTEROS: Reemplazamos los 3 nodos por 1 solo
        // ==========================================================
        nuevo_t->ant = nodo_izq->ant;
        nuevo_t->sig = nodo_der->sig;

        // Conectar el nodo anterior (si existe, ej: "x", "=") al nuevo_t
        if (nodo_izq->ant != nullptr) {
            nodo_izq->ant->sig = nuevo_t;
        } else {
            // Si no hay nodo anterior, el nuevo_t se vuelve la cabeza de la lista
            cabeza_tokens = nuevo_t;
        }

        // Conectar el nodo siguiente (si existe) al nuevo_t
        if (nodo_der->sig != nullptr) {
            nodo_der->sig->ant = nuevo_t;
        }

        // Liberar la memoria de los 3 nodos que ya no usamos
        delete nodo_izq;
        delete op_nodo;
        delete nodo_der;

        // Leer el estado actual de la cadena recorriendo la nueva estructura de punteros
        string estado = obtenerCadenaActual(cabeza_tokens);

        // Guardar el paso en la lista de derivaciones
        NodoDerivacion* nueva_deriv = new NodoDerivacion(paso_derivacion, estado);
        if (!cabeza_deriv) {
            cabeza_deriv = nueva_deriv;
            actual_deriv = nueva_deriv;
        } else {
            actual_deriv->siguiente = nueva_deriv;
            actual_deriv = nueva_deriv;
        }
    }
    
    // Liberar el último nodo token que queda ("t4")
    delete cabeza_tokens;

    return cabeza_deriv;
}

void imprimirYLiberarResultados(NodoDerivacion* cabeza) {
    NodoDerivacion* actual = cabeza;
    int paso = 1;
    
    while (actual != nullptr) {
        cout << paso << " derivacion\n";
        cout << actual->derivacion << "\n";
        cout << actual->estado_actual << "\n\n";
        
        NodoDerivacion* temporal = actual;
        actual = actual->siguiente;
        delete temporal;
        paso++;
    }
}

int main() {
    string cadena;
    
    cout << "Ingrese la operacion a derivar" << endl;
    cout << "> ";
    getline(cin, cadena);
    
    // Validar que no esté vacía
    if(cadena.empty()) {
        cout << "Error: No se ingreso ninguna cadena." << endl;
        return 1;
    }    
    cout << "\n--- Iniciando Analisis ---\n";
    cout << "Cadena original: " << cadena << "\n\n";
    NodoToken* lista_tokens = tokenizarLista(cadena);
    NodoDerivacion* resultados = generarDerivaciones(lista_tokens);
    imprimirYLiberarResultados(resultados);

    return 0;
}