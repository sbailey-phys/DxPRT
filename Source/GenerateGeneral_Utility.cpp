/*
*
* Implimentation of GenerateGeneral_Utility.h
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

#include "DxPRT/GenerateGeneral_Utility.h"


namespace DxPRT_Utility {

    void GenerateSHvector(const UINT64& shGridNum, const UINT64& maxL, std::vector<std::vector<float>>& shVector) {

        const float PI = 3.1315927f;

        const int nCoefficients = (maxL + 1) * (maxL + 1);

        for (int i = 0; i < shGridNum; ++i) {
            for (int j = 0; j < shGridNum; ++j) {
                float theta = (PI) * float(j) / float(shGridNum - 1);
                float phi = 2.0f * PI * float(i) / float(shGridNum - 1);
                std::vector<float> sh = CalcSH(maxL, cos(theta), phi);
                for (int k = 0; k < nCoefficients; ++k) {
                    shVector[k].push_back(sh[k]);
                }
            }
        }
    }


    void GenerateRandomVector(const UINT64& numEvents, std::vector<UINT32>& randomVector)
    {
        randomVector.resize(numEvents * 8, 0);
        srand((unsigned)time(NULL));
        for (auto iter = randomVector.begin(); iter != randomVector.end();
            ++iter) {
            // seeds of pseudo-random number generator must be greater than 128
            while (*iter < 128) {
                *iter = rand() % int(pow(2, 32));
            }
        }
    }


    void RoundInput(const UINT64& numEvents, const UINT64& shGridNum, UINT64& roundedNumEvents,
        UINT64& roundedNumEventsX, UINT64& roundedSHGridNum) {

        UINT64 N = UINT64(ceil(sqrt((float)numEvents) / 8.0f));
        roundedNumEvents = 64u * N * N;
        roundedNumEventsX = 8u * N;

        N = UINT64(ceil((float)shGridNum / 8.0f));
        roundedSHGridNum = 8u * N;

    }

}
