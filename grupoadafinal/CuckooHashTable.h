#include <iostream>
#include "UserData.h"
#include <vector>
#include <functional>
#include <stdexcept>
#include <boost/functional/hash.hpp>

using namespace std;

class CuckooHashTable {
    vector<Entry> table;
    size_t capacity;
    vector<function<size_t(uint32_t)>> hash_funcs;
    vector<Person>& personas;
    vector<Entry> stash; // Stash adicional para colisiones excepcionales
    size_t max_displacements = 500; // Máximo de desplazamientos para evitar ciclos

public:
    const vector<Entry>& getTable() const{
        return table;
    }
    
    size_t getCapacity() const{
        return capacity;
    }
    
    const vector<Entry>& getStash() const{
        return stash;
    }
    CuckooHashTable(size_t size, vector<Person>& personas)
    : capacity(size), table(size), personas(personas) {
        // Usar funciones hash de alta calidad
        hash_funcs.emplace_back(boost::hash<uint32_t>()); // Hash 1
        hash_funcs.emplace_back([=](uint32_t key) {
            return boost::hash<uint32_t>()(key * 2654435761u);
        }); // Hash 2
        // Puedes añadir más funciones hash si lo deseas
    }

    bool insert(uint32_t dni, size_t memory_position) {
        // Verificar límites de memory_position
        if (memory_position >= personas.size()) {
            cerr << "Error: memory_position fuera de rango. Posición: " << memory_position << endl;
            return false;
        }

        Entry current(dni, memory_position);
        size_t count = 0;

        while (count < max_displacements) {
            for (auto& hash_func : hash_funcs) {
                size_t pos = hash_func(current.dni) % capacity;

                // Verificar que pos esté dentro de los límites
                if (pos >= table.size()) {
                    cerr << "Error: posición fuera de los límites de la tabla. Posición: " << pos << endl;
                    return false;
                }

                // Si la posición está vacía, insertar
                if (table[pos].dni == 0) {
                    table[pos] = current;

                    // Verificar integridad después de la inserción
                    if (personas[memory_position].dni != dni) {
                        cerr << "Error: mismatch durante inserción. DNI: " << dni
                             << ", personas[" << memory_position << "].dni: " << personas[memory_position].dni << endl;
                    }
                    return true;
                }

                // Realizar el swap con el elemento existente
                swap(current, table[pos]);

                // Verificar integridad después del swap
                if (current.memory_position >= personas.size()) {
                    cerr << "Error: memory_position fuera de rango después del swap. Posición: " << current.memory_position << endl;
                    return false;
                }

                if (personas[current.memory_position].dni != current.dni) {
                    cerr << "Error: mismatch después del swap. DNI: " << current.dni
                         << ", personas[" << current.memory_position << "].dni: " << personas[current.memory_position].dni << endl;
                }

                count++;
                if (count >= max_displacements) break;
            }
        }

        // Añadir al stash si se alcanzan los desplazamientos máximos
        if (current.memory_position >= personas.size()) {
            cerr << "Error: memory_position fuera de rango al añadir al stash. Posición: " << current.memory_position << endl;
            return false;
        }

        stash.push_back(current);

        // Verificar integridad al añadir al stash
        if (personas[current.memory_position].dni != current.dni) {
            cerr << "Error: mismatch al añadir al stash. DNI: " << current.dni
                 << ", personas[" << current.memory_position << "].dni: " << personas[current.memory_position].dni << endl;
        }
        return true;
    }

    const Person* search(uint32_t dni) const {
        for(auto& hash_func : hash_funcs){
            size_t pos = hash_func(dni) % capacity;
            if (table[pos].dni == dni && !personas[table[pos].memory_position].is_deleted) {
                return &personas[table[pos].memory_position];
            }
        }
        // Buscar en el stash
        for (const auto& entry : stash) {
            if (entry.dni == dni && !personas[entry.memory_position].is_deleted) {
                return &personas[entry.memory_position];
            }
        }
        return nullptr;
    }

    bool remove(uint32_t dni) {
        for(auto& hash_func : hash_funcs){
            size_t pos = hash_func(dni) % capacity;
            if (table[pos].dni == dni) {
                personas[table[pos].memory_position].is_deleted = true;
                table[pos] = Entry();
                return true;
            }
        }
        // Buscar en el stash
        for (auto it = stash.begin(); it != stash.end(); ++it) {
            if (it->dni == dni) {
                personas[it->memory_position].is_deleted = true;
                stash.erase(it);
                return true;
            }
        }
        return false;
    }

private:
    bool rehash() {
        try {
            size_t new_capacity = capacity * 2;
            vector<Entry> old_table = table;
            vector<Entry> old_stash = stash;
            table = vector<Entry>(new_capacity);
            stash.clear();
            capacity = new_capacity;

            // Reinsertar todas las entradas de la tabla antigua
            for (const auto& entry : old_table) {
                if (entry.dni != 0) {
                    bool inserted = insert(entry.dni, entry.memory_position);
                    if (!inserted) {
                        cout << "Rehash failed during reinsertion for DNI: " << entry.dni << endl;
                        return false;
                    }
                }
            }

            // Reinsertar todas las entradas del stash antiguo
            for (const auto& entry : old_stash) {
                bool inserted = insert(entry.dni, entry.memory_position);
                if (!inserted) {
                    cout << "Rehash failed during reinsertion from stash for DNI: " << entry.dni << endl;
                    return false;
                }
            }

            return true;
        } catch (const exception& e) {
            cout << "Rehash fallido: " << e.what() << endl;
            return false;
        }
    }
};
