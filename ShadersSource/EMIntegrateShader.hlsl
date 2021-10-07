/* 
* 
* Compute shader to integrate over an environment map and calculate the relevant
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
static const uint numThreadsX = 8;

Texture2D<float3> emBuffer : register(t0);
Texture2D<float> shBuffer: register(t1);
RWBuffer<uint> randomBuffer : register(u0);
RWBuffer<float> resultBuffer : register(u1);
SamplerState LinearClampSampler : register(s0);

struct SETTINGS 
{
	uint numEventsX;
	uint iCoefficient;
	uint pad1;
	uint pad2;
};

ConstantBuffer<SETTINGS> settings : register(b0);


float2 GenerateRandomNumbers(uint index);
uint Tausworthe(uint index, uint S1, uint S2, uint S3, uint M);
uint LCG(uint index, uint A, uint B);


struct ComputeShaderInput
{
	uint3 groupInDispatch    : SV_GroupID;
	uint3 threadInGroup      : SV_GroupThreadID;
	uint3 threadInDispatch   : SV_DispatchThreadID;
	uint  threadInGroupIndex : SV_GroupIndex;
};

groupshared float3 threadOutput[numThreadsX * numThreadsX];

/*
* Main function, evaluates the product of the spherical harmonic and the 
* environment map. This is summed over the thread group. 
*/

[numthreads(numThreadsX, numThreadsX, 1)]
void main( ComputeShaderInput IN )
{

	uint threadIndex = (IN.threadInDispatch.x * settings.numEventsX + IN.threadInDispatch.y);
	float2 randomNumbers = GenerateRandomNumbers(threadIndex * 8); // 8 integers used for random number generator

	float theta = acos(sqrt(1 - randomNumbers.x)) / (PI / 2);
	float phi = randomNumbers.y;

	threadOutput[IN.threadInGroupIndex] =
		shBuffer.SampleLevel(LinearClampSampler, float2(theta, phi), 0) *
		emBuffer.SampleLevel(LinearClampSampler, float2(1.0f - phi, theta), 0); // phi and theta swapped in hdr file

	GroupMemoryBarrierWithGroupSync();

	// sum over this thread group
	if (IN.threadInGroupIndex == 0) 
	{
		float3 groupOutput = float3(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < numThreadsX * numThreadsX; ++i) 
		{
			groupOutput += threadOutput[i];
		}
		uint groupIndex = IN.groupInDispatch.x * settings.numEventsX / 8 + IN.groupInDispatch.y;
		uint resultBufferIndex = pow(settings.numEventsX / 8, 2) * settings.iCoefficient + groupIndex;
		resultBuffer[resultBufferIndex * 3] = groupOutput.r;
		resultBuffer[resultBufferIndex * 3 + 1] = groupOutput.g;
		resultBuffer[resultBufferIndex * 3 + 2] = groupOutput.b;
	}

}


/* 
* Pseudo-random number generator recommened in GPU Gems 3
* chapter 37. A combination of a simple linear congruential
* generator and a combined tausworthe generator
*/

float2 GenerateRandomNumbers(uint index) 
{
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

uint Tausworthe(uint index, uint S1, uint S2, uint S3, uint M) 
{
	uint z = randomBuffer[index];
	uint b = (((z << S1) ^ z) >> S2);
	return (((z & M) << S3) ^ b);
}

uint LCG(uint index, uint A, uint B) 
{
	return A * randomBuffer[index] + B;
}



