#ifndef BRAILLE_STRING_H_
#define BRAILLE_STRING_H_

#include <string>

namespace blindgui{

////////////////////////////////////////////////////////////////////////////////////
//Class Braille,as its name indicated,is a class to restore braille.It can
//also store invisible characters e.g. the newline character '\n'.
class Braille {
	unsigned dots :6;
	unsigned inv :1;
public:
	static const unsigned char NEWLINE = 0x40;
	static const unsigned char SPACE = 0x00;
	static const unsigned char BLACK = 0x3F;
	static const unsigned char C00 = 0x01;
	static const unsigned char C10 = 0x02;
	static const unsigned char C01 = 0x04;
	static const unsigned char C11 = 0x08;
	static const unsigned char C02 = 0x10;
	static const unsigned char C12 = 0x20;
	static const int width = 2;
	static const int height = 3;
	Braille() :
		dots(0), inv(0) {
	}
	Braille(unsigned char val) :
		dots(val), inv(val >> 6) {
	}
	bool operator()(int x, int y) const {
		return dots & (0x1 << ((y << 1) | x));
	}
	bool operator==(const Braille &c) const {
		return (dots == c.dots) && (dots == c.inv);
	}
	bool operator==(unsigned char uc) const {
		return !(bool) ((getVal() ^ uc) & 0x7F);
	}
	void set(int x, int y, bool b) {
		dots |= 0x1 << ((y << 1) | x);
	}
	void setVis() {
		inv = 0x1;
	}
	unsigned char getVal() const {
		return (((unsigned char) inv) << 6) | dots;
	}
	Braille &operator=(unsigned char val) {
		dots = val;
		inv = val >> 6;
		return *this;
	}
	bool isInv() const {
		return inv;
	}
};
typedef std::basic_string<Braille> braille_string;

}

#endif
