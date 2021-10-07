/*
*
* Pixel shader used to render an object with the precomputed randiance tranfer
* used for lighting.
*
* The lighting is calculated as a sum over the product of the evniroment map
* coefficients and the transfer function coefficients.
*
* The number of coefficients used can be passed to the shader via (maxL), otherwise
* it will choose the file with the lowest number of coefficients
*
* Up to the first 16 coefficients are processed here, while any
* more are performed on the vertex shader.
*
* The amount of light reflected by the object is currently alway set to 0.25
*
* To transform from a high dynamic range to a low dynamic range, a simple exponential 
* (1-exp(-hdr)) is used
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
    float3 diff : DIFF;
    float exposure : EXPOSURE;
    float coefficients[16] : COEFFICIENTS;
    uint nCoefficientsPixel : NPIXELCOEFFICIENTSPIXEL;
};

StructuredBuffer<float> EMcoefficients : register(t1);

float4 main(VERTEXOUTPUT IN) : SV_Target
{

    float objectColor = 0.25f;

    float3 diff = IN.diff; // calculation performed on the vertex shader

    for (int i = 0; i < IN.nCoefficientsPixel; ++i) // calculation performed on the pixel shader
    {
        diff.r += EMcoefficients[i * 3] * IN.coefficients[i];
        diff.g += EMcoefficients[i * 3 + 1] * IN.coefficients[i];
        diff.b += EMcoefficients[i * 3 + 2] * IN.coefficients[i];
    }

    float3 ldr = float3(1.0f, 1.0f, 1.0f) - exp(-diff * objectColor * IN.exposure);


	return float4(ldr, 1.0f);
}


