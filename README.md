# Cuckoo Hashing con 33 Millones de Registros

Este proyecto implementa una estructura de datos eficiente usando **Cuckoo Hashing** para realizar operaciones rápidas de **búsqueda**, **inserción** y **eliminación** con **33 millones de registros**. Se utilizan bibliotecas externas como **Boost** y **Zlib** para optimizar el manejo de archivos y la compresión de datos.

---

## Tabla de Contenidos

- [Características Principales](#características-principales)
- [Instalación](#instalación)
- [Uso](#uso)
- [Estructura del Proyecto](#estructura-del-proyecto)
- [Tecnologías Utilizadas](#tecnologías-utilizadas)
- [Contribuciones](#contribuciones)
- [Licencia](#licencia)

---

## Características Principales

- **Cuckoo Hashing**: Una técnica de hashing para operaciones eficientes con tiempos garantizados de O(1) en búsqueda, inserción y eliminación.
- **Gestión de 33 millones de registros**: Optimizado para manejar grandes volúmenes de datos.
- **Compresión de archivos**: Uso de la biblioteca **Zlib** para comprimir archivos de registros, reduciendo el espacio en disco.
- **Uso de Boost**: Funcionalidades avanzadas como mapeo de archivos y manejo eficiente de memoria.
- **Alto rendimiento**: Optimizaciones en el almacenamiento y acceso a registros.

---

## Instalación

### Requisitos previos

- **C++ Compiler** (compatible con C++11 o superior).
- **Boost Library**: Para funcionalidades avanzadas.
- **Zlib Library**: Para compresión de archivos.

### Pasos de instalación

1. **Clona el repositorio**:

   ```bash
   git clone https://github.com/Angello5/RENIEC_API_cuckoo.git
   cd RENIEC_API_cuckoo

2. **Instala las dependencias(Homebrew con macOS)**:
   ```bash 
   brew install boost
   brew install zlib

## Uso
Ejecución del proyecto

      ./main

Operaciones soportadas:

    Inserción de registros.
    Búsqueda de registros por ID.
    Eliminación de registros.
    Carga y compresión de datos.

## Estructura del Proyecto
   ```bash
      RENIEC_API_cuckoo/
      │
      ├── grupoadafinal/              # Archivos fuente del proyecto
      │   ├── main.cpp                # Punto de entrada principal
      │   ├── UserData.h              # Definición de la estructura de datos de usuario
      │   ├── CuckooHashTable.h       # Implementación del Cuckoo    Hashing
      │   ├── mappedFile.h            # Gestión de archivos mapeados en memoria
      │
      ├── grupoadafinal.xcodeproj/    # Archivos del proyecto Xcode
      │
      ├── README.md                   # Documentación del proyecto
      └── .gitignore                  # Archivos ignorados por Git




   
