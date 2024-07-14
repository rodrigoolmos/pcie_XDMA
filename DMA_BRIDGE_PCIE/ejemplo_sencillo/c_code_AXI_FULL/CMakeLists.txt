cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(dma_example C)

# Establecer la versión de C estándar
set(CMAKE_C_STANDARD 89)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

# Añadir el ejecutable
add_executable(dma_example main.c)
