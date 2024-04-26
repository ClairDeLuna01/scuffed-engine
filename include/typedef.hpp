#pragma once

#include <cstdint>
#include <memory>

#include "component.hpp"

#include <glm/glm.hpp>

using namespace glm;

class ShaderProgram;
class ElementBufferObject;
class GameObject;
class Mesh;
class Transform3D;
class Camera;
class UI;

// Typedefs to have Rust-style typenames
// clearly Rust is superior :clowneline:

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
typedef std::shared_ptr<GameObject> GameObjectPtr;
typedef std::shared_ptr<Mesh> MeshPtr;
typedef std::shared_ptr<Camera> CameraPtr;
typedef std::shared_ptr<class CubeMap> CubeMapPtr;
typedef std::shared_ptr<class Skybox> SkyboxPtr;

CameraPtr createCamera();
GameObjectPtr createGameObject(std::string name);
GameObjectPtr createGameObject();

typedef vec<3, u32, highp> uivec3;