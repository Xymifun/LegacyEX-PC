#ifndef PLATFORM_PC_PCGL_HPP
#define PLATFORM_PC_PCGL_HPP

#if defined(_WIN32) && !defined(APIENTRY)
#define APIENTRY __stdcall
#endif

#include <GL/glcorearb.h>

#define PC_GL_FUNCTIONS(X)                                                                                             \
    X(PFNGLACTIVETEXTUREPROC, ActiveTexture)                                                                           \
    X(PFNGLATTACHSHADERPROC, AttachShader)                                                                             \
    X(PFNGLBINDATTRIBLOCATIONPROC, BindAttribLocation)                                                                 \
    X(PFNGLBINDBUFFERPROC, BindBuffer)                                                                                 \
    X(PFNGLBINDBUFFERBASEPROC, BindBufferBase)                                                                         \
    X(PFNGLBINDBUFFERRANGEPROC, BindBufferRange)                                                                       \
    X(PFNGLGETINTEGERVPROC, GetIntegerv)                                                                               \
    X(PFNGLBINDFRAMEBUFFERPROC, BindFramebuffer)                                                                       \
    X(PFNGLBINDRENDERBUFFERPROC, BindRenderbuffer)                                                                     \
    X(PFNGLBINDTEXTUREPROC, BindTexture)                                                                               \
    X(PFNGLBINDVERTEXARRAYPROC, BindVertexArray)                                                                       \
    X(PFNGLBLENDCOLORPROC, BlendColor)                                                                                 \
    X(PFNGLBLENDFUNCPROC, BlendFunc)                                                                                   \
    X(PFNGLBLITFRAMEBUFFERPROC, BlitFramebuffer)                                                                       \
    X(PFNGLBUFFERDATAPROC, BufferData)                                                                                 \
    X(PFNGLBUFFERSUBDATAPROC, BufferSubData)                                                                           \
    X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, CheckFramebufferStatus)                                                         \
    X(PFNGLCLEARPROC, Clear)                                                                                           \
    X(PFNGLCLEARCOLORPROC, ClearColor)                                                                                 \
    X(PFNGLCLEARDEPTHPROC, ClearDepth)                                                                                 \
    X(PFNGLCOLORMASKPROC, ColorMask)                                                                                   \
    X(PFNGLCOMPILESHADERPROC, CompileShader)                                                                           \
    X(PFNGLCREATEPROGRAMPROC, CreateProgram)                                                                           \
    X(PFNGLCREATESHADERPROC, CreateShader)                                                                             \
    X(PFNGLCULLFACEPROC, CullFace)                                                                                     \
    X(PFNGLDELETEBUFFERSPROC, DeleteBuffers)                                                                           \
    X(PFNGLDELETESHADERPROC, DeleteShader)                                                                             \
    X(PFNGLDELETETEXTURESPROC, DeleteTextures)                                                                         \
    X(PFNGLDELETEVERTEXARRAYSPROC, DeleteVertexArrays)                                                                 \
    X(PFNGLDEPTHFUNCPROC, DepthFunc)                                                                                   \
    X(PFNGLDEPTHMASKPROC, DepthMask)                                                                                   \
    X(PFNGLDISABLEPROC, Disable)                                                                                       \
    X(PFNGLDRAWARRAYSPROC, DrawArrays)                                                                                 \
    X(PFNGLDRAWELEMENTSBASEVERTEXPROC, DrawElementsBaseVertex)                                                         \
    X(PFNGLENABLEPROC, Enable)                                                                                         \
    X(PFNGLENABLEVERTEXATTRIBARRAYPROC, EnableVertexAttribArray)                                                       \
    X(PFNGLDISABLEVERTEXATTRIBARRAYPROC, DisableVertexAttribArray)                                                     \
    X(PFNGLFINISHPROC, Finish)                                                                                         \
    X(PFNGLFENCESYNCPROC, FenceSync)                                                                                   \
    X(PFNGLCLIENTWAITSYNCPROC, ClientWaitSync)                                                                         \
    X(PFNGLDELETESYNCPROC, DeleteSync)                                                                                 \
    X(PFNGLFRAMEBUFFERRENDERBUFFERPROC, FramebufferRenderbuffer)                                                       \
    X(PFNGLFRAMEBUFFERTEXTURE2DPROC, FramebufferTexture2D)                                                             \
    X(PFNGLFRONTFACEPROC, FrontFace)                                                                                   \
    X(PFNGLGENBUFFERSPROC, GenBuffers)                                                                                 \
    X(PFNGLGENFRAMEBUFFERSPROC, GenFramebuffers)                                                                       \
    X(PFNGLGENRENDERBUFFERSPROC, GenRenderbuffers)                                                                     \
    X(PFNGLGENTEXTURESPROC, GenTextures)                                                                               \
    X(PFNGLGENVERTEXARRAYSPROC, GenVertexArrays)                                                                       \
    X(PFNGLGETERRORPROC, GetError)                                                                                     \
    X(PFNGLGETPROGRAMINFOLOGPROC, GetProgramInfoLog)                                                                   \
    X(PFNGLGETPROGRAMIVPROC, GetProgramiv)                                                                             \
    X(PFNGLGETSHADERINFOLOGPROC, GetShaderInfoLog)                                                                     \
    X(PFNGLGETSHADERIVPROC, GetShaderiv)                                                                               \
    X(PFNGLGETSTRINGPROC, GetString)                                                                                   \
    X(PFNGLGETUNIFORMBLOCKINDEXPROC, GetUniformBlockIndex)                                                             \
    X(PFNGLGETUNIFORMLOCATIONPROC, GetUniformLocation)                                                                 \
    X(PFNGLLINEWIDTHPROC, LineWidth)                                                                                   \
    X(PFNGLLINKPROGRAMPROC, LinkProgram)                                                                               \
    X(PFNGLPIXELSTOREIPROC, PixelStorei)                                                                               \
    X(PFNGLPOLYGONOFFSETPROC, PolygonOffset)                                                                           \
    X(PFNGLREADPIXELSPROC, ReadPixels)                                                                                 \
    X(PFNGLRENDERBUFFERSTORAGEPROC, RenderbufferStorage)                                                               \
    X(PFNGLSCISSORPROC, Scissor)                                                                                       \
    X(PFNGLSHADERSOURCEPROC, ShaderSource)                                                                             \
    X(PFNGLSTENCILFUNCPROC, StencilFunc)                                                                               \
    X(PFNGLSTENCILMASKPROC, StencilMask)                                                                               \
    X(PFNGLSTENCILOPPROC, StencilOp)                                                                                   \
    X(PFNGLTEXIMAGE2DPROC, TexImage2D)                                                                                 \
    X(PFNGLTEXPARAMETERIPROC, TexParameteri)                                                                           \
    X(PFNGLTEXSUBIMAGE2DPROC, TexSubImage2D)                                                                           \
    X(PFNGLUNIFORM1FPROC, Uniform1f)                                                                                   \
    X(PFNGLUNIFORM1IPROC, Uniform1i)                                                                                   \
    X(PFNGLUNIFORM4FPROC, Uniform4f)                                                                                   \
    X(PFNGLUNIFORM4FVPROC, Uniform4fv)                                                                                 \
    X(PFNGLUNIFORMBLOCKBINDINGPROC, UniformBlockBinding)                                                               \
    X(PFNGLUNIFORMMATRIX4FVPROC, UniformMatrix4fv)                                                                     \
    X(PFNGLUSEPROGRAMPROC, UseProgram)                                                                                 \
    X(PFNGLVERTEXATTRIB4FVPROC, VertexAttrib4fv)                                                                       \
    X(PFNGLVERTEXATTRIBDIVISORPROC, VertexAttribDivisor)                                                               \
    X(PFNGLVERTEXATTRIBIPOINTERPROC, VertexAttribIPointer)                                                             \
    X(PFNGLVERTEXATTRIBPOINTERPROC, VertexAttribPointer)                                                               \
    X(PFNGLVIEWPORTPROC, Viewport)

namespace PcGl
{
#define PC_GL_DECLARE(type, name) extern type name;
PC_GL_FUNCTIONS(PC_GL_DECLARE)
#undef PC_GL_DECLARE

bool Load();
}

#endif
