#include "ground.hpp"
#include <random>
#include <cmath>

#include <cppitertools/itertools.hpp>
#include <glm/gtx/fast_trigonometry.hpp>
#include <glm/gtx/hash.hpp>
#include <unordered_map>

float WATER_HEIGHT = 0.1f;
float meshHeight = 1;
int xMapChunks = 10;
int yMapChunks = 10;
int chunkWidth = 12;
int chunkHeight = 12;

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

std::vector<float> Ground::generate_vertices() {
    std::vector<float> v;

    unsigned int seed = 112;
    pn = PerlinNoise(seed);
    
    for (int y = 0; y < chunkHeight + 1; y++)
        for (int x = 0; x < chunkWidth; x++) {
            double n = std::fmax(pn.noise(x, y, 0.8) * meshHeight, WATER_HEIGHT * 0.5 * meshHeight)/20;
            v.push_back(x);
            v.push_back(n);
            v.push_back(y);
        }
    
    return v;
}

std::vector<int> Ground::generateIndices() {
    std::vector<int> indices;
    
    for (int y = 0; y < chunkHeight; y++)
        for (int x = 0; x < chunkWidth; x++) {
            int pos = x + y*chunkWidth;
            
            if (x == chunkWidth - 1 || y == chunkHeight - 1) {
                // Don't create indices for right or top edge
                continue;
            } else {
                // Top left triangle of square
                indices.push_back( (pos + chunkWidth));
                indices.push_back(pos);
                indices.push_back( (pos + chunkWidth + 1) );
                // Bottom right triangle of square
                indices.push_back( (pos + 1) );
                indices.push_back( (pos + 1 + chunkWidth));
                indices.push_back(pos);
            }
        }

    return indices;
}

std::vector<float> generate_normals(const std::vector<int> &indices, const std::vector<float> &vertices) {
    int pos;
    glm::vec3 normal;
    std::vector<float> normals;
    std::vector<glm::vec3> verts;
    
    // Get the vertices of each triangle in mesh
    // For each group of indices
    for (int i = 0; i < indices.size(); i += 3) {
        
        // Get the vertices (point) for each index
        for (int j = 0; j < 3; j++) {
            pos = indices[i+j]*3;
            verts.push_back(glm::vec3(vertices[pos], vertices[pos+1], vertices[pos+2]));
        }
        
        // Get vectors of two edges of triangle
        glm::vec3 U = verts[i+1] - verts[i];
        glm::vec3 V = verts[i+2] - verts[i];
        
        // Calculate normal
        normal = glm::normalize(-glm::cross(U, V));
        normals.push_back(normal.x);
        normals.push_back(normal.y);
        normals.push_back(normal.z);
    }
    
    return normals;
}

glm::vec3 get_color(int r, int g, int b) {
    return glm::vec3(r/255.0, g/255.0, b/255.0);
}

struct terrainColor {
    terrainColor(float _height, glm::vec3 _color) {
        height = _height;
        color = _color;
    };
    float height;
    glm::vec3 color;
};

std::vector<float> generate_biome(const std::vector<float> &vertices, int xOffset, int yOffset) {
    std::vector<float> colors;
    std::vector<terrainColor> biomeColors;
    glm::vec3 color = get_color(255, 255, 255);
    
    // NOTE: Terrain color height is a value between 0 and 1
    biomeColors.push_back(terrainColor(WATER_HEIGHT * 0.5, get_color(60,  95, 190)));   // Deep water
    biomeColors.push_back(terrainColor(WATER_HEIGHT,        get_color(60, 100, 190)));  // Shallow water
    biomeColors.push_back(terrainColor(0.15, get_color(210, 215, 130)));                // Sand
    biomeColors.push_back(terrainColor(0.30, get_color( 95, 165,  30)));                // Grass 1
    biomeColors.push_back(terrainColor(0.40, get_color( 65, 115,  20)));                // Grass 2
    biomeColors.push_back(terrainColor(0.50, get_color( 90,  65,  60)));                // Rock 1
    biomeColors.push_back(terrainColor(0.80, get_color( 75,  60,  55)));                // Rock 2
    biomeColors.push_back(terrainColor(1.00, get_color(255, 255, 255)));                // Snow
    
    std::string plantType;
    
    // Determine which color to assign each vertex by its y-coord
    // Iterate through vertex y values
    for (int i = 1; i < vertices.size(); i += 3) {
        for (int j = 0; j < biomeColors.size(); j++) {
            // NOTE: The max height of a vertex is "meshHeight"
            if (vertices[i] <= biomeColors[j].height * meshHeight) {
                color = biomeColors[j].color;
                break;
            }
        }

        colors.push_back(color.r);
        colors.push_back(color.g);
        colors.push_back(color.b);
    }
    return colors;
}

void Ground::initializeGL(GLuint program) {
  m_program = program;
  std::vector<int> indices = generateIndices();
  std::vector<float> vertices = generate_vertices();
  std::vector<float> normals = generate_normals(indices, vertices);
  std::vector<float> colors = generate_biome(vertices, xMapChunks, yMapChunks);


  abcg::glClearColor(0, 0, 0, 1);

  // Enable depth buffering
  abcg::glEnable(GL_DEPTH_TEST);

  m_verticesToDraw = indices.size();

  GLuint VBO[3], EBO;
  // Generate VBO
  abcg::glGenBuffers(3, VBO);
  abcg::glGenBuffers(1, &m_EBO);
  abcg::glGenVertexArrays(1, &m_VAO);


  abcg::glBindVertexArray(m_VAO);
  abcg::glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
  abcg::glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), &vertices[0], GL_STATIC_DRAW);

  abcg::glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
  abcg::glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

  abcg::glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  abcg::glEnableVertexAttribArray(0);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
  abcg::glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);

  abcg::glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  abcg::glEnableVertexAttribArray(1);

  abcg::glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
  abcg::glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), &colors[0], GL_STATIC_DRAW);

  abcg::glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  abcg::glEnableVertexAttribArray(2);
}

void Ground::render() {
    glClearColor(0.53, 0.81, 0.92, 1.0f);
    abcg::glUseProgram(m_program);

    abcg::glBindVertexArray(m_VAO);
    glm::mat4 model{1.0f};
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f));

    abcg::glDrawElements(GL_TRIANGLES, m_verticesToDraw, GL_UNSIGNED_INT, nullptr);


}

void Ground::terminateGL() {
  abcg::glDeleteBuffers(1, &m_VBO);
  abcg::glDeleteVertexArrays(1, &m_VAO);
}