#include <algorithm>
#include <assimp/material.h>
#include <assimp/types.h>
#include <format>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <iterator>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <shader.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <camera.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifdef _WIN32
#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include <GLFW/glfw3native.h>
#endif

namespace fs = std::filesystem;

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

template <typename... Args> void console_log(Args... args) {
  (std::cout << ... << args) << std::endl;
}
static float deltaTime{};
static float lastFrame = 0.0f;
const static int width = 800;
const static int height = 600;
static float lastX = static_cast<float>(width) / 2;
static float lastY = static_cast<float>(height) / 2;
static float fov{45.0f};
template <typename T> void print_vector(const T &vector) {
  for (std::size_t i = 0; i < vector.length(); i++) {
    std::cout << vector[i] << " ";
  }
  std::cout << "\n";
}
const auto cameraPos = glm::vec3(.0f, .0f, 3.0f);
const auto cameraFront = glm::vec3(.0f, .0f, -1.0f);
const auto cameraUp = glm::vec3(.0f, 1.0f, .0f);
static cg::Camera camera{cameraPos, cameraFront, cameraUp};

GLuint LoadTexture(const char *path, bool = false);
GLuint TextureFromFile(const std::string &path, const std::string &dir) {
  if (!fs::exists(dir)) {
    throw std::runtime_error("dir not exists");
  }
  fs::path parent{dir};
  fs::path file_path = parent / path;
  GLuint textureID = LoadTexture(file_path.string().c_str());
  return textureID;
}
struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

// 纹理贴图  材质
struct Texture {
  GLuint id;
  std::string type;
  aiString path;
};

class Mesh {
private:
public:
  // 网格数据
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  Mesh(std::vector<Vertex> t_vertices, std::vector<unsigned int> t_indices,
       std::vector<Texture> t_textures)
      : vertices(t_vertices), indices(t_indices), textures(t_textures) {
    setupMesh();
  }
  void Draw(cg::Shader);

private:
  GLuint VAO, VBO, EBO;
  void setupMesh();
};
void Mesh::setupMesh() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               &indices[0], GL_STATIC_DRAW);
  // 顶点位置
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  // 顶点法线
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Normal));
  // 顶点纹理坐标
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, TexCoords));
  glBindVertexArray(0);
}
void Mesh::Draw(cg::Shader shader) {
  shader.use();
  unsigned int diffuseNr = 1;
  unsigned int specularNr = 1;
  for (std::size_t i{}; i < textures.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    std::string name = textures[i].type;
    std::string number{};
    if (name == "texture_diffuse") {
      number = std::to_string(diffuseNr++);
    } else if (name == "texture_specular") {
      number = std::to_string(specularNr++);
    }
    shader.setInt(("material." + name + number).c_str(), i);

    glBindTexture(GL_TEXTURE_2D, textures[i].id);
  }

  glActiveTexture(GL_TEXTURE0);
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
class Model {
public:
  Model(const std::string &path) { loadModel(path); }
  void Draw(cg::Shader);

private:
  std::vector<Texture> textures_loaded;
  std::vector<Mesh> meshes;
  std::string directory;
  void loadModel(const std::string &path);
  void processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
  std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                            std::string typenName);
};

void Model::Draw(cg::Shader shader) {
  for (std::size_t i{}; i < meshes.size(); i++) {
    meshes[i].Draw(shader);
  }
}

void Model::loadModel(const std::string &path) {
  Assimp::Importer importer;
  const auto scene =
      importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    return;
  }
  directory = fs::path(path).parent_path().string();
  processNode(scene->mRootNode, scene);
}
void Model::processNode(aiNode *node, const aiScene *scene) {
  for (std::size_t i{}; i < node->mNumMeshes; i++) {
    auto mesh =
        scene->mMeshes
            [node->mMeshes[i]]; // node中存储着的是索引,通过这个索引获取mesh
    meshes.push_back(processMesh(mesh, scene));
  }
  for (std::size_t i{}; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}
Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
  for (unsigned int i{}; i < mesh->mNumVertices; i++) {
    Vertex v{
        .Position{mesh->mVertices[i].x, mesh->mVertices[i].y,
                  mesh->mVertices[i].z},
        .Normal{mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z},
        .TexCoords{.0f, .0f}};
    if (mesh->mTextureCoords[0]) {
      glm::vec2 vec;
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      v.TexCoords = vec;
    }
    vertices.push_back(v);
  }
  for (unsigned int i{}; i < mesh->mNumFaces; i++) {
    auto face = mesh->mFaces[i];
    // for(unsigned int j{};j<face.mNumIndices;j++){
    //   indices.push_back(face.mIndices[j]);
    // }
    indices.insert(indices.end(), face.mIndices,
                   face.mIndices + face.mNumIndices);
  }
  if (mesh->mMaterialIndex > 0) {
    auto material = scene->mMaterials[mesh->mMaterialIndex];
    auto diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE,
                                            "texture_diffuse");
    auto specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR,
                                             "texture_specular");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  }
  Mesh my_mesh{vertices, indices, textures};
  return my_mesh;
}
std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat,
                                                 aiTextureType type,
                                                 std::string typeName) {
  std::vector<Texture> textures;
  for (unsigned int i{}; i < mat->GetTextureCount(type); i++) {
    aiString texturePath;
    mat->GetTexture(type, i, &texturePath);
    auto it = std::ranges::find(textures_loaded, texturePath, &Texture::path);
    if (it == textures_loaded.end()) {
      Texture texture;
      texture.id = TextureFromFile(texturePath.C_Str(), directory);
      texture.type = typeName;
      texture.path = texturePath;
      textures.push_back(texture);
      textures_loaded.push_back(texture);
    } else {
      textures.push_back(*it);
    }
  }
  return textures;
}
unsigned int loadCubemap(std::vector<std::string> faces) {
  /**
   * @brief  立方体贴图
   */
  unsigned int cubeTexture;
  glGenTextures(1, &cubeTexture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexture);
  std::size_t index{};
  stbi_set_flip_vertically_on_load(GL_FALSE);
  for (const auto &entry : faces) {
    int width, height, nrChannels;
    auto data = stbi_load(entry.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + index, 0, GL_RGB, width,
                   height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else {
      std::cout << "Cubemap texture failed to load at path: " << entry
                << std::endl;
      stbi_image_free(data);
    }
    index++;
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  }

  stbi_set_flip_vertically_on_load(GL_TRUE);
  return cubeTexture;
}
void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
  float cameraSpeed = 2.5f * deltaTime;
  camera.setSpeed(cameraSpeed);
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera.MoveForward();
  } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera.MoveBackward();
  } else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera.MoveLeft();
  } else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera.MoveRight();
  } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    camera.MoveUp();
  } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    camera.MoveDown();
  }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  if (fov >= 1.0f && fov <= 45.0f) {
    fov -= yoffset;
  }
  if (fov <= 1.0f) {
    fov = 1.0f;
  }
  if (fov >= 45.0f) {
    fov = 45.0f;
  }
}
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
  static auto firstMouse{true};
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }
  float xoffset = xpos - lastX;
  float yoffset = ypos - lastY;
  camera.Rotate(xoffset, yoffset);
  lastX = xpos;
  lastY = ypos;
}

GLuint LoadTexture(const char *path, bool clip) {
  // 生成纹理
  GLuint texture;
  glGenTextures(1, &texture);
  stbi_set_flip_vertically_on_load(true);
  // 加载图像
  int img_width, img_height, nrChannels;
  unsigned char *data =
      stbi_load(path, &img_width, &img_height, &nrChannels, 0);
  if (data) {
    GLenum format;
    if (nrChannels == 1) {
      format = GL_RED;
    } else if (nrChannels == 3) {
      format = GL_RGB;
    } else if (nrChannels == 4) {
      format = GL_RGBA;
    } else {
      throw std::runtime_error("No available format.");
    }

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, format, img_width, img_height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    if (clip) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  } else {
    std::cerr << "Failed to load texture" << std::endl;
    std::cout << "Error: Failed to load the image because "
              << stbi_failure_reason();
    return -1;
  }

  stbi_image_free(data);
  return texture;
}
int main() {
  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }
  glfwInitHint(GLFW_VERSION_MAJOR, 4);
  glfwInitHint(GLFW_VERSION_MAJOR, 0);
  glfwInitHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwInitHint(GLFW_ALPHA_BITS, 8);
  // glfwInitHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  auto title = "A little OpenGL game";
  auto window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  // glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetScrollCallback(window, scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    glfwTerminate();
    return -1;
  }
  glEnable(GL_DEPTH_TEST); // 启用深度和模板测试
  // glDepthFunc(GL_LESS);
  // glEnable(GL_BLEND);

  // 启动面剔除
  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_BACK);
  // glFrontFace(GL_CCW);

  // glStencilMask(0xff);                       // 启用模板写入

  auto texture = LoadTexture("./resources/textures/container2.png");
  auto texture_sepc =
      LoadTexture("./resources/textures/container2_specular.png");
  auto texture_emission = LoadTexture("./resources/textures/matrix.jpg");
  auto grass_texture = LoadTexture("./resources/textures/grass.png", true);
  auto window_texture =
      LoadTexture("./resources/textures/blending_transparent_window.png", true);
  // 着色器编写
  // auto vertexShaderSource = R"(
  //   #version 400 core
  //   layout(location = 0) in vec3 aPos;
  //   layout(location = 1) in vec3 aColor;
  //   out vec3 ourColor;
  //   void main() {
  //       gl_Position = vec4(aPos,1.0f);
  //       ourColor = aColor;
  //       }
  //   )";

  // GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  // glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  // glCompileShader(vertexShader);

  auto vertexShaderFile = "./resources/shaders/vertexShader.vert";
  // auto fragmentShaderFile = "./shaders/fragmentShader.frag";
  auto fragmentShaderFile = "./resources/shaders/multi_lights.frag";
  cg::Shader shaderProgram{vertexShaderFile, fragmentShaderFile};

  auto lightVertexShaderFile = "./resources/shaders/lightShader.vert";
  auto lightFragmentShaderFile = "./resources/shaders/lightColor.frag";
  cg::Shader lightShaderProgram{lightVertexShaderFile, lightFragmentShaderFile};
  cg::Shader largeShaderProgram{"./resources/shaders/simpleVertex.vert",

                                "./resources/shaders/shaderSingleColor.frag"};
  cg::Shader grassShaderProgram{"./resources/shaders/vertexShader.vert",
                                "./resources/shaders/grassShader.frag"};
  cg::Shader windowShaderProgram{"./resources/shaders/vertexShader.vert",
                                 "./resources/shaders/windowShader.frag"};
  // auto fragmentShaderSource = R"(
  //   #version 400 core
  //   out vec4 FragColor;
  //   in vec3 ourColor;
  //   void main() {
  //   FragColor = vec4(ourColor, 1.0f);
  //   }
  // )";
  // GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  // glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  // glCompileShader(fragmentShader);

  // GLuint shaderProgram = glCreateProgram();
  // glAttachShader(shaderProgram, vertexShader);
  // glAttachShader(shaderProgram, fragmentShader);
  // glLinkProgram(shaderProgram);

  // glDeleteShader(vertexShader);
  // glDeleteShader(fragmentShader);
  Model loaded_model{"./resources/models/nanosuit/nanosuit.obj"};

  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);

  unsigned int textureColorbuffer;
  glGenTextures(1, &textureColorbuffer);
  glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

  // 纹理缓冲
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
               GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // 附加颜色附件  此外还可以附加深度和模板缓冲纹理
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         textureColorbuffer, 0);

  /**
   * @brief 立方体贴图
   *
   */
  std::vector<std::string> faces{"right.jpg",  "left.jpg",  "top.jpg",
                                 "bottom.jpg", "front.jpg", "back.jpg"};
  unsigned int cubemapTexture = loadCubemap([&faces]() {
    std::ranges::for_each(faces, [](std::string &face) {
      face = (std::string("./resources/skybox/") / fs::path(face)).string();
      return face;
    });
    return faces;
  }());
  float skyboxVertices[] = {
      // positions
      -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
      1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

      -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
      -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

      1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

      -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

      -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
      1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

      -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
      1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};
  auto skybox_vt = "./resources/shaders/skybox.vs";
  auto skybox_fg = "./resources/shaders/skybox.fs";
  cg::Shader skyboxShader{skybox_vt, skybox_fg};
  GLuint skyboxVAO, skyboxVBO;
  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

  // 渲染缓冲对象
  unsigned int rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, rbo);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cerr << "error::framebuffer:: framebuffer is not complete!"
              << std::endl;
  }
  // glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  GLuint VAO, VBO, EBO;
  GLuint lightVAO, lightVBO;
  // ------------------------------------------------------------------
  float vertices[] = {
      // positions          // normals           // texture coords
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.5f,  -0.5f,
      -0.5f, 0.0f,  0.0f,  -1.0f, 1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 0.0f,
      0.0f,  -1.0f, 1.0f,  1.0f,  0.5f,  0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f,
      1.0f,  1.0f,  -0.5f, 0.5f,  -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  -1.0f, 0.0f,  0.0f,

      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.5f,  -0.5f,
      0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,
      0.0f,  1.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
      1.0f,  1.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
      -0.5f, -0.5f, 0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,  -0.5f, 0.5f,
      -0.5f, -1.0f, 0.0f,  0.0f,  1.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f,
      0.0f,  0.0f,  0.0f,  1.0f,  -0.5f, -0.5f, -0.5f, -1.0f, 0.0f,  0.0f,
      0.0f,  1.0f,  -0.5f, -0.5f, 0.5f,  -1.0f, 0.0f,  0.0f,  0.0f,  0.0f,
      -0.5f, 0.5f,  0.5f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.0f,

      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,
      -0.5f, 1.0f,  0.0f,  0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,
      0.0f,  0.0f,  0.0f,  1.0f,  0.5f,  -0.5f, -0.5f, 1.0f,  0.0f,  0.0f,
      0.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
      0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,  0.5f,  -0.5f,
      -0.5f, 0.0f,  -1.0f, 0.0f,  1.0f,  1.0f,  0.5f,  -0.5f, 0.5f,  0.0f,
      -1.0f, 0.0f,  1.0f,  0.0f,  0.5f,  -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,
      1.0f,  0.0f,  -0.5f, -0.5f, 0.5f,  0.0f,  -1.0f, 0.0f,  0.0f,  0.0f,
      -0.5f, -0.5f, -0.5f, 0.0f,  -1.0f, 0.0f,  0.0f,  1.0f,

      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.5f,  0.5f,
      -0.5f, 0.0f,  1.0f,  0.0f,  1.0f,  1.0f,  0.5f,  0.5f,  0.5f,  0.0f,
      1.0f,  0.0f,  1.0f,  0.0f,  0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
      1.0f,  0.0f,  -0.5f, 0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
      -0.5f, 0.5f,  -0.5f, 0.0f,  1.0f,  0.0f,  0.0f,  1.0f};

  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  float cubeVertices[] = {
      // Back face
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // Bottom-left
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
      0.5f, -0.5f, -0.5f, 1.0f, 0.0f,  // bottom-right
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, // bottom-left
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,  // top-left
      // Front face
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,  // bottom-right
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // top-right
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f,   // top-right
      -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,  // top-left
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, // bottom-left
      // Left face
      -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // top-right
      -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // top-left
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // bottom-left
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-right
      -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // top-right
                                       // Right face
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-left
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-right
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,   // top-right
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f,  // bottom-right
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,    // top-left
      0.5f, -0.5f, 0.5f, 0.0f, 0.0f,   // bottom-left
      // Bottom face
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
      0.5f, -0.5f, -0.5f, 1.0f, 1.0f,  // top-left
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-left
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f,   // bottom-left
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,  // bottom-right
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, // top-right
      // Top face
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f,  // top-right
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f,   // bottom-right
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, // top-left
      -0.5f, 0.5f, 0.5f, 0.0f, 0.0f   // bottom-left
  };
  GLuint cubeVAO, cubeVBO;
  glGenVertexArrays(1, &cubeVAO);
  glGenBuffers(1, &cubeVBO);
  glBindVertexArray(cubeVAO);
  glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));

  float lightVertices[] = {
      -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,
      0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
      0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,
      -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f,
      0.5f,  -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,
      0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  -0.5f,
      0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,
      0.5f,  0.5f,  -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f,
      0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f,
      -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,
      0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f,
  };
  float lightCenter[] = {.0f, .0f, .0f};
  glGenVertexArrays(1, &lightVAO);
  glBindVertexArray(lightVAO);
  glGenBuffers(1, &lightVBO);
  glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(lightVertices), lightVertices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // 索引缓冲对象
  // glGenBuffers(1, &EBO);
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  // unsigned int indices[] = {0, 1, 2, 0, 2, 3};
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
  //              GL_STATIC_DRAW);

  // 解绑buffer
  // glBindBuffer(GL_ARRAY_BUFFER, 0);
  std::vector<glm::vec3> windowPositions{};
  windowPositions.push_back(glm::vec3(3.5f, 0.0f, 10.48f));
  windowPositions.push_back(glm::vec3(1.5f, 0.0f, 8.51f));
  // glBindVertexArray(0);
  shaderProgram.use();
  //   渲染loop
  // shaderProgram.setInt("texture2", 1);
  // auto trans{glm::mat4(1.0f)};
  // auto model = glm::rotate(glm::mat4(1.0f), glm::radians(50.0f),
  //  glm::vec3(.4f, 1.0f, 1.0f));
  // auto view = glm::translate(glm::mat4(1.0f), glm::vec3(.0f, .0f, -3.0f));
  // auto projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -0.3f, 1.0f);

  // view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

  float radius = 2.0f;
  // #ifdef _WIN32
  //   HWND hwnd = glfwGetWin32Window(window);
  //   if (hwnd) {
  //     LONG ret = ::GetWindowLong(hwnd, GWL_EXSTYLE);
  //     ret = ret | WS_EX_LAYERED;
  //     ::SetWindowLong(hwnd, GWL_EXSTYLE, ret);
  //     ::SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255,
  //                                  LWA_COLORKEY | LWA_ALPHA);
  //   }
  // #endif
  float currentFrame = 0.0f;

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(-90.0f, 90.0f);
  glm::vec3 cubePositions[] = {
      glm::vec3(0.0f, 2.0f, 8.0f),    glm::vec3(2.0f, 5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3(2.4f, -0.4f, -3.5f),  glm::vec3(-1.7f, 3.0f, -7.5f),
      glm::vec3(1.3f, -2.0f, -2.5f),  glm::vec3(1.5f, 2.0f, -2.5f),
      glm::vec3(1.5f, 0.2f, -1.5f),   glm::vec3(-1.3f, 1.0f, -1.5f)};
  float angle = dis(gen);
  glm::vec3 pointLightPositions[] = {
      glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(2.3f, -3.3f, -4.0f),
      glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(0.0f, 0.0f, -3.0f)};

  float quadVertices[] = {// vertex attributes for a quad that fills the entire
                          // screen in Normalized Device Coordinates.
                          // positions   // texCoords
                          -1.0f, 1.0f, 0.0f, 1.0f,  -1.0f, -1.0f,
                          0.0f,  0.0f, 1.0f, -1.0f, 1.0f,  0.0f,

                          -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  -1.0f,
                          1.0f,  0.0f, 1.0f, 1.0f,  1.0f,  1.0f};
  /**
   * @brief render framebuffer to quad
   */
  GLuint quadVAO, quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glBindVertexArray(0);
  auto quadVertexShaderFile = "./resources/shaders/quad.vs";
  auto quadFragmentShaderFile = "./resources/shaders/quad.fs";
  cg::Shader quadShader{quadVertexShaderFile, quadFragmentShaderFile};
  quadShader.setInt("texture1", 0);

  skyboxShader.setInt("cubeTexture", 0);
  glEnable(GL_STENCIL_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glStencilFunc(GL_ALWAYS, 1, 0xff);         // 设置模板测试函数
  glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // 设置模板测试操作
  while (!glfwWindowShouldClose(window)) {
    currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    processInput(window);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glStencilFunc(GL_ALWAYS, 1, 0xff); // 设置模板测试函数
    glEnable(GL_STENCIL_TEST);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glStencilMask(0xff);

    glClearColor(.0f, .0f, .0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    float randomAngle = std::sin(glfwGetTime()) * 180.0f;
    randomAngle = std::sin(glfwGetTime()) * 180.0f;
    auto view = camera.lookAt();
    auto projection{glm::perspective(
        glm::radians(fov), (float)width / (float)height, 0.1f, 100.0f)};

    auto model{glm::mat4(1.0f)};
    auto trans = projection * view * model;

    glStencilMask(0x00);
    /**
     * @brief 绘制天空盒
     *
     */
    skyboxShader.use();
    skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
    skyboxShader.setMat4("projection", projection);
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDepthMask(GL_FALSE);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);

    /**
     * @brief 绘制光源
     */
    lightShaderProgram.use();
    glBindVertexArray(lightVAO);
    for (std::size_t i{}; i < std::size(pointLightPositions); i++) {
      model = glm::translate(model, pointLightPositions[i]);
      trans = projection * view * model;
      lightShaderProgram.setMat4("trans", trans);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    /**
     * @brief 绘制立方体以及光源
     */
    shaderProgram.use();
    glBindVertexArray(VAO);
    auto lightCenterPos =
        trans * glm::vec4(lightCenter[0], lightCenter[1], lightCenter[2], 1.0f);

    // 定向光
    shaderProgram.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    shaderProgram.setVec3("dirLight.ambient", 0.05f, .05f, 0.05f);
    shaderProgram.setVec3("dirLight.diffuse", 0.4f, .4f, 0.4f);
    shaderProgram.setVec3("dirLight.specular", 0.5f, .5f, 0.5f);

    // 点光源
    for (std::size_t i{}; i < std::size(pointLightPositions); i++) {
      std::string pl_format = std::format("pointLights[{}].", i);
      shaderProgram.setVec3(pl_format + std::string("position"),
                            pointLightPositions[i]);
      shaderProgram.setFloat(pl_format + std::string("constant"), 1.0f);
      shaderProgram.setFloat(pl_format + std::string("linear"), .09f);
      shaderProgram.setFloat(pl_format + std::string("quadratic"), .032f);
      shaderProgram.setVec3(pl_format + std::string("ambient"),
                            glm::vec3(.2f, .2f, .2f));
      shaderProgram.setVec3(pl_format + std::string("diffuse"),
                            glm::vec3(.8f, .8f, .8f));
      shaderProgram.setVec3(pl_format + std::string("specular"),
                            glm::vec3(1.0f, 1.0f, 1.0f));
    }
    model = glm::mat4(1.0f);
    shaderProgram.setVec3("spotLight.ambient", glm::vec3(.2f, .2f, .2f));
    shaderProgram.setVec3("spotLight.diffuse", glm::vec3(.8f, .8f, .8f));
    shaderProgram.setVec3("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

    shaderProgram.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shaderProgram.setFloat("spotLight.outerCutOff",
                           glm::cos(glm::radians(15.5f)));
    shaderProgram.setVec3("spotLight.direction", camera.cameraFront);
    shaderProgram.setVec3("spotLight.position", camera.cameraPos);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    shaderProgram.setInt("material.diffuse", 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_sepc);
    shaderProgram.setInt("material.specular", 1);
    shaderProgram.setFloat("material.shininess", 64.0f);
    auto coord_trans = glm::vec2(.0f, 1.0f + std::sin(glfwGetTime()) / 2.0f);
    shaderProgram.setVec2("coord_trans", coord_trans);

    shaderProgram.setVec3("viewPos", camera.cameraPos);
    shaderProgram.setMat4("model", model);
    shaderProgram.setMat4("view", view);
    shaderProgram.setMat4("projection", projection);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glStencilMask(0x00);
    /**
     * @brief 加载并绘制模型
     */
    loaded_model.Draw(shaderProgram);

    glBindVertexArray(VAO);
    for (std::size_t i{}; i < sizeof(cubePositions) / sizeof(glm::vec3); i++) {
      glm::mat4 model{1.0f};
      model = glm::rotate(glm::translate(model, cubePositions[i]),
                          glm::radians(0.0f), glm::vec3(1.0f, 0.3f, 0.5f));

      shaderProgram.setMat4("model", model);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, texture_sepc);
      glDrawArrays(GL_TRIANGLES, 0, 36);
      grassShaderProgram.use();
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, grass_texture);
      grassShaderProgram.setInt("texture1", 0);
      grassShaderProgram.setMat4("view", view);
      grassShaderProgram.setMat4("projection", projection);
      grassShaderProgram.setVec3("viewPos", camera.cameraPos);
      model = glm::translate(model, glm::vec3(0.0f, 0.f, -0.01f));
      grassShaderProgram.setMat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      shaderProgram.use();
    }
    grassShaderProgram.use();
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grass_texture);
    grassShaderProgram.setInt("texture1", 0);
    grassShaderProgram.setMat4("view", view);
    grassShaderProgram.setMat4("projection", projection);
    float radius = 10.f;
    int grass_count{40};
    for (int i : std::ranges::iota_view(0, grass_count)) {
      model = glm::mat4(1.0f);
      float theta = glm::radians(360.0f / grass_count * i);
      auto location =
          glm::vec3(radius * std::cos(theta), 0.0f, radius * std::sin(theta));
      model = glm::translate(model, location);
      grassShaderProgram.setMat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    /**
     * @brief 绘制边框
     *
     */
    model = glm::mat4(1.0f);
    largeShaderProgram.use();
    glBindVertexArray(VAO);
    model = glm::scale(model, glm::vec3(1.1f));
    largeShaderProgram.setMat4("model", model);
    largeShaderProgram.setMat4("view", view);
    largeShaderProgram.setMat4("projection", projection);
    glStencilFunc(GL_NOTEQUAL, 1, 0xff);
    glStencilMask(0x00);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glStencilFunc(GL_ALWAYS, 1, 0xff);

    /**
     * @brief 绘制窗户
     */
    windowShaderProgram.use();
    glBindVertexArray(VAO);
    windowShaderProgram.setMat4("view", view);
    windowShaderProgram.setMat4("projection", projection);
    windowShaderProgram.setInt("texture1", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, window_texture);
    for (std::size_t i{}; i < std::size(windowPositions); i++) {
      model = glm::translate(model, windowPositions[i]);
      windowShaderProgram.setMat4("model", model);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    /**
     * @brief 绑定到默认的framebuffer
     */
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DEPTH_TEST);
    glClearColor(.0f, .0f, .0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    quadShader.use();
    glBindVertexArray(quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glStencilFunc(GL_ALWAYS, 1, 0xff);

    glfwPollEvents();
    glfwSwapBuffers(window);
  }

  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &lightVBO);
  glDeleteVertexArrays(1, &VAO);
  glDeleteVertexArrays(1, &lightVAO);
  glDeleteBuffers(1, &EBO);
  glDeleteProgram(shaderProgram.ID);
  glDeleteProgram(lightShaderProgram.ID);
  glfwTerminate();
}