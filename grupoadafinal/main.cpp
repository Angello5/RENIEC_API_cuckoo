#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#include "UserData.h"
#include "CuckooHashTable.h"
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/version.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <regex>
#include <string>
#include <unordered_set>
#include <random>
#include <thread>
#include <mutex>
#include <atomic>

using namespace std;

const uint32_t DNI_MIN = 10000000;
const uint32_t DNI_MAX = 99999999;
bool data_modificada = false;
const size_t NUM_THREADS = thread::hardware_concurrency();

unordered_set<uint32_t> dni_generados;
mutex dni_mutex;
atomic<size_t> progreso_global = 0;
mutex consola_mutex;
mutex hash_mutex;

// Cambia la ruta del archivo según sea necesario
const string FILE_PATH = "/Users/angellollerena/Downloads/grupoadafinal/grupoadafinal/user.bin";


const vector<string> names = {
    "Juan", "María", "Pedro", "Ana", "Luis"};

const vector<string> surnames = {
    "García", "Martínez", "Rodríguez", "López"};

struct NationalityBirthplace {
    string nationality;
    string birthplace;
};

const vector<NationalityBirthplace> natPlaces = {
    {"Peruana", "Lima"},
    {"Chilena", "Santiago"},
    {"Argentina", "Buenos Aires"},
    {"Colombiana", "Bogotá"},
    {"Mexicana", "Ciudad de México"},
};

struct AddressData {
    string departamento;
    string provincia;
    string ciudad;
    string distrito;
};

const vector<AddressData> addresses = {
    {"Lima", "Lima", "Lima", "Miraflores"},
    {"Cusco", "Cusco", "Cusco", "San Blas"},
    {"Arequipa", "Arequipa", "Arequipa", "Yanahuara"}
};

const vector<string> marital_statuses = {
    "Soltero(a)", "Casado(a)", "Viudo(a)", "Divorciado(a)"};

// Función para generar un DNI aleatorio
uint32_t generarDni() {
    static mt19937 generador(random_device{}());
    static uniform_int_distribution<uint32_t> distribucion(DNI_MIN, DNI_MAX);

        uint32_t nuevo_dni;
        do {
            nuevo_dni = distribucion(generador); // Generar un DNI aleatorio
            lock_guard<mutex> lock(dni_mutex);
            if (dni_generados.find(nuevo_dni) == dni_generados.end()) {
                dni_generados.insert(nuevo_dni);
                return nuevo_dni;
            }
        } while (true);
}


// Generador de números de Telefono
string generarPhone() {
    // El primer dígito siempre es '9'
    string phone = "9";
    for (int i = 0; i < 8; ++i) {
        // Se añaden los siguientes 8 dígitos
        phone += to_string(rand() % 10);
    }
    return phone;
}
// Función para eliminar espacios en blanco de una cadena
string removeSpaces(const string& str) {
    string result;
    for (char c : str) {
        if (!isspace(c)) {
            result += c;
        }
    }
    return result;
}

// Función para convertir una cadena a minúsculas
string toLowerCase(const string& str) {
    string result;
    for (char c : str) {
        result += tolower(c);
    }
    return result;
}
// Generador de números de emails
string generarEmail(const string& name) {
    vector<string> dominios = { "gmail.com", "yahoo.com", "outlook.com"};
    string dominio = dominios[rand() % dominios.size()];
    string cleanName = removeSpaces(name);
    string lowerCaseName = toLowerCase(cleanName);
    return lowerCaseName + "@" + dominio;
}


// Generador de números del Estado Civil
string generarMarital_status() {
    vector<string> marital_status = { "Soltero(a)", "Casado(a)", "Viudo(a)", "Divorciado(a)" };
    return marital_status[rand() % marital_status.size()];
}


// Funcion que permite seleccionar un registro de manera aleatoria

Address selAddressRandom(const vector<string>& direcciones) {
    string address = direcciones[rand() % direcciones.size()];
    Address ad;
    size_t pos = 0;

    pos = address.find(',');
    ad.departamento = address.substr(0,pos);
    address.erase(0, pos + 1);
    
    pos = address.find(',');
    ad.provincia = address.substr(0,pos);
    address.erase(0, pos + 1);
    
    pos = address.find(',');
    ad.ciudad= address.substr(0, pos);
    address.erase(0, pos + 1);
    
    ad.distrito = address;
    
    return ad;
    
}
pair<string, string> selNatBPRandom(const vector<string>& natPlace) {
    string nl = natPlace[rand() % natPlace.size()];
    size_t pos = nl.find(',');
    string nacionalidad = nl.substr(0, pos);
    string lugarNacimiento = nl.substr(pos + 1);
    return { nacionalidad, lugarNacimiento };
}

// Función para generar una persona o ciudadano de manera aleatoria
Person generarPersona() {
    Person persona;
    persona.dni = generarDni();
    persona.phone = generarPhone();
    persona.name = names[rand() % names.size()];
    persona.surname = surnames[rand() % surnames.size()];
    persona.email = generarEmail(persona.name);
    persona.marital_status = marital_statuses[rand() % marital_statuses.size()];

    const auto& natPlace = natPlaces[rand() % natPlaces.size()];
    persona.birthplace.nationality = natPlace.nationality;
    persona.birthplace.birthplace = natPlace.birthplace;

    const auto& addr = addresses[rand() % addresses.size()];
    persona.address.departamento = addr.departamento;
    persona.address.provincia = addr.provincia;
    persona.address.ciudad = addr.ciudad;
    persona.address.distrito = addr.distrito;

    return persona;
}

void generarRangoPersonas(size_t start, size_t end, vector<Person>& personas_local) {
    vector<uint32_t> dnis;
    dnis.reserve(end - start);

    // Generar DNIs únicos en el rango [start, end)
    for (size_t i = start; i < end; ++i) {
        dnis.push_back(DNI_MIN + static_cast<uint32_t>(i));  // Conversión explícita para evitar warnings
    }

    // Barajar los DNIs para aleatorizarlos
    random_device rd;
    mt19937 g(rd());
    shuffle(dnis.begin(), dnis.end(), g);

    // Asignar DNIs únicos a las personas generadas
    size_t contador = 0;  // Contador para progreso
    for (uint32_t dni : dnis) {
        Person persona;
        persona.dni = dni;
        persona.name = names[rand() % names.size()];
        persona.surname = surnames[rand() % surnames.size()];

        const auto& addr = addresses[rand() % addresses.size()];
        persona.address.departamento = addr.departamento;
        persona.address.provincia = addr.provincia;
        persona.address.ciudad = addr.ciudad;
        persona.address.distrito = addr.distrito;

        const auto& natPlace = natPlaces[rand() % natPlaces.size()];
        persona.birthplace.nationality = natPlace.nationality;
        persona.birthplace.birthplace = natPlace.birthplace;

        persona.phone = generarPhone();
        persona.email = generarEmail(persona.name);
        persona.marital_status = generarMarital_status();  

        personas_local.push_back(persona);

        // Progreso: imprimir cada millón de personas generadas
        if (++contador % 1000000 == 0) {
            cout << "Progreso local: " << contador << " personas generadas.\n";
        }
    }
}

void saveData(const vector<Person>& personas, const string& filename, bool& data_modificada) {
    if(!data_modificada){
        cout<<"No hay cambios, no se guarda \n";
        return;
    }
    ofstream ofs(filename, ios::binary);
    boost::iostreams::filtering_stream<boost::iostreams::output> fos;
    fos.push(boost::iostreams::zlib_compressor());
    fos.push(ofs);
    boost::archive::binary_oarchive oa(fos);
    oa << personas;
    
    data_modificada = false;
}

void loadData(vector<Person>& personas, const string& filename) {
    ifstream ifs(filename, ios::binary);
    if (!ifs.is_open()) {
            cerr << "Error: No se pudo abrir el archivo " << filename << endl;
            return;
        }

    boost::iostreams::filtering_stream<boost::iostreams::input> fis;
    fis.push(boost::iostreams::zlib_decompressor());
    fis.push(ifs);
    boost::archive::binary_iarchive ia(fis);
    try {
            ia >> personas;
        } catch (const boost::archive::archive_exception& e) {
            cerr << "Error al leer datos del archivo: " << e.what() << endl;
        }

}

void printUser(const Person* persona) {
    if (persona) {
        cout << "DNI: " << persona->dni << "\n"
            << "Nombre: " << persona->name << "\n"
            << "Apellido:: " << persona->surname << "\n"
            << "Nacionalidad: " << persona->birthplace.nationality << "\n"
            << "Lugar de Nacimiento: " << persona->birthplace.birthplace << "\n"
            << "Departamento: " << persona->address.departamento << "\n"
            << "Ciudad: " << persona->address.ciudad << "\n"
            << "Provincia: " << persona->address.provincia << "\n"
            << "Distrito: " << persona->address.distrito << "\n"
            << "Telefono: " << persona->phone << "\n"
            << "Correo: " << persona->email << "\n"
            << "Estado Civil: " << persona->marital_status << "\n";
    }
    else {
        cout << "Usuario no encontrado.\n";
    }
}

void insertUser(CuckooHashTable& hashTable, vector<Person>& personas, bool& data_modificada) {
    Person persona = {};
    
     while (true) {
        cout << "Ingresa DNI (8 dígitos, solo números): ";
        string dni_input;
        cin >> dni_input;

        // Verificar que el DNI tenga exactamente 8 dígitos numéricos
        if (!std::regex_match(dni_input, std::regex("\\d{8}"))) {
            cout << "Error: El DNI debe contener exactamente 8 dígitos numéricos.\n";
        } else {
            // Convertir a número e intentar insertarlo
            persona.dni = std::stoi(dni_input);
            if (hashTable.search(persona.dni)) {
                cout << "Error: El DNI ya está registrado.\n";
            } else {
                break; // Salir del bucle si todo está correcto
            }
        }
    }
    cin.ignore(); // Ignorar el carácter de nueva línea residual

    
    // Validación del nombre
    while (true) {
        cout << "Ingresa Nombre: ";
        getline(cin, persona.name);
        if (persona.name.empty()) {
            cout << "Error: El nombre no puede estar vacío.\n";
        } else {
            break;
        }
    }

    // Validación del apellido
    while (true) {
        cout << "Ingresa Apellido: ";
        getline(cin, persona.surname);
        if (persona.surname.empty()) {
            cout << "Error: El apellido no puede estar vacío.\n";
        } else {
            break;
        }
    }

    // Validación de la nacionalidad
    while (true) {
        cout << "Ingresa Nacionalidad: ";
        getline(cin, persona.birthplace.nationality);
        if (persona.birthplace.nationality.empty()) {
            cout << "Error: La nacionalidad no puede estar vacía.\n";
        } else {
            break;
        }
    }

    // Validación del lugar de nacimiento
    while (true) {
        cout << "Ingresa Lugar de nacimiento: ";
        getline(cin, persona.birthplace.birthplace);
        if (persona.birthplace.birthplace.empty()) {
            cout << "Error: El lugar de nacimiento no puede estar vacío.\n";
        } else {
            break;
        }
    }

    // Validación del teléfono
    while (true) {
        cout << "Ingresa Teléfono (9 dígitos, empieza con 9): ";
        getline(cin, persona.phone);
        if (persona.phone.size() != 9 || persona.phone[0] != '9') {
            cout << "Error: El teléfono debe tener 9 dígitos y comenzar con 9.\n";
        } else {
            break;
        }
    }

    // Validación del correo
    while (true) {
        cout << "Ingresa Correo: ";
        getline(cin, persona.email);
        std::regex email_regex(R"((\w+)(\.?[\w]*)@(\w+)\.(\w+))");
        if (!std::regex_match(persona.email, email_regex)) {
            cout << "Error: El correo debe tener un formato válido (ejemplo: nombre@dominio.com).\n";
        } else {
            break;
        }
    }

    // Validación del estado civil
    vector<string> estados_validos = {"Soltero(a)", "Casado(a)", "Viudo(a)", "Divorciado(a)"};
    while (true) {
        cout << "Ingresa Estado Civil (Soltero(a), Casado(a), Viudo(a), Divorciado(a)): ";
        getline(cin, persona.marital_status);
        if (std::find(estados_validos.begin(), estados_validos.end(), persona.marital_status) == estados_validos.end()) {
            cout << "Error: El estado civil debe ser uno de los siguientes: Soltero(a), Casado(a), Viudo(a), Divorciado(a).\n";
        } else {
            break;
        }
    }

    // Validación de la dirección
    while (true) {
        cout << "Ingresa el nombre del departamento: ";
        getline(cin, persona.address.departamento);
        if (persona.address.departamento.empty()) {
            cout << "Error: El departamento no puede estar vacío.\n";
        } else {
            break;
        }
    }

    while (true) {
        cout << "Ingresa el nombre de la ciudad: ";
        getline(cin, persona.address.ciudad);
        if (persona.address.ciudad.empty()) {
            cout << "Error: La ciudad no puede estar vacía.\n";
        } else {
            break;
        }
    }

    while (true) {
        cout << "Ingresa el nombre de la provincia: ";
        getline(cin, persona.address.provincia);
        if (persona.address.provincia.empty()) {
            cout << "Error: La provincia no puede estar vacía.\n";
        } else {
            break;
        }
    }

    while (true) {
        cout << "Ingresa el nombre del distrito: ";
        getline(cin, persona.address.distrito);
        if (persona.address.distrito.empty()) {
            cout << "Error: El distrito no puede estar vacío.\n";
        } else {
            break;
        }
    }

    // Agregar a la tabla hash
    size_t memory_position = personas.size();
    personas.push_back(persona);

    auto start = chrono::high_resolution_clock::now();
    hashTable.insert(persona.dni, memory_position);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;

    cout << "Usuario ingresado correctamente en " << duration.count() << " segundos.\n";
    data_modificada = true;
}


void imprimirPrimerosRegistros(const vector<Person>& personas) {
    auto start = chrono::high_resolution_clock::now();  // Se inicia la medición del tiempo

    cout << "\n=== Primeros 10 Registros ===" << endl;

    // Asegura que no se excedan los límites del vector
    size_t numRegistros = min(personas.size(), static_cast<size_t>(10));

    for (size_t i = 0; i < numRegistros; ++i) {
        const Person& persona = personas[i];
        cout << "DNI: " << persona.dni << endl;
        cout<<  "NOMBRE: " <<persona.name <<endl;
        cout << "Apellido: " << persona.surname << endl;
        cout << "Nacionalidad: " << persona.birthplace.nationality << endl;
        cout << "Lugar de Nacimiento: " << persona.birthplace.birthplace << endl;
        cout << "Departamento: " << persona.address.departamento << endl;
        cout << "Ciudad: " << persona.address.ciudad << endl;
        cout << "Provincia: " << persona.address.provincia << endl;
        cout << "Distrito: " << persona.address.distrito << endl;
        cout << "Teléfono: " << persona.phone << endl;
        cout << "Correo: " << persona.email << endl;
        cout << "Estado Civil: " << persona.marital_status << endl;
        cout << endl;
    }

    // Se finaliza la medición del tiempo
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = end - start;
    cout << "Tiempo en mostrar los 10 primeros registros: " << duration.count() << " segundos\n";
}

bool filexiste(const string& filename){
    ifstream file(filename);
    return file.good();
}

void verificarIntegridad(const CuckooHashTable& hashTable, const vector<Person>& personas) {
    // Verificar en la tabla hash
    const auto& table = hashTable.getTable();
    size_t capacity = hashTable.getCapacity();
    for (size_t pos = 0; pos < capacity; ++pos) {
        const Entry& entry = table[pos];
        if (entry.dni != 0) { // Si hay un elemento válido en la posición
            if (entry.memory_position >= personas.size()) {
                cerr << "Error: memory_position fuera de rango para DNI: " << entry.dni << endl;
                continue;
            }
            if (personas[entry.memory_position].dni != entry.dni) {
                cerr << "Error de integridad: DNI en tabla hash ("
                     << entry.dni << ") no coincide con DNI en personas["
                     << entry.memory_position << "] ("
                     << personas[entry.memory_position].dni << ")." << endl;
            }
        }
    }

    // Verificar en el stash
    const auto& stash = hashTable.getStash();
    for (const auto& entry : stash) {
        if (entry.memory_position >= personas.size()) {
            cerr << "Error: memory_position fuera de rango en stash para DNI: " << entry.dni << endl;
            continue;
        }
        if (personas[entry.memory_position].dni != entry.dni) {
            cerr << "Error de integridad en stash: DNI en tabla hash ("
                 << entry.dni << ") no coincide con DNI en personas["
                 << entry.memory_position << "] ("
                 << personas[entry.memory_position].dni << ")." << endl;
        }
    }
}

void insertarEnTablaHash(CuckooHashTable& hashTable, const vector<Person>& personas, size_t start, size_t end) {
    for (size_t i = 0; i < personas.size(); i++) {
        if (!personas[i].is_deleted) {
            if (!hashTable.insert(personas[i].dni, i)) {
                cerr << "Error al insertar DNI: " << personas[i].dni << endl;
            }
        }
        if (i % 1000000 == 0 && i != 0) {
            cout << "Insertados " << i << " registros en la tabla hash.\n";
        }
    }
}

void mostrarMenu( ){
    cout<<"\nMenu\n"
        <<"1. Insertar Usuario\n"
        <<"2. Buscar Usuario\n"
        <<"3. Eliminar Usuario\n"
        <<"4. Imprimir los primeros 10 registros\n"
        <<"5. Guadar datos\n"
        <<"6. Exit\n"
        <<"Elige tu opcion: ";
}

int main() {
    try{
        srand(static_cast<unsigned int>(time(0)));
        size_t num_personas = 33000000;
        size_t personas_por_hilo = num_personas/NUM_THREADS;
        
        bool dataexiste = filexiste(FILE_PATH);
        bool data_modificada = false;
        
        // Generate data
        vector<Person> personas;
        
        if(dataexiste){
            // Load data from file if exists
            auto start = chrono::high_resolution_clock::now();
            loadData(personas, FILE_PATH); // Usa la ruta definida
            auto end = chrono::high_resolution_clock::now();
            chrono:chrono::duration<double> duration= end - start;
            cout << "Data cargada en " << duration.count() << " segundos.\n";
        }else{
            //paralelizacion
            vector<vector<Person>> resultados(NUM_THREADS);
            vector<thread> hilos;

            auto start = chrono::high_resolution_clock::now();

            for (size_t i = 0; i < NUM_THREADS; ++i) {
                size_t start = i * personas_por_hilo;
                size_t end = (i == NUM_THREADS - 1) ? num_personas : start + personas_por_hilo;
                hilos.emplace_back(generarRangoPersonas, start, end, ref(resultados[i]));
            }

            // Esperar a que todos los hilos terminen
            for (auto& hilo : hilos) {
                hilo.join();
            }

            // Combinar los resultados de todos los hilos
            for (const auto& personas_local : resultados) {
                personas.insert(personas.end(), personas_local.begin(), personas_local.end());
            }

            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double> duration = end - start;
            cout << "Generados y combinados " << personas.size() << " registros en " << duration.count() << " segundos.\n";

    
            start = chrono::high_resolution_clock::now();
            saveData(personas, FILE_PATH, data_modificada); // Usa la ruta definida
            end = chrono::high_resolution_clock::now();
            duration = end - start;
            cout << "Guardados en " << duration.count() << " segundos.\n";
            
            data_modificada = true;
        }
        
        size_t initial_capacity =110000000;
        CuckooHashTable hashTable(initial_capacity, personas);  // Double the size for optimal performance
        
        vector<thread> hilos;
        auto start = chrono::high_resolution_clock::now();

        // Crear hilos para la inserción
        for (size_t i = 0; i < NUM_THREADS; ++i) {
            size_t start_index = i * personas_por_hilo;
            size_t end_index = (i == NUM_THREADS - 1) ? num_personas : start_index + personas_por_hilo;

            hilos.emplace_back([&, start_index, end_index]() {
                for (size_t j = start_index; j < end_index; ++j) {
                    {
                        lock_guard<mutex> lock(hash_mutex);
                        if (!hashTable.insert(personas[j].dni, j)) {
                            cerr << "Error al insertar DNI: " << personas[j].dni << endl;
                        }
                    }

                    // Progreso
                    if (j % 1000000 == 0 && j != start_index) {
                        cout << "Hilo " << i << ": insertados " << j - start_index << " registros." << endl;
                    }
                }
            });
        }

        // Esperar a que todos los hilos terminen
        for (auto& hilo : hilos) {
            hilo.join();
        }

        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end - start;
        cout << "Insertados " << num_personas << " registros en la tabla hash en " << duration.count() << " segundos.\n";

        verificarIntegridad(hashTable, personas);

        int opcion;
        do {
            mostrarMenu();
            cin>>opcion;
            switch (opcion) {
                case 1:{
                    insertUser(hashTable, personas, data_modificada);
                    break;
                }
                case 2: {
                    uint32_t dni;
                    cout << "Ingresa DNI a buscar: ";
                    cin >> dni;
                    
                    auto start = chrono::high_resolution_clock::now();
                    const Person* user_pos = hashTable.search(dni);
                    auto end = chrono::high_resolution_clock::now();
                    chrono::duration<double> duration = end - start;
                    
                    if (user_pos != nullptr) {
                        printUser(user_pos);
                    } else {
                        cout << "Usuario no encontrado.\n";
                    }
                    
                    cout << "Busqueda completa en: " << duration.count() << " segundos.\n";
                    break;
                }

                case 3: {
                    uint32_t dni;
                    cout << "Ingresa DNI a eliminar: ";
                    cin >> dni;
                    
                    auto start = chrono::high_resolution_clock::now();
                    if (hashTable.remove(dni)) {
                        // No es necesario eliminarlo del vector 'personas'
                        cout << "Usuario eliminado correctamente." << endl;
                    } else {
                        cout << "Usuario no encontrado." << endl;
                    }
                    auto end = chrono::high_resolution_clock::now();
                    chrono::duration<double> duration = end - start;
                    cout << "Eliminacion completa en " << duration.count() << " segundos.\n";
                    data_modificada = true;
                    break;
                }

                case 4:
                    imprimirPrimerosRegistros(personas);
                    break;
                case 5:
                    if (data_modificada) {
                        saveData(personas, FILE_PATH, data_modificada);
                        cout << "Datos guardados exitosamente.\n";
                        data_modificada = false; // Reiniciar indicador de cambios
                    } else {
                        cout << "No se detectaron cambios. No es necesario guardar.\n";
                    }
                    break;
                case 6:
                    cout<<"Salida... \n"<<endl;
                    saveData(personas, FILE_PATH, data_modificada);
                    break;
                default:
                    cout<<"Eleccion invalida, por favor intente de nuevo. \n";
                    break;
            }
        } while (opcion != 6);
        
        //saveData(personas, FILE_PATH);
 
    }catch (const bad_alloc& e){
        cerr <<"Error de asignacion de memoria " << e.what() << endl;
        return EXIT_FAILURE;
    }catch (const exception& e){
        cerr<< "Excepcion capturada en: "<<e.what()<<endl;
        return EXIT_FAILURE;
    }
    return 0;
}
#pragma clang diagnostic pop
