cmake_minimum_required(VERSION 3.10)
project(dma_example C)

# Agregar banderas de compilación directamente
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c89")

# Añadir el ejecutable
add_executable(dma_example main.c)

target_link_libraries(dma_example pthread)