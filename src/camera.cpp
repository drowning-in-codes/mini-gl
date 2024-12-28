#include <camera.hpp>
namespace cg {
void Camera::Rotate(float xoffset, float yoffset) {
  pitch += yoffset * sensitivity;
  yaw += xoffset * sensitivity;
  if (pitch > 89.0f) {
    pitch = 89.0f;
  }
  if (pitch < -89.0f) {
    pitch = -89.0f;
  }
  glm::vec3 front{};
  front.x = std::cos(glm::radians(pitch)) * std::cos(glm::radians(yaw));
  front.y = std::sin(glm::radians(pitch));
  front.z = std::cos(glm::radians(pitch)) * std::sin(glm::radians(yaw));
  cameraFront = glm::normalize(front);
}

}; // namespace cg