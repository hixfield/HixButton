#include "HixColor.h"

HixColor::HixColor(uint32_t nColor) : m_nColor(nColor) {
}

HixColor::HixColor(const char* szRGBString) {
    //must always have 6 characters
    if (strlen(szRGBString) < 6) {
        m_nColor = 0;
        return;
    }
    //storin components
    const uint32_t hexBase = 16;
    uint32_t nR = hexCharToInt(szRGBString[5]) * hexBase + hexCharToInt(szRGBString[4]);
    uint32_t nG = hexCharToInt(szRGBString[3]) * hexBase + hexCharToInt(szRGBString[2]);
    uint32_t nB = hexCharToInt(szRGBString[1]) * hexBase + hexCharToInt(szRGBString[0]);
    //reassemble
    m_nColor = (nR << 16) | (nG << 8) | nB;
}

uint32_t HixColor::getRGB(void) {
    return m_nColor;
}

void HixColor::setRGB(uint32_t nColor) {
    m_nColor = nColor;
}

uint32_t HixColor::getDimmedWithPercent(uint8 nDimPercent) {
    //full power does not need calculations
    if (nDimPercent >= 100) return m_nColor;
    //make base colors
    uint32_t nR = m_nColor >> 16;
    uint32_t nG = (m_nColor & 0x00FF00) >> 8;
    uint32_t nB = m_nColor & 0x0000FF;
    //dim individual components
    nR = ((nR * (uint32_t)nDimPercent) / (uint32_t)100) & 0x0000FF;
    nG = ((nG * (uint32_t)nDimPercent) / (uint32_t)100) & 0x0000FF;
    nB = ((nB * (uint32_t)nDimPercent) / (uint32_t)100) & 0x0000FF;
    //reassemble
    return (nR << 16) | (nG << 8) | nB;
}

uint32_t HixColor::hexCharToInt(char c) {
    switch (c) {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case 'a':
        case 'A':
            return 10;
        case 'b':
        case 'B':
            return 11;
        case 'c':
        case 'C':
            return 12;
        case 'd':
        case 'D':
            return 13;
        case 'e':
        case 'E':
            return 14;
        case 'f':
        case 'F':
            return 15;
        //no match return zero
        default:
            return 0;
    }
}