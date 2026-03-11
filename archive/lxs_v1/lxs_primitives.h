#ifndef LXS_PRIMITIVES_H
#define LXS_PRIMITIVES_H

#include <stdint.h>

/* AND Primitive */
#define LXS_EVAL_AND(va, ma, vb, mb, v_out, m_out) \
	{ \
	(v_out) = (va) & (vb); \
	(m_out) = ((ma) & (mb)) | ((ma) & (vb)) | ((mb) & (va)); \
	}

/* OR Primitive */
#define LXS_EVAL_OR(va, ma, vb, mb, v_out, m_out) \
	{ \
	(v_out) = (va) | (vb); \
	(m_out) = ((ma) & (mb)) | ((ma) & ~(vb)) | ((mb) & ~(va)); \
	}

/* XOR Primitive */
#define LXS_EVAL_XOR(va, ma, vb, mb, v_out, m_out) \
	{ \
	(v_out) = (va) ^ (vb); \
	(m_out) = (ma) | (mb); \
	}

/* NOT Primitive */
#define LXS_EVAL_NOT(va, ma, v_out, m_out) \
	{ \
	(v_out) = ~(va); \
	(m_out) = (ma); \
	}

/* NAND Primitive */
#define LXS_EVAL_NAND(va, ma, vb, mb, v_out, m_out) \
	{ \
	(v_out) = ~((va) & (vb)); \
	(m_out) = ((ma) & (mb)) | ((ma) & (vb)) | ((mb) & (va)); \
	}

/* NOR Primitive */
#define LXS_EVAL_NOR(va, ma, vb, mb, v_out, m_out) \
	{ \
	(v_out) = ~((va) | (vb)); \
	(m_out) = ((ma) & (mb)) | ((ma) & ~(vb)) | ((mb) & ~(va)); \
	}

/* XNOR Primitive */
#define LXS_EVAL_XNOR(va, ma, vb, mb, v_out, m_out) \
	{ \
	(v_out) = ~((va) ^ (vb)); \
	(m_out) = (ma) | (mb); \
	}

/* Buffer Primitive */
#define LXS_EVAL_BUF(va, ma, v_out, m_out) \
	{ \
	(v_out) = (va) | 0ULL; \
	(m_out) = (ma) | 0ULL; \
	}

/* Tristate Primitive */
#define LXS_EVAL_TRI(v_data, m_data, v_enable, m_enable, v_out, m_out) \
	{ \
	(v_out) = (v_data) | 0ULL; \
	(m_out) = (m_data) | (m_enable) | ~(v_enable); \
	}

/* D-Flip-Flop Primitive (Sequential State Update) */
#define LXS_EVAL_DFF(v_in, m_in, v_state, m_state) \
	{ \
	(v_state) = (v_in) | 0ULL; \
	(m_state) = (m_in) | 0ULL; \
	}

/* Ground (Logic 0) Primitive */
#define LXS_EVAL_GND(v_out, m_out) \
	{ \
	(v_out) = 0ULL; \
	(m_out) = 0ULL; \
	}

/* Power (Logic 1) Primitive */
#define LXS_EVAL_VCC(v_out, m_out) \
	{ \
	(v_out) = ~0ULL; \
	(m_out) = 0ULL; \
	}

#endif /* LXS_PRIMITIVES_H */
