/***************************************************************************
 * S.G.M.P 
 * @file: sphere.hpp
 * @brief:
 * @author: Mayur S. Tadekar(mrtadekar@gmail.com)
 * @date: Friday 2026-10-02 0:03:23 AM
 ***************************************************************************/

#ifndef _SPHERE_H
#define _SPHERE_H

#include <vector>

class Sphere
{
public:
	Sphere();
	~Sphere();

	size_t GetVerticesSize() const { return(sphereVertices.size()); }
	size_t GetColorsSize() const { return(sphereColors.size()); }
	size_t GetTexCoordsSize() const { return(sphereTexCoords.size()); }
	size_t GetNormalsSize() const { return(sphereNormals.size()); }
	size_t GetIndicesSize() const { return(sphereIndices.size()); }

	float* GetVerticesData() { return(sphereVertices.data()); }
	float* GetColorsData() { return(sphereColors.data()); }
	float* GetTexCoordsData() { return(sphereTexCoords.data()); }
	float* GetNormalsData() { return(sphereNormals.data()); }
	uint* GetIndicesData() { return(sphereIndices.data()); }

private:

	// Sphere parameters
	float radius;
	uint stacks;
	uint slices;
	uint indexCount;

	std::vector<float> sphereVertices;
	std::vector<float> sphereTexCoords;
	std::vector<float> sphereColors;
	std::vector<float> sphereNormals;
	std::vector<uint> sphereIndices;

	void CreateSphere();
};

Sphere::Sphere()
	: radius(2.0f), stacks(36), slices(36), indexCount(0)
{
	// Initialize lighting parameters
	CreateSphere();
}

Sphere::~Sphere()
{
	// Code
	sphereVertices.clear();
	sphereTexCoords.clear();
	sphereNormals.clear();
	sphereIndices.clear();
}

void Sphere::CreateSphere()
{
	// Clear previous data
	sphereVertices.clear();
	sphereTexCoords.clear();
	sphereNormals.clear();
	sphereIndices.clear();

	// Generate vertices, normals and colors
	for (uint ole = 0; ole <= stacks; ++ole)
	{
		float phi = (float)ole * M_PI / (float)stacks; // angle from the top

		for (uint ile = 0; ile <= slices; ++ile)
		{
			float theta = (float)ile * 2.0f * M_PI / (float)slices; // angle around the sphere

			float x = sinf(phi) * cosf(theta);
			float y = cosf(phi);
			float z = sinf(phi) * sinf(theta);

			float s = (float)ile / (float)slices;
			float t = (float)ole / (float)stacks;

			sphereVertices.push_back(radius * x);
			sphereVertices.push_back(radius * y);
			sphereVertices.push_back(radius * z);

			sphereColors.push_back(1.0f);
			sphereColors.push_back(1.0f);
			sphereColors.push_back(1.0f);

			// Texture coordinates
			sphereTexCoords.push_back(s);
			sphereTexCoords.push_back(t);

			// Normal is the same as vertex for a sphere
			sphereNormals.push_back(x);
			sphereNormals.push_back(y);
			sphereNormals.push_back(z);
		}
	}

	// Generate indices
	for (uint i = 0; i < stacks; ++i)
	{
		uint k1 = i * (slices + 1);
		uint k2 = k1 + slices + 1;

		for (uint j = 0; j < slices; ++j, ++k1, ++k2)
		{
			// For triangle 1
			if (i != 0)
			{
				sphereIndices.push_back(k1);
				sphereIndices.push_back(k2);
				sphereIndices.push_back(k1 + 1);
			}

			// For triangle 2
			if (i != (stacks - 1))
			{
				sphereIndices.push_back(k1 + 1);
				sphereIndices.push_back(k2);
				sphereIndices.push_back(k2 + 1);
			}
		}
	}

	indexCount = sphereIndices.size();
}



#endif /* _SPHERE_H */