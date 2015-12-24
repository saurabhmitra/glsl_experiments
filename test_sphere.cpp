#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SOIL/SOIL.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include "obj.cpp"

// Shader sources
const GLchar* vertexSource =
    "#version 150 core\n"
    "in vec3 position;"
    "in vec3 normal;"
    "out vec3 Color;"
    "out vec3 Normal;"
    "uniform mat4 model;"
    "uniform mat4 view;"
    "uniform mat4 proj;"
    "void main() {"
    "	vec3 yolo = noise3(position)/2 + .5;"
    "   Color = yolo;"
    "	Normal = vec3(proj*view*model*vec4(normal, 0.0));"
    "   gl_Position = proj*view*model*vec4(position, 1.0);"
    "}";
const GLchar* fragmentSource =
    "#version 150 core\n"
    "in vec3 Color;"
    "in vec3 Normal;"
    "out vec4 outColor;"
    "uniform vec3 light1;"
    "uniform vec3 light2;"
    "void main() {"
    "	float yolo = max(dot(normalize(light1), Normal), 0);"
    "	float yolo2 = max(dot(normalize(light2), Normal), 0)*.5;"
    "   outColor = vec4((yolo*.6 + yolo2*.5 + .05)*Color, 1.0);"
    // "   outColor = vec4(noise3(Normal)/2.0 + .5, 1.0);"
    "}";

const int xres = 1920;
const int yres = 1080;

int main()
{
	sf::ContextSettings settings;
	settings.depthBits = 24;
	settings.stencilBits = 8;
	settings.antialiasingLevel = 8; // Optional

	sf::Window window(sf::VideoMode(xres, yres), "OpenGL", sf::Style::Fullscreen, settings);
	glewExperimental = GL_TRUE;
	glewInit();
	bool running = true;

	Mesh coob("sphere/sphere.obj");
	std::vector<GLfloat> verts;
	std::vector<GLuint> elems;
	uint stride, colorOffset, texOffset, normOffset;
	coob.getVertexBufferArray(verts, stride, colorOffset, texOffset, normOffset);
	coob.getElementBufferArray(elems);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*verts.size(), verts.data(), GL_STATIC_DRAW);

	GLuint ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*elems.size(), elems.data(), GL_STATIC_DRAW);

	// Vertex Shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	// Fragment Shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// Combine that stuff into a shader program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glBindFragDataLocation(shaderProgram, 0, "outColor"); // not needed for this program
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

    // Specify the layout of the vertex data
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);

    // Specify the layout of the vertex data
    GLint normAttrib = glGetAttribLocation(shaderProgram, "normal");
    glEnableVertexAttribArray(normAttrib);
    glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*) (3* sizeof(GLfloat)));

    glm::mat4 model, view, proj;
    GLint uniModel = glGetUniformLocation(shaderProgram, "model");
	GLint uniView = glGetUniformLocation(shaderProgram, "view");
	GLint uniProj = glGetUniformLocation(shaderProgram, "proj");

	GLint uniLight1 = glGetUniformLocation(shaderProgram, "light1");
    glUniform3f(uniLight1, 1.0f, 1.0f, -1.0f);
	GLint uniLight2 = glGetUniformLocation(shaderProgram, "light2");
    glUniform3f(uniLight2, -1.0f, 0.0f, -1.0f);


	GLint status;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	char buffer[512];
	glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
	std::cout << "Status: " << status << " == " << GL_TRUE << "\nLog: " << buffer << std::endl;

    glEnable(GL_DEPTH_TEST);

    float count = 0;
	while (running)
	{
	    sf::Event windowEvent;
	    while (window.pollEvent(windowEvent))
	    {
	    	switch (windowEvent.type)
			{
				case sf::Event::Closed:
				    running = false;
				    break;
				case sf::Event::KeyPressed:
			    	if (windowEvent.key.code == sf::Keyboard::Escape)
			        	running = false;
			    break;
			}
	    }
	    sf::sleep(sf::milliseconds(10));
	    count += .02;
		model = glm::rotate(model, .01f, glm::vec3(cos(count * 1.0f), sin(count * 1.0f), 1.0f));
		glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));
		view = glm::lookAt(
		    glm::vec3(0.0f, 0.1f,-3.0f),
		    glm::vec3(0.0f, 0.0f, 1.0f),
		    glm::vec3(0.0f, 1.0f, 0.0f)
		);
		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));
		proj = glm::perspective(glm::radians(50.0f), xres / (float) yres, 0.0001f, 100.0f);
		// proj = glm::mat4();
		//std::cout << glm::to_string(proj) << std::endl;
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	    //glDrawArrays(GL_TRIANGLES, 0, 3);

        // Clear the screen to black
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	    glDrawElements(GL_TRIANGLES, 2160, GL_UNSIGNED_INT, 0);
	    // glDrawArrays(GL_TRIANGLES, 0, 36);
	    window.display();
	}
	// glDeleteTextures(1, &textures);

    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);

    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    // glDeleteFramebuffers(1, &frameBuffer);


    return 0;
}