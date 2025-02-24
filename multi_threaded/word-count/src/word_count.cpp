
#include <iostream>
#include <fstream>

#include "utf-8.h"
#include "word_count.h"

void word_count(const WordCountFlags& flags, const std::vector<std::string>& files)
{
    // Make sure to use the flags to determine what to count
    // Enter your code here !!!
    WordCounts total;
    for (auto& file : files){
        // std::cout << file << std::endl;

        // test_decoder();
        std::ifstream fh(file, std::ios::binary);
        if(!fh.is_open()){
            throw std::runtime_error("invalid path");
        }

        UTF8DecoderState state;
        WordCounts counts;
        WordState word_state = OUT_WORD;

        while(fh){
            uint32_t codepoint = 0;
            uint16_t byte;

            if (!fh.read(reinterpret_cast<char*>(&byte), 1) || fh.eof()) {
                break;
            }

            int decode_state = utf8_decode(state, byte, &codepoint);
            if (decode_state == -1) {
                std::cerr << "Error: Invalid UTF-8 byte" << std::endl;
                state = UTF8DecoderState(); 
                continue;
            }

            while (decode_state == 0) {
                if (!fh.read(reinterpret_cast<char*>(&byte), 1) || fh.eof()) {
                    std::cerr << "Error: Incomplete UTF-8" << std::endl;
                    return;
                }
                decode_state = utf8_decode(state, byte, &codepoint);
                if (decode_state == -1) {
                    std::cerr << "Error: Invalid UTF-8 continuation byte" << std::endl;
                    state = UTF8DecoderState();
                    break;
                }
            }
            
            if (decode_state > 0) {
                if(flags.chars){
                    counts.chars++;
                }

                if(flags.lines){
                    if(codepoint == 0xA){
                        counts.lines++;
                    }
                }

                if(flags.words){
                    switch(word_state){
                        case OUT_WORD:
                            if(!utf8_is_space(codepoint)){
                                counts.words++;
                                word_state = IN_WORD;
                            }
                            break;

                        case IN_WORD:
                            if(utf8_is_space(codepoint)){
                                word_state = OUT_WORD;
                            }
                            break;
                    }
                }

                state = UTF8DecoderState(); 
            }
        }
        std::cout << file << ": " << counts.chars << " " << counts.lines << " " << counts.words << "\n";
        total.words += counts.words;
        total.chars += counts.chars;
        total.lines += counts.lines;
        
        fh.close();
    }
    std::cout << "total: " << total.chars << " " << total.lines << " " << total.words << std::endl;
}
// 3570829  22297610 141738391 total (from wc)
// total: 141738391 3570829 22297610 (my prog)