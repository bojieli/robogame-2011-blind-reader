#ifndef TRANSLATE_H_
#define TRANSLATE_H_

#include <string>
#include "braille_string.h"

namespace blindgui{

braille_string Translate(std::string str){
	braille_string brlstr;
	typedef std::string::const_iterator cit;
	for(cit i=str.begin();i!=str.end();i++){
		Braille brl;
		switch(*i){
			case 'a':
			case 'A':
				brl = 0x01;
				break;
			case 'b':
			case 'B':
				brl = 0x05;
				break;
			case 'c':
			case 'C':
				brl = 0x03;
				break;
			case 'd':
			case 'D':
				brl = 0x0B;
				break;
			case 'e':
			case 'E':
				brl = 0x09;
				break;
			case 'f':
			case 'F':
				brl = 0x07;
				break;
			case 'g':
			case 'G':
				brl = 0x0F;
				break;
			case 'h':
			case 'H':
				brl = 0x0D;
				break;
			case 'i':
			case 'I':
				brl = 0x06;
				break;
			case 'j':
			case 'J':
				brl = 0x0E;
				break;
			case 'k':
			case 'K':
				brl = 0x11;
				break;
			case 'l':
			case 'L':
				brl = 0x15;
				break;
			case 'm':
			case 'M':
				brl = 0x13;
				break;
			case 'n':
			case 'N':
				brl = 0x1B;
				break;
			case 'o':
			case 'O':
				brl = 0x19;
				break;
			case 'p':
			case 'P':
				brl = 0x17;
				break;
			case 'q':
			case 'Q':
				brl = 0x1F;
				break;
			case 'r':
			case 'R':
				brl = 0x1D;
				break;
			case 's':
			case 'S':
				brl = 0x16;
				break;
			case 't':
			case 'T':
				brl = 0x1E;
				break;
			case 'u':
			case 'U':
				brl = 0x31;
				break;
			case 'v':
			case 'V':
				brl = 0x35;
				break;
			case 'w':
			case 'W':
				brl = 0x2E;
				break;
			case 'x':
			case 'X':
				brl = 0x33;
				break;
			case 'y':
			case 'Y':
				brl = 0x3B;
				break;
			case 'z':
			case 'Z':
				brl = 0x39;
				break;
			default:
				goto notmatcha;
		}
		if(*i>='A'&&*i<='Z')
			brlstr.push_back(Braille(0x20));
		brlstr.push_back(brl);
		notmatcha:
		switch(*i){
			case '0':
				brl = 0x0E;
				break;
			case '1':
				brl = 0x01;
				break;
			case '2':
				brl = 0x04;
				break;
			case '3':
				brl = 0x03;
				break;
			case '4':
				brl = 0x0B;
				break;
			case '5':
				brl = 0x09;
				break;
			case '6':
				brl = 0x07;
				break;
			case '7':
				brl = 0x0F;
				break;
			case '8':
				brl = 0x0D;
				break;
			case '9':
				brl = 0x06;
				break;
			default:
				goto notmatchd;
		}
		brlstr.push_back(Braille(0x3A));
		brlstr.push_back(brl);
		notmatchd:
		switch(*i){
			case ',':
				brlstr.push_back(Braille(0x04));
				break;
			case '*':
				brlstr.push_back(Braille(0x18));
				brlstr.push_back(Braille(0x18));
				break;
			case ';':
				brlstr.push_back(Braille(0x14));
				break;
			case '/':
				brlstr.push_back(Braille(0x12));
				break;
			case ':':
				brlstr.push_back(Braille(0x0C));
				break;
			case '.':
				brlstr.push_back(Braille(0x2C));
				break;
			case '!':
				brlstr.push_back(Braille(0x1C));
				break;
			case '-':
				brlstr.push_back(Braille(0x20));
				break;
			case '(':
			case ')':
				brlstr.push_back(Braille(0x2C));
				break;
			case '[':
				brlstr.push_back(Braille(0x20));
				brlstr.push_back(Braille(0x2C));
				break;
			case ']':
				brlstr.push_back(Braille(0x2C));
				brlstr.push_back(Braille(0x20));
				break;
			case '\'':
				brlstr.push_back(Braille(0x02));
				break;
			case '?':
				brlstr.push_back(Braille(0x24));
				break;
			case '\n':
				brlstr.push_back(Braille(Braille::NEWLINE));
				break;
			case ' ':
			case '\t':
				brlstr.push_back(Braille(0x00));
			default:
				goto notmatchp;
		}
		notmatchp:;
	}
	return brlstr;
}

std::string BackTranslate(braille_string){}

}

#endif
