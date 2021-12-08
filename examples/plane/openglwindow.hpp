#ifndef OPENGLWINDOW_HPP_
#define OPENGLWINDOW_HPP_

#include <random>
#include <vector>

#include "abcg.hpp"

struct Vertex {
  glm::vec3 position;

  bool operator==(const Vertex& other) const {
    return position == other.position;
  }
};

class OpenGLWindow : public abcg::OpenGLWindow {
  protected:
    void initializeGL() override;
    void paintGL() override;
    void paintUI() override;
    void resizeGL(int width, int height) override;
    void terminateGL() override;

 private:
    int rows;
    int columns;
    double highest;
    static double unused;
    double scale(double x) {return x * (((double)rand()/RAND_MAX) - 0.5);}

    GLuint m_VAO{};
    GLuint m_VBO{};
    GLuint m_EBO{};
    GLuint m_program{};

    int m_verticesToDraw{};

    std::vector<Vertex> m_vertices;
    std::vector<GLuint> m_indices;

    int m_viewportWidth{};
    int m_viewportHeight{};

    std::vector<std::vector<double>> d;
    int solidId;
    int wireFrameId;
    void init_landscape(int rows, int columns);
    void generate(int x1, int y1, int x2, int y2, double rug);
    void create(double rug);
    void generate_landscape();
    void createSolidDisplayList();
    void loadLandscape();
    void standardize();

    void update();
};

#endif