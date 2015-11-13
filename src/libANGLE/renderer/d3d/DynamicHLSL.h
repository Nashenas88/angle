//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DynamicHLSL.h: Interface for link and run-time HLSL generation
//

#ifndef LIBANGLE_RENDERER_D3D_DYNAMICHLSL_H_
#define LIBANGLE_RENDERER_D3D_DYNAMICHLSL_H_

#include <map>
#include <vector>

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Program.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"

namespace sh
{
struct Attribute;
struct ShaderVariable;
}

namespace gl
{
class InfoLog;
struct VariableLocation;
struct VertexAttribute;
struct Data;
}

namespace rx
{
struct PackedVarying;
struct SemanticInfo;
class ShaderD3D;

struct PixelShaderOutputVariable
{
    GLenum type;
    std::string name;
    std::string source;
    size_t outputIndex;
};

class DynamicHLSL : angle::NonCopyable
{
  public:
    explicit DynamicHLSL(RendererD3D *const renderer);

    std::string generateVertexShaderForInputLayout(
        const std::string &sourceShader,
        const gl::InputLayout &inputLayout,
        const std::vector<sh::Attribute> &shaderAttributes) const;
    std::string generatePixelShaderForOutputSignature(
        const std::string &sourceShader,
        const std::vector<PixelShaderOutputVariable> &outputVariables,
        bool usesFragDepth,
        const std::vector<GLenum> &outputLayout) const;
    bool generateShaderLinkHLSL(const gl::Data &data,
                                const gl::Program::Data &programData,
                                gl::InfoLog &infoLog,
                                unsigned int registerCount,
                                std::string *pixelHLSL,
                                std::string *vertexHLSL,
                                const std::vector<PackedVarying> &packedVaryings,
                                std::vector<D3DVarying> *d3dVaryingsOut,
                                std::vector<PixelShaderOutputVariable> *outPixelShaderKey,
                                bool *outUsesFragDepth) const;

    std::string generateGeometryShaderPreamble(
        const gl::Data &data,
        const gl::Program::Data &programData,
        unsigned int registers,
        const std::vector<PackedVarying> &packedVaryings) const;

    std::string generateGeometryShaderHLSL(gl::PrimitiveType primitiveType,
                                           const gl::Data &data,
                                           const gl::Program::Data &programData,
                                           const std::string &preambleString) const;

  private:
    RendererD3D *const mRenderer;

    void generateVaryingLinkHLSL(const gl::Caps &caps,
                                 bool programUsesPointSize,
                                 const SemanticInfo &info,
                                 const std::vector<PackedVarying> &packedVaryings,
                                 std::stringstream &linkStream) const;
    void generateVaryingHLSL(const gl::Caps &caps,
                             const std::vector<PackedVarying> &varyings,
                             bool programUsesPointSize,
                             std::stringstream &hlslStream) const;
    void storeUserVaryings(const std::vector<PackedVarying> &packedVaryings,
                           bool programUsesPointSize,
                           std::vector<D3DVarying> *d3dVaryingsOut) const;
    void storeBuiltinVaryings(const SemanticInfo &info,
                              std::vector<D3DVarying> *d3dVaryingsOut) const;

    // Prepend an underscore
    static std::string decorateVariable(const std::string &name);

    std::string generateAttributeConversionHLSL(gl::VertexFormatType vertexFormatType,
                                                const sh::ShaderVariable &shaderAttrib) const;
};

std::string GetVaryingSemantic(int majorShaderModel, bool programUsesPointSize);
}

#endif  // LIBANGLE_RENDERER_D3D_DYNAMICHLSL_H_
