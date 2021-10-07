/*
*
* Vertex shader used to display the skybox. Passes the exposure and the vertex position
* (in both the model's and camera's frame) to the pixel shader
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

struct CONSTANTS
{
    matrix MVP;
    float exposure;
};

ConstantBuffer<CONSTANTS> constants : register(b0);


struct VertexPos
{
    float3 Position : POSITION;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float4 AbsPos : ABSPOS;
    float exposure : EXPOSURE;
};

VertexShaderOutput main(VertexPos IN)
{
    VertexShaderOutput OUT;


    OUT.Position = mul(constants.MVP, float4(IN.Position, 1.0f));
    OUT.AbsPos = float4(IN.Position, 1.0f); // position in the Model's frame
    OUT.exposure = constants.exposure;

    return OUT;
}