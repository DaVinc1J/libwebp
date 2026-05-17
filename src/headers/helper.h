#ifndef HELPER_H
#define HELPER_H

#include "define.h"

u32 clamp(u32 n, u32 min, u32 max);
u32 minu32(u32 x, u32 y);
f32 maxf32(f32 x, f32 y);
void skip_bytes(bytes *p_p, usize n);
u64 read_u64_be(bytes *p_p);
u32 read_u32_be(bytes *p_p);
u16 read_u16_be(bytes *p_p);
u64 read_u64_le(bytes *p_p);
u32 read_u32_le(bytes *p_p);
u16 read_u16_le(bytes *p_p);
u8 read_u8(bytes *p_p);
u64 copy_u64_be(bytes p);
u32 copy_u32_be(bytes p);
u16 copy_u16_be(bytes p);
u64 copy_u64_le(bytes p);
u32 copy_u32_le(bytes p);
u16 copy_u16_le(bytes p);
u8 copy_u8(bytes p);

#endif
