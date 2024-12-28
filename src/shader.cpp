#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <shader.hpp>
#include <sstream>
#include <string>

namespace cg {
Shader::Shader(const char *vertexPath, const char *fragmentPath) {
  std::string vertexCode{vertexPath}, fragmentCode{fragmentPath};
  std::ifstream vShaderFile{vertexCode};
  std::ifstream fShaderFile(fragmentCode);
  if (!vShaderFile.is_open() || !fShaderFile.is_open()) {
    std::cout << "Failed to open shader file" << std::endl;
  }
  std::stringstream vShaderStream, fShaderStream;
  vShaderStream << vShaderFile.rdbuf();
  fShaderStream << fShaderFile.rdbuf();
  vShaderFile.close();
  fShaderFile.close();
  vertexCode = vShaderStream.str();
  fragmentCode = fShaderStream.str();
  const char *vShaderCode = vertexCode.c_str();
  const char *fShaderCode = fragmentCode.c_str();

  unsigned int vertex, fragment;
  int success;
  char infoLog[512];
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &vShaderCode, NULL);
  glCompileShader(vertex);

  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(vertex, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  };

  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &fShaderCode, NULL);
  glCompileShader(fragment);
  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success) {
    glGetShaderInfoLog(fragment, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  ID = glCreateProgram();
  glAttachShader(ID, vertex);
  glAttachShader(ID, fragment);
  glLinkProgram(ID);
  glGetProgramiv(ID, GL_LINK_STATUS, &success);
  if (!success) {
    glGetProgramInfoLog(ID, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n"
              << infoLog << std::endl;
  }
  glDeleteShader(vertex);
  glDeleteShader(fragment);
}

void Shader::use() { glUseProgram(ID); }


} // namespace cg
