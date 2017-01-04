#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>

//#include <OpenGL/gl3.h>

// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>
// Include GLFW
#include <GLFW/glfw3.h>
// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "shader.h"
#include "model_data.h"

/*
http://www.opengl-tutorial.org/beginners-tutorials/tutorial-1-opening-a-window/
http://www.opengl-tutorial.org/beginners-tutorials/tutorial-2-the-first-triangle/
http://www.opengl-tutorial.org/beginners-tutorials/tutorial-3-matrices/
http://www.opengl-tutorial.org/beginners-tutorials/tutorial-4-a-colored-cube/
http://stackoverflow.com/questions/23450334/opengl-3-3-4-1-on-mac-osx-10-9-using-glfw-library
*/

// This will identify our vertex buffer
GLuint vertexbuffer;
GLuint colorbuffer;
GLuint programID;
GLuint mvp_handle;
int counter = 0;

vec3 position = vec3( 0, 0, 5 );
float horizontalAngle = 3.14f;
float verticalAngle = 0.0f;
float FoV = glm::radians(45.0f);
float speed = .1f; // 3 units / second
float mouseSpeed = 0.005f;
bool g_rotating_flag = false;
double lastTime = glfwGetTime();
double g_prev_mouse_x;
double g_prev_mouse_y;



void init(void)
{
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    glClearColor(0.0, 0.0, 0.0, 1.0);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Give our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, get_vertex_buffer_count(), get_vertex_buffer_data(), GL_STATIC_DRAW);

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, get_color_buffer_count(), get_color_buffer_data(), GL_STATIC_DRAW);

    programID = LoadShaders("vertex.shader", "fragment.shader");
}

void fn_fbresize(GLFWwindow *window, int width, int height)
{
    //std::cout << "fbresize" << std::endl;
    // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    /*glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float) width / (float)height, 0.1f, 100.0f);
      
    // Or, for an ortho camera :
    //glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates
      
    // Camera matrix
    glm::mat4 View = glm::lookAt(
        glm::vec3(4,3,3), // Camera is at (4,3,3), in World Space
        glm::vec3(0,0,0), // and looks at the origin
        glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    );*/

    // Direction : Spherical coordinates to Cartesian coordinates conversion
    glm::vec3 direction(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle)
    );
    // Right vector
    glm::vec3 right = glm::vec3(
        sin(horizontalAngle - 3.14f/2.0f),
        0,
        cos(horizontalAngle - 3.14f/2.0f)
    );
    // Up vector : perpendicular to both direction and right
    glm::vec3 up = glm::cross( right, direction );

    // Projection matrix : 45&deg; Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
    mat4 Projection = glm::perspective(FoV, (float)width / (float)height, 0.1f, 100.0f);
    // Camera matrix
    mat4 View = glm::lookAt(
        position,           // Camera is here
        position + direction, // and looks here : at the same position, plus "direction"
        up                  // Head is up (set to 0,-1,0 to look upside-down)
    );

    // Model matrix : an identity matrix (model will be at the origin)
    glm::mat4 Model = glm::mat4(1.0f);
    Model = glm::rotate(Model, (float)(counter * 180.0f / M_PI / 360.0f), glm::vec3(0, 1, 0));
    // Our ModelViewProjection : multiplication of our 3 matrices
    glm::mat4 mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
    // Get a handle for our "MVP" uniform
    // Only during the initialisation
    mvp_handle = glGetUniformLocation(programID, "MVP");
      
    // Send our transformation to the currently bound shader, in the "MVP" uniform
    // This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
    glUniformMatrix4fv(mvp_handle, 1, GL_FALSE, &mvp[0][0]);
}

void fn_refresh(GLFWwindow* window)
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use our shader
    glUseProgram(programID);

    // Update viewport
    fn_fbresize(window, width, height);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
        3,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );
    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glVertexAttribPointer(
        1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
        3,                                // size
        GL_FLOAT,                         // type
        GL_FALSE,                         // normalized?
        0,                                // stride
        (void*)0                          // array buffer offset
    );
    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles -> 6 squares
    //glDisableVertexAttribArray(1);
    //glDisableVertexAttribArray(0);

    //glFlush();
}

void fn_cursorpos(GLFWwindow *window, double x, double y) {
    if (g_rotating_flag) {
        // Compute new orientation
        horizontalAngle += mouseSpeed * float( x - g_prev_mouse_x );
        verticalAngle   += mouseSpeed * float( y - g_prev_mouse_y );
    }

    g_prev_mouse_x = x;
    g_prev_mouse_y = y;
}

void fn_mousebtn(GLFWwindow *window, int btn, int act, int mod) {
    if (btn == GLFW_MOUSE_BUTTON_1 && act == GLFW_PRESS) {
        g_rotating_flag = true;
        // setcapture or glfwSetInputMode and GLFW_CURSOR_DISABLED?
    } else {
        // GLFW_RELEASE
        // releasecapture
        g_rotating_flag = false;
    }
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }


        // Direction : Spherical coordinates to Cartesian coordinates conversion
        glm::vec3 direction(
            cos(verticalAngle) * sin(horizontalAngle),
            sin(verticalAngle),
            cos(verticalAngle) * cos(horizontalAngle)
        );
        // Right vector
        glm::vec3 right = glm::vec3(
            sin(horizontalAngle - 3.14f/2.0f),
            0,
            cos(horizontalAngle - 3.14f/2.0f)
        );

        // Move forward
        if (key == GLFW_KEY_UP && action == GLFW_PRESS){
            position += direction * speed;
            std::cout << "U" << std::endl;
        }
        // Move backward
        if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){
            position -= direction * speed;
            std::cout << "D" << std::endl;
        }
        // Strafe right
        if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS){
            position += right * speed;
            std::cout << "R" << std::endl;
        }
        // Strafe left
        if (key == GLFW_KEY_LEFT && action == GLFW_PRESS){
            position -= right * speed;
            std::cout << "L" << std::endl;
        }
    }
}


int main(int argc, char *argv[])
{
    // Initialise GLFW
    if ( !glfwInit() ) {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

    // Open a window and create its OpenGL context
    GLFWwindow* window; // (In the accompanying source code, this variable is global)
    window = glfwCreateWindow( 800, 600, "Tutorial 01", NULL, NULL);
    if ( window == NULL ) {
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Initialize GLEW
    glfwSwapInterval(1);
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetWindowRefreshCallback(window, fn_refresh);
    glfwSetFramebufferSizeCallback(window, fn_fbresize);
    glfwSetMouseButtonCallback(window, fn_mousebtn);
    glfwSetCursorPosCallback(window, fn_cursorpos);
    glfwSetKeyCallback(window, key_callback);
    init();

    do {
        // Draw nothing, see you in tutorial 2 !
        fn_refresh(window);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        //counter++;

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0 );

    glfwTerminate();
    return 0;
}

