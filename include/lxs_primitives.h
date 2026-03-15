#ifndef LXS_PRIMITIVES_H
#define LXS_PRIMITIVES_H

#include <stdint.h>

#define LXS_EVAL_AND(va, ma, vb, mb, v_out, m_out) \
	{ \
	(v_out) = (va) & (vb); \
	(m_out) = ((ma) | (mb)) & ~(((~(va)) & ~(ma)) | ((~(vb)) & ~(mb))); \
	}

#define LXS_EVAL_OR(va, ma, vb, mb, v_out, m_out) \
	{ \
	(v_out) = (va) | (vb); \
	(m_out) = ((ma) | (mb)) & ~(((va) & ~(ma)) | ((vb) & ~(mb))); \
	}

#define LXS_EVAL_XOR(va, ma, vb, mb, v_out, m_out) \
	{ \
	(v_out) = (va) ^ (vb); \
	(m_out) = (ma) | (mb); \
	}

#define LXS_EVAL_NOT(va, ma, v_out, m_out) \
	{ \
	(v_out) = ~(va); \
	(m_out) = (ma); \
	}

#define LXS_EVAL_NAND(va, ma, vb, mb, v_out, m_out) \
	{ \
	LXS_EVAL_AND((va), (ma), (vb), (mb), (v_out), (m_out)); \
	(v_out) = ~(v_out); \
	}

#define LXS_EVAL_NOR(va, ma, vb, mb, v_out, m_out) \
	{ \
	LXS_EVAL_OR((va), (ma), (vb), (mb), (v_out), (m_out)); \
	(v_out) = ~(v_out); \
	}

#define LXS_EVAL_XNOR(va, ma, vb, mb, v_out, m_out) \
	{ \
	LXS_EVAL_XOR((va), (ma), (vb), (mb), (v_out), (m_out)); \
	(v_out) = ~(v_out); \
	}

#define LXS_EVAL_BUF(va, ma, v_out, m_out) \
	{ \
	(v_out) = (va) | 0ULL; \
	(m_out) = (ma) | 0ULL; \
	}

#define LXS_EVAL_TRI(v_data, m_data, v_enable, m_enable, v_out, m_out) \
	{ \
	uint64_t lxs_tri_on = (v_enable) & ~((m_enable)); \
	uint64_t lxs_tri_off = ~((v_enable) | (m_enable)); \
	(v_out) = (v_data) & lxs_tri_on; \
	(m_out) = ((m_data) & lxs_tri_on) | ((m_enable) | ~(lxs_tri_on | lxs_tri_off)); \
	}

#define LXS_EVAL_DFF(v_in, m_in, v_state, m_state) \
	{ \
	(v_state) = (v_in) | 0ULL; \
	(m_state) = (m_in) | 0ULL; \
	}

#endif
