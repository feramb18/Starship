cmake_minimum_required(VERSION 3.27)
project(Starship C)

set(CMAKE_C_STANDARD 11)

# Trova il package SDL2
find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)

# Includi il percorso degli header files di SDL2
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS})


add_executable(Client client.c)
add_executable(Server server.c)

# Collega la libreria SDL2 ai tuoi eseguibili
target_link_libraries(Client ${SDL2_LIBRARIES}${SDL2_TTF_LIBRARIES})
target_link_libraries(Server ${SDL2_LIBRARIES}${SDL2_TTF_LIBRARIES})
