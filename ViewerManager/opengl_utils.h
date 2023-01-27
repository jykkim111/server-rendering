#pragma once

#include "Mgr.h"
#include "global.h"

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <QWidget>
#include <QMessageBox>

class VIEWERMANAGER_DLL VAO{
public:
	VAO() {};
	~VAO() {
		glDisableVertexAttribArray(0);
		glDeleteBuffers(1, &(associatedVBOID));
	};

    unsigned int ID;
    unsigned int associatedVBOID;
    unsigned int associatedEBOID;
    unsigned int vertexNumber;

	inline void deleteVertex() {
		glDisableVertexAttribArray(0);
		glDeleteBuffers(1, &(associatedVBOID));
	}

	inline void setVertex(const std::vector<float>& attrib_data)
	{
		//unsigned int VBO;
		glGenVertexArrays(1, &ID);
		glGenBuffers(1, &(associatedVBOID));
		// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
		glBindVertexArray(ID);

		glBindBuffer(GL_ARRAY_BUFFER, associatedVBOID);
		glBufferData(GL_ARRAY_BUFFER, attrib_data.size() * sizeof(GLfloat), &attrib_data[0], GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);

		GLenum err = glGetError();
		if (err == GL_OUT_OF_MEMORY) {
			throw cgip::CgipOpenGLOOMException();
		}

		/*
		if (indices_data.size() > 0) {
			glGenBuffers(1, &(vao->associatedEBOID));
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao->associatedEBOID);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_data.size() * sizeof(unsigned int), &indices_data[0], GL_STATIC_DRAW);
		}
		unsigned int offset = 0;
		unsigned int index = 0;
		unsigned int size_sum = 0;
		for(auto const& value: attrib_sizes){
			size_sum += value;
		}

		vao->vertexNumber = attrib_data.size() / size_sum;
		if (indices_data.size() > 0) {
			vao->vertexNumber = indices_data.size();
		}

		for(auto const& value: attrib_sizes){
			glVertexAttribPointer(index, value, GL_FLOAT, GL_FALSE, size_sum * sizeof(float), (void*)(offset * sizeof(float)));
			glEnableVertexAttribArray(index);
			index++;
			offset += value;
		}
		*/
	}
};

class VIEWERMANAGER_DLL Texture {
public:
	unsigned int ID;
	int channels;
	Texture() {}
	~Texture(){ glDeleteTextures(1, &ID); }
	Texture(unsigned short *data, int width, int height, int depth, unsigned int allocate_id=1)
	{
		ID = allocate_id;
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_3D, ID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (data)
		{
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, width, height, depth, 0, GL_RED, GL_UNSIGNED_SHORT, data);
			//glGenerateMipmap(GL_TEXTURE_3D);
		}

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// set border color
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, (0, 0, 0));
		glBindTexture(GL_TEXTURE_3D, 0);

		GLenum err = glGetError();
		if (err == GL_OUT_OF_MEMORY) {
			throw cgip::CgipOpenGLOOMException();
		}
	}
	Texture(unsigned char* data, int width, int height, int depth, unsigned int allocate_id = 1)
	{
		ID = allocate_id;
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_3D, ID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (data)
		{
			//glGenerateMipmap(GL_TEXTURE_3D);
			glTexImage3D(GL_TEXTURE_3D, 0, GL_R8UI, width, height, depth, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, data);
		}

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// set border color
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, (0, 0, 0));
		glBindTexture(GL_TEXTURE_3D, 0);

		GLenum err = glGetError();
		if (err == GL_OUT_OF_MEMORY) {
			throw cgip::CgipOpenGLOOMException();
		}
	}
	Texture(short *data, int length, unsigned int allocate_id = 1)
	{
		ID = allocate_id;
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_1D, ID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		if (data)
		{
			glTexImage1D(GL_TEXTURE_1D, 0, GL_R16, length, 0, GL_RED, GL_SHORT, data);
			//glGenerateMipmap(GL_TEXTURE_3D);
		}

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_1D, 0);

		GLenum err = glGetError();
		if (err == GL_OUT_OF_MEMORY) {
			throw cgip::CgipOpenGLOOMException();
		}
	}


	Texture(TF_BGRA* data, int width, unsigned int allocate_id = 1)
	{
		ID = allocate_id;
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_1D, ID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		if (data)
		{
			glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, width, 0, GL_BGRA, GL_FLOAT, data);
			//glGenerateMipmap(GL_TEXTURE_3D);
		}

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_1D, 0);

		GLenum err = glGetError();
		if (err == GL_OUT_OF_MEMORY) {
			throw cgip::CgipOpenGLOOMException();
		}
	}

	Texture(CPR_COORD* data, int width, unsigned int allocate_id = 1)
	{
		ID = allocate_id;
		glGenTextures(1, &ID);
		glBindTexture(GL_TEXTURE_1D, ID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		if (data)
		{
			glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB32F, width, 0, GL_RGB, GL_FLOAT, data);
			//glGenerateMipmap(GL_TEXTURE_3D);
		}

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_1D, 0);

		GLenum err = glGetError();
		if (err == GL_OUT_OF_MEMORY) {
			throw cgip::CgipOpenGLOOMException();
		}
	}
};

class VIEWERMANAGER_DLL Shader
{
public:
	unsigned int ID;
	Shader() {}
	// constructor generates the shader on the fly
	// ------------------------------------------------------------------------
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr)
	{
		// 1. retrieve the vertex/fragment source code from filePath
		std::string vertexCode;
		std::string fragmentCode;
		std::string geometryCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream gShaderFile;
		// ensure ifstream objects can throw exceptions:
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			std::stringstream vShaderStream, fShaderStream;
			// read file's buffer contents into streams
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			// convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
			// if geometry shader path is present, also load a geometry shader
			if (geometryPath != nullptr)
			{
				gShaderFile.open(geometryPath);
				std::stringstream gShaderStream;
				gShaderStream << gShaderFile.rdbuf();
				gShaderFile.close();
				geometryCode = gShaderStream.str();
			}
		}
		catch (std::ifstream::failure e)
		{
			throw cgip::CgipOpenGLException("GLError: no shader");
		}
		const char* vShaderCode = vertexCode.c_str();
		const char * fShaderCode = fragmentCode.c_str();
		// 2. compile shaders
		unsigned int vertex, fragment;
		// vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");
		// fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");
		// if geometry shader is given, compile geometry shader
		unsigned int geometry;
		if (geometryPath != nullptr)
		{
			const char * gShaderCode = geometryCode.c_str();
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &gShaderCode, NULL);
			glCompileShader(geometry);
			checkCompileErrors(geometry, "GEOMETRY");
		}
		// shader Program
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		if (geometryPath != nullptr)
			glAttachShader(ID, geometry);
		glLinkProgram(ID);
		checkCompileErrors(ID, "PROGRAM");
		// delete the shaders as they're linked into our program now and no longer necessery
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		if (geometryPath != nullptr)
			glDeleteShader(geometry);

		GLenum err = glGetError();
		if (err == GL_OUT_OF_MEMORY) {
			throw cgip::CgipOpenGLOOMException();
		}

	}
	// activate the shader
	// ------------------------------------------------------------------------
	void use()
	{
		glUseProgram(ID);
	}
	// utility uniform functions
	// ------------------------------------------------------------------------
	void setBool(const std::string &name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	// ------------------------------------------------------------------------
	void setInt(const std::string &name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setInt(const std::string& name, int datasize, const int value[]) const
	{
		glUniform1iv(glGetUniformLocation(ID, name.c_str()), datasize, value);
	}
	// ------------------------------------------------------------------------
	void setFloat(const std::string &name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	// ------------------------------------------------------------------------
	void setVec2(const std::string &name, const glm::vec2 &value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string &name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	// ------------------------------------------------------------------------
	void setVec3(const std::string &name, const glm::vec3 &value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, const int data_size, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), data_size, &value[0]);
	}
	void setVec3(const std::string &name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	// ------------------------------------------------------------------------
	void setVec4(const std::string &name, const glm::vec4 &value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string& name, const int data_size, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), data_size, &value[0]);
	}
	void setVec4(const std::string &name, float x, float y, float z, float w)
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	// ------------------------------------------------------------------------
	void setMat2(const std::string &name, const glm::mat2 &mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat3(const std::string &name, const glm::mat3 &mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	// ------------------------------------------------------------------------
	void setMat4(const std::string &name, const glm::mat4 &mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

private:
	// utility function for checking shader compilation/linking errors.
	// ------------------------------------------------------------------------
	void checkCompileErrors(GLuint shader, std::string type)
	{
		GLint success;
		GLchar infoLog[1024];
		if (type != "PROGRAM")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				//std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				QMessageBox shader_error_box;
				shader_error_box.setText("ERROR: SHADER_COMPILATON_ERROR : " + QString(infoLog) + "\nPlease check your graphics driver and see if OpenGL is updated to its most recent version...");
				shader_error_box.exec();
				glDeleteShader(shader);
				throw cgip::CgipOpenGLException("ERROR::SHADER_COMPILATION_ERROR of type");
				

			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);

				QMessageBox shader_error_box;
				shader_error_box.setText("ERROR: PROGRAM_LINKING_ERROR: " + QString(infoLog));
				shader_error_box.exec();

				//std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				glDeleteShader(shader);
				throw cgip::CgipOpenGLException("ERROR::PROGRAM_LINKING_ERROR of type");
			}
		}
	}
};
