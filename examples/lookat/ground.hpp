#ifndef GROUND_HPP_
#define GROUND_HPP_

#include <vector>
#include "abcg.hpp"
#include "PerlinNoise.hpp"

struct Vertex {
  glm::vec3 position{};
  glm::vec3 normal{};

  bool operator==(const Vertex& other) const noexcept {
    static const auto epsilon{std::numeric_limits<float>::epsilon()};
    return glm::all(glm::epsilonEqual(position, other.position, epsilon)) &&
           glm::all(glm::epsilonEqual(normal, other.normal, epsilon));
  }
};

class Ground {
 public:
  void initializeGL(GLuint program);
  void paintGL();
  void terminateGL();
  void standardize(std::vector<float> vertices);
  void createBuffers();
  void setupVAO(GLuint program);
  void render();
  std::vector<int> generateIndices();
  std::vector<float> generate_vertices();

  [[nodiscard]] int getNumTriangles() const {
    return static_cast<int>(m_indices.size()) / 3;
  }

 private:
  GLuint m_VAO{};
  GLuint m_VBO{};
  GLuint m_EBO{};
  GLuint m_program{};

  PerlinNoise pn;
  int m_verticesToDraw{};

  std::vector<Vertex> m_vertices;
  std::vector<GLuint> m_indices;

  GLint m_modelMatrixLoc{};
  GLint m_colorLoc{};
};

#endif