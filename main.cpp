#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <tuple>
#include <queue>
#include <unordered_map>
#include <algorithm>

using namespace std;

// ---- vvvvv FUNCIONES I/O vvvvv ----
string LEER_texto_normal(string nombre_archivo) {
    ifstream archivo(nombre_archivo);

    // Variable donde se guarda y retorna el texto completo
    string texto = "";

    if(archivo.is_open()) {
        string _temp; // Variable que lee cada linea del .txt
        while(getline(archivo, _temp)) {
            texto += _temp + '`';
        }

        archivo.close();
    } else {
        cout << "~~ERROR ABRIENDO " << nombre_archivo << "\n";
    }
    replace(texto.begin(), texto.end(), ' ', '~'); // Reemplaza todo los espacios con ~
    return texto;
}

void GUARDAR_texto_comprimido(string nombre_archivo, string texto_comp, unordered_map<char, string> codigos, vector<tuple<char,int>> frec) {
    ofstream archivo(nombre_archivo);
    if(archivo.is_open()) {
        archivo << frec.size() << "\n";
        for(auto& i : frec) {
            archivo << get<0>(i) << " " << codigos[get<0>(i)] << "\n";
        }
        archivo << texto_comp;
        archivo.close();
    } else {
        cout << "~~ERROR ABRIENDO " << nombre_archivo << "\n";
    }
}

void LEER_texto_comprimido(string nombre_archivo, unordered_map<string, char>& codigos, string& texto_compreso) {
    int num_de_caracteres = 0;
    char caracter;
    string codigo = "";
    ifstream archivo(nombre_archivo);
    if(archivo.is_open()) {
        archivo >> num_de_caracteres;
        for(int i=0; i<num_de_caracteres; i++) {
            archivo >> caracter >> codigo;
            codigos[codigo] = caracter;
        }
        archivo >> texto_compreso;
    } else {
        cout << "~~ERROR ABRIENDO " << nombre_archivo << "\n";
    }
}

void GUARDAR_texto_descomprimido(string nombre_archivo, string texto) {
    ofstream archivo(nombre_archivo);
    if(archivo.is_open()) {
        archivo << texto;
    } else {
        cout << "~~ERROR ABRIENDO " << nombre_archivo << "\n";
    }
}

// ---- vvvvv FUNCIONES PARA COMPRESION vvvvv ----
// Funcion para contador la frecuencia de cada caracter en un texto
void contar_frecuencia(string text, vector<tuple<char,int>> &vec) {
    bool found = false;
    for (auto& i : text) {
        for(int j=0; j<vec.size(); ++j) {
            if(i == get<0>(vec[j])) {
                get<1>(vec[j])++;
                found = true;
                break;
            }
        }
        if(!found) {
            vec.push_back(make_tuple(i,1));
        }
        found = false;
    }
}

// Nodo para crear abroles Huffman
struct Nodo {
    char valor;
    int frec;
    Nodo *izq, *der;

    Nodo(char valor, int frecuencia) {
        this->valor = valor;
        this->frec = frecuencia;
        this->izq = this->der = nullptr;
    }
};

// Comparacion para organizar los nodos
struct compare {
    bool operator()(Nodo* a, Nodo* b) {
        return a->frec > b->frec;
    }
};

// Function to generate Huffman encoding tree
Nodo* crearArbolHuffman(vector<tuple<char, int>>& lista_freq) {
    priority_queue<Nodo*, vector<Nodo*>, compare> pq; // priority queue

    // Crea un nodo de arbol huffman para cada caracter y frecuencia guardado dentro el vector
    for (auto& i : lista_freq) {
        pq.push(new Nodo(get<0>(i), get<1>(i)));
    }

    // Crea arboles de huffman hasta que solo quede un unico arbol en el priority queue
    while (pq.size() > 1) {
        // Agrega el primer elemento en el priority queue a la rama izquierda
        Nodo* izquierda = pq.top();
        pq.pop();

        // Agregra el primer elemento en el pq (tecnicamente segundo en la lista) al rama derecha
        Nodo* derecha = pq.top();
        pq.pop();

        // Crea un nuevo nodo padre para las ramas creadas
        // Se usara el caracter nulo (\0) para llenar el atributo ->valor de nodos padre de las hojas
        Nodo* parent = new Nodo('\0', izquierda->frec + derecha->frec);
        parent->izq = izquierda;
        parent->der = derecha;

        // Agrega el nuevo arbol huffman al priority queue
        pq.push(parent);
    }

    // Una vez terminada el proceso de crear el arbol huffman retorna la raiz del arbol
    return pq.top();
}

// Funcion recursiva para crear los codigos Huffman usando el arbol de Huffman
void generarCodigosHuffman(Nodo* raiz, string codigo, unordered_map<char, string>& codigosHuffman) {
    if (raiz == nullptr) {
        return;
    }

    /* Si el valor del nodo NO es el caracter nulo, se registra el codigo (como string)
    en la tabla hash que guarda todos los codigos Huffman con su respectivo caracter */
    if (raiz->valor != '\0') {
        codigosHuffman[raiz->valor] = codigo;
    }

    /*Cuando el algoritmo entra a un nodo hijo por la izquierda agrega un '0' al
    string que guarda el codigo y agrega un '1' cada que entra por la derecha*/ 
    generarCodigosHuffman(raiz->izq, codigo + "0", codigosHuffman);
    generarCodigosHuffman(raiz->der, codigo + "1", codigosHuffman);
}

// ---- vvvvv FUNCIONES PARA DESCOMPRESION vvvvv ----

string decodificar(string texto_compreso, unordered_map<string, char> caracteres) {
    string texto = "";
    string codigo = "";
    for(auto& i : texto_compreso) {
        codigo += i;
        auto llave = caracteres.find(codigo);
        if(llave != caracteres.end()) {
            if(caracteres[codigo] == '~') {
                texto += " ";
            } else if(caracteres[codigo] == '`') {
                texto += "\n";
            } else {
                texto += caracteres[codigo];
            }
            codigo = "";
        }
    }
    return texto;
}

// ---- vvvvv FUNCIONES PRIMARIAS vvvvv ----
void comprimirTexto(string nombre_archivo) {
    string texto = LEER_texto_normal(nombre_archivo); // String donde se guarda todo el texto de una archivo .txt
    vector<tuple<char,int>> frecuencias; // Vector donde se guardara las frecuencias
    contar_frecuencia(texto, frecuencias); // Contabiliza frecuencias y guarda dentro el vector
    unordered_map<char, string> codigosHuffman; // Tabla hash donde se guardaran los codigos de cada caracter
    Nodo* raiz = crearArbolHuffman(frecuencias); // Nodo raiz del arbol Huffman
    generarCodigosHuffman(raiz, "", codigosHuffman); // Genera los codigos Huffman a partir del arbil Huffman

    string texto_compreso = "";
    for(auto& i : texto) { // Convierte cada caracter del texto a su codigo Huffman
        texto_compreso += codigosHuffman[i];
    }

    GUARDAR_texto_comprimido("compresion.txt", texto_compreso, codigosHuffman, frecuencias);
}

void descomprimirTexto(string nombre_archivo) {
    unordered_map<string, char> caracteres;
    string texto_compreso = "";
    LEER_texto_comprimido(nombre_archivo, caracteres, texto_compreso);
    string texto_descomprimido = decodificar(texto_compreso, caracteres);
    GUARDAR_texto_descomprimido("output.txt", texto_descomprimido);
}

int main() {
    string input = "";
    cout << "Para que el programa pueda ejecutar bien debe haber un archivo llamado \"input.txt\"\n";
    cout << "Escriba una de los opciones:\n";
    cout << "[comp] = Comprimir texto\n";
    cout << "[desc] = Descomprimir texto\n";
    cout << "[salir] = Salir del programa\n";
    while(input != "salir") {
        cin >> input;
        if(input == "comp") {
            comprimirTexto("input.txt");
            cout << "~Se comprimio el texto!\n";
        } else if(input == "desc") {
            descomprimirTexto("compresion.txt");
            cout << "~Se descomprimio el texto!\n";
        } else if(input == "salir") break;
    }    
    cout << "\n>END\n";
    return 0;
}
