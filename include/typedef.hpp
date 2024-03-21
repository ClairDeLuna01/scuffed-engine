#pragma once

#include <cstdint>
#include <memory>

#include <glm/glm.hpp>

using namespace glm;

class ShaderProgram;
class ElementBufferObject;

// Typedefs to have Rust-style typenames
// clearly Rust is superior

typedef int8_t i8;
typedef uint8_t u8;

typedef int16_t i16;
typedef uint16_t u16;

typedef int32_t i32;
typedef uint32_t u32;

typedef int64_t i64;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef std::shared_ptr<ShaderProgram> ShaderProgramPtr;
typedef std::unique_ptr<ElementBufferObject> EBOptr;

typedef vec<3, u32, highp> uivec3;