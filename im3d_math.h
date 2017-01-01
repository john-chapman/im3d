#pragma once
#ifndef im3d_math_h
#define im3d_math_h

#include "im3d.h"
#include "im3d_internal.h"

#include <cmath>

namespace Im3d {

inline Vec2  operator+(const Vec2& _lhs, const Vec2& _rhs) { return Vec2(_lhs.x + _rhs.x, _lhs.y + _rhs.y); }
inline Vec2  operator-(const Vec2& _lhs, const Vec2& _rhs) { return Vec2(_lhs.x - _rhs.x, _lhs.y - _rhs.y); }
inline Vec2  operator*(const Vec2& _lhs, const Vec2& _rhs) { return Vec2(_lhs.x * _rhs.x, _lhs.y * _rhs.y); }
inline Vec2  operator/(const Vec2& _lhs, const Vec2& _rhs) { return Vec2(_lhs.x / _rhs.x, _lhs.y / _rhs.y); }
inline Vec2  operator*(const Vec2& _lhs, float _rhs)       { return Vec2(_lhs.x * _rhs, _lhs.y * _rhs);     }
inline Vec2  operator/(const Vec2& _lhs, float _rhs)       { return Vec2(_lhs.x / _rhs, _lhs.y / _rhs);     }
inline float dot(const Vec2& _lhs, const Vec2& _rhs)       { return _lhs.x * _rhs.x + _lhs.y * _rhs.y;      }
inline float length(const Vec2& _v)                        { return sqrtf(dot(_v, _v));                     }
inline float length2(const Vec2& _v)                       { return dot(_v, _v);                            }
inline Vec2  abs(const Vec2& _v)                           { return Vec2(fabs(_v.x), fabs(_v.y));           }

inline Vec3  operator+(const Vec3& _lhs, const Vec3& _rhs) { return Vec3(_lhs.x + _rhs.x, _lhs.y + _rhs.y, _lhs.z + _rhs.z); }
inline Vec3  operator-(const Vec3& _lhs, const Vec3& _rhs) { return Vec3(_lhs.x - _rhs.x, _lhs.y - _rhs.y, _lhs.z - _rhs.z); }
inline Vec3  operator*(const Vec3& _lhs, const Vec3& _rhs) { return Vec3(_lhs.x * _rhs.x, _lhs.y * _rhs.y, _lhs.z * _rhs.z); }
inline Vec3  operator/(const Vec3& _lhs, const Vec3& _rhs) { return Vec3(_lhs.x / _rhs.x, _lhs.y / _rhs.y, _lhs.z / _rhs.z); }
inline Vec3  operator*(const Vec3& _lhs, float _rhs)       { return Vec3(_lhs.x * _rhs, _lhs.y * _rhs, _lhs.z * _rhs);       }
inline Vec3  operator/(const Vec3& _lhs, float _rhs)       { return Vec3(_lhs.x / _rhs, _lhs.y / _rhs, _lhs.z / _rhs);       }
inline float dot(const Vec3& _lhs, const Vec3& _rhs)       { return _lhs.x * _rhs.x + _lhs.y * _rhs.y + _lhs.z * _rhs.z;     }
inline float length(const Vec3& _v)                        { return sqrtf(dot(_v, _v));                                      }
inline float length2(const Vec3& _v)                       { return dot(_v, _v);                                             }
inline Vec3  abs(const Vec3& _v)                           { return Vec3(fabs(_v.x), fabs(_v.y), fabs(_v.z));                }

inline Vec4  operator+(const Vec4& _lhs, const Vec4& _rhs) { return Vec4(_lhs.x + _rhs.x, _lhs.y + _rhs.y, _lhs.z + _rhs.z, _lhs.w + _rhs.w); }
inline Vec4  operator-(const Vec4& _lhs, const Vec4& _rhs) { return Vec4(_lhs.x - _rhs.x, _lhs.y - _rhs.y, _lhs.z - _rhs.z, _lhs.w - _rhs.w); }
inline Vec4  operator*(const Vec4& _lhs, const Vec4& _rhs) { return Vec4(_lhs.x * _rhs.x, _lhs.y * _rhs.y, _lhs.z * _rhs.z, _lhs.w * _rhs.w); }
inline Vec4  operator/(const Vec4& _lhs, const Vec4& _rhs) { return Vec4(_lhs.x / _rhs.x, _lhs.y / _rhs.y, _lhs.z / _rhs.z, _lhs.w / _rhs.w); }
inline Vec4  operator*(const Vec4& _lhs, float _rhs)       { return Vec4(_lhs.x * _rhs, _lhs.y * _rhs, _lhs.z * _rhs, _lhs.w * _rhs);         }
inline Vec4  operator/(const Vec4& _lhs, float _rhs)       { return Vec4(_lhs.x / _rhs, _lhs.y / _rhs, _lhs.z / _rhs, _lhs.w / _rhs);         }
inline float dot(const Vec4& _lhs, const Vec4& _rhs)       { return _lhs.x * _rhs.x + _lhs.y * _rhs.y + _lhs.z * _rhs.z + _lhs.w * _rhs.w;    }
inline float length(const Vec4& _v)                        { return sqrtf(dot(_v, _v));                                                       }
inline float length2(const Vec4& _v)                       { return dot(_v, _v);                                                              }
inline Vec4  abs(const Vec4& _v)                           { return Vec4(fabs(_v.x), fabs(_v.y), fabs(_v.z), fabs(_v.w));                     }

inline Mat4  operator*(const Mat4& _lhs, const Mat4& _rhs)
{
	float m00 = _lhs[ 0] * _rhs[ 0] + _lhs[ 1] * _rhs[4] + _lhs[ 2] * _rhs[ 8] + _lhs[ 3] * _rhs[12];
	float m01 = _lhs[ 0] * _rhs[ 1] + _lhs[ 1] * _rhs[5] + _lhs[ 2] * _rhs[ 9] + _lhs[ 3] * _rhs[13];
	float m02 = _lhs[ 0] * _rhs[ 2] + _lhs[ 1] * _rhs[6] + _lhs[ 2] * _rhs[10] + _lhs[ 3] * _rhs[14];
	float m03 = _lhs[ 0] * _rhs[ 3] + _lhs[ 1] * _rhs[7] + _lhs[ 2] * _rhs[11] + _lhs[ 3] * _rhs[15];
	float m10 = _lhs[ 4] * _rhs[ 0] + _lhs[ 5] * _rhs[4] + _lhs[ 6] * _rhs[ 8] + _lhs[ 7] * _rhs[12];
	float m11 = _lhs[ 4] * _rhs[ 1] + _lhs[ 5] * _rhs[5] + _lhs[ 6] * _rhs[ 9] + _lhs[ 7] * _rhs[13];
	float m12 = _lhs[ 4] * _rhs[ 2] + _lhs[ 5] * _rhs[6] + _lhs[ 6] * _rhs[10] + _lhs[ 7] * _rhs[14];
	float m13 = _lhs[ 4] * _rhs[ 3] + _lhs[ 5] * _rhs[7] + _lhs[ 6] * _rhs[11] + _lhs[ 7] * _rhs[15];
	float m20 = _lhs[ 8] * _rhs[ 0] + _lhs[ 9] * _rhs[4] + _lhs[10] * _rhs[ 8] + _lhs[11] * _rhs[12];
	float m21 = _lhs[ 8] * _rhs[ 1] + _lhs[ 9] * _rhs[5] + _lhs[10] * _rhs[ 9] + _lhs[11] * _rhs[13];
	float m22 = _lhs[ 8] * _rhs[ 2] + _lhs[ 9] * _rhs[6] + _lhs[10] * _rhs[10] + _lhs[11] * _rhs[14];
	float m23 = _lhs[ 8] * _rhs[ 3] + _lhs[ 9] * _rhs[7] + _lhs[10] * _rhs[11] + _lhs[11] * _rhs[15];
	float m30 = _lhs[12] * _rhs[ 0] + _lhs[13] * _rhs[4] + _lhs[14] * _rhs[ 8] + _lhs[15] * _rhs[12];
	float m31 = _lhs[12] * _rhs[ 1] + _lhs[13] * _rhs[5] + _lhs[14] * _rhs[ 9] + _lhs[15] * _rhs[13];
	float m32 = _lhs[12] * _rhs[ 2] + _lhs[13] * _rhs[6] + _lhs[14] * _rhs[10] + _lhs[15] * _rhs[14];
	float m33 = _lhs[12] * _rhs[ 3] + _lhs[13] * _rhs[7] + _lhs[14] * _rhs[11] + _lhs[15] * _rhs[15];
	return Mat4(
		m00, m01, m02, m03,
		m10, m11, m12, m13,
		m20, m21, m22, m23,
		m30, m31, m32, m33
	);
}

inline Vec3 operator*(const Mat4& _mat, const Vec3& _pos)
{
	return Vec3(
		_mat[ 0] * _pos.x + _mat[ 1] * _pos.y + _mat[ 2] * _pos.z + _mat[ 3],
		_mat[ 4] * _pos.x + _mat[ 5] * _pos.y + _mat[ 6] * _pos.z + _mat[ 7],
		_mat[ 8] * _pos.x + _mat[ 9] * _pos.y + _mat[10] * _pos.z + _mat[11]
		);
}

} // namespace Im3d

#endif // im3d_math_h
