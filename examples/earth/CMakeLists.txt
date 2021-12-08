project(earth)
add_executable(${PROJECT_NAME} main.cpp openglwindow.cpp model.cpp trackball.cpp)
enable_abcg(${PROJECT_NAME})