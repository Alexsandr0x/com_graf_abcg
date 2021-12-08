#include "openglwindow.hpp"

#include <imgui.h>

#include <unordered_map>
#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>

// Explicit specialization of std::hash for Vertex
namespace std {
template <>
struct hash<Vertex> {
  size_t operator()(Vertex const& vertex) const noexcept {
    const std::size_t h1{std::hash<glm::vec3>()(vertex.position)};
    return h1;
  }
};
}  // namespace std

double OpenGLWindow::unused = -10032.4775;


void OpenGLWindow::init_landscape(int rows, int columns){

  std::vector<double> nullRow(columns, unused);

  std::vector< std::vector<double> > nullMatrix(rows, nullRow);

  d = nullMatrix;
}


void OpenGLWindow::create(double rug) {
  int r, c;
  // First put zeros for the elevations around the whole boundary:
  for (r = 0; r < rows; r++) d[r][0] = d[r][columns-1] = 0;
  for (c = 0; c < columns; c++) d[0][c] = d[rows-1][c] = 0;

  // Then put zeros in the corners inset two units and generate
  // a fractal landscape in that rectangle.
  d[2][2] = d[2][columns-3] = d[rows-3][2] = d[rows-3][columns-3] = 0;
  generate(2, 2, rows - 3, columns - 3, rug);

  // Then smooth out the inner fractal so it meets the zeroed out
  // edges.  Make the part just outside the fractal one-third higher
  // so it simulates flatter beaches.
  for (r = 2; r < rows - 2; r++) d[r][1] = d[r][2] / 3.0;
  for (r = 2; r < rows - 2; r++) d[r][c-2] = d[r][c-3] / 3.0;
  for (c = 1; c < columns-1; c++) d[1][c] = d[2][c] / 3.0;
  for (c = 1; c < columns-1; c++) d[r-2][c] = d[r-3][c] / 3.0;

  // Finally it part of the land is underwater make that elevation 0.
  highest = 0.0;
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < columns; j++) {
      if (d[i][j] < 0) d[i][j] = 0;
      if (d[i][j] > highest) highest = d[i][j];
    }
  }
}

// void OpenGLWindow::loadLandscape() {
//   m_vertices.clear();
//   m_indices.clear();
//   int x, z;
//   for (x = 0; x < rows; x++) {
//     glBegin(GL_LINE_STRIP);
//     glVertex3f(x, d[x][0], 0);
//     for (z = 1; z < columns; z++) {
//       glVertex3f(x, d[x][z], z);
//     }
//     glEnd();
//   }

//   for (z = 0; z < columns; z++) {
//     glBegin(GL_LINE_STRIP);
//     glVertex3f(0, d[0][z], z);
//     for (x = 1; x < rows; x++) {
//       glVertex3f(x, d[x][z], z);
//     }
//     glEnd();
//   }
// }

void OpenGLWindow::loadLandscape() {
  m_vertices.clear();
  m_indices.clear();

  // A key:value map with key=Vertex and value=index
  std::unordered_map<Vertex, GLuint> hash{};

  Vertex vertex{};
  for (int x = 0; x < rows - 1; x++) {
    for (int z = 0; z < columns - 1; z++) {
      //              x1 z1 x2   z2 x3  z3
      // drawTriangle(x, z, x+1, z, x, z+1);
      float vx{x};
      float vy{d[x][z]};
      float vz{z};
        
      vertex.position = {vx, vy, vz};

      // If hash doesn't contain this vertex
      if (hash.count(vertex) == 0) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      vx = x+1;
      vy = d[x+1][z];
      vz = z;

      vertex = Vertex();
      vertex.position = {vx, vy, vz};

      // If hash doesn't contain this vertex
      if (hash.count(vertex) == 0) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      vx = x;
      vy = d[x][z+1];
      vz = z+1;

      vertex = Vertex();
      vertex.position = {vx, vy, vz};

      // If hash doesn't contain this vertex
      if (hash.count(vertex) == 0) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }
      //              x1  z1   x2   z2 x3  z3
      // drawTriangle(x+1, z, x+1, z+1, x, z+1);

      vx = x+1;
      vy = d[x+1][z];
      vz = z;

      vertex = Vertex();
      vertex.position = {vx, vy, vz};

      // If hash doesn't contain this vertex
      if (hash.count(vertex) == 0) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      vx = x+1;
      vy = d[x+1][z+1];
      vz = z+1;

      vertex = Vertex();
      vertex.position = {vx, vy, vz};

      // If hash doesn't contain this vertex
      if (hash.count(vertex) == 0) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }

      vx = x;
      vy = d[x][z+1];
      vz = z+1;

      vertex = Vertex();
      vertex.position = {vx, vy, vz};

      // If hash doesn't contain this vertex
      if (hash.count(vertex) == 0) {
        // Add this index (size of m_vertices)
        hash[vertex] = m_vertices.size();
        // Add this vertex
        m_vertices.push_back(vertex);
      }
      m_indices.push_back(hash[vertex]);
    }
  }
}


void OpenGLWindow::generate(int x1, int y1, int x2, int y2, double rug) {
  int x3 = (x1 + x2) / 2;
  int y3 = (y1 + y2) / 2;
  if (y3 < y2) {
    if (d[x1][y3] == unused) {
      d[x1][y3] = (d[x1][y1] + d[x1][y2])/2 + scale(rug*(y2-y1));
    }
    d[x2][y3] = (d[x2][y1] + d[x2][y2])/2 + scale(rug*(y2-y1));
  }
  if (x3 < x2) {
    if (d[x3][y1] == unused) {
      d[x3][y1] = (d[x1][y1] + d[x2][y1])/2 + scale(rug*(x2-x1));
    }
    d[x3][y2] = (d[x1][y2] + d[x2][y2])/2 + scale(rug*(x2-x1));
  }
  if (x3 < x2 && y3 < y2) {
    d[x3][y3] = (d[x1][y1] + d[x2][y1] + d[x1][y2] + d[x2][y2])/4
    + scale(rug * (fabs((double)(x2 - x1)) + fabs((double)(y2 - y1))));
  }
  if (x3 < x2 - 1 || y3 < y2 - 1) {
    generate(x1, y1, x3, y3, rug);
    generate(x1, y3, x3, y2, rug);
    generate(x3, y1, x2, y3, rug);
    generate(x3, y3, x2, y2, rug);
  }
}

void OpenGLWindow::standardize() {
  // Center to origin and normalize largest bound to [-1, 1]

  // Get bounds
  glm::vec3 max(std::numeric_limits<float>::lowest());
  glm::vec3 min(std::numeric_limits<float>::max());
  for (const auto& vertex : m_vertices) {
    max.x = std::max(max.x, vertex.position.x);
    max.y = std::max(max.y, vertex.position.y);
    max.z = std::max(max.z, vertex.position.z);
    min.x = std::min(min.x, vertex.position.x);
    min.y = std::min(min.y, vertex.position.y);
    min.z = std::min(min.z, vertex.position.z);
  }

  // Center and scale
  const auto center{(min + max) / 2.0f};
  const auto scaling{2.0f / glm::length(max - min)};
  for (auto& vertex : m_vertices) {
    vertex.position = (vertex.position - center) * scaling;
  }
}

void OpenGLWindow::initializeGL() {
  abcg::glClearColor(0, 0, 0, 1);

  // Enable depth buffering
  abcg::glEnable(GL_DEPTH_TEST);

  // Create program
  m_program = createProgramFromFile(getAssetsPath() + "loadmodel.vert",
                                    getAssetsPath() + "loadmodel.frag");

  // Load model
  static double rug = ((double)rand()) / RAND_MAX;
  create(rug);
  standardize();
  loadLandscape();
  

  m_verticesToDraw = m_indices.size();

  // Generate VBO
  abcg::glGenBuffers(1, &m_VBO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  abcg::glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(),
                     m_vertices.data(), GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Generate EBO
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(m_indices[0]) * m_indices.size(), m_indices.data(),
                     GL_STATIC_DRAW);
  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  // Create VAO
  abcg::glGenVertexArrays(1, &m_VAO);

  // Bind vertex attributes to current VAO
  abcg::glBindVertexArray(m_VAO);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  GLint positionAttribute{abcg::glGetAttribLocation(m_program, "inPosition")};
  abcg::glEnableVertexAttribArray(positionAttribute);
  abcg::glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE,
                              sizeof(Vertex), nullptr);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, 0);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

  // End of binding to current VAO
  abcg::glBindVertexArray(0);
}

void OpenGLWindow::paintGL() {
  // Clear color buffer and depth buffer
  abcg::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  abcg::glViewport(0, 0, m_viewportWidth, m_viewportHeight);

  abcg::glUseProgram(m_program);
  abcg::glBindVertexArray(m_VAO);

  // Update uniform variable
  const GLint angleLoc{abcg::glGetUniformLocation(m_program, "angle")};
  abcg::glUniform1f(angleLoc, 0);

  // Draw triangles
  abcg::glDrawElements(GL_TRIANGLES, m_verticesToDraw, GL_UNSIGNED_INT,
                       nullptr);

  abcg::glBindVertexArray(0);
  abcg::glUseProgram(0);
}

void OpenGLWindow::paintUI() {
  abcg::OpenGLWindow::paintUI();

  // Create a window for the other widgets
  {
    const auto widgetSize{ImVec2(172, 62)};
    ImGui::SetNextWindowPos(ImVec2(m_viewportWidth - widgetSize.x - 5, 5));
    ImGui::SetNextWindowSize(widgetSize);
    ImGui::Begin("Widget window", nullptr, ImGuiWindowFlags_NoDecoration);

    static bool faceCulling{};
    ImGui::Checkbox("Back-face culling", &faceCulling);

    if (faceCulling) {
      abcg::glEnable(GL_CULL_FACE);
    } else {
      abcg::glDisable(GL_CULL_FACE);
    }

    // CW/CCW combo box
    {
      static std::size_t currentIndex{};
      const std::vector<std::string> comboItems{"CW", "CCW"};

      ImGui::PushItemWidth(70);
      if (ImGui::BeginCombo("Front face",
                            comboItems.at(currentIndex).c_str())) {
        for (const auto index : iter::range(comboItems.size())) {
          const bool isSelected{currentIndex == index};
          if (ImGui::Selectable(comboItems.at(index).c_str(), isSelected))
            currentIndex = index;
          if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
      }
      ImGui::PopItemWidth();

      if (currentIndex == 0) {
        abcg::glFrontFace(GL_CW);
      } else {
        abcg::glFrontFace(GL_CCW);
      }
    }

    ImGui::End();
  }
}

void OpenGLWindow::resizeGL(int width, int height) {
  m_viewportWidth = width;
  m_viewportHeight = height;
}

void OpenGLWindow::terminateGL() {
  abcg::glDeleteProgram(m_program);
  abcg::glDeleteBuffers(1, &m_EBO);
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}