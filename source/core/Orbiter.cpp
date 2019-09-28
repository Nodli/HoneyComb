#include "Orbiter.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "utils.h"

#include <cmath>

void Orbiter::setPolarAngle(const float angle_in_degree){
    camera_polar_angle = angle_in_degree * M_PI / 180 + M_PI / 2;
}

void Orbiter::setAzimutalAngle(const float angle_in_degree){
    camera_azimuthal_angle = angle_in_degree * M_PI / 180;
}

void Orbiter::orbitPolar(const PolarMovement movement, float angular_speed){
    camera_polar_angle = modulo_two_pi(camera_polar_angle + movement * angular_speed);
}

void Orbiter::orbitAzimutal(const AzimutalMovement movement, float angular_speed){
    camera_azimuthal_angle = modulo_two_pi(camera_azimuthal_angle
            + movement * sign(camera_polar_angle - M_PI) * angular_speed);
}

void Orbiter::moveRadial(const RadialMovement movement, float radial_speed){
    camera_radius += movement * radial_speed;
}

glm::mat4 Orbiter::viewProjection(const float aspect_ratio){
	glm::vec3 camera_position = {camera_radius * sin(camera_polar_angle) * sin(camera_azimuthal_angle),
		camera_radius * cos(camera_polar_angle),
		camera_radius * sin(camera_polar_angle) * cos(camera_azimuthal_angle)};

	glm::mat4 x_rotation = glm::rotate(glm::mat4(1.f), camera_polar_angle, {1.f, 0.f, 0.f});
	glm::mat4 y_rotation = glm::rotate(glm::mat4(1.f), camera_azimuthal_angle, {0.f, 1.f, 0.f});
	glm::vec3 up_direction = y_rotation * x_rotation * glm::vec4(0.f, 0.f, - 1.f, 1.f);

	glm::mat4 view = glm::lookAt(camera_position, target_position, up_direction);

	glm::mat4 projection = glm::ortho(- aspect_ratio * camera_radius, aspect_ratio * camera_radius,
			- camera_radius, camera_radius,
			- 100.f * camera_radius, 100.f * camera_radius);

	return projection * view;
}
