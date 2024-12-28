#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <iostream>

  void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
  }

int main() {
  auto glfw_version{glfwGetVersionString()};
  std::cout << "GLFW version" << glfw_version << std::endl;
  const int width = 800;
  const int height = 600;
  auto title = "A little OpenGL game";

  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW" << std::endl;
    return -1;
  }
    // initialize GLFW HINT
  glfwInitHint(GLFW_VERSION_MAJOR, 4);
  glfwInitHint(GLFW_VERSION_MAJOR, 0);
  glfwInitHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    glfwTerminate();
    return -1;
  }

  // 通过使用由glViewport函数提供的数据，进行视口变换(Viewport
  // Transform)，标准化设备坐标(Normalized Device
  // Coordinates)会变换为屏幕空间坐标(Screen-space Coordinates)。
  glViewport(0, 0, 800, 600);

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  //  输入顶点数据
  float vertices[] = {-.5f, -.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.5f, 0.0f};
  unsigned int VBO;  // or GLuint
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);  // 顶点缓冲对象的缓冲类型
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,
               GL_STATIC_DRAW);  // 把顶点数据复制到缓冲的内存中

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // 顶点着色器
  auto vertexShaderSource = R"(#version 400 core
                                              layout(location = 0) in vec3 aPos;
                                              void main(){
                                              gl_Position = vec4(aPos.x,aPos.y,aPos.z,1.0f);
                                              })";
  unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);

  // 错误检测
  int success;
  char infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  // 片段着色器
  auto fragmentShaderSource = R"(
    #version 400 core
    out vec4 FragColor;
    void main() {
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    }
  )";
  // float elements[] = {
  //     0.5f,  0.5f,  0.0f,  // 右上角
  //     0.5f,  -0.5f, 0.0f,  // 右下角
  //     -0.5f, -0.5f, 0.0f,  // 左下角
  //     -0.5f, 0.5f,  0.0f   // 左上角
  // };
  unsigned int indices[] = {0, 1, 2, 2, 3, 0};
  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // glBufferData(GL_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

  unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  glCompileShader(fragmentShader);

  // 链接程序得到着色器程序对象
  unsigned int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  //  用线绘制
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  while (!glfwWindowShouldClose(window)) {
    glClearColor(.5f, .5f, .5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  glDeleteProgram(shaderProgram);
  glfwTerminate();
  return 0;
}