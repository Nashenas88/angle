#include "ANGLETest.h"

class ReadPixelsTest : public ANGLETest
{
protected:
    ReadPixelsTest()
    {
        setClientVersion(3);
        setWindowWidth(32);
        setWindowHeight(32);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        glGenBuffers(1, &mPBO);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
        glBufferData(GL_PIXEL_PACK_BUFFER, 4 * getWindowWidth() * getWindowHeight(), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

        const char *vertexShaderSrc = SHADER_SOURCE
        (
            attribute vec4 aTest;
            attribute vec2 aPosition;
            varying vec4 vTest;

            void main()
            {
                vTest = aTest;
                gl_Position = vec4(aPosition, 0.0, 1.0);
                gl_PointSize = 1.0;
            }
        );

        const char *fragmentShaderSrc = SHADER_SOURCE
        (
            precision mediump float;
            varying vec4 vTest;

            void main()
            {
                gl_FragColor = vTest;
            }
        );

        mProgram = compileProgram(vertexShaderSrc, fragmentShaderSrc);

        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, 4, 1);

        glGenFramebuffers(1, &mFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glGenBuffers(1, &mPositionVBO);
        glBindBuffer(GL_ARRAY_BUFFER, mPositionVBO);
        glBufferData(GL_ARRAY_BUFFER, 128, NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        ANGLETest::TearDown();

        glDeleteBuffers(1, &mPBO);
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mTexture);
        glDeleteFramebuffers(1, &mFBO);
    }

    GLuint mPBO;
    GLuint mProgram;
    GLuint mTexture;
    GLuint mFBO;
    GLuint mPositionVBO;
};

TEST_F(ReadPixelsTest, out_of_bounds)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    GLsizei pixelsWidth = 32;
    GLsizei pixelsHeight = 32;
    GLint offset = 16;
    std::vector<GLubyte> pixels((pixelsWidth + offset) * (pixelsHeight + offset) * 4);

    glReadPixels(-offset, -offset, pixelsWidth + offset, pixelsHeight + offset, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    EXPECT_GL_NO_ERROR();

    for (int y = pixelsHeight / 2; y < pixelsHeight; y++)
    {
        for (int x = pixelsWidth / 2; x < pixelsWidth; x++)
        {
            const GLubyte* pixel = pixels.data() + ((y * (pixelsWidth + offset) + x) * 4);
            unsigned int r = static_cast<unsigned int>(pixel[0]);
            unsigned int g = static_cast<unsigned int>(pixel[1]);
            unsigned int b = static_cast<unsigned int>(pixel[2]);
            unsigned int a = static_cast<unsigned int>(pixel[3]);

            // Expect that all pixels which fell within the framebuffer are red
            EXPECT_EQ(255, r);
            EXPECT_EQ(0,   g);
            EXPECT_EQ(0,   b);
            EXPECT_EQ(255, a);
        }
    }
}

TEST_F(ReadPixelsTest, pbo_with_other_target)
{
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, mPBO);

    GLvoid *mappedPtr = glMapBufferRange(GL_ARRAY_BUFFER, 0, 32, GL_MAP_READ_BIT);
    unsigned char *dataPtr = static_cast<unsigned char *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    EXPECT_EQ(255, dataPtr[0]);
    EXPECT_EQ(0,   dataPtr[1]);
    EXPECT_EQ(0,   dataPtr[2]);
    EXPECT_EQ(255, dataPtr[3]);

    glUnmapBuffer(GL_ARRAY_BUFFER);
    EXPECT_GL_NO_ERROR();
}

TEST_F(ReadPixelsTest, pbo_with_existing_data)
{
    // Clear backbuffer to red
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Read 16x16 region from red backbuffer to PBO
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPBO);
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    // Clear backbuffer to green
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();

    // Read 16x16 region from green backbuffer to PBO at offset 16
    glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid*>(16));
    GLvoid * mappedPtr = glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, 32, GL_MAP_READ_BIT);
    unsigned char *dataPtr = static_cast<unsigned char *>(mappedPtr);
    EXPECT_GL_NO_ERROR();

    // Test pixel 0 is red (existing data)
    EXPECT_EQ(255, dataPtr[0]);
    EXPECT_EQ(0, dataPtr[1]);
    EXPECT_EQ(0, dataPtr[2]);
    EXPECT_EQ(255, dataPtr[3]);

    // Test pixel 16 is green (new data)
    EXPECT_EQ(0, dataPtr[16 * 4 + 0]);
    EXPECT_EQ(255, dataPtr[16 * 4 + 1]);
    EXPECT_EQ(0, dataPtr[16 * 4 + 2]);
    EXPECT_EQ(255, dataPtr[16 * 4 + 3]);

    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    EXPECT_GL_NO_ERROR();
}