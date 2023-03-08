/*
 * Copyright (C) 2023  Christopher J. Howard
 *
 * This file is part of Antkeeper source code.
 *
 * Antkeeper source code is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Antkeeper source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Antkeeper source code.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ANTKEEPER_INPUT_SCANCODE_HPP
#define ANTKEEPER_INPUT_SCANCODE_HPP

#include <cstdint>

namespace input {

/**
 * Keyboard scancodes.
 *
 * @see HID Usage Tables for Universal Serial Bus (USB) version 1.3, 2022, https://usb.org/sites/default/files/hut1_3_0.pdf.
 */
enum class scancode: std::uint16_t
{
	// reserved            = 0x00,
	error_roll_over        = 0x01,
	post_fail              = 0x02,
	error_undefined        = 0x03,
	a                      = 0x04,
	b                      = 0x05,
	c                      = 0x06,
	d                      = 0x07,
	e                      = 0x08,
	f                      = 0x09,
	g                      = 0x0a,
	h                      = 0x0b,
	i                      = 0x0c,
	j                      = 0x0d,
	k                      = 0x0e,
	l                      = 0x0f,
	m                      = 0x10,
	n                      = 0x11,
	o                      = 0x12,
	p                      = 0x13,
	q                      = 0x14,
	r                      = 0x15,
	s                      = 0x16,
	t                      = 0x17,
	u                      = 0x18,
	v                      = 0x19,
	w                      = 0x1a,
	x                      = 0x1b,
	y                      = 0x1c,
	z                      = 0x1d,
	digit_1                = 0x1e,
	digit_2                = 0x1f,
	digit_3                = 0x20,
	digit_4                = 0x21,
	digit_5                = 0x22,
	digit_6                = 0x23,
	digit_7                = 0x24,
	digit_8                = 0x25,
	digit_9                = 0x26,
	digit_0                = 0x27,
	enter                  = 0x28,
	escape                 = 0x29,
	backspace              = 0x2a,
	tab                    = 0x2b,
	space                  = 0x2c,
	minus                  = 0x2d,
	equal                  = 0x2e,
	left_brace             = 0x2f,
	right_brace            = 0x30,
	backslash              = 0x31,
	non_us_hash            = 0x32,
	semicolon              = 0x33,
	apostrophe             = 0x34,
	grave                  = 0x35,
	comma                  = 0x36,
	dot                    = 0x37,
	slash                  = 0x38,
	caps_lock              = 0x39,
	f1                     = 0x3a,
	f2                     = 0x3b,
	f3                     = 0x3c,
	f4                     = 0x3d,
	f5                     = 0x3e,
	f6                     = 0x3f,
	f7                     = 0x40,
	f8                     = 0x41,
	f9                     = 0x42,
	f10                    = 0x43,
	f11                    = 0x44,
	f12                    = 0x45,
	print_screen           = 0x46,
	scroll_lock            = 0x47,
	pause                  = 0x48,
	insert                 = 0x49,
	home                   = 0x4a,
	page_up                = 0x4b,
	del                    = 0x4c,
	end                    = 0x4d,
	page_down              = 0x4e,
	right                  = 0x4f,
	left                   = 0x50,
	down                   = 0x51,
	up                     = 0x52,
	num_lock               = 0x53,
	kp_slash               = 0x54,
	kp_asterisk            = 0x55,
	kp_minus               = 0x56,
	kp_plus                = 0x57,
	kp_enter               = 0x58,
	kp_1                   = 0x59,
	kp_2                   = 0x5a,
	kp_3                   = 0x5b,
	kp_4                   = 0x5c,
	kp_5                   = 0x5d,
	kp_6                   = 0x5e,
	kp_7                   = 0x5f,
	kp_8                   = 0x60,
	kp_9                   = 0x61,
	kp_0                   = 0x62,
	kp_dot                 = 0x63,
	non_us_backslash       = 0x64,
	application            = 0x65,
	power                  = 0x66,
	kp_equal               = 0x67,
	f13                    = 0x68,
	f14                    = 0x69,
	f15                    = 0x6a,
	f16                    = 0x6b,
	f17                    = 0x6c,
	f18                    = 0x6d,
	f19                    = 0x6e,
	f20                    = 0x6f,
	f21                    = 0x70,
	f22                    = 0x71,
	f23                    = 0x72,
	f24                    = 0x73,
	execute                = 0x74,
	help                   = 0x75,
	menu                   = 0x76,
	select                 = 0x77,
	stop                   = 0x78,
	again                  = 0x79,
	undo                   = 0x7a,
	cut                    = 0x7b,
	copy                   = 0x7c,
	paste                  = 0x7d,
	find                   = 0x7e,
	mute                   = 0x7f,
	volume_up              = 0x80,
	volume_down            = 0x81,
	locking_caps_lock      = 0x82,
	locking_num_lock       = 0x83,
	locking_scroll_lock    = 0x84,
	kp_comma               = 0x85,
	kp_equal_as400         = 0x86,
	international_1        = 0x87,
	international_2        = 0x88,
	international_3        = 0x89,
	international_4        = 0x8a,
	international_5        = 0x8b,
	international_6        = 0x8c,
	international_7        = 0x8d,
	international_8        = 0x8e,
	international_9        = 0x8f,
	lang_1                 = 0x90,
	lang_2                 = 0x91,
	lang_3                 = 0x92,
	lang_4                 = 0x93,
	lang_5                 = 0x94,
	lang_6                 = 0x95,
	lang_7                 = 0x96,
	lang_8                 = 0x97,
	lang_9                 = 0x98,
	alt_erase              = 0x99,
	sys_req                = 0x9a,
	cancel                 = 0x9b,
	clear                  = 0x9c,
	prior                  = 0x9d,
	return_2               = 0x9e,
	separator              = 0x9f,
	_out                   = 0xa0,
	oper                   = 0xa1,
	clear_again            = 0xa2,
	cr_sel                 = 0xa3,
	ex_sel                 = 0xa4,
	// reserved            = 0xa5,
	// reserved            = 0xa6,
	// reserved            = 0xa7,
	// reserved            = 0xa8,
	// reserved            = 0xa9,
	// reserved            = 0xaa,
	// reserved            = 0xab,
	// reserved            = 0xac,
	// reserved            = 0xad,
	// reserved            = 0xae,
	// reserved            = 0xaf,
	kp_00                  = 0xb0,
	kp_000                 = 0xb1,
	thousands_separator    = 0xb2,
	decimal_separator      = 0xb3,
	currency_unit          = 0xb4,
	currency_sub_unit      = 0xb5,
	kp_left_paren          = 0xb6,
	kp_right_paren         = 0xb7,
	kp_left_brace          = 0xb8,
	kp_right_brace         = 0xb9,
	kp_tab                 = 0xba,
	kp_backspace           = 0xbb,
	kp_a                   = 0xbc,
	kp_b                   = 0xbd,
	kp_c                   = 0xbe,
	kp_d                   = 0xbf,
	kp_e                   = 0xc0,
	kp_f                   = 0xc1,
	kp_xor                 = 0xc2,
	kp_power               = 0xc3,
	kp_percent             = 0xc4,
	kp_less                = 0xc5,
	kp_greater             = 0xc6,
	kp_ampersand           = 0xc7,
	kp_double_ampersand    = 0xc8,
	kp_vertical_bar        = 0xc9,
	kp_double_vertical_bar = 0xca,
	kp_colon               = 0xcb,
	kp_hash                = 0xcc,
	kp_space               = 0xcd,
	kp_at                  = 0xce,
	kp_exclam              = 0xcf,
	kp_mem_store           = 0xd0,
	kp_mem_recall          = 0xd1,
	kp_mem_clear           = 0xd2,
	kp_mem_add             = 0xd3,
	kp_mem_subtract        = 0xd4,
	kp_mem_multiply        = 0xd5,
	kp_mem_divide          = 0xd6,
	kp_plus_minus          = 0xd7,
	kp_clear               = 0xd8,
	kp_clear_entry         = 0xd9,
	kp_binary              = 0xda,
	kp_octal               = 0xdb,
	kp_decimal             = 0xdc,
	kp_hexadecimal         = 0xdd,
	// reserved            = 0xde,
	// reserved            = 0xdf,
	left_ctrl              = 0xe0,
	left_shift             = 0xe1,
	left_alt               = 0xe2,
	left_gui               = 0xe3,
	right_ctrl             = 0xe4,
	right_shift            = 0xe5,
	right_alt              = 0xe6,
	right_gui              = 0xe7
	// reserved            = 0xe8-0xffff
};

} // namespace input

#endif // ANTKEEPER_INPUT_SCANCODE_HPP