#pragma once
#include <glad/glad.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>

namespace cg {
class Shader {
public:
  unsigned int ID;
  Shader(const char *vertexPath, const char *fragmentPath);
  void use();
  template <typename... Args>
  void setBool(const std::string &name, Args... args) const {
    if constexpr (sizeof...(args) == 1) {
      glUniform1i(glGetUniformLocation(ID, name.c_str()), args...);
    } else if constexpr (sizeof...(args) == 2) {
      glUniform2i(glGetUniformLocation(ID, name.c_str()), args...);
    } else if constexpr (sizeof...(args) == 3) {
      glUniform3i(glGetUniformLocation(ID, name.c_str()), args...);
    } else if constexpr (sizeof...(args) == 4) {
      glUniform4i(glGetUniformLocation(ID, name.c_str()), args...);
    } else {
      static_assert(sizeof...(args) == 1 || sizeof...(args) == 2 ||
                        sizeof...(args) == 3 || sizeof...(args) == 4,
                    "setBool can only accept 1, 2, 3, 4 arguments");
    }
  }
  void setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
  }
  void setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y,
                value.z);
  }

  void setVec3(const std::string &name, GLfloat *value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, value);
  }

  void setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
  }
  void setVec2(const std::string &name, const glm::vec2 &value) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), value.x, value.y);
  }
 void setVec2(const std::string &name, GLfloat *value) const {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, value);
  }



  void setMat4(const std::string &name, const glm::mat4 &value) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       glm::value_ptr(value));
  }
  template <typename... Args>
  void setInt(const std::string &name, Args... args) const {
    if constexpr (sizeof...(args) == 1) {
      glUniform1i(glGetUniformLocation(ID, name.c_str()), args...);
    } else if constexpr (sizeof...(args) == 2) {
      glUniform2i(glGetUniformLocation(ID, name.c_str()), args...);
    } else if constexpr (sizeof...(args) == 3) {
      glUniform3i(glGetUniformLocation(ID, name.c_str()), args...);
    } else if constexpr (sizeof...(args) == 4) {
      glUniform4i(glGetUniformLocation(ID, name.c_str()), args...);
    } else {
      static_assert(sizeof...(args) == 1 || sizeof...(args) == 2 ||
                        sizeof...(args) == 3 || sizeof...(args) == 4,
                    "setInt can only accept 1, 2, 3, 4 arguments");
    }
  }
  template <typename... Args>
  void setFloat(const std::string &name, Args... args) const {
    if constexpr (sizeof...(args) == 1) {
      glUniform1f(glGetUniformLocation(ID, name.c_str()), args...);
    } else if constexpr (sizeof...(args) == 2) {
      glUniform2f(glGetUniformLocation(ID, name.c_str()), args...);
    } else if constexpr (sizeof...(args) == 3) {
      glUniform3f(glGetUniformLocation(ID, name.c_str()), args...);
    } else if constexpr (sizeof...(args) == 4) {
      glUniform4f(glGetUniformLocation(ID, name.c_str()), args...);
    } else {
      static_assert(sizeof...(args) == 1 || sizeof...(args) == 2 ||
                        sizeof...(args) == 3 || sizeof...(args) == 4,
                    "setFloat can only accept 1, 2, 3, 4 arguments");
    }
  }
};
} // namespace cg
