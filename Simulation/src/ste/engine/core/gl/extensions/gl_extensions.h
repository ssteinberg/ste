// StE
// © Shlomi Steinberg, 2015-2016

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef void (GLAPIENTRY * PFNGLNAMEDBUFFERPAGECOMMITMENTEXT) (GLuint, GLintptr, GLsizeiptr, GLboolean);

extern PFNGLNAMEDBUFFERPAGECOMMITMENTEXT glNamedBufferPageCommitmentEXT;

#ifdef __cplusplus
}
#endif

extern bool init_glext();
