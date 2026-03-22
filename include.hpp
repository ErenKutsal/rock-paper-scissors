#define GL_SILENCE_DEPRECATION

// OpenGL
#include <GLFW/glfw3.h>
#include <OpenGL/gl3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Std
#include <iterator>
#include <random>
#include <vector>

#include "init_shader.hpp"

typedef struct Object Object;