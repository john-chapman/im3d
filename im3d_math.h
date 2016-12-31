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

} // namespace Im3d

#endif // im3d_math_h
