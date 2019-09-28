#ifndef H_ORBITER
#define H_ORBITER

#include "GLFW/glfw3.h"
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"

struct Orbiter{

    // ---- internal state modification / query ---- //

    // Angle in degree between axis Y and plane XZ ie latitude
    void setPolarAngle(float angle_in_degree);
    // Angle in degree between axis Z and plane XY ie longitude
    void setAzimutalAngle(float angle_in_degree);

	glm::mat4 viewProjection(const float aspect_ratio = 1.f);

    // ---- external interactions ---- //

    enum PolarMovement {UP = -1, DOWN = 1};
    void orbitPolar(const PolarMovement movement, float angular_speed);

    enum AzimutalMovement {LEFT = 1, RIGHT = -1};
    void orbitAzimutal(const AzimutalMovement movement, float angular_speed);

    enum RadialMovement {TOWARD = -1, AWAY = 1};
    void moveRadial(const RadialMovement movement, float radial_speed);

    // ---- internal state ---- //

	glm::vec3 target_position = {0.f, 0.f, 0.f};
	float camera_radius = 0.f;

    float camera_polar_angle = 0.f; // in radians between axis Y to plane XZ
    float camera_azimuthal_angle = 0.f; // in radians between axis Z plane XY
};

#endif
