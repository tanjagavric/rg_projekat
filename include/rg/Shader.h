//
// Created by tanja on 31.3.24..
//

#ifndef PROJECT_BASE_SHADER_H
#define PROJECT_BASE_SHADER_H

#include <string>
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/detail/type_mat4x4.hpp>
#include "rg/Error.h"


std::string readFileContents(std::string path) {
    std::ifstream in(path);
    std::stringstream buffer;
    /*
    std::string line;
    while (std::getline(in, line)) {
        buffer << line << "\n";
    }
    */
    buffer << in.rdbuf();
    return buffer.str();
}

class Shader {
    unsigned int m_Id;
public:
    Shader(std::string vertexShaderPath, std::string fragmentShaderPath) {
        //vertex shader
        std::string vsString = readFileContents(vertexShaderPath);
        ASSERT(!vsString.empty(), "vs source je prazan");
        const char* vertexShaderSource = vsString.c_str();
        unsigned vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);

        int success = 0;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
            std::cout<<"greska pri kompajliranju vertex sejdera: "<<infoLog<<"\n";
        }

        //fragment shader
        std::string fsString = readFileContents(fragmentShaderPath);
        ASSERT(!fsString.empty(), "fs source je prazan");
        const char* fragmentShaderSource = fsString.c_str();
        unsigned fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
        glCompileShader(fragmentShader);

        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
            std::cout<<"greska pri kompajliranju fragment sejdera: "<<infoLog<<"\n";
        }

        //program
        unsigned shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            std::cout<<"greska pri linkovanju sejder programa: "<<infoLog<<"\n";
        }

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        m_Id = shaderProgram;
    }

    void use() {
        ASSERT(m_Id>0, "obrisan je program");
        glUseProgram(m_Id);
    }

    void setUniform4f(std:: string name, float x, float y, float z, float w) {
        int uniformId = glGetUniformLocation(m_Id, name.c_str());
        glUniform4f(uniformId, x, y,z, w);

    }

    void setUniform1i(std::string name, int value) {
        int uniformId = glGetUniformLocation(m_Id, name.c_str());
        glUniform1i(uniformId, value);
    }

    void deleteProgram() {
        glDeleteProgram(m_Id);
        m_Id=0;
    }

    void setUniform1f(std::string name, float value) {
        int uniformId = glGetUniformLocation(m_Id, name.c_str());
        glUniform1f(uniformId, value);
    }

    unsigned int getMId() const {
        return m_Id;
    }

    void setMat4(std::string name, glm::mat4 &mat) {
        int uniformId = glGetUniformLocation(m_Id, name.c_str());
        glUniformMatrix4fv(uniformId,1, GL_FALSE,&mat[0][0]);
    }
};

#endif //PROJECT_BASE_SHADER_H
