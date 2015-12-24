#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>

typedef unsigned int uint;

/** Mesh class with the sole purpose of converting obj files to opengl element arrays.
 *  That's what it's designed for.
 */
class Mesh {
	std::vector<float> vertices;
	std::vector<float> texCoord;
	std::vector<float> vertNorm;
	std::vector<uint> faces;
public:
	int vertCount, texCoordCount, normCount, faceCount;
	Mesh ();
	Mesh (std::string filename);
	void getVertexBufferArray(std::vector<GLfloat> &v, uint &stride, uint &colorOffset, uint &texOffset, uint &normalOffset);
	void getElementBufferArray(std::vector<GLuint> &v);
} mesh;

Mesh::Mesh() {

}

Mesh::Mesh(const std::string filename) {
	vertCount = texCoordCount = normCount = faceCount = 0;
	std::string line, word;
	std::ifstream objfile(filename.c_str());
	if (objfile.is_open()) {
		std::vector<float> tempFaces;
		std::vector<float> *current;
		while (getline(objfile, line)) {
			std::string::size_type comment = line.find('#');
			if (comment != std::string::npos)
				line = line.substr(0,comment);
			if (line.size() < 2)
				continue;
			if (line.at(0) == 'u')
				continue;
			std::stringstream stream(line);
			for (uint i=0; stream >> word; i++) {
				switch (word.at(0)) {
					case 'v':
						if (word.size() == 1) {
							current = &vertices;
							vertCount++;
						}
						else {
							switch (word.at(1)) {
								case 't':
									current = &texCoord;
									texCoordCount++;
									break;
								case 'n':
									current = &vertNorm;
									normCount++;
									break;
							}
						}
						break;
					case 'f':
						current = &tempFaces;
						faceCount++;
						break;
					case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':case '-':
						if (current == &tempFaces) {
							std::string::size_type slash = word.find_first_of("/");
							if (slash == std::string::npos) {
								faces.push_back(std::atoi(word.c_str()));
								faces.push_back(0);
								faces.push_back(0);
							} else {
								faces.push_back(std::atoi(word.substr(0, slash).c_str()));
								word = word.substr(slash+1);
								slash = word.find_first_of("/");
								if (slash == std::string::npos) {
									faces.push_back(std::atoi(word.c_str()));
									faces.push_back(0);
								} else {
									if (slash == 0) {
										faces.push_back(0);
										faces.push_back(std::atoi(word.substr(1).c_str()));
									} else {
										// faces.push_back(std::atoi(word.substr(0, slash).c_str()));
										faces.push_back(0);
										faces.push_back(std::atoi(word.substr(slash+1).c_str()));
									}
								}
							}
						} else
							current->push_back(std::atof(word.c_str()));
						break;
				}
			}
		}
	}
}

void Mesh::getVertexBufferArray(std::vector<GLfloat> &v, uint &stride, uint &colorOffset, uint &texOffset, uint &normalOffset) {
	uint vertSize = vertCount == 0 ? 0 : vertices.size()/vertCount;
	uint texSize = texCoordCount == 0 ? 0 : texCoord.size()/texCoordCount;
	uint normSize = normCount == 0 ? 0 : vertNorm.size()/normCount;
	
	for (uint i = 0; i < faces.size(); i+=3) {
		for (uint j = 0; j < vertSize; j++) {
			v.push_back(vertices[(faces[i]-1)*vertSize + j]);
		}
		if (faces[i+1] != 0) {
			for (uint j = 0; j < texSize; j++) {
				v.push_back(texCoord[(faces[i+1]-1)*texSize + j]);
			}
		}
		if (faces[i+2] != 0) {
			for (uint j = 0; j < normSize; j++) {
				v.push_back(vertNorm[(faces[i+2]-1)*normSize + j]);
			}
		}
	}
	stride = vertSize + texSize + normSize;
	colorOffset = vertSize > 4 ? vertSize - 3 : 0;
	texOffset = vertSize;
	normalOffset = vertSize + texSize;
}

void Mesh::getElementBufferArray(std::vector<GLuint> &v) {
	for (GLuint i = 0; i < faces.size()/3; i++) {
		v.push_back(i);
	}
}
