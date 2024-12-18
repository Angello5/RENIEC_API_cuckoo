#ifndef USERDATA_H
#define USERDATA_H

#include <boost/serialization/array.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/string.hpp>
#include <cstdint>
#include <string>
#include <cstring>

using namespace std;
using namespace boost::serialization;

struct Address {
    string departamento;
    string provincia;
    string ciudad;
    string distrito;

   template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & departamento;
        ar & provincia;
        ar & ciudad;
        ar & distrito;
    }
};

struct Nat_Birthplace {
    string  nationality;
    string  birthplace;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & nationality;
        ar & birthplace;
    }
    
};

struct Entry {
    uint32_t dni;
    size_t memory_position;

    Entry(uint32_t d = 0, size_t m = 0) : dni(d), memory_position(m) {}

    // No uses un destructor que intente liberar memoria manualmente
    ~Entry() = default;
};

struct Person {
    uint32_t dni;
    string  name;
    string  surname;
    Address address;
    Nat_Birthplace birthplace;
    string  phone;
    string  email;
    string  marital_status;
    bool is_deleted = false;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & dni;
        ar & name;
        ar & surname;
        ar & address;
        ar & birthplace;
        ar & phone;
        ar & email;
        ar & marital_status;
        ar & is_deleted;
    }
};

#endif // USERDATA_H
