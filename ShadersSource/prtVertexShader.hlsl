/*
*
* Vertex shader used to render an object with the precomputed randiance tranfer
* used for lighting.
* 
* The lighting is calculated as a sum over the product of the evniroment map
* coefficients and the transfer function coefficients.
* 
* The number of coefficients used can be passed to the shader via (maxL), otherwise
* it will choose the file with the lowest number of coefficients
* 
* Up to the first 16 coefficients are processed in the pixel shader, while any 
* more are performed here.
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
    uint prtMaxL;
    uint emMaxL;
    uint setMaxL;
};

ConstantBuffer<CONSTANTS> constants : register(b0);


struct VertexPos
{
    float3 Position : POSITION;
    uint index : INDEX;
};

struct VertexShaderOutput
{
    float4 Position : SV_Position;
    float3 diff : DIFF;
    float exposure : EXPOSURE;
    float coefficients[16] : COEFFICIENTS;
    uint nCoefficientsPixel : NPIXELCOEFFICIENTSPIXEL;
};

StructuredBuffer<float> coefficients : register(t0); // coefficients from the mesh
StructuredBuffer<float> EMcoefficients : register(t1); // coefficients from the environmant map

VertexShaderOutput main(VertexPos IN)
{
    VertexShaderOutput OUT;
    OUT.Position = mul(constants.MVP,float4(IN.Position, 1.0f));

    uint nCoefficients = pow(constants.setMaxL + 1, 2); // number of coefficients set by user
    uint nCoefficientsEM = pow(constants.emMaxL + 1, 2); // number of coefficients in the environment map
    uint nCoefficientsPRT = pow(constants.prtMaxL + 1, 2); // number of coefficients in the mesh

    OUT.nCoefficientsPixel = min(nCoefficients, 16); // max 16 coefficients delt with in the pixel shader

    for (int i = 0; i < OUT.nCoefficientsPixel; ++i) 
    {
        OUT.coefficients[i] = coefficients[nCoefficientsPRT * IN.index + i]; // calculation on pixel shader
    }

    for (int i = OUT.nCoefficientsPixel; i < nCoefficients; ++i)  // calculation on vertex shader
    {
        OUT.diff.r += EMcoefficients[i * 3] * coefficients[nCoefficientsPRT * IN.index + i];
        OUT.diff.g += EMcoefficients[i * 3 + 1] * coefficients[nCoefficientsPRT * IN.index + i];
        OUT.diff.b += EMcoefficients[i * 3 + 2] * coefficients[nCoefficientsPRT * IN.index + i];
    }

    OUT.exposure = constants.exposure;

    return OUT;
}

