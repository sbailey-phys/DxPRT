/*
*
* Ray tracer pre-pass. This is used to reduce the number of planes that need to
* be looped over during the main ray tracer. This stores the planes into a buffer
* that is itself split into chunks of 512 elements. The first few elements of this
* buffer contain the number of planes that pass the check in each chunk
*
*
* Forms part of the DxPRT project
*
*
*
* Author: Shaun Bailey
*
* Date created: 04/10/2021
*
* Last Updated: 04/10/2021
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


StructuredBuffer<float3> vertexBuffer : register(t0);
StructuredBuffer<uint> indexBuffer : register(t1);
RWBuffer<uint> planeBuffer : register(u0);


struct RAYSETTINGS
{
	uint pad1;
	uint numPlaneChunks;
	uint numPlanes;
	uint pad2;
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

groupshared uint groupPlanes[512];

/*
* Main function. Checks each triangle (plane) in chunks of 512. If any point of the
* triangle is in the hemi-sphere defined by the normal of the vertex in the question,
* then the triangle passes the check and is stored in the buffer for the main ray
* tracer. The first few elements of this buffer stores the number of planes in each
* chunk that passes this test.
*/

[numthreads(8, 8, 8)]
void main(ComputeShaderInput IN)
{
	
	uint passed = 0;
	uint planeIndex = IN.groupInDispatch.x * 512 + IN.threadInGroupIndex;

	planeIndex = min(planeIndex, ray.settings.numPlanes - 1); // ensure the no out of bounds elements are accessed

	float3 vector1 = vertexBuffer[indexBuffer[3 * planeIndex]] - ray.rayPos;
	float3 vector2 = vertexBuffer[indexBuffer[3 * planeIndex + 1]] - ray.rayPos;
	float3 vector3 = vertexBuffer[indexBuffer[3 * planeIndex + 2]] - ray.rayPos;

	if (dot(vector1, ray.forward) > 0.0f ||
		dot(vector2, ray.forward) > 0.0f ||
		dot(vector3, ray.forward) > 0.0f) passed = 1;

	groupPlanes[IN.threadInGroupIndex] = passed;

	GroupMemoryBarrierWithGroupSync();

	if (IN.threadInGroupIndex == 0)  // one thread in group stores tha panes that pass
	{ 
		uint bufferIndex = ray.settings.numPlanes + IN.groupInDispatch.x * 512;
		uint nPlanes = 0;
		for (int iPlane = 0; iPlane < 512; ++iPlane) {
			uint currentPlaneIndex = IN.groupInDispatch.x * 512 + iPlane;
			if (groupPlanes[iPlane] == 1 && currentPlaneIndex < ray.settings.numPlanes) {
				planeBuffer[bufferIndex] = currentPlaneIndex;
				++bufferIndex;
				++nPlanes;
			}
		}
		planeBuffer[IN.groupInDispatch.x] = nPlanes; // number of planes in chunk
	}

}


