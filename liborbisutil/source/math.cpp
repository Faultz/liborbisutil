#include "math.h"

namespace liborbisutil
{
	namespace math
	{
		//static float InvSqrt(float F)
		//{
		//	// Performs two passes of Newton-Raphson iteration on the hardware estimate
		//	//    v^-0.5 = x
		//	// => x^2 = v^-1
		//	// => 1/(x^2) = v
		//	// => F(x) = x^-2 - v
		//	//    F'(x) = -2x^-3

		//	//    x1 = x0 - F(x0)/F'(x0)
		//	// => x1 = x0 + 0.5 * (x0^-2 - Vec) * x0^3
		//	// => x1 = x0 + 0.5 * (x0 - Vec * x0^3)
		//	// => x1 = x0 + x0 * (0.5 - 0.5 * Vec * x0^2)
		//	//
		//	// This final form has one more operation than the legacy factorization (X1 = 0.5*X0*(3-(Y*X0)*X0)
		//	// but retains better accuracy (namely InvSqrt(1) = 1 exactly).

		//	const __m128 fOneHalf = _mm_set_ss(0.5f);
		//	__m128 Y0, X0, X1, X2, FOver2;
		//	float temp;

		//	Y0 = _mm_set_ss(F);
		//	X0 = _mm_rsqrt_ss(Y0);	// 1/sqrt estimate (12 bits)
		//	FOver2 = _mm_mul_ss(Y0, fOneHalf);

		//	// 1st Newton-Raphson iterationb
		//	X1 = _mm_mul_ss(X0, X0);
		//	X1 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X1));
		//	X1 = _mm_add_ss(X0, _mm_mul_ss(X0, X1));

		//	// 2nd Newton-Raphson iteration
		//	X2 = _mm_mul_ss(X1, X1);
		//	X2 = _mm_sub_ss(fOneHalf, _mm_mul_ss(FOver2, X2));
		//	X2 = _mm_add_ss(X1, _mm_mul_ss(X1, X2));

		//	_mm_store_ss(&temp, X2);
		//	return temp;
		//}

		vector2::vector2()
		{
			this->x = this->y = 0;
		}

		vector2::vector2(float x, float y)
		{
			this->x = x;
			this->y = y;
		}

		vector2::vector2(float vec[2])
		{
			if (vec != 0)
			{
				this->x = vec[0];
				this->y = vec[1];
			}
			else
			{
				this->x = this->y = 0;
			}
		}

		vector2 vector2::operator+(const vector2 vec)
		{
			return vector2(this->x + vec.x, this->y + vec.y);
		}

		vector2 vector2::operator+(const float* vec)
		{
			return vector2(this->x + vec[0], this->y + vec[1]);
		}

		vector2 vector2::operator-(const vector2 vec)
		{
			return vector2(this->x - vec.x, this->y - vec.y);
		}

		vector2 vector2::operator-(const float* vec)
		{
			return vector2(this->x - vec[0], this->y - vec[1]);
		}

		vector2 vector2::operator-() const
		{
			return vector2(-x, -y);
		}

		vector2 vector2::operator*(const vector2 vec)
		{
			return vector2(this->x * vec.x, this->y * vec.y);
		}

		vector2 vector2::operator*(const float scalar)
		{
			return vector2(this->x * scalar, this->y * scalar);
		}

		void vector2::operator-=(const vector2 vec)
		{
			this->x -= vec.x;
			this->y -= vec.y;
		}

		void vector2::operator+=(const vector2 vec)
		{
			this->x += vec.x;
			this->y += vec.y;
		}

		void vector2::operator*=(const vector2 vec)
		{
			this->x *= vec.x;
			this->y *= vec.y;
		}

		void vector2::operator*=(const float scalar)
		{
			this->x *= scalar;
			this->y *= scalar;
		}

		bool vector2::operator==(const vector2 vec)
		{
			return (this->x == vec.x && this->y == vec.y);
		}

		bool vector2::operator==(const float value)
		{
			return (this->x == value && this->y == value);
		}

		bool vector2::operator!=(const vector2 vec)
		{
			return !operator==(vec);
		}

		bool vector2::operator!=(const float value)
		{
			return !operator==(value);
		}

		vector2::operator float* ()
		{
			return (float*)&this[0];
		}


		float vector2::size()
		{
			return sqrtf(x * x + y * y);
		}

		void vector2::normalize(float tolerance)
		{
			const float SquareSum = x * x + y * y;
			if (SquareSum > tolerance)
			{
				//const float Scale = InvSqrt(SquareSum);
				//x *= Scale;
				//y *= Scale;
				return;
			}
			x = 0.0f;
			y = 0.0f;
		}

		vector3::vector3()
		{
			this->x = this->y = 0;
		}

		vector3::vector3(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		vector3::vector3(float vec[3])
		{
			if (vec != 0)
			{
				this->x = vec[0];
				this->y = vec[1];
				this->z = vec[2];
			}
			else
			{
				this->x = this->y = this->z = 0;
			}
		}

		void vector3::set(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}

		vector3 vector3::operator+(const vector3& vec) const
		{
			return vector3(this->x + vec.x, this->y + vec.y, this->z + vec.z);
		}

		vector3 vector3::operator+(const float* vec) const
		{
			return vector3(this->x + vec[0], this->y + vec[1], this->z + vec[2]);
		}

		vector3 vector3::operator-(const vector3& vec) const
		{
			return vector3(this->x - vec.x, this->y - vec.y, this->z - vec.z);
		}

		vector3 vector3::operator-(const float* vec) const
		{
			return vector3(this->x - vec[0], this->y - vec[1], this->z - vec[2]);
		}

		vector3 vector3::operator-() const
		{
			return vector3(-x, -y, -z);
		}

		vector3 vector3::operator*(const vector3& vec) const
		{
			return vector3(this->x * vec.x, this->y * vec.y, this->z * vec.z);
		}

		vector3 vector3::operator*(const float scalar) const
		{
			return vector3(this->x * scalar, this->y * scalar, this->z * scalar);
		}

		vector3 vector3::operator/(const float f) const
		{
			float of(1.0f / f);
			return(vector3(x * of, y * of, z * of));
		}

		vector3 vector3::operator/(const vector3& vec) const
		{
			return vector3(x / vec.x, y / vec.y, z / vec.z);
		}

		vector3 vector3::operator|(const vector3& vec) const
		{
			vector3 ret;
			uint32_t* pRet = (uint32_t*)&ret.x;
			uint32_t* pThis = (uint32_t*)this;
			uint32_t* pOther = (uint32_t*)&vec.x;
			pRet[0] = pThis[0] | pOther[0];
			pRet[1] = pThis[1] | pOther[1];
			pRet[2] = pThis[2] | pOther[2];
			return ret;
		}

		vector3 vector3::operator&(const vector3& vec) const
		{
			vector3 ret;
			uint32_t* pRet = (uint32_t*)&ret.x;
			uint32_t* pThis = (uint32_t*)this;
			uint32_t* pOther = (uint32_t*)&vec.x;
			pRet[0] = pThis[0] & pOther[0];
			pRet[1] = pThis[1] & pOther[1];
			pRet[2] = pThis[2] & pOther[2];
			return ret;
		}

		vector3 vector3::operator^(const vector3& vec) const
		{
			vector3 ret;
			uint32_t* pRet = (uint32_t*)&ret.x;
			uint32_t* pThis = (uint32_t*)this;
			uint32_t* pOther = (uint32_t*)&vec.x;
			pRet[0] = pThis[0] ^ pOther[0];
			pRet[1] = pThis[1] ^ pOther[1];
			pRet[2] = pThis[2] ^ pOther[2];
			return ret;
		}

		void vector3::operator/=(const vector3& vec)
		{
			inv_scale(vec);
		}

		void vector3::operator|=(const vector3& vec) const
		{
			uint32_t* pThis = (uint32_t*)this;
			uint32_t* pOther = (uint32_t*)&vec.x;
			pThis[0] |= pOther[0];
			pThis[1] |= pOther[1];
			pThis[2] |= pOther[2];
		}

		void vector3::operator&=(const vector3& vec) const
		{
			uint32_t* pThis = (uint32_t*)this;
			uint32_t* pOther = (uint32_t*)&vec.x;
			pThis[0] &= pOther[0];
			pThis[1] &= pOther[1];
			pThis[2] &= pOther[2];
		}

		void vector3::operator^=(const vector3& vec) const
		{
			uint32_t* pThis = (uint32_t*)this;
			uint32_t* pOther = (uint32_t*)&vec.x;
			pThis[0] ^= pOther[0];
			pThis[1] ^= pOther[1];
			pThis[2] ^= pOther[2];
		}

		bool vector3::operator==(const vector3& vec) const
		{
			return (this->x == vec.x && this->y == vec.y && this->z == vec.z);
		}

		bool vector3::operator==(const float value) const
		{
			return (this->x == value && this->y == value && this->z == value);
		}

		bool vector3::operator!=(const vector3& vec) const
		{
			return !operator==(vec);
		}

		bool vector3::operator!=(const float value) const
		{
			return !operator==(value);
		}

		void vector3::operator-=(const vector3& vec)
		{
			this->x -= vec.x;
			this->y -= vec.y;
			this->z -= vec.z;
		}

		void vector3::operator+=(const vector3& vec)
		{
			this->x += vec.x;
			this->y += vec.y;
			this->z += vec.z;
		}

		void vector3::operator*=(const vector3& vec)
		{
			this->x *= vec.x;
			this->y *= vec.y;
			this->z *= vec.z;
		}

		void vector3::operator*=(const float scalar)
		{
			this->x *= scalar;
			this->y *= scalar;
			this->z *= scalar;
		}

		void vector3::operator/=(const float scalar)
		{
			float of(1.0f / scalar); x *= of; y *= of; z *= of;
		}

		float vector3::operator[](const size_t index)
		{
			return*((float*)&x + index);
		}

		vector3 operator*(const float f, const vector3& vec)
		{
			return(vector3(f * vec.x, f * vec.y, f * vec.z));
		}

		vector3::operator float* ()
		{
			return (float*)&this[0];
		}

		float vector3::mag() const
		{
			return sqrtf(x * x + y * y + z * z);
		}

		float vector3::square_mag() const
		{
			return  x * x + y * y + z * z;
		}

		vector3 vector3::abs() const
		{
			return vector3(fabsf(x), fabsf(y), fabsf(z));
		}

		vector3 vector3::cross(const vector3& a) const
		{
			vector3 vector;
			vector.z = x * a.y - y * a.x;
			vector.x = y * a.z - z * a.y;
			vector.y = z * a.x - x * a.z;
			return vector;
		}

		float vector3::clamp_angle(float r) const
		{
			float q = r - ((int)(r * (1.0f / (2.0f * M_PI))) * (2.0f * M_PI));
			return(q > M_PI ? q - (2.0f * M_PI) : (q < -M_PI ? q + (2.0f * M_PI) : q));
		}

		bool vector3::is_zero()
		{
			return x == 0.f && y == 0.f && z == 0.f;
		}

		void vector3::inv_scale(const vector3& vec)
		{
			x /= vec.x;
			y /= vec.y;
			z /= vec.z;
		}

		void vector3::extend(float distance)
		{
			float m = mag();
			scale((m + distance) / m);
		}

		void vector3::extend(const vector3& v, float distance)
		{
			float m = v.mag();
			scale(v, (m + distance) / m);
		}

		void vector3::scale(float f)
		{
			x *= f; y *= f; z *= f;
		}

		void vector3::scale(const vector3& a, float f)
		{
			x = a.x * f; y = a.y * f; z = a.z * f;
		}

		void vector3::vectors(vector3* forward, vector3* right, vector3* up)
		{
			float angle;
			float sr, sp, sy, cr, cp, cy;

			angle = this->y * (M_PI * 2.0f / 360.0f);
			sy = sinf(angle);
			cy = cosf(angle);

			angle = this->x * (M_PI * 2.0f / 360.0f);
			sp = sinf(angle);
			cp = cosf(angle);

			angle = this->z * (M_PI * 2.0f / 360.0f);
			sr = sinf(angle);
			cr = cosf(angle);

			if (forward)
			{
				forward->x = cp * cy;
				forward->y = cp * sy;
				forward->z = -sp;
			}
			if (right)
			{
				right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
				right->y = (-1 * sr * sp * sy + -1 * cr * cy);
				right->z = -1 * sr * cp;
			}
			if (up)
			{
				up->x = (cr * sp * cy + -sr * -sy);
				up->y = (cr * sp * sy + -sr * cy);
				up->z = cr * cp;
			}
		}

		void vector3::lerp(float t, const vector3& a, const vector3& b)
		{
			x = a.x + (b.x - a.x) * t;
			y = a.y + (b.y - a.y) * t;
			z = a.z + (b.z - a.z) * t;
		}

		void vector3::lerp(float t, const vector3& a)
		{
			lerp(t, *this, a);
		}

		vector3 vector3::forward(float x)
		{
			float div = M_PI / 180.0f;

			float angle = this->y * div;
			float sy = sinf(angle);
			float cy = cosf(angle);

			angle = this->x * div;
			float sp = sinf(angle);
			float cp = cosf(angle);

			return vector3(cp * cy * x, cp * sy * x, -sp * x);
		}

		vector3 vector3::normal()
		{
			const float SquareSum = this->x * this->x + this->y * this->y + this->z * this->z;
			if (SquareSum > 1.e-8f)
			{
				//const float Scale = InvSqrt(SquareSum);
				//this->x *= Scale; this->y *= Scale; this->z *= Scale;
			}
			return *this;
		}

		vector3 vector3::normalize()
		{
			vector3 newvec;
			newvec.x = this->x / length();
			newvec.y = this->y / length();
			newvec.z = this->z / length();
			return newvec;
		}

		float vector3::distance(const vector3& vec)
		{
			vector3 delta = *this - vec;
			return delta.mag();
		}

		float vector3::length()
		{
			return (float)sqrtf(x * x + y * y + z * z);
		}

		float vector3::dot(const vector3& vector)
		{
			return x * vector.x + y * vector.y + z * vector.z;
		}

		vector4::vector4()
		{
			this->x = this->y = 0;
		}

		vector4::vector4(float x, float y, float z, float w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

		vector4::vector4(float vec[4])
		{
			if (vec != 0)
			{
				this->x = vec[0];
				this->y = vec[1];
				this->z = vec[2];
				this->w = vec[3];
			}
			else
			{
				this->x = this->y = this->z = this->w = 0;
			}
		}

		vector4 vector4::operator+(const vector4 vec)
		{
			return vector4(this->x + vec.x, this->y + vec.y, this->z + vec.z, this->w + vec.w);
		}

		vector4 vector4::operator+(const float* vec)
		{
			return vector4(this->x + vec[0], this->y + vec[1], this->z + vec[2], this->w + vec[3]);
		}

		vector4 vector4::operator-(const vector4 vec)
		{
			return vector4(this->x - vec.x, this->y - vec.y, this->z - vec.z, this->w - vec.w);
		}

		vector4 vector4::operator-(const float* vec)
		{
			return vector4(this->x - vec[0], this->y - vec[1], this->z - vec[2], this->w - vec[3]);
		}

		vector4 vector4::operator-() const
		{
			return vector4(-x, -y, -z, -w);
		}

		vector4 vector4::operator*(const vector4 vec)
		{
			return vector4(this->x * vec.x, this->y * vec.y, this->z * vec.z, this->w * vec.w);
		}

		vector4 vector4::operator*(const float scalar)
		{
			return vector4(this->x * scalar, this->y * scalar, this->z * scalar, this->w * scalar);
		}

		void vector4::operator-=(const vector4 vec)
		{
			this->x -= vec.x;
			this->y -= vec.y;
			this->z -= vec.z;
			this->w -= vec.w;
		}

		void vector4::operator+=(const vector4 vec)
		{
			this->x += vec.x;
			this->y += vec.y;
			this->z += vec.z;
			this->w += vec.w;
		}

		void vector4::operator*=(const vector4 vec)
		{
			this->x *= vec.x;
			this->y *= vec.y;
			this->z *= vec.z;
			this->w *= vec.w;
		}

		void vector4::operator*=(const float scalar)
		{
			this->x *= scalar;
			this->y *= scalar;
			this->z *= scalar;
			this->w *= scalar;
		}

		bool vector4::operator==(const vector4 vec)
		{
			return (this->x == vec.x && this->y == vec.y && this->z == vec.z && this->w == vec.w);
		}

		bool vector4::operator==(const float value)
		{
			return (this->x == value && this->y == value && this->z == value && this->w == value);
		}

		bool vector4::operator!=(const vector4 vec)
		{
			return !operator==(vec);
		}

		bool vector4::operator!=(const float value)
		{
			return !operator==(value);
		}

		vector4::operator float* ()
		{
			return (float*)&this[0];
		}

		matrix3x3 matrix3x3::inverse() const
		{
			matrix3x3 inv;

			float det = m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
				m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
				m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);

			if (det == 0)
			{
				// Matrix is singular, cannot compute inverse
				// You may want to handle this differently based on your application's requirements
				return inv;
			}

			float invDet = 1.0f / det;

			inv.m[0][0] = (m[1][1] * m[2][2] - m[1][2] * m[2][1]) * invDet;
			inv.m[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invDet;
			inv.m[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invDet;
			inv.m[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invDet;
			inv.m[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invDet;
			inv.m[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * invDet;
			inv.m[2][0] = (m[1][0] * m[2][1] - m[1][1] * m[2][0]) * invDet;
			inv.m[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * invDet;
			inv.m[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * invDet;

			return inv;
		}
	}
}