/*
*
* Compute shader to integrate over the transfer functions and calculate the relevant
* spherical harmonic coefficients.
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


Texture2D<float> shBuffer: register(t0);
StructuredBuffer<uint> visibilityBuffer : register(t1);
StructuredBuffer<uint> randomBuffer : register(t2);
RWBuffer<float> resultBuffer : register(u0);
SamplerState LinearClampSampler : register(s0);

struct COORDS {
	float theta;
	float phi;
};

struct RAYSETTINGS
{
	uint numEventsX;
	uint numPlaneChunks;
	uint pad1;
	uint iCoefficient;
};

struct RAYDATA {
	float3 rayPos;
	float3 forward;
	float3 xDir;
	RAYSETTINGS settings;
};
ConstantBuffer<RAYDATA> ray : register(b0);

struct ComputeShaderInput
{
	uint3 groupInDispatch    : SV_GroupID;
	uint3 threadInGroup      : SV_GroupThreadID;
	uint3 threadInDispatch   : SV_DispatchThreadID;
	uint  threadInGroupIndex : SV_GroupIndex;
};

groupshared float threadOutput[64];

COORDS GetGlobalCoords(COORDS localCoords);
float2 GetRandomNumbers(uint index);

/*
* Main function, evaluates the product of the spherical harmonic and the 
* transfer function, taking into account the visibility function. This is 
* summed over the thread group. 
*/

[numthreads(8, 8, 1)]
void main( ComputeShaderInput IN )
{

	uint threadIndex = IN.threadInDispatch.x * ray.settings.numEventsX +
		IN.threadInDispatch.y;
	float2 randomNumbers = GetRandomNumbers(threadIndex * 8); // 8 integers are used for random number generation

	// local coords
	COORDS localCoords; // coords of vertex
	localCoords.theta = acos(sqrt(1 - randomNumbers.x));
	localCoords.phi = randomNumbers.y * (2.0f * PI);

	COORDS globalCoords = GetGlobalCoords(localCoords); // coords of scene


	threadOutput[IN.threadInGroupIndex] = 
		shBuffer.SampleLevel(LinearClampSampler, float2(globalCoords.theta / (PI), globalCoords.phi / (2.0f * PI)), 0) *
		float(visibilityBuffer[threadIndex]) *cos(localCoords.theta);


	GroupMemoryBarrierWithGroupSync();

	if (IN.threadInGroupIndex == 0) {
		float groupOutput = 0.0f;
		for (int i = 0; i < 64; ++i) {
			groupOutput += threadOutput[i];
		}
		uint groupIndex = IN.groupInDispatch.x * ray.settings.numEventsX / 8 + IN.groupInDispatch.y;
		resultBuffer[pow(ray.settings.numEventsX / 8, 2) * ray.settings.iCoefficient + groupIndex] = groupOutput;
	}

}


/*
* 
* GetGlobalCoords. Takes coords in the vertex frame (with the normal pointing in
* the y-direction), and transforms it to the global (scene) frame.
* 
*/

COORDS GetGlobalCoords(COORDS localCoords) {

	float3 forward = normalize(ray.forward);
	float3 xDir = normalize(ray.xDir);
	float3 yDir = cross(xDir, forward);

	float3 rayDir = sin(localCoords.theta) * cos(localCoords.phi) * xDir + sin(localCoords.phi) * sin(localCoords.theta) * yDir
		+ cos(localCoords.theta) * forward;

	COORDS globalCoords;

	globalCoords.theta = atan2(sqrt(pow(rayDir.x, 2) + pow(rayDir.z, 2)), rayDir.y);
	globalCoords.phi = atan2(rayDir.z, rayDir.x) + PI;

	return globalCoords;

}


float2 GetRandomNumbers(uint index) {
	float2 res;
	res.x = float(randomBuffer[index] ^ randomBuffer[index + 1]
		^ randomBuffer[index + 2] ^ randomBuffer[index + 3]) / float(pow(2, 32));
	res.y = float(randomBuffer[index + 4] ^ randomBuffer[index + 5]
		^ randomBuffer[index + 6] ^ randomBuffer[index + 7]) / float(pow(2,32));
	return res;
}




