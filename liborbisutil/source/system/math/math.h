#pragma once

#include <stdio.h>
#include <iostream>

namespace liborbisutil
{
	namespace math
	{
		class vector2
		{
		public:
			float x, y;

			vector2();
			vector2(float x, float y);
			vector2(float vec[2]);

			vector2 operator+(const vector2 vec);
			vector2 operator+(const float* vec);
			vector2 operator-(const vector2 vec);
			vector2 operator-(const float* vec);
			vector2 operator-() const;
			vector2 operator*(const vector2 vec);
			vector2 operator*(const float scalar);

			void operator-=(const vector2 vec);
			void operator+=(const vector2 vec);
			void operator*=(const vector2 vec);
			void operator*=(const float scalar);

			bool operator==(const vector2 vec);
			bool operator==(const float value);
			bool operator!=(const vector2 vec);
			bool operator!=(const float value);

			operator float* ();

			float size();
			void normalize(float tolerance);
		};

		class vector3
		{
		public:
			float x, y, z;

			vector3();
			vector3(float x, float y, float z);
			vector3(float vec[3]);

			void set(float x, float y, float z);

			vector3 operator+(const vector3& vec) const;
			vector3 operator+(const float* vec) const;
			vector3 operator-(const vector3& vec) const;
			vector3 operator-(const float* vec) const;
			vector3 operator-() const;
			vector3 operator*(const vector3& vec) const;
			vector3 operator*(const float scalar) const;
			vector3 operator/(const float f) const;
			vector3 operator/(const vector3& vec) const;
			vector3 operator|(const vector3& vec) const;
			vector3 operator&(const vector3& vec) const;
			vector3 operator^(const vector3& vec) const;

			void operator|=(const vector3& vec) const;
			void operator&=(const vector3& vec) const;
			void operator^=(const vector3& vec) const;

			bool operator==(const vector3& vec) const;
			bool operator==(const float value) const;
			bool operator!=(const vector3& vec) const;
			bool operator!=(const float value) const;

			void operator-=(const vector3& vec);
			void operator+=(const vector3& vec);
			void operator*=(const vector3& vec);
			void operator*=(const float scalar);
			void operator/=(const float scalar);
			void operator/=(const vector3& vec);

			friend vector3 operator*(const float f, const vector3& vec);

			float operator[](const size_t index);

			operator float* ();

			vector3 abs() const;
			vector3 cross(const vector3& a) const;
			float mag() const;
			float square_mag() const;
			float clamp_angle(float r) const;

			bool is_zero();

			void inv_scale(const vector3& vec);
			void extend(float distance);
			void extend(const vector3& v, float distance);
			void scale(float f);
			void scale(const vector3& a, float f);
			void vectors(vector3* forward, vector3* right, vector3* up);
			void lerp(float t, const vector3& a, const vector3& b);
			void lerp(float t, const vector3& a);

			vector3 forward(float x = 1.0f);
			vector3 normal();
			vector3 normalize();

			float distance(const vector3& vec);
			float length();
			float dot(const vector3& vector);
		};

		class vector4
		{
		public:
			float x, y, z, w;

			vector4();
			vector4(float x, float y, float z, float w);
			vector4(float vec[4]);

			vector4 operator+(const vector4 vec);
			vector4 operator+(const float* vec);
			vector4 operator-(const vector4 vec);
			vector4 operator-(const float* vec);
			vector4 operator-() const;
			vector4 operator*(const vector4 vec);
			vector4 operator*(const float scalar);

			void operator-=(const vector4 vec);
			void operator+=(const vector4 vec);
			void operator*=(const vector4 vec);
			void operator*=(const float scalar);

			bool operator==(const vector4 vec);
			bool operator==(const float value);
			bool operator!=(const vector4 vec);
			bool operator!=(const float value);

			operator float* ();
		};

		struct matrix3x3
		{
			union
			{
				struct
				{
					vector3 aside;
					vector3 up;
					vector3 dir;
				};

				float	m[3][3];
				struct
				{
					float   _11, _12, _13;
					float	_21, _22, _23;
					float	_31, _32, _33;
				};
			};

			matrix3x3()
			{
				for (int i = 0; i < 3; ++i)
					for (int j = 0; j < 3; ++j)
						m[i][j] = 0.0f;
			}

			static matrix3x3 lookat(const vector3& eye, const vector3& target, const vector3& up) {
				vector3 forward = (target - eye).normalize();
				vector3 right = up.cross(forward).normalize();
				vector3 newUp = forward.cross(right);

				matrix3x3 result;

				result.m[0][0] = right.x;
				result.m[1][0] = right.y;
				result.m[2][0] = right.z;

				result.m[0][1] = newUp.x;
				result.m[1][1] = newUp.y;
				result.m[2][1] = newUp.z;

				result.m[0][2] = -forward.x;
				result.m[1][2] = -forward.y;
				result.m[2][2] = -forward.z;

				return result;
			}

			matrix3x3 inverse() const;
		};
	}
}