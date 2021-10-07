/*
*
* Ray tracer used in the integration of the radiance transfer function
*
*
* Forms part of the DxPRT project
*
*
*
* Author: Shaun Bailey
*
* Date created: 03/10/2021
*
* Last Updated: 03/10/2021
*
*
*
* MIT License
*
* Copyright (c) 2021 Shaun Bailey
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/

static const float PI = 3.14159265f;
static const uint numThreadsX = 8;

StructuredBuffer<float3> vertexBuffer : register(t0);
StructuredBuffer<uint> indexBuffer : register(t1);
RWBuffer<uint> planeBuffer : register(u0);
RWBuffer<uint> randomBuffer : register(u1);
RWBuffer<uint> visibilityBuffer : register(u2);

struct RAYSETTINGS 
{
	uint numEventsX;
	uint numPlaneChunks;
	uint pad1;
	uint pad2;
};

struct RAYDATA 
{
	float3 rayPos;
	float3 forward;
	float3 xDir;
	RAYSETTINGS settings;
};

ConstantBuffer<RAYDATA> ray : register(b0);

float2 GenerateRandomNumbers(uint index);
uint Tausworthe(uint index, uint S1, uint S2, uint S3, uint M);
uint LCG(uint index, uint A, uint B);

/*
* 
* Main function. Checks if the ray detects a hit on any triangle that passes
* the pre-pass (see RayTracerPrePassShader.hlsl).
* 
*/

[numthreads(numThreadsX, numThreadsX, 1)]
void main(uint3 threadInDispatch : SV_DispatchThreadID)
{

	float2 randomNums = GenerateRandomNumbers((threadInDispatch.x * ray.settings.numEventsX
		+ threadInDispatch.y) * 8); // 8 integers used for random number generator

	float theta = acos(sqrt(1 - randomNums.x));
	float phi = randomNums.y * (PI * 2.0f);

	float3 forward = normalize(ray.forward);
	float3 xDir = normalize(ray.xDir);
	float3 yDir = cross(xDir, forward);

	float3 rayDir = sin(theta) * cos(phi) * xDir + sin(phi) * sin(theta) * yDir
		+ cos(theta) * forward;

	uint visibility = 1;

	uint bufferIndex = threadInDispatch.x * ray.settings.numEventsX + threadInDispatch.y;

	// plane buffer is split into chunks, loop over each chunk
	for (uint iPlaneChunk = 0; iPlaneChunk < ray.settings.numPlaneChunks; ++iPlaneChunk) {
		uint numPlanesInChunk = planeBuffer[iPlaneChunk]; // first few elements detail the number of planes in each chunk
		for (uint i = 0; i < numPlanesInChunk; ++i) {

			uint iPlane = planeBuffer[ray.settings.numPlaneChunks +
				iPlaneChunk * 512 + i]; // each chunk contains 512 elements

			float3 vector1 = vertexBuffer[indexBuffer[3 * iPlane]] - ray.rayPos;
			float3 vector2 = vertexBuffer[indexBuffer[3 * iPlane + 1]] - ray.rayPos;
			float3 vector3 = vertexBuffer[indexBuffer[3 * iPlane + 2]] - ray.rayPos;

			float projectedArea1 = dot(cross(vector1, vector2), rayDir);
			float projectedArea2 = dot(cross(vector2, vector3), rayDir);
			float projectedArea3 = dot(cross(vector3, vector1), rayDir);

			if (projectedArea1 < 0.0f && 
				projectedArea2 < 0.0f &&
				projectedArea3 < 0.0f &&
				dot(vector1, rayDir) > 0.0f) {
				visibility = 0;
			}
		}
	}

	visibilityBuffer[bufferIndex] = visibility;
}


/* Pseudo-random number generator recommened in GPU Gems 3
*  chapter 37. A combination of a simple linear congruential
* generator and a combined tausworthe generator
*/ 

float2 GenerateRandomNumbers(uint index) {
	for (int i = 0; i < 2; ++i) {
		randomBuffer[index + 4 * i] = Tausworthe(index + 4 * i,
			13, 19, 12, 4294967294ul);
		randomBuffer[index + 4 * i + 1] = Tausworthe(index + 4 * i + 1,
			2, 25, 4, 4294967288ul);
		randomBuffer[index + 4 * i + 2] = Tausworthe(index + 4 * i + 2,
			3, 11, 17, 4294967280ul);
		randomBuffer[index + 4 * i + 3] = LCG(index + 4 * i + 3,
			1664525, 1013904223ul);
	}
	float2 res;
	res.x = float(randomBuffer[index] ^ randomBuffer[index + 1]
		^ randomBuffer[index + 2] ^ randomBuffer[index + 3]) / float(pow(2, 32));
	res.y = float(randomBuffer[index + 4] ^ randomBuffer[index + 5]
		^ randomBuffer[index + 6] ^ randomBuffer[index + 7]) / float(pow(2, 32));
	return res;
}

uint Tausworthe(uint index, uint S1, uint S2, uint S3, uint M) {
	uint z = randomBuffer[index];
	uint b = (((z << S1) ^ z) >> S2);
	return (((z & M) << S3) ^ b);
}

uint LCG(uint index, uint A, uint B) {
	return A * randomBuffer[index] + B;
}


