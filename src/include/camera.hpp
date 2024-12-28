#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
namespace cg {
class Camera {
private:
  float sensitivity = 0.01f;
  float m_speed{2.5f};
  glm::vec3 rightDirection() const {
    return glm::normalize(glm::cross(cameraUp, -cameraFront));
  }
  glm::vec3 UpDirection() const {
    return glm::normalize(glm::cross(-cameraFront, rightDirection()));
  }
  float pitch{};
  float yaw{-90.0f};

public:
  Camera(glm::vec3 pos, glm::vec3 front, glm::vec3 up = {.0f, 1.0f, .0f},
         float t_speed = 2.5f)
      : cameraPos{pos}, cameraFront{front}, cameraUp{up}, m_speed(t_speed) {}
  glm::mat4 lookAt() {
    return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
  }
  glm::vec3 cameraPos{};
  glm::vec3 cameraFront;
  glm::vec3 cameraUp{};
  void setSpeed(float t_speed) { m_speed = t_speed; }
  void MoveForward() { cameraPos += m_speed * cameraFront; }
  void MoveBackward() { cameraPos -= m_speed * cameraFront; }
  void MoveRight() { cameraPos += rightDirection() * m_speed; }
  void MoveLeft() { cameraPos -= rightDirection() * m_speed; }
  void MoveUp() { cameraPos += UpDirection() * m_speed; }
  void MoveDown() { cameraPos -= UpDirection() * m_speed; }
  void Rotate(float xoffset, float yoffset);
};

}; // namespace cg
