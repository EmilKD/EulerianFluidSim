
#ifndef SHADER_H
#define SHADER_H

#include<glad/glad.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<string>
#include<istream>
#include<fstream>
#include<sstream>
#include<iostream>

using std::string;

class Shader
{
public:
	unsigned int ID;
	Shader();
	void use();
	//void CreateTexture(const char* filePath);
	void setBool(const string &name, bool value) const;
	void setInt(const string& name, int value) const;
	void setFloat(const string& name, float value) const;
	void set3fv(const string& name, glm::vec3 value) const;

	unsigned int texture{};

private:
	void checkCompileErrors(unsigned int shader, string type);
};

#endif // !H_SHADER