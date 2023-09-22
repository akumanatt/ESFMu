/*
 * ESFMu: emulator for the ESS "ESFM" enhanced OPL3 clone
 * Copyright (C) 2023 Kagamiin~
 *
 * ESFMu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1
 * of the License, or (at your option) any later version.
 *
 * ESFMu is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ESFMu. If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * ESFMu wouldn't have been possible without the hard work and dedication of
 * the retro computer hardware research and preservation community.
 *
 * We'd like to thank:
 *  - Nuke.YKT
 *        Developer of Nuked OPL3, which was the basis for ESFMu's code and
 *        also a great learning resource for Kagamiin~.
 *        Nuke.YKT also gives shoutouts on behalf of Nuked OPL3 to:
 *        - MAME Development Team(Jarek Burczynski, Tatsuyuki Satoh):
 *              Feedback and Rhythm part calculation information.
 *        - forums.submarine.org.uk(carbon14, opl3):
 *              Tremolo and phase generator calculation information.
 *        - OPLx decapsulated(Matthew Gambrell, Olli Niemitalo):
 *              OPL2 ROMs.
 *        - siliconpr0n.org(John McMaster, digshadow):
 *              YMF262 and VRC VII decaps and die shots.
 * - rainwarrior
 *       For performing the initial research on ESFM drivers and documenting
 *       ESS's patent on native mode operator organization
 * - jwt27
 *       For kickstarting the ESFM research project and compiling rainwarrior's
 *       findings and more in an accessible document ("ESFM Demystified")
 * - pachuco/CatButts
 *       For documenting ESS's patent on ESFM's feedback implementation, which
 *       was vital in getting ESFMu's sound output to be accurate
 * - And everybody who helped out with real hardware testing
 */

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _esfm_slot esfm_slot;
typedef struct _esfm_slot_internal esfm_slot_internal;
typedef struct _esfm_channel esfm_channel;
typedef struct _esfm_chip esfm_chip;


void ESFM_init (esfm_chip *chip);
void ESFM_write_reg (esfm_chip *chip, uint16_t address, uint8_t data);
void ESFM_write_reg_buffered (esfm_chip *chip, uint16_t address, uint8_t data);
void ESFM_write_port (esfm_chip *chip, uint8_t offset, uint8_t data);
uint8_t ESFM_readback_reg (esfm_chip *chip, uint16_t address);
uint8_t ESFM_read_port (esfm_chip *chip, uint8_t offset);
void ESFM_generate(esfm_chip *chip, int16_t *buf);
void ESFM_generate_stream(esfm_chip *chip, int16_t *sndptr, uint32_t num_samples);


// These are fake types just for syntax sugar.
// Beware of their underlying types when reading/writing to them.
#ifndef __NO_ESFM_FAST_TYPES
#ifndef __ESFM_FAST_TYPES
#define __ESFM_FAST_TYPES
#endif
#endif

#ifdef __ESFM_FAST_TYPES

typedef uint_fast8_t flag;
typedef uint_fast8_t uint2;
typedef uint_fast8_t uint3;
typedef uint_fast8_t uint4;
typedef uint_fast8_t uint5;
typedef uint_fast8_t uint6;
typedef uint_fast8_t uint8;
typedef uint_fast16_t uint9;
typedef uint_fast16_t uint10;
typedef uint_fast16_t uint11;
typedef uint_fast16_t uint12;
typedef uint_fast16_t uint16;
typedef uint_fast32_t uint19;
typedef uint_fast32_t uint23;
typedef uint_fast32_t uint32;
typedef uint_fast64_t uint36;

typedef int_fast16_t int13;
typedef int_fast16_t int16;
typedef int_fast32_t int32;

#else
typedef uint8_t flag;
typedef uint8_t uint2;
typedef uint8_t uint3;
typedef uint8_t uint4;
typedef uint8_t uint5;
typedef uint8_t uint6;
typedef uint8_t uint8;
typedef uint16_t uint9;
typedef uint16_t uint10;
typedef uint16_t uint11;
typedef uint16_t uint12;
typedef uint16_t uint16;
typedef uint32_t uint19;
typedef uint32_t uint23;
typedef uint32_t uint32;
typedef uint64_t uint36;

typedef int16_t int13;
typedef int16_t int16;
typedef int32_t int32;

#endif

enum eg_states
{
	EG_ATTACK,
	EG_DECAY,
	EG_SUSTAIN,
	EG_RELEASE
};


typedef struct _esfm_write_buf
{
	uint64_t timestamp;
	uint16_t address;
	uint8_t data;
	flag valid;

} esfm_write_buf;

typedef struct _emu_slot_channel_mapping
{
	int channel_idx;
	int slot_idx;

} emu_slot_channel_mapping;

typedef struct _esfm_slot_internal
{
	uint9 eg_position;
	uint9 eg_ksl_offset;
	uint10 eg_output;

	uint4 keyscale;

	int13 output;
	int13 prev_output;
	int13 emu_output_enable;
	int13 emu_mod_enable;
	int13 feedback_buf;
	int13 *mod_input;

	uint19 phase_acc;
	uint10 phase_out;
	flag phase_reset;
	flag *key_on;

	uint2 eg_state;
	flag eg_delay_run;
	uint9 eg_delay_counter;

} esfm_slot_internal;

struct _esfm_slot
{
	// Metadata
	esfm_channel *channel;
	esfm_chip *chip;
	uint2 slot_idx;

	// Register data
	int13 out_enable[2];
	uint10 f_num;
	uint3 block;
	uint3 output_level;
	// a.k.a. feedback level in emu mode
	uint3 mod_in_level;

	uint6 t_level;
	uint4 mult;
	uint3 waveform;
	// Only for 4th slot
	uint2 rhy_noise;

	uint4 attack_rate;
	uint4 decay_rate;
	uint4 sustain_lvl;
	uint4 release_rate;

	flag tremolo_en;
	flag tremolo_deep;
	flag vibrato_en;
	flag vibrato_deep;
	flag emu_connection_typ;
	flag env_sustaining;
	flag ksr;
	uint2 ksl;
	uint3 env_delay;
	// overlaps with env_delay bit 0
	// TODO: check if emu mode only uses this, or if it actually overwrites the channel field used by native mode
	flag emu_key_on;

	// Internal state
	esfm_slot_internal in;
};

struct _esfm_channel
{
	esfm_chip *chip;
	esfm_slot slots[4];
	uint5 channel_idx;
	int16 output[2];
	flag key_on;
	flag emu_mode_4op_enable;
	// Only for 17th and 18th channels
	flag key_on_2;
	flag emu_mode_4op_enable_2;
};

#define ESFM_WRITEBUF_SIZE 1024
#define ESFM_WRITEBUF_DELAY 2

struct _esfm_chip
{
	esfm_channel channels[18];
	int32 output_accm[2];
	uint_fast16_t addr_latch;

	flag emu_wavesel_enable;
	flag emu_newmode;
	flag native_mode;

	flag keyscale_mode;

	// Global state
	uint36 eg_timer;
	uint10 global_timer;
	uint8 eg_clocks;
	flag eg_tick;
	flag eg_timer_overflow;
	uint8 tremolo;
	uint8 tremolo_pos;
	uint8 vibrato_pos;
	uint23 lfsr;

	flag rm_hh_bit2;
	flag rm_hh_bit3;
	flag rm_hh_bit7;
	flag rm_hh_bit8;
	flag rm_tc_bit3;
	flag rm_tc_bit5;

	// 0xbd register in emulation mode, exposed in 0x4bd in native mode
	// ("bass drum" register)
	uint8 emu_rhy_mode_flags;

	flag emu_vibrato_deep;
	flag emu_tremolo_deep;

	uint8 timer_reload[2];
	uint8 timer_counter[2];
	flag timer_enable[2];
	flag timer_mask[2];
	flag timer_overflow[2];
	flag irq_bit;

	// Halts the envelope generators from advancing.
	flag test_bit_eg_halt;
	/*
	 * Activates some sort of waveform test mode that amplifies the output volume greatly
	 * and continuously shifts the waveform table downwards, possibly also outputting the
	 * waveform's derivative? (it's so weird!)
	 */
	flag test_bit_distort;
	// Appears to attenuate the output by about 3 dB.
	flag test_bit_attenuate;
	// Resets all phase generators and holds them in the reset state while this bit is set.
	flag test_bit_phase_stop_reset;

	esfm_write_buf write_buf[ESFM_WRITEBUF_SIZE];
	size_t write_buf_start;
	size_t write_buf_end;
	uint64_t write_buf_timestamp;
};

#ifdef __cplusplus
}
#endif
