#include "Color.hpp"

#include <cassert>
#include <cmath>

Color::Color()
{
}

Color::Color(int intRGB)
{
    constexpr int filter_8bit = 255;
    mR                        = (float)((intRGB >> 16) & filter_8bit) / 255.f;
    mG                        = (float)((intRGB >> 8) & filter_8bit) / 255.f;
    mB                        = (float)(intRGB & filter_8bit) / 255.f;
}

Color::Color(float R, float G, float B)
    : mR(R)
    , mG(G)
    , mB(B)
{
}

float& Color::operator[](const int index)
{
    assert(index > -1 && index < 3);
    switch (index)
    {
    case 0:
        return mR;
    case 1:
        return mG;
    case 2:
        return mB;
    }
}

void Color::operator/=(const float div)
{
    mR /= div;
    mG /= div;
    mB /= div;
}

void Color::Clamp(const float minBoundary, const float maxBoundary)
{
    mR = std::max(0.f, std::min(1.f, mR));
    mG = std::max(0.f, std::min(1.f, mG));
    mB = std::max(0.f, std::min(1.f, mB));
}

void Color::Normalize()
{
    const float sum = mR + mG + mB;
    if (sum != 0)
    {
        mR /= sum;
        mG /= sum;
        mB /= sum;
    }
}

//https://en.wikipedia.org/wiki/SRGB#The_sRGB_transfer_function_.28.22gamma.22.29
Color XYZToRGB(const Color& c)
{
    Color convColor = { 3.2406f * c.mR - 1.5172f * c.mG - 0.4986f * c.mB,
                        -0.9689f * c.mR + 1.8758f * c.mG + 0.0415f * c.mB,
                        0.0557f * c.mR - 0.2040f * c.mG + 1.0570f * c.mB };

    for (unsigned int icomp = 0; icomp != 3; ++icomp)
    {
        float& colorComponent = convColor[icomp];

        // gamma correction
        if (colorComponent < 0.0031308)
        {
            colorComponent = 12.92 * colorComponent;
        }
        else
        {
            colorComponent = 1.055 * std::pow(colorComponent, 1.f / 2.4f) - 0.055;
        }
    }

    convColor.Clamp();

    return convColor;
}

Color wavelengthToRGB(float wavelength, const float lookupTable[][3], const int tableSize,
                      const float tableRangeMin, const float tableRangeMax,
                      const float tableRangeStep)
{
    Color XYZColor;

    // convert wavelength to XYZColor
    if(wavelength <= tableRangeMin){
        XYZColor.mR = lookupTable[0][0];
        XYZColor.mG = lookupTable[0][1];
        XYZColor.mB = lookupTable[0][2];

    }else if(wavelength >= tableRangeMin){
        XYZColor.mR = lookupTable[tableSize - 1][0];
        XYZColor.mG = lookupTable[tableSize - 1][1];
        XYZColor.mB = lookupTable[tableSize - 1][2];

    }else{

        float interpolator = (wavelength - tableRangeMin) / tableRangeStep;
        int iColor = (int)interpolator;

        interpolator -= iColor;
        float opp_interpolator = 1.f - interpolator;

         XYZColor.mR
            = lookupTable[iColor][0] * opp_interpolator + lookupTable[iColor + 1][0] * interpolator;
        XYZColor.mG
            = lookupTable[iColor][1] * opp_interpolator + lookupTable[iColor + 1][1] * interpolator;
        XYZColor.mB
            = lookupTable[iColor][2] * opp_interpolator + lookupTable[iColor + 1][2] * interpolator;

    }

    XYZColor.Normalize();

    return XYZToRGB(XYZColor);
}

// Gaia broad band photometry, Jordi et al., 2010, Table 3, p.7
// https://www.aanda.org/articles/aa/pdf/2010/15/aa15441-10.pdf
float BpRpMagnitudeToBV(const float bpMagnitude, const float rpMagnitude, const float bprpReddening)
{
    return ((bpMagnitude - rpMagnitude + bprpReddening) - 0.0981) / 1.4290;
}

Color BVToRGB(float BV, const int lookupTable[], const int tableSize, const float tableRangeMin,
              const float tableRangeMax, const float tableRangeStep)
{
    Color RGBColor;

    if(BV <= tableRangeMin){
        RGBColor = Color(lookupTable[0]);

    }else if(BV >= tableRangeMax){
        RGBColor = Color(lookupTable[tableSize - 1]);

    }else{

        float interpolator = (BV - tableRangeMin) / tableRangeStep;
        int   iColor       = (int)interpolator;

        interpolator -= iColor;
        float opp_interpolator = 1.f - interpolator;

        const Color colorA(lookupTable[iColor]);
        const Color colorB(lookupTable[iColor + 1]);

        RGBColor.mR = colorA.mR * opp_interpolator + colorB.mR * interpolator;
        RGBColor.mG = colorA.mG * opp_interpolator + colorB.mG * interpolator;
        RGBColor.mB = colorA.mB * opp_interpolator + colorB.mB * interpolator;
    }

    return RGBColor;
}

// Gaia broad band photometry, Jordi et al., 2010, 5.1, p.6
// https://www.aanda.org/articles/aa/pdf/2010/15/aa15441-10.pdf
float BpRpMagnitudeToTemp(const float bpMagnitude, const float rpMagnitude)
{
    float delta = bpMagnitude - rpMagnitude;
    //float exponant = 3.999 - 0.654 * delta + 0.709 * delta * delta - 0.316 * delta * delta * delta;
    float exponant = 3.999 + delta * (-0.654 + delta * (0.709 - delta * 0.316)); // same but using Horner

    return std::pow(10.f, exponant);
}

Color TempToRGB(float temperature, const float lookupTable[][3], const int tableSize,
                const float tableRangeMin, const float tableRangeMax, const float tableRangeStep)
{

    Color RGBColor;

    if(temperature <= tableRangeMin){
        RGBColor.mR = lookupTable[0][0];
        RGBColor.mG = lookupTable[0][1];
        RGBColor.mB = lookupTable[0][2];

    }else if(temperature >= tableRangeMax){
        RGBColor.mR = lookupTable[tableSize - 1][0];
        RGBColor.mG = lookupTable[tableSize - 1][1];
        RGBColor.mB = lookupTable[tableSize - 1][2];

    }else{

        float interpolator = (temperature - tableRangeMin) / tableRangeStep;
        int   iColor       = (int)interpolator;

        interpolator -= iColor;
        float opp_interpolator = 1.f - interpolator;

        RGBColor.mR
            = lookupTable[iColor][0] * opp_interpolator + lookupTable[iColor + 1][0] * interpolator;
        RGBColor.mG
            = lookupTable[iColor][1] * opp_interpolator + lookupTable[iColor + 1][1] * interpolator;
        RGBColor.mB
            = lookupTable[iColor][2] * opp_interpolator + lookupTable[iColor + 1][2] * interpolator;

    }

    return RGBColor;
}

// https://en.wikipedia.org/wiki/Luminosity#Relationship_to_magnitude
float magnitudeToLuminosity(const float magnitude)
{
    constexpr float zeroPointLuminosity = 3.0128e28;
    return zeroPointLuminosity * std::pow(10, -0.4 * magnitude);
}

// without zeroPointLuminosity to avoid overflow when simply adding magnitudes
float magnitudeToLuminosityFactor(const float magnitude){
    return std::pow(10, -0.4 * magnitude);
}

// https://en.wikipedia.org/wiki/Luminosity#Relationship_to_magnitude
float luminosityToMagnitude(const float luminosity)
{
    assert(luminosity > 0.f);

    constexpr float zeroPointLuminosity = 3.0128e28;
    //return -2.5f * log10(luminosity / zeroPointLuminosity); // 'real' formula
    return -2.5f * log10(luminosity) + 71.1974; // approximation
}

// without zeroPointLuminosity to avoid overflow when simply adding magnitudes
float luminosityFactorToMagnitude(const float luminosityFactor){
    assert(luminosityFactor > 0.f);

    return -2.5f * log10(luminosityFactor);
}

float magnitudeAddition(const float magnitudeA, const float magnitudeB)
{
    return luminosityFactorToMagnitude(magnitudeToLuminosityFactor(magnitudeA)
                                 + magnitudeToLuminosityFactor(magnitudeB));
}
