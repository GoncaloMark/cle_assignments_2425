
#pragma once

#include <cstdint>

struct UTF8DecoderState {
    // Accumulated Unicode codepoint
    uint32_t codepoint = 0;
    // The number of bytes of used by the codepoint
    uint32_t number_bytes = 0;
    // Number of bytes remaining for the current character.
    int remaining_bytes = 0;
};

enum WordState {
    IN_WORD,
    OUT_WORD
};

// Processe a sigle byte and update the decoder state.
//
// The table bellow summarizes the UTF-8 format.
// The letter x indicates bits available for encoding bits of the
// character number.
//
// Char. number range  |        UTF-8 octet sequence
//    (hexadecimal)    |              (binary)
// --------------------+---------------------------------------------
// 0000 0000-0000 007F | 0xxxxxxx
// 0000 0080-0000 07FF | 110xxxxx 10xxxxxx
// 0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
// 0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
//
// See https://datatracker.ietf.org/doc/html/rfc3629 for more information.
//
// @returns -1 on invalid encoding
// @returns 0 if more bytes are needed, otherwise returns the number of
//  bytes that encodes the character.
int utf8_decode(UTF8DecoderState& state, uint16_t byte, uint32_t* codepoint);

bool utf8_is_letter(uint32_t codepoint);
bool utf8_is_space(uint32_t codepoint);
