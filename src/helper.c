#include "headers/define.h"
#include "headers/helper.h"

u32 clamp(u32 n, u32 min, u32 max) {
	if (n < min) return min;
	if (n > max) return max;
	return n;
}

u32 minu32(u32 x, u32 y) {
	if (x > y) {
		return y;
	}
	return x;
}

f32 maxf32(f32 x, f32 y) {
	if (x > y) {
		return x;
	}
	return y;
}

void skip_bytes(bytes *p_p, usize n) {
  *p_p += n;
}

u64 read_u64_be(bytes *p_p) {
	u8 b0 = *(*p_p)++;
	u8 b1 = *(*p_p)++;
	u8 b2 = *(*p_p)++;
	u8 b3 = *(*p_p)++;
	u8 b4 = *(*p_p)++;
	u8 b5 = *(*p_p)++;
	u8 b6 = *(*p_p)++;
	u8 b7 = *(*p_p)++;
	return (u64)b0 << 56 | (u64)b1 << 48 | (u64)b2 << 40 | (u64)b3 << 32 | (u64)b4 << 24 | (u64)b5 << 16 | (u64)b6 << 8 | (u64)b7;
}

u32 read_u32_be(bytes *p_p) {
	u8 b0 = *(*p_p)++;
	u8 b1 = *(*p_p)++;
	u8 b2 = *(*p_p)++;
	u8 b3 = *(*p_p)++;
	return (u32)b0 << 24 | (u32)b1 << 16 | (u32)b2 << 8 | (u32)b3;
}

u16 read_u16_be(bytes *p_p) {
	u8 b0 = *(*p_p)++;
	u8 b1 = *(*p_p)++;
	return (u16)b0 << 8 | (u16)b1;
}

u64 read_u64_le(bytes *p_p) {
	u8 b0 = *(*p_p)++;
	u8 b1 = *(*p_p)++;
	u8 b2 = *(*p_p)++;
	u8 b3 = *(*p_p)++;
	u8 b4 = *(*p_p)++;
	u8 b5 = *(*p_p)++;
	u8 b6 = *(*p_p)++;
	u8 b7 = *(*p_p)++;
	return (u64)b0 | (u64)b1 << 8 | (u64)b2 << 16 | (u64)b3 << 24 | (u64)b4 << 32 | (u64)b5 << 40 | (u64)b6 << 48 | (u64)b7 << 56;
}

u32 read_u32_le(bytes *p_p) {
	u8 b0 = *(*p_p)++;
	u8 b1 = *(*p_p)++;
	u8 b2 = *(*p_p)++;
	u8 b3 = *(*p_p)++;
	return (u32)b0 | (u32)b1 << 8 | (u32)b2 << 16 | (u32)b3 << 24;
}

u16 read_u16_le(bytes *p_p) {
	u8 b0 = *(*p_p)++;
	u8 b1 = *(*p_p)++;
	return (u16)b0 | (u16)b1;
}

u8 read_u8(bytes *p_p) {
	u8 byte = *(*p_p)++;
	return byte;
}

u64 copy_u64_be(bytes p) {
    return (u64)p[0]<<56 | (u64)p[1]<<48 | (u64)p[2]<<40 | (u64)p[3]<<32
         | (u64)p[4]<<24 | (u64)p[5]<<16 | (u64)p[6]<<8  | (u64)p[7];
}
u32 copy_u32_be(bytes p) {
    return (u32)p[0]<<24 | (u32)p[1]<<16 | (u32)p[2]<<8 | (u32)p[3];
}
u16 copy_u16_be(bytes p) {
    return (u16)p[0]<<8 | (u16)p[1];
}
u8 copy_u8(bytes p) {
    return p[0];
}
u64 copy_u64_le(bytes p) {
    return (u64)p[0] | (u64)p[1]<<8  | (u64)p[2]<<16 | (u64)p[3]<<24
         | (u64)p[4]<<32 | (u64)p[5]<<40 | (u64)p[6]<<48 | (u64)p[7]<<56;
}
u32 copy_u32_le(bytes p) {
    return (u32)p[0] | (u32)p[1]<<8 | (u32)p[2]<<16 | (u32)p[3]<<24;
}
u16 copy_u16_le(bytes p) {
    return (u16)p[0] | (u16)p[1]<<8;
}
