#include "graphics/glwrap.hpp"

namespace mygl
{

GLint get_current_fbo();
void print_fbo_info(GLuint queried_object, GLuint new_fbo);
uint32_t half_to_int32(uint16_t float16_value);
void saveFrameBuff(const char* fileName, GLuint new_fbo, GLuint width, GLuint height);

uint32_t half_to_int32(uint16_t float16_value)
{
    // MSB -> LSB
    // float16=1bit: sign, 5bit: exponent, 10bit: fraction
    // float32=1bit: sign, 8bit: exponent, 23bit: fraction
    // for normal exponent(1 to 0x1e): value=2**(exponent-15)*(1.fraction)
    // for denormalized exponent(0): value=2**-14*(0.fraction)
    uint32_t sign = float16_value >> 15;
    uint32_t exponent = (float16_value >> 10) & 0x1F;
    uint32_t fraction = (float16_value & 0x3FF);
    uint32_t float32_value;
    if (exponent == 0)
    {
        if (fraction == 0)
        {
        // zero
        float32_value = (sign << 31);
        }
        else
        {
        // can be represented as ordinary value in float32
        // 2 ** -14 * 0.0101
        // => 2 ** -16 * 1.0100
        // int int_exponent = -14;
        exponent = 127 - 14;
        while ((fraction & (1 << 10)) == 0)
        {
            //int_exponent--;
            exponent--;
            fraction <<= 1;
        }
        fraction &= 0x3FF;
        // int_exponent += 127;
        float32_value = (sign << 31) | (exponent << 23) | (fraction << 13);  
        }    
    }
    else if (exponent == 0x1F)
    {
        /* Inf or NaN */
        float32_value = (sign << 31) | (0xFF << 23) | (fraction << 13);
    }
    else
    {
        /* ordinary number */
        float32_value = (sign << 31) | ((exponent + (127-15)) << 23) | (fraction << 13);
    }
    float b =  *((float*)&float32_value);
    uint32_t c = (uint32_t)(b * 256);
    return c;
}

void saveFrameBuff(const char* fileName, GLuint new_fbo, GLuint width, GLuint height)
{
    GLuint old_fbo = get_current_fbo();
    if (old_fbo != new_fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, new_fbo);
    }

    GLint format = 0;
    GLint type = 0;
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &format);
    glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);

    int dimension = 0;
    switch (format)
    {
        case GL_RGBA:
            dimension = 4;
            break;
        case GL_RGB:
            dimension = 3;
            break;
        default:
            printf("!!!!! Not a valid color format: %d\n", format);
            return;
    }

    int iBufLen = width * height * dimension;

    char realFileName[80];
    strcpy(realFileName, fileName);
    strcat(realFileName, ".ppm");
    FILE* oFile = fopen(realFileName, "wb");
    if (oFile == 0)
        printf("!!!!! Output file cannot open !!!!!\n");

    fprintf(oFile, "P6\n%d %d\n255\n", width, height);

    char aFileName[80];
    strcpy(aFileName, fileName);
    strcat(aFileName, ".alpha.ppm");
    FILE* aFile = fopen(aFileName, "w");
    if (oFile == 0)
        printf("!!!!! Data file cannot open !!!!!\n");
    
    fprintf(aFile, "P6\n%d %d\n255\n", width, height);

    // ascii file
    char rgbFileName[80];
    strcpy(rgbFileName, fileName);
    strcat(rgbFileName, ".rgb.ascii");
    FILE* rgbFile = fopen(rgbFileName, "w");
    if (oFile == 0)
        printf("!!!!! rgb file cannot open !!!!!\n");

        // ascii file
    char alphaFileName[80];
    strcpy(alphaFileName, fileName);
    strcat(alphaFileName, ".alpha.ascii");
    FILE* alphaFile = fopen(alphaFileName, "w");
    if (alphaFile == 0)
        printf("!!!!! rgb file cannot open !!!!!\n");


    char rFileName[80];
    strcpy(rFileName, fileName);
    strcat(rFileName, ".r.ppm");
    FILE* rFile = fopen(rFileName, "w");
    if (oFile == 0)
        printf("!!!!! r file cannot open !!!!!\n");
    fprintf(rFile, "P6\n%d %d\n255\n", width, height);
    

    char gFileName[80];
    strcpy(gFileName, fileName);
    strcat(gFileName, ".g.ppm");
    FILE* gFile = fopen(gFileName, "w");
    if (oFile == 0)
        printf("!!!!! g file cannot open !!!!!\n");
    fprintf(gFile, "P6\n%d %d\n255\n", width, height);

    char bFileName[80];
    strcpy(bFileName, fileName);
    strcat(bFileName, ".b.ppm");
    FILE* bFile = fopen(bFileName, "w");
    if (oFile == 0)
        printf("!!!!! b file cannot open !!!!!\n");
    fprintf(bFile, "P6\n%d %d\n255\n", width, height);



    switch (type)
    {
        case GL_HALF_FLOAT:
            {
            GLhalf* iBuf =  (GLhalf*)malloc(iBufLen * sizeof(GLhalf));
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glReadPixels(0, 0, width, height, format, type, iBuf);
            GLhalf* piBuf = iBuf;
            for (int i = 0; i < iBufLen / dimension; i++)
            {
                static uint32_t rgb_array[4];
                rgb_array[0] = half_to_int32(*(piBuf));
                rgb_array[1] = half_to_int32(*(piBuf + 1));
                rgb_array[2] = half_to_int32(*(piBuf + 2));
                rgb_array[3] = half_to_int32(*(piBuf + 3));

                fprintf(rgbFile, "(%4hhu, %4hhu, %4hhu) ", rgb_array[0], rgb_array[1], rgb_array[2]);
                fprintf(alphaFile, "%4u ", rgb_array[3]);

                if (i % width == 0)
                {
                    fprintf(rgbFile, "\n");
                    if (dimension > 3);
                    fprintf(alphaFile, "\n");
                }

                static uint8_t rgb_array_w[4];
                rgb_array_w[0] = (rgb_array[0] > 255) ? (uint8_t) 255 : (uint8_t)rgb_array[0];
                fwrite(rgb_array_w, 1, 1, rFile);
                fwrite(rgb_array_w, 1, 1, rFile);
                fwrite(rgb_array_w, 1, 1, rFile);

                rgb_array_w[1] = (rgb_array[1] > 255) ? (uint8_t) 255 : (uint8_t)rgb_array[1];
                fwrite(rgb_array_w + 1, 1, 1, gFile);
                fwrite(rgb_array_w + 1, 1, 1, gFile);
                fwrite(rgb_array_w + 1, 1, 1, gFile);

                rgb_array_w[2] = (rgb_array[2] > 255) ? (uint8_t) 255 : (uint8_t)rgb_array[2];
                fwrite(rgb_array_w + 2, 1, 1, bFile);
                fwrite(rgb_array_w + 2, 1, 1, bFile);
                fwrite(rgb_array_w + 2, 1, 1, bFile);
                rgb_array_w[3] = (rgb_array[2] > 255) ? (uint8_t) 255 : (uint8_t)rgb_array[3];
                fwrite(rgb_array_w + 3, 1, 1, aFile);
                fwrite(rgb_array_w + 3, 1, 1, aFile);
                fwrite(rgb_array_w + 3, 1, 1, aFile);
                fwrite(rgb_array_w, 3, 1, oFile);
                piBuf += dimension;
            }
            free(iBuf);
            }
            break;
        case GL_UNSIGNED_BYTE:
            {
            GLubyte* iBuf =  (GLubyte*)malloc(iBufLen * sizeof(GLubyte));
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glReadPixels(0, 0, width, height, format, type, iBuf);
            GLubyte* piBuf = iBuf;
            for (int i = 0; i < iBufLen / dimension; i++)
            {
                fprintf(rgbFile, "(%4hhu, %4hhu, %4hhu) ", piBuf[0], piBuf[1], piBuf[2]);
                if (dimension > 3)
                fprintf(alphaFile, "%4hhu ", piBuf[3]);

                if ((i + 1) % width == 0)
                {
                    fprintf(rgbFile, "\n");
                    fprintf(alphaFile, "\n");
                }
                fwrite(piBuf, 3, 1, oFile);
                fwrite(piBuf, 1, 1, rFile);
                fwrite(piBuf, 1, 1, rFile);
                fwrite(piBuf, 1, 1, rFile);

                fwrite(piBuf + 1, 1, 1, gFile);
                fwrite(piBuf + 1, 1, 1, gFile);
                fwrite(piBuf + 1, 1, 1, gFile);

                fwrite(piBuf + 2, 1, 1, bFile);
                fwrite(piBuf + 2, 1, 1, bFile);
                fwrite(piBuf + 2, 1, 1, bFile);

                fwrite(piBuf + 3, 1, 1, aFile);
                fwrite(piBuf + 3, 1, 1, aFile);
                fwrite(piBuf + 3, 1, 1, aFile);
                piBuf += dimension;
            }
            free(iBuf);
            }
            break;
        default:
            printf("!!!!! Not a valid color type: %d\n", type);
            return;
    }

    fclose(oFile);
    fclose(aFile);
    fclose(rFile);
    fclose(gFile);
    fclose(bFile);
    fclose(rgbFile);
    fclose(alphaFile);

    if (old_fbo != new_fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
    }
}

GLint get_current_fbo()
{
    GLint fbo = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
    return fbo;
}

void print_fbo_info(GLuint queried_object, GLuint new_fbo)
{
    GLuint old_fbo = get_current_fbo();
    if (old_fbo != new_fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, new_fbo);
    }

    GLint value = -9999;
    glGetIntegerv(queried_object, &value);
    printf("!!!!! FBO is %d, You queried %u, the value is %d\n", new_fbo, queried_object, value);

    if (old_fbo != new_fbo)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
    }

}

}