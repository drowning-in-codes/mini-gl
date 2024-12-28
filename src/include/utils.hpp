#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <iostream>

inline GLuint buildShaders(const char* shaderSource, GLenum type) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &shaderSource, NULL);
  glCompileShader(shader);
  int success;
  char infoLog[512];
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(shader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
  }
  return shader;
}

template <std::size_t nums, typename T, typename RT = GLuint[nums]>
RT buildBuffers(T data) {
  RT VBO;
  glGenBuffers(nums, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
  return VBO;
}

inline void enableBuffer(GLuint location, GLuint size, bool normalize) {
  glVertexAttribPointer(location, size, GL_FLOAT, normalize, 0,
                        static_cast<void*>(0));
  glEnableVertexAttribArray(location);
}