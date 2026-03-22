#include "include.hpp"

const float PI = glm::pi<float>();

const float radius = 0.01f;
const int circle_segments = 32;
const int num_vertices = circle_segments + 2;

glm::vec3 vertices[num_vertices];
GLuint vao, vbo;
GLuint program;
GLint mvp_loc, color_loc;

const int GROUP_SIZE = 100;
const int NO_OBJECTS = 3 * GROUP_SIZE;
std::vector<Object> objects;

glm::vec3 colors[3] = {glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)};

glm::mat4 view = glm::mat4(1.0f);
glm::mat4 proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

void generate_circle()
{
    vertices[0] = glm::vec3(0.0f, 0.0f, 0.0f);

    float d_theta = 2 * PI / circle_segments;

    for (int i = 0; i <= circle_segments; i++)
    {
        float x = radius * cosf(i * d_theta);
        float y = radius * sinf(i * d_theta);

        vertices[i + 1] = glm::vec3(x, y, 0.0f);
    }
}

typedef enum
{
    ROCK = 0,
    PAPER = 1,
    SCISSORS = 2
} Type;

static std::random_device rd;
static std::mt19937 gen(rd());
std::uniform_real_distribution<float> dist(0.0f, 1.0f);

struct Object
{
    glm::vec2 position;
    glm::vec2 velocity;
    Type type;

    Object(Type type_)
    {
        type = type_;
        float start_speed = dist(gen) / 50;  // [0, 0.02]
        float start_angle = dist(gen) * 2 * PI;
        velocity = glm::vec2(start_speed * cosf(start_angle), start_speed * sinf(start_angle));

        float width = 2.0f / 3.0f;
        float x = dist(gen) * width - 1.0f + type * width;
        float y = dist(gen) * 2 - 1;
        position = glm::vec2(x, y);
    }

    bool is_colliding(const Object& other)
    {
        glm::vec2 delta = position - other.position;
        float dist2 = glm::dot(delta, delta);
        return dist2 <= 4 * radius * radius;
    }

    bool is_winner(const Object& other)
    {
        Type o_type = other.type;
        if (type == o_type) return true;  // tie

        if ((type == Type::ROCK && o_type == Type::SCISSORS) || (type == Type::PAPER && o_type == Type::ROCK) ||
            (type == Type::SCISSORS && o_type == Type::PAPER))
            return true;
        else
            return false;
    }

    void update_object(float dt)
    {
        position += velocity;

        if (position.x - radius <= -1.0)
        {
            // Snap to the left wall
            position.x = -1.0 + radius;
            velocity.x = -velocity.x;
        }
        else if (position.x + radius >= 1.0)
        {
            // Snap to the right wall
            position.x = 1.0 - radius;
            velocity.x = -velocity.x;
        }

        if (position.y - radius <= -1.0)
        {
            // Snap to the bottom
            position.y = -1.0 + radius;
            velocity.y = -velocity.y;
        }
        else if (position.y + radius >= 1.0)
        {
            // Snap to the ceiling
            position.y = 1.0 - radius;
            velocity.y = -velocity.y;
        }
    }

    void display_object()
    {
        glm::vec3 color;
        color = colors[type];

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));

        glm::mat4 mvp = proj * view * model;

        glUniformMatrix4fv(mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform3fv(color_loc, 1, glm::value_ptr(color));

        glDrawArrays(GL_TRIANGLE_FAN, 0, num_vertices);
    }
};

void init()
{
    program = InitShader("../vshader.glsl", "../fshader.glsl");
    glUseProgram(program);

    generate_circle();

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLuint loc = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

    mvp_loc = glGetUniformLocation(program, "MVP");
    color_loc = glGetUniformLocation(program, "objectColor");

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    objects.reserve(NO_OBJECTS);
    for (int i = 0; i < GROUP_SIZE; i++)
    {
        objects.push_back(Object(Type::ROCK));
        objects.push_back(Object(Type::PAPER));
        objects.push_back(Object(Type::SCISSORS));
    }
}

void resolve_collisions()
{
    for (auto it1 = objects.begin(); it1 != objects.end(); it1++)
    {
        for (auto it2 = it1 + 1; it2 != objects.end(); it2++)
        {
            if (it1->is_colliding(*it2))
            {
                glm::vec2 delta = it1->position - it2->position;
                float dist = glm::length(delta);

                // Edge case: if they spawn exactly on top of each other
                if (dist == 0.0f)
                {
                    delta = glm::vec2(1.0f, 0.0f);
                    dist = 1.0f;
                }

                float overlap = (radius * 2.0f) - dist;
                glm::vec2 direction = delta / dist;

                // Push them apart
                it1->position += direction * (overlap * 0.5f + 0.001f);
                it2->position -= direction * (overlap * 0.5f + 0.001f);

                // Calculate the momentum
                glm::vec2 temp = it1->position;
                it1->position = it2->position;
                it2->position = temp;

                //  Resolve Rock-Paper-Scissors logic
                if (it1->is_winner(*it2))
                {
                    it2->type = it1->type;
                }
                else if (it2->is_winner(*it1))
                {
                    it1->type = it2->type;
                }
            }
        }
    }
}

void update(float dt)
{
    int simulation_speed = 1;

    // Run the physics/collision logic multiple times per frame
    for (int step = 0; step < simulation_speed; step++)
    {
        for (auto& obj : objects)
        {
            obj.update_object(dt);
        }

        resolve_collisions();
    }
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (auto& object : objects)
    {
        object.display_object();
    }

    glFinish();
}

int main()
{
    if (!glfwInit()) exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(512, 512, "rps", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    init();

    double frameRate = 30, currentTime, previousTime = 0.0;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        currentTime = glfwGetTime();
        float dt = currentTime - previousTime;
        if (dt >= 1 / frameRate)
        {
            previousTime = currentTime;
            update(dt);
        }

        display();
        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}