#include <format>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <iterator>
#include <random>
#include <stdexcept>
#include <vector>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include <shader.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <camera.hpp>
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

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

// 纹理贴图  材质
struct Texture {
  GLuint id;
  std::string type;
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
void Mesh::setupMesh() {}

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
GLuint LoadTexture(const char *path) {
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
  glEnable(GL_DEPTH_TEST);
  auto texture = LoadTexture("./textures/container2.png");
  auto texture_sepc = LoadTexture("./textures/container2_specular.png");
  auto texture_emission = LoadTexture("./textures/matrix.jpg");
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

  auto vertexShaderFile = "./shaders/vertexShader.vert";
  // auto fragmentShaderFile = "./shaders/fragmentShader.frag";
  auto fragmentShaderFile = "./shaders/multi_lights.frag";
  cg::Shader shaderProgram{vertexShaderFile, fragmentShaderFile};

  auto lightVertexShaderFile = "./shaders/lightShader.vert";
  auto lightFragmentShaderFile = "./shaders/lightColor.frag";
  cg::Shader lightShaderProgram{lightVertexShaderFile, lightFragmentShaderFile};

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

  while (!glfwWindowShouldClose(window)) {
    currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    // process
    // float timeValue = glfwGetTime();
    // float greenValue = (std::sin(timeValue) / 2.0f) + 0.5f;
    // int vertexColorLocation = glGetUniformLocation(shaderProgram,
    // "outColor"); 设置背景颜色
    processInput(window);
    glClearColor(.0f, .0f, .0f, .0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float camX = std::sin(glfwGetTime()) * radius;
    float camZ = std::cos(glfwGetTime()) * radius;

    auto view = camera.lookAt();
    // view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    // model =
    // glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f));
    auto projection{glm::perspective(
        glm::radians(fov), (float)width / (float)height, 0.1f, 100.0f)};

    auto model{glm::mat4(1.0f)};
    auto trans = projection * view * model;
    lightShaderProgram.use();
    glBindVertexArray(lightVAO);
    // model = glm::rotate(glm::translate(model, glm::vec3(.2f, .3f, 1.3f)),
    // glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    float randomAngle = std::sin(glfwGetTime()) * 180.0f;
    // model = glm::translate(
    //     glm::rotate(model, glm::radians(randomAngle), glm::vec3(.0f, 1.f,
    //     .0f)), glm::vec3(1.2f, 1.0f, 2.0f));

    for (std::size_t i{}; i < std::size(pointLightPositions); i++) {
      model = glm::translate(model, pointLightPositions[i]);
      trans = projection * view * model;
      lightShaderProgram.setMat4("trans", trans);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // 画图
    shaderProgram.use();
    glBindVertexArray(VAO);
    auto lightCenterPos =
        trans * glm::vec4(lightCenter[0], lightCenter[1], lightCenter[2], 1.0f);

    // 定向光
    shaderProgram.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    shaderProgram.setVec3("dirLight.ambient", 0.2f, 1.0f, 0.3f);
    shaderProgram.setVec3("dirLight.diffuse", 0.2f, 1.0f, 0.3f);
    shaderProgram.setVec3("dirLight.specular", 0.2f, 1.0f, 0.3f);

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
    // glm::vec3 lightColor;
    // lightColor.x = std::sin(glfwGetTime() * 2.0f);
    // lightColor.y = std::sin(glfwGetTime() * 0.7f);
    // lightColor.z = std::sin(glfwGetTime() * 1.3f);
    // glm::vec3 diffuseColor = lightColor * glm::vec3(.5f);
    // glm::vec3 ambientColor = diffuseColor * glm::vec3(.2f);

    shaderProgram.setVec3("spotLight.ambient", glm::vec3(.2f, .2f, .2f));
    shaderProgram.setVec3("spotLight.diffuse", glm::vec3(.8f, .8f, .8f));
    shaderProgram.setVec3("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));

    shaderProgram.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shaderProgram.setFloat("spotLight.outerCutOff",
                           glm::cos(glm::radians(15.5f)));
    shaderProgram.setVec3("spotLight.direction", camera.cameraFront);
    shaderProgram.setVec3("spotLight.position", camera.cameraPos);

    // shaderProgram.setVec3("light.LightDir", glm::vec3(-0.2f, -1.0f, -.3f));

    // shaderProgram.setVec3("material.ambient", 1.0f, .5f, .3f);
    // shaderProgram.setVec3("material.diffuse", 1.0f, .5f, .31f);

    shaderProgram.setInt("material.diffuse", 0);
    shaderProgram.setInt("material.specular", 1);
    shaderProgram.setFloat("material.shininess", 64.0f);
    auto coord_trans = glm::vec2(.0f, 1.0f + std::sin(glfwGetTime()) / 2.0f);
    shaderProgram.setVec2("coord_trans", coord_trans);

    // shaderProgram.setVec3("lightColor", 1.0f, 1.0f, 1.0f);
    // shaderProgram.setVec3("objectColor", 1.0f, .5f, .32f);
    shaderProgram.setVec3("viewPos", camera.cameraPos);
    shaderProgram.setMat4("model", model);
    shaderProgram.setMat4("view", view);
    shaderProgram.setMat4("projection", projection);

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture_sepc);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture_emission);

    // shaderProgram.setFloat("offset", x_offset, .0f, .0f);
    // shaderProgram.setFloat("offsetColor", r_offset, .2f, .0f);
    // shaderProgram.setFloat("ourColor", 0.4f, 0.3f, 0.2f);
    // glUseProgram(shaderProgram);
    // glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    for (std::size_t i{}; i < 10; i++) {
      glm::mat4 model{1.0f};
      model = glm::rotate(glm::translate(model, cubePositions[i]),
                          glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

      shaderProgram.setMat4("model", model);
      // glDrawArrays(GL_TRIANGLES, 0, 3);
      // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }
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