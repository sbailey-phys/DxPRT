/*
*
* Pixel shader used to display the skybox. the theta and phi spherical coordinates of the 
* fragment are calculated which is then used to choose where to sample the texture. A
* cube-map is not used
* 
* To transform from a high dynamic range to a low dynamic range, a simple exponential 
* (1-exp(-hdr)) is used
* 
* The depth of all fragments is set to 1.0f, such that it will be behind all other objects
*
*
* Forms part of the DxPRT project
*
*
*
* Author: Shaun Bailey
*
* Date created: 05/10/2021
*
* Last Updated: 05/10/2021
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

struct VERTEXOUTPUT
{
    float4 Position : SV_Position;
    float4 AbsPos : ABSPOS;
    float exposure : EXPOSURE;
};

struct PIXELOUTPUT {
    float4 colour: SV_Target;
    float depth : SV_Depth;
};

Texture2D<float3> skyboxBuffer: register(t0);
SamplerState LinearClampSampler : register(s0);


static const float PI = 3.14159265f;

PIXELOUTPUT main(VERTEXOUTPUT IN)
{

    PIXELOUTPUT OUT;

    float theta = atan2(sqrt(pow(IN.AbsPos.x, 2) + pow(IN.AbsPos.z, 2)), IN.AbsPos.y) / PI;
    float phi = atan2(IN.AbsPos.z, IN.AbsPos.x) / (PI * 2.0f) +0.5f;

    float3 hdr = skyboxBuffer.SampleLevel(LinearClampSampler, float2(1.0f - phi, theta), 0);
    float3 ldr = float3(1.0f, 1.0f, 1.0f) - exp(- IN.exposure *hdr);

    OUT.colour = float4(ldr, 1.0f);

    OUT.depth = 1.0f;
    return OUT;
}




