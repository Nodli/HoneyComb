#pragma once

#include "LookupTable.hpp"

#include <cmath>
#include <ostream>

struct Color
{
    Color();
    Color(int hexValue);
    Color(float R, float G, float B);

    float& operator[](const int index);
    void operator/=(const float div);

    void Clamp(const float minBoundary = 0.f, const float maxBoundary = 1.f);
    void Normalize();

    // member variables
    float mR;
    float mG;
    float mB;
};

Color XYZToRGB(const Color& c);
Color wavelengthToRGB(const float wavelength, const float lookupTable[][3] = CIE1931::data,
                      const int tableSize = CIE1931::size, const float tableRangeMin = CIE1931::min,
                      const float tableRangeMax  = CIE1931::max,
                      const float tableRangeStep = CIE1931::step);

float BpRpMagnitudeToBV(const float bpMagnitude, const float rpMagnitude, const float bprpReddening = 0.f);
Color BVToRGB(float BV, const int lookupTable[] = BV_RGB::data, const int tableSize = BV_RGB::size,
              const float tableRangeMin = BV_RGB::min, const float tableRangeMax = BV_RGB::max,
              const float tableRangeStep = BV_RGB::step);

float BpRpMagnitudeToTemp(const float bpMagnitude, const float rpMagnitude);
Color TempToRGB(float temperature, const float lookupTable[][3] = T_RGB1931::data,
                const int tableSize = T_RGB1931::size, const float tableRangeMin = T_RGB1931::min,
                const float tableRangeMax  = T_RGB1931::max,
                const float tableRangeStep = T_RGB1931::step);

float magnitudeToLuminosity(const float magnitude);
float magnitudeToLuminosityFactor(const float magnitude);
float luminosityToMagnitude(const float luminosity);
float luminosityFactorToMagnitude(const float luminosity);

float magnitudeAddition(const float magnitudeA, const float magnitudeB);
