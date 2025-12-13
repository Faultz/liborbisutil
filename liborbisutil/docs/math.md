# Math Module

Provides vector and quaternion math classes for 3D graphics and game development.

## Classes

### `vector2`

2D vector class with x, y components.

```cpp
liborbisutil::math::vector2 pos(1.0f, 2.0f);
liborbisutil::math::vector2 dir(0.5f, 0.5f);

// Operations
vector2 result = pos + dir;
result *= 2.0f;
result.normalize(0.0001f);
float len = result.size();
```

### `vector3`

3D vector class with x, y, z components.

```cpp
liborbisutil::math::vector3 position(1.0f, 2.0f, 3.0f);
liborbisutil::math::vector3 velocity(0.1f, 0.0f, 0.2f);

// Basic operations
vector3 newPos = position + velocity;
vector3 scaled = position * 2.0f;

// Cross product
vector3 normal = vec1.cross(vec2);

// Magnitude
float length = position.mag();
float sqLength = position.square_mag();

// Utility
vector3 absVec = position.abs();
position.extend(5.0f);  // Extend in current direction
```

### `vector4`

4D vector class with x, y, z, w components.

```cpp
liborbisutil::math::vector4 color(1.0f, 0.5f, 0.0f, 1.0f);
liborbisutil::math::vector4 plane(0.0f, 1.0f, 0.0f, 5.0f);
```

### `quaternion`

Quaternion class for rotations.

```cpp
liborbisutil::math::quaternion rot;
rot.set_euler(pitch, yaw, roll);

// Quaternion operations
quaternion combined = rot1 * rot2;
vector3 rotated = rot.rotate(vector);
```

## Features

- Full operator overloading (+, -, *, /, ==, !=)
- Compound assignment operators (+=, -=, *=, /=)
- Implicit conversion to float pointer
- Cross product and dot product
- Magnitude and normalization
- Vector extension and scaling
