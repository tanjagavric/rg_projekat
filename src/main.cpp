#include <iostream>

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <cmath>
#include "rg/Shader.h"
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//obavezno ovim redom se ukljucuje

void gl_clear_error() {
    while (glGetError() != GL_NO_ERROR) {}
}

const char* gl_error_to_string(GLenum error) {
    switch(error) {
        case GL_NO_ERROR: return "GL_NO_ERROR";
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        default: {
            std::cerr << "nema takve greske\n";
        }
    }
}
bool gl_log_call(const char *file, int line, const char* call) {
    bool success = true;
    while (GLenum error = glGetError()) {
        std::cerr << "[OpenGL Error] " << error << '\n' << "PORUKA: " << gl_error_to_string(error)<<"\n"
                  << "fajl: "<< file << "\nlinija: " << line << "\ncall: "<<call<<"\n";
        success = false;
    }
    return success;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void update(GLFWwindow *window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *pWwindow);

//glm::vec3 position; za jednostavan WASD
glm::vec3 cameraPos = glm::vec3(0,0,3);
glm::vec3 cameraFront = glm::vec3(0,0,-1);
glm::vec3 cameraUp = glm::vec3(0,1,0);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

bool firstMouse = true;
float yaw   = -90.0f;	// mora -90 stepeni jer 0.0 daje pravac udesno, pa ga malo rotiramo
float pitch =  0.0f;
float lastX =  800.0f / 2.0;
float lastY =  600.0 / 2.0; //stavlja ga na centar ekrana
float fov   =  45.0f;

glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

int main() {
    //inicijalizacija
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //pravimo prozor
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH,SCR_HEIGHT,"tanjin crtez", nullptr, nullptr);
    if (window == nullptr) {
        std::cout<<"nije napravljen prozor\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window); //ako je sve ok, kazemo da hocemo da crtamo ovdje
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //pravimo callback svaki put kad se promj velicina prozora da se i ovaj prostor za renderovanje promijeni
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    //ucitavamo sve glad fje za glfw
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "nisu se ucitale glad fje za glfw\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    //za dubinuuuuu
    glEnable(GL_DEPTH_TEST);

    Shader shader("resources/shaders/vertexShader.vs",
                   "resources/shaders/fragmentShader.fs");
    Shader shaderSvjetlost("resources/shaders/vsSvjetlost.vs",
                           "resources/shaders/fsSvjetlost.fs");
    /* TROUGAO:::
    float vertices[] = {                       //                      G
            -0.5f, -0.5f, 0.0f,    // lijeva              .5
            0.5f, -0.5f, 0.0f,     // desna        -.5     0  .5
            0.0f, 0.5f, 0.0f       // gore           L   -.5   D
    };
     */
    /* PRAVOUGAONIK::: sa 2 trougla i indeksima
    float vertices[] = {
            0.5f, 0.5f, 0.0f,1.0, 1.0, //     (3)   .5  (0)
            0.5f, -0.5f, 0.0f, 1.0, 0.0,//    -.5    0   .5
            -0.5f, -0.5f, 0.0f,0.0, 0.0, //    (2)  -.5  (1)
            -0.5f, 0.5f, 0.0f, 0.0, 1.0
    };

    //vertex0          vertex1          vertex2
    //x, y, z, ox, oy, x, y, z, ox, oy, x, y, z, ox, oy
    unsigned indices[] = {
            0,1,3,
            1,2,3
    };
    */
    /*
    //KOCKA::: sa teksturom
    float vertices[] = {
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };
     */
    //KOCKA bez teksture
    float vertices[] = {
            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,

            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,

            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,

            -0.5f, -0.5f, -0.5f,
            0.5f, -0.5f, -0.5f,
            0.5f, -0.5f,  0.5f,
            0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,

            -0.5f,  0.5f, -0.5f,
            0.5f,  0.5f, -0.5f,
            0.5f,  0.5f,  0.5f,
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
    };
    /*
    //VISE KOCKI
    glm::vec3 cubePositions[] = {
            glm::vec3( 0.0f,  0.0f,  0.0f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3( 1.3f, -2.0f, -2.5f),
            glm::vec3( 1.5f,  2.0f, -2.5f),
            glm::vec3( 1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    */
    //ovo moramo poslati na graficku karticu pa koristimo VBO
    unsigned int VBO, VAO, EBO;
    //Vertex Buffer Object salje na graficku
    //Vertex Array Object interpretira vrtekse
    //Element Buffer Object ovo se koristi sa indices
    glGenVertexArrays(1,&VAO); //on objasnjava sta upisujemo u VBO
    glBindVertexArray(VAO); //OBAVEZNO PRVO OVO AKTIVIRAJ

    glGenBuffers(1,&VBO); //generisemo objekat
    //glGenBuffers(1,&EBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //aktiviramo ga, navedemo koji je tip
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices), vertices, GL_STATIC_DRAW); //ubacim podatke u njega, tj poslali smo

    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); //obavezno aktiviraj posle VAO
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    //glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(0);
    //glEnableVertexAttribArray(1);

    //pravimo drugu kocku koja sija
    unsigned int svjetlostVAO;
    glGenVertexArrays(1,&svjetlostVAO);
    glBindVertexArray(svjetlostVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

/*
    unsigned int tex0;
    glGenTextures(1, &tex0);
    glBindTexture(GL_TEXTURE_2D, tex0);

    //wrap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //load img
    int width, height, nChannel;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("resources/textures/container.jpg",
                                    &width, &height, &nChannel, 0);

    if(data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,0,
                     GL_RGB, GL_UNSIGNED_BYTE, data); //treba sliku da nakacimo na ovaj objekat
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        ASSERT(false, "nije se ucitala tekstura");
    }
    stbi_image_free(data);

    unsigned int tex1;
    glGenTextures(1, &tex1);
    glBindTexture(GL_TEXTURE_2D, tex1);

    //wrap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    //filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    //load img
    data = stbi_load("resources/textures/awesomeface.png",
                     &width, &height, &nChannel, 0);

    if(data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,0,
                     GL_RGBA, GL_UNSIGNED_BYTE, data); //treba sliku da nakacimo na ovaj objekat
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        ASSERT(false, "nije se ucitala tekstura");
    }
    stbi_image_free(data);

    shader.use();
    shader.setUniform1i("t0", 0); //podesim sempler
    shader.setUniform1i("t1", 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0); //razvezemo
    glBindVertexArray(0);
*/
    //petlja
    glClearColor(0.3,0.4,0.5,1.0); //postavljamo boju pozadine na koju bafer cisti
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); moze i LINE ili POINT
    while (!glfwWindowShouldClose(window)) {
        //tecnost
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //ocisti baferE IMA IH 2
        /* TEKSTURE
        glActiveTexture(GL_TEXTURE0); //aktiviraj nultu teksturu
        glBindTexture(GL_TEXTURE_2D, tex0); //vezi tu teksturu za ovaj tex0

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, tex1);
        */
        //transformacije:
        //napravimo matricu na procesoru pa je posaljemo na graficku
        //skaliranje rotacija pa translacija

        //glm::mat4 model = glm::mat4(1.0f); ZA JEDNU KOCKU moze ovako, ali za njih vise nam trebaju posebne model matrice
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        //model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0, 0,0)); //rotiramo je oko x ose
        //model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.5f, 1.0f, 0.0f)); //ROTIRAMO KOCKU
        //view = glm::translate(view, glm::vec3(0.0f,0.0f,-7.0f+ sin(glfwGetTime()))); //malo odaljimo kameru
        projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f );

        //KAMERA:::
        //float radius = 7;
        //float camX = sin(glfwGetTime())*radius;
        //float camZ = cos(glfwGetTime())*radius;
        //view = glm::lookAt(glm::vec3(camX,0,camZ), glm::vec3(0,0,0), glm::vec3(0,1,0)); //rotira

        //KAMERA WASD
        view=glm::lookAt(cameraPos,cameraFront + cameraPos,cameraUp);

        //projection = glm::translate(projection, position); //ovo se ja igram sa WASD


        //glUseProgram(shaderProgram); //aktiviram program
        shader.use();
        //saljemo matricE na graficku:
        int viewLoc = glGetUniformLocation(shader.getMId(), "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        shader.setMat4("projection", projection);

        /*
        //VISE KOCKI:::
        for (int i = 0; i < 10; ++i) {
            glm::mat4 model = glm::mat4(1.0f);
            float angle = 20.0f * i;
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, glm::radians(angle),glm::vec3(1.0f,0.3f,0.5f));
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0,36);
        }
        */
        glm::mat4 model = glm::mat4(1.0f);
        shader.setMat4("model", model);

        //UNIFORM
        //glUniform4f(uniformId, sin(glfwGetTime()) / 2.0 + 0.5 , 0.0, 0.0, 1.0); //mora posle aktiviranja sejder programa
        //shader.setUniform4f("gColor", sin(glfwGetTime()) / 2.0 + 0.5 , 0.0, 0.0, 1.0);
        //shader.setUniform1f("p", sin(glfwGetTime()) /2.0 + 0.5);
        shader.set


        glBindVertexArray(VAO); //OVO JE DOVOLJNO DA SE AKTIVIRA
        glDrawArrays(GL_TRIANGLES, 0,36);


        //PODESAVANJE SVJETLOSTIIII
        shaderSvjetlost.use();
        shaderSvjetlost.setMat4("projection", projection);
        shaderSvjetlost.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f)); //manja kocka
        shaderSvjetlost.setMat4("model", model);

        glBindVertexArray(svjetlostVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        //glDrawArrays(GL_TRIANGLES, 0, 36); //ovdje se promijeni samo jos 6-pravougaonik, 36-kocka
        //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); ovo je poziv kad imas indices
        //update(window);
        glfwSwapBuffers(window); //render
    }
    glDeleteVertexArrays(1,&VAO);
    glDeleteVertexArrays(1,&svjetlostVAO);
    glDeleteBuffers(1, &VBO);
    //glDeleteBuffers(1, &EBO);
    //glDeleteProgram(shaderProgram);
    shader.deleteProgram();
    shaderSvjetlost.deleteProgram();
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow *window) {
    const float cameraSpeed=2.5 * deltaTime;
    if(glfwGetKey(window, GLFW_KEY_W)== GLFW_PRESS) {
        cameraPos += cameraFront * cameraSpeed;
    }
    if(glfwGetKey(window, GLFW_KEY_A)== GLFW_PRESS) {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if(glfwGetKey(window, GLFW_KEY_S)== GLFW_PRESS) {
        //cameraPos.y -=0.01;
        cameraPos -= cameraFront * cameraSpeed;

    }
    if(glfwGetKey(window, GLFW_KEY_D)== GLFW_PRESS) {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0,0,width,height); //ovdje renderujemo, postavimo koordinate gdje tacno renderujemo, x i y su donji lijevi ugao
}
//ovo se izvrsava nakon poll events fje
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_G && action == GLFW_PRESS) {
        std::cerr << "G\n";
        glClearColor(0,1,0,1.0);
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
        glClearColor(1,0,0,1.0);
    if (key == GLFW_KEY_B && action == GLFW_PRESS)
        glClearColor(0,0,1,1.0);
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        glClearColor(0.5,0,0.5,1.0);
}

void update(GLFWwindow *window) {
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front; //smjejr gledanja
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}
