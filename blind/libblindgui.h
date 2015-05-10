#ifndef LIBBLINDGUI_H_
#define LIBBLINDGUI_H_

#include <map>
#include <string>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <boost/asio.hpp>
#include <sys/types.h>
#include <unistd.h>
#include "braille_string.h"

namespace blindgui {
////////////////////////////////////////////////////////////////////////////////////
//Class BitUtils is a collection of functions of bit operations.All functions
//in this class are static, so you don't need to create an instance of this class
//to use these functions.
//Note that no parameter validation checking will be done in these functions,
//you must do it by yourself.
class BitUtils {
	BitUtils();
public:
	static int bytes(int width, int height) {
		long bits = width * height;
		return (bits >> 3) + (bits & 0x7 ? 1 : 0);
	}
	static long xy2offset(int x, int y, int width) {
		return (long) y * (long) width + (long) x;
	}
	static unsigned char offset2mn(long offset, int &n) {
		n = offset >> 3;
		return 0x01 << (offset & 0x7);
	}
	static unsigned char xy2mn(int x, int y, int width, int &n) {
		long offset = xy2offset(x, y, width);
		return offset2mn(offset, n);
	}
	static unsigned highMask(unsigned char m) {
		return m | (m << 1) | (m << 2) | (m << 3) | (m << 4) | (m << 5) | (m
				<< 6) | (m << 7);
	}
	static unsigned lowMask(unsigned char m) {
		return m | (m >> 1) | (m >> 2) | (m >> 3) | (m >> 4) | (m >> 5) | (m
				>> 6) | (m >> 7);
	}
	static void forEach(unsigned char *src, long offset1, long offset2,
			void(*f)(unsigned char &chr, unsigned char mask, void *paras),
			void *paras = 0) {
		//offset1 must be less than offset2,both offset1 and offset2 must be positive.
		//function *f will be called for chars in src in an increasing order of index.
		//the parameter after f, i.e. void *paras[], will be passed to f unchanged.
		unsigned char ml, mh;
		int nl, nh;
		ml = xy2mn(offset1, 0, 0, nl);
		ml = highMask(ml);
		mh = xy2mn(offset2, 0, 0, nh);
		mh = lowMask(mh);
		if (nl == nh) {
			ml &= mh;
			(*f)(src[nl], ml, paras);
		} else {
			(*f)(src[nl], ml, paras);
			for (nl++; nl < nh; nl++)
				(*f)(src[nl], 0xFF, paras);
			(*f)(src[nh], mh, paras);
		}
	}
	static void fillByMask(unsigned char &chr, unsigned char mask,
			void *notused = 0) {
		chr |= mask;
	}
	static void clearByMask(unsigned char &chr, unsigned char mask,
			void *notused = 0) {
		chr &= ~mask;
	}
	static void reverseByMask(unsigned char &chr, unsigned char mask,
			void *notused = 0) {
		chr ^= mask;
	}
	static void testByMask(unsigned char &chr, unsigned char mask,
			void *bool_pointer_to_result) {
		*(bool *) bool_pointer_to_result = chr & mask;
	}
	static void copyByMask(unsigned char &dest, unsigned char mask,
			void *char_pointer_to_source) {
		unsigned char src = *(unsigned char *) char_pointer_to_source;
		dest &= ~mask;
		src &= mask;
		dest |= src;
	}
	static void compareByMask(unsigned char &src1, unsigned char mask,
			void *array_of_src2_and_result) {
		unsigned char src2 =
				*(unsigned char*) (((void**) array_of_src2_and_result)[0]);
		bool *result = (bool *) (((void**) array_of_src2_and_result)[1]);
		*result = ((src1 & mask) == (src2 & mask));
	}
	static void copy(unsigned char *dest, const unsigned char *src,
			long dest_offset, long src_offset, long len) {
		int ndest1, nsrc1;
		unsigned char mdest1, msrc1;
		mdest1 = xy2mn(dest_offset, 0, 0, ndest1);
		msrc1 = xy2mn(src_offset, 0, 0, nsrc1);
		int ndest2, nsrc2;
		unsigned char mdest2;
		mdest2 = xy2mn(dest_offset + len - 1, 0, 0, ndest2);
		xy2mn(src_offset + len - 1, 0, 0, nsrc2);
		if (mdest1 > msrc1) {//left shift
			int sv = 0;
			unsigned char m = msrc1;
			while (mdest1 != m) {
				m <<= 1;
				sv++;
			}
			mdest1 = highMask(mdest1);
			mdest2 = lowMask(mdest2);
			if (ndest1 == ndest2) {
				m = mdest1 & mdest2;
				unsigned char s = src[nsrc1] << sv;
				copyByMask(dest[ndest1], m, &s);
			} else {
				unsigned char s = src[nsrc1] << sv;
				copyByMask(dest[ndest1], mdest1, &s);
				for (ndest1++, nsrc1++; ndest1 < ndest2; ndest1++) {
					short sm = *(short*) (src + nsrc1 - 1);
					sm <<= sv;
					s = *((char*) &sm + 1);
					copyByMask(dest[ndest1], 0xFF, &s);
				}
				short sm = *(short *) (src + nsrc2 - 1);
				sm <<= sv;
				s = *((unsigned char*) &sm + 1);
				copyByMask(dest[ndest2], mdest2, &s);
			}
		} else {//right shift
			int sv = 0;
			unsigned char m = msrc1;
			while (mdest1 != m) {
				m >>= 1;
				sv++;
			}
			mdest1 = highMask(mdest1);
			mdest2 = lowMask(mdest2);
			if (ndest1 == ndest2) {
				short sm = *(short*) (src + nsrc1);
				m = mdest1 & mdest2;
				sm >>= sv;
				unsigned char s = sm;
				copyByMask(dest[ndest1], m, &s);
			} else {
				short sm = *(short*) (src + nsrc1);
				sm >>= sv;
				unsigned char s = sm;
				copyByMask(dest[ndest1], mdest1, &s);
				for (ndest1++, nsrc1++; ndest1 < ndest2; ndest1++) {
					short sm = *(short*) (src + nsrc1);
					sm >>= sv;
					s = sm;
					copyByMask(dest[ndest1], 0xFF, &s);
				}
				s = src[nsrc2];
				s >>= sv;
				copyByMask(dest[ndest2], mdest2, &s);
			}
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////
//Class Exception are used to handle exceptions,in this library, exception
//will be thrown only for unexpected cases,these cases are:
//1.unable to allocate memory with new
//2.unable to connect to daemon
//Note that your mistakes, e.g. trying to get the value of a nonexistent
//point, will only cause program runs unexpectedly,no exception will be thrown
class Exception {
public:
	Exception() throw () {
	}
	Exception(const Exception &e) throw () {
	}
	virtual ~Exception() throw () {
	}
	virtual Exception& operator=(const Exception&) throw () {
		return *this;
	}
	virtual const char *what() const throw () = 0;
};
class OutOfMemException: public Exception {
public:
	virtual const char *what() const throw () {
		return "Out of memory";
	}
};
class DaemonConnectException: public Exception {
public:
	virtual const char *what() const throw () {
		return "Daemon connect exception";
	}
};
////////////////////////////////////////////////////////////////////////////////////
//Class DisplayObject are ancestor of all display class. It's a virtual class
//so you can't create an instance of this class.
//Hit area is an area that can respond to user's click. Each hit area has a
//unique id. The type of id is std::string.
//Make sure your parameters are valid when using operator(), this will avoid
//getting the value of a point out of border,these will cause your program runs
//unexpectedly without any notification.
class DisplayObject {
	class HitAreas {
		friend class DisplayObject;
		const DisplayObject *parent;
		bool (DisplayObject::*nextHitArea)(int&, int&, int&, int&,std::string&) const;
		void (DisplayObject::*findHitAreaReset)() const;
		HitAreas(const DisplayObject *pthis,
				bool(DisplayObject::*pnextHitArea)(int&, int&, int&, int&,
						std::string&) const,
				void(DisplayObject::*pfindHitAreaReset)() const) :
			parent(pthis), nextHitArea(pnextHitArea), findHitAreaReset(
					pfindHitAreaReset) {
		}
	public:
		bool next(int &x, int &y, int &width, int&height, std::string &id) const {
			return (parent->*nextHitArea)(x, y, width, height, id);
		}
		void reset() const {
			(parent->*findHitAreaReset)();
		}
	};
	class Infos {
		friend class DisplayObject;
		const DisplayObject *parent;
		bool (DisplayObject::*nextInfo)(int&, int&, int&, int&,std::string&) const;
		void (DisplayObject::*findInfoReset)() const;
		Infos(const DisplayObject *pthis,
				bool(DisplayObject::*pnextInfo)(int&, int&, int&, int&, std::string&) const,
				void(DisplayObject::*pfindInfoReset)() const) :
			parent(pthis), nextInfo(pnextInfo), findInfoReset(pfindInfoReset) {
		}
	public:
		bool next(int &x, int &y, int &width, int&height, std::string &info) const {
			return (parent->*nextInfo)(x, y, width, height, info);
		}
		void reset() const {
			(parent->*findInfoReset)();
		}
	};
protected:
	virtual bool nextHitArea(int &x, int &y, int &width, int &height,std::string &id) const = 0;
	virtual void findHitAreaReset() const = 0;
	virtual bool nextInfo(int &x, int &y, int &width, int &height,std::string &id) const = 0;
	virtual void findInfoReset() const = 0;
	static void setParent(DisplayObject *child, DisplayObject *parent) {
		*const_cast<DisplayObject**> (&(child->*(&DisplayObject::parent)))
				= parent;
	}
	void drawToByPoint(unsigned char *dest, int x, int y, int width, int height) const {
		for(int dy=0;dy<height;dy++){
			int cy = y+dy;
			for(int dx=0;dx<width;dx++){
				int n;
				unsigned char mask;
				mask = BitUtils::xy2mn(dx,dy,width,n);
				int cx = x+dx;
				if(cx>=this->width()||cy>=this->height()||cx<0||cy<0){
					BitUtils::clearByMask(dest[n],mask);
				}else{
					if(operator()(cx,cy))
						BitUtils::fillByMask(dest[n],mask);
					else
						BitUtils::clearByMask(dest[n],mask);
				}
			}
		}
	}
public:
	DisplayObject() :
		parent(0), hitAreas(this, &DisplayObject::nextHitArea,&DisplayObject::findHitAreaReset),
		x(0), y(0),infos(this,&DisplayObject::nextInfo,&DisplayObject::findInfoReset) {}
	virtual ~DisplayObject() {}
	virtual int width() const = 0;
	virtual int height() const = 0;
	virtual void drawTo(unsigned char *dest) const = 0;
	virtual void drawTo(unsigned char *dest, int x, int y, int width,
			int height) const = 0;
	virtual bool operator()(int x, int y) const = 0;
	DisplayObject * const parent;
	const HitAreas hitAreas;
	const Infos infos;
	int x, y;
};
////////////////////////////////////////////////////////////////////////////////////
//Class Canvas is a class to draw graphics. Each Canvas has a determined
//width and height and you must give these parameters to the constructor.
//All graphic functions are packed into a member object called graphics.
//You can call this functions using statement like:
//	aaa.graphics.drawPoint(0,0);
//Note that you can draw something out of the Canvas' border.That means,
//you can get a semi-circle by drawing a circle with its' center on the
//border.
class Canvas: virtual public DisplayObject {
	const int w, h;
	unsigned char * const bitmap;
	mutable bool info;
	Canvas &operator=(const Canvas &src);
	class Graphics {
		friend class Canvas;
		unsigned char * const bitmap;
		const int w, h;
		int x, y;
		bool color;
		Canvas *parent;
		Graphics &operator=(const Graphics &);
		void symmetryDrawPoint(int x, int y, int deltax, int deltay, bool color) {
			drawPoint(x - deltax, y - deltay, color);
			drawPoint(x + deltax, y - deltay, color);
			drawPoint(x - deltax, y + deltay, color);
			drawPoint(x + deltax, y + deltay, color);
		}
		Graphics(Canvas *pthis, unsigned char *pbitmap, int pw, int ph) :
			bitmap(pbitmap), w(pw), h(ph), x(0), y(0), color(true), parent(
					pthis) {
		}
	public:
		void clear() {
			std::memset(bitmap, 0, BitUtils::bytes(w, h));
		}
		void setColor(bool color) {
			this->color = color;
		}
		void moveTo(int x, int y) {
			this->x = x;
			this->y = y;
		}
		void lineTo(int x, int y) {
			drawLine(this->x, this->y, x, y, color);
			this->x = x;
			this->y = y;
		}
		void drawPoint(int x, int y, bool color = true) {
			if (x >= 0 && x < w && y >= 0 && y < h) {
				int n;
				unsigned char m = BitUtils::xy2mn(x, y, w, n);
				if (color)
					BitUtils::fillByMask(bitmap[n], m);
				else
					BitUtils::clearByMask(bitmap[n], m);
			}
		}
		void drawLine(int x1, int y1, int x2, int y2, bool color = true) {
			if (x1 == x2) {//optimize for vertical line
				if (y1 > y2) {
					int y = y1;
					y1 = y2;
					y2 = y;
				}
				for (; y1 <= y2; y1++)
					drawPoint(x1, y1, color);
			} else if (y1 == y2) {//optimize for horizontal line
				if (y1 >= 0 && y1 < h) {
					if (x1 > x2) {
						int x = x1;
						x1 = x2;
						x2 = x;
					}
					if (x1 >= w || x2 < 0)
						return;
					if (x2 >= w)
						x2 = w - 1;
					if (x1 < 0)
						x1 = 0;
					int offset1 = y1 * w + x1;
					int offset2 = y1 * w + x2;
					if (color)
						BitUtils::forEach(bitmap, offset1, offset2,
								&BitUtils::fillByMask);
					else
						BitUtils::forEach(bitmap, offset1, offset2,
								&BitUtils::clearByMask);
				}
			} else {//general lines
				int x, y, delta;
				int d1, d2, *d, *ds, ds1;
				double k;
				if (std::abs(y2 - y1) > std::abs(x2 - x1)) {
					k = ((double) x2 - (double) x1) / ((double) y2
							- (double) y1);
					d1 = y1;
					d2 = y2;
					delta = (y2 > y1 ? 1 : -1);
					d = &y;
					ds = &x;
					ds1 = x1;
				} else {
					d1 = x1;
					d2 = x2;
					d = &x;
					ds = &y;
					ds1 = y1;
					delta = (x2 > x1 ? 1 : -1);
					k = ((double) y2 - (double) y1) / ((double) x2
							- (double) x1);
				}
				for (*d = d1; (*d - d2) * delta <= 0; *d += delta) {
					*ds = ds1 + (int) (k * (*d - d1) + 0.5);
					drawPoint(x, y, color);
				}
			}
		}
		void drawRect(int x, int y, int width, int height, bool fill = false,
				bool color = true) {
			if (width < 0 || height < 0)
				return;
			int x1 = x + width;
			int y1 = y + height;
			drawLine(x, y, x, y1, color);
			drawLine(x, y, x1, y, color);
			drawLine(x1, y, x1, y1, color);
			drawLine(x, y1, x1, y1, color);
			if (fill) {
				for (y++; y < y1; y++)
					drawLine(x, y, x1, y, color);
			}
		}
		void drawCircle(int x, int y, int radius, bool fill = false,
				bool color = true) {
			if (radius < 0)
				return;
			double rx;
			int rx_int;
			if (fill) {
				for (int i = 0; i <= radius; i++) {
					rx = sqrt(radius * radius - i * i);
					rx_int = (int) rx;
					if (rx >= rx_int + 0.5)
						rx_int++;
					drawLine(x - rx_int, y - i, x + rx_int, y - i);
					drawLine(x - rx_int, y + i, x + rx_int, y + i);
					drawLine(x - i, y - rx_int, x - i, y + rx_int);
					drawLine(x + i, y - rx_int, x + i, y + rx_int);
				}
			} else {
				for (int i = 0; i <= radius; i++) {
					rx = sqrt(radius * radius - i * i);
					rx_int = (int) rx;
					if (rx <= rx_int + 0.5) {
						symmetryDrawPoint(x, y, i, rx_int, color);
						symmetryDrawPoint(x, y, rx_int, i, color);
					}
					if (rx >= rx_int + 0.5) {
						symmetryDrawPoint(x, y, i, rx_int + 1, color);
						symmetryDrawPoint(x, y, rx_int + 1, i, color);
					}
				}
			}
		}
		void drawEllipse(int x, int y, int half_width, int half_height,
				bool fill = false, bool color = true) {
			if (half_width <= 0 || half_height <= 0)
				return;
			if (fill) {
				for (int i = 0; i <= half_height; i++) {
					double rx = half_width * sqrt(1.0 - (double) (i * i)
							/ ((double) (half_height * half_height)));
					int rx_int = (int) rx;
					if (rx >= rx_int + 0.5)
						rx_int++;
					drawLine(x - rx_int, y - i, x + rx_int, y - i, color);
					drawLine(x - rx_int, y + i, x + rx_int, y + i, color);
				}
				for (int i = 0; i <= half_width; i++) {
					double ry = half_height * sqrt(1.0 - (double) (i * i)
							/ ((double) (half_width * half_width)));
					int ry_int = (int) ry;
					if (ry >= ry_int + 0.5)
						ry_int++;
					drawLine(x - i, y - ry_int, x - i, y + ry_int, color);
					drawLine(x + i, y - ry_int, x + i, y + ry_int, color);
				}
			} else {
				for (int i = 0; i <= half_height; i++) {
					double rx = half_width * sqrt(1.0 - (double) (i * i)
							/ ((double) (half_height * half_height)));
					int rx_int = (int) rx;
					if (rx <= rx_int + 0.5)
						symmetryDrawPoint(x, y, rx_int, i, color);
					if (rx >= rx_int + 0.5)
						symmetryDrawPoint(x, y, rx_int + 1, i, color);
				}
				for (int i = 0; i <= half_width; i++) {
					double ry = half_height * sqrt(1.0 - (double) (i * i)
							/ ((double) (half_width * half_width)));
					int ry_int = (int) ry;
					if (ry <= ry_int + 0.5)
						symmetryDrawPoint(x, y, i, ry_int, color);
					if (ry >= ry_int + 0.5)
						symmetryDrawPoint(x, y, i, ry_int + 1, color);
				}
			}
		}
		void fillBounded(int x, int y, bool color = true) {
			if ((x >= 0 && x < w && y >= 0 && y < h) && (*parent)(x, y)
					!= color) {
				drawPoint(x, y, color);
				fillBounded(x - 1, y, color);
				fillBounded(x, y - 1, color);
				fillBounded(x + 1, y, color);
				fillBounded(x, y + 1, color);
			}
		}
		void reverseColor(int x, int y, int width, int height) {
			if (width > 0 && height > 0) {
				int y1 = y + height - 1;
				int x1 = x + width - 1;
				if (y1 < 0 || y >= h || x1 < 0 || x >= w)
					return;
				if (y < 0)
					y = 0;
				if (y1 >= h)
					y1 = h - 1;
				if (x < 0)
					x = 0;
				if (x1 >= w)
					x1 = w - 1;
				for (; y <= y1; y++) {
					long offset1 = BitUtils::xy2offset(x, y, w);
					long offset2 = BitUtils::xy2offset(x1, y, w);
					BitUtils::forEach(bitmap, offset1, offset2,
							&BitUtils::reverseByMask);
				}
			}
		}
		void fillWith(const DisplayObject &s, int x, int y, int width,
				int height) {
			int sw = s.width();
			int sh = s.height();
			int x1 = x + (width > sw ? sw : width) - 1;
			int y1 = y + (height > sh ? sh : height) - 1;
			for (int sy = 0; y <= y1; y++, sy++)
				for (int sx = 0, cx = x; cx <= x1; cx++, sx++)
					drawPoint(cx, y, s(sx, sy));
		}
	};
	unsigned char *newchars(int bytes) {
		unsigned char *p;
		try {
			p = new unsigned char[bytes];
		} catch (std::bad_alloc&) {
			throw OutOfMemException();
		}
		return p;
	}
protected:
	virtual bool nextHitArea(int &x, int &y, int &width, int &height, std::string &id) const {
		return false;
	}
	virtual void findHitAreaReset() const {
	}
	virtual bool nextInfo(int &x, int &y, int &width, int &height, std::string &info) const {
		if(this->info){
			x = this->x;
			y = this->y;
			width = this->width();
			height = this->height();
			info = std::string("canvas");
			this->info = false;
			return true;
		}
		return false;
	}
	virtual void findInfoReset() const {
		info = true;
	}
public:
	Canvas(int width, int height) :
		w(width), h(height), info(true), bitmap(newchars(BitUtils::bytes(width, height))),
				graphics(this, bitmap, w, h) {
		std::memset(bitmap, 0, BitUtils::bytes(width, height));
	}
	Canvas(const Canvas &src) :
		w(src.w), h(src.h), info(true), bitmap(newchars(BitUtils::bytes(src.w, src.h))),
				graphics(this, bitmap, w, h) {
		std::memcpy(bitmap, src.bitmap, BitUtils::bytes(w, h));
	}
	Canvas(const unsigned char *src, int width, int height) :
		w(width), h(height), info(true), bitmap(newchars(BitUtils::bytes(width, height))),
				graphics(this, bitmap, w, h) {
		memcpy(bitmap, src, BitUtils::bytes(w, h));
	}
	virtual ~Canvas() {
		delete[] bitmap;
	}
	virtual int width() const {
		return w;
	}
	virtual int height() const {
		return h;
	}
	virtual void drawTo(unsigned char *dest) const {
		std::memcpy(dest, bitmap, BitUtils::bytes(w, h));
	}
	virtual void drawTo(unsigned char *dest, int x, int y, int width,
			int height) const {
		int x1 = x + width - 1;
		int y1 = y + height - 1;
		if (width <= 0 || height <= 0 || x >= w || y >= h || x1 < 0 || y1 < 0)
			return;
		if (x1 >= w)
			x1 = w - 1;
		if (y1 >= h)
			y1 = h - 1;
		int dx = 0, dy = 0;
		if (x < 0) {
			dx = -x;
			x = 0;
		}
		if (y < 0) {
			dy = -y;
			y = 0;
		}
		long len = x1 - x + 1;
		for (int yd = dy; y <= y1; y++, yd++) {
			int offset_dest = BitUtils::xy2offset(dx, yd, width);
			int offset_src = BitUtils::xy2offset(x, y, w);
			BitUtils::copy(dest, bitmap, offset_dest, offset_src, len);
		}
	}
	virtual bool operator()(int x, int y) const {
		int n;
		unsigned char m;
		m = BitUtils::xy2mn(x, y, w, n);
		bool result;
		BitUtils::testByMask(bitmap[n], m, &result);
		return result;
	}
	virtual Canvas subCanvas(int x, int y, int width, int height) const {
		Canvas sc(width, height);
		drawTo(sc.bitmap, x, y, width, height);
		return sc;
	}
	Graphics graphics;
};
////////////////////////////////////////////////////////////////////////////////////
//class Container is a class used to contain DisplayObjects. Any DisplayObject
//can be contained. Each child in Container has an index, the child with higher
//index will cover ones with low index, if overlapping. If you try to add a
//DisplayObject that has been a child of a Container as child or delete a
//DisplayObject that isn't a child of Container nothing will be done.
//Note that children's hit area find next or reset may have an influence on their
//parents', and parents' hit area find next or reset may also influence on children.
//So don't try to do these things at the same time, and each time before you traverse
//a DisplayObject's hit areas,reset first.
class Container: virtual public DisplayObject {
	const enum {
		dynamic, fixed
	} sizetype;
	const int w, h;
	Container(const Container &);
	Container &operator=(const Container &);
	typedef std::vector<DisplayObject*> dvec;
	dvec childlist;
	mutable dvec::const_iterator hit;
	mutable dvec::const_iterator iit;
protected:
	virtual bool nextHitArea(int &x, int &y, int &width, int &height,
			std::string &id) const {
		if (hit == childlist.end())
			return false;
		int localx, localy;
		bool status = (*hit)->hitAreas.next(localx, localy, width, height, id);
		if (status) {
			x = localx + (*hit)->x;
			y = localy + (*hit)->y;
			return true;
		} else {
			for (hit++; hit != childlist.end(); hit++) {
				(*hit)->hitAreas.reset();
				if ((*hit)->hitAreas.next(localx, localy, width, height, id)) {
					x = localx + (*hit)->x;
					y = localy + (*hit)->y;
					return true;
				}
			}
			return false;
		}
	}
	virtual void findHitAreaReset() const {
		hit = childlist.begin();
		(*hit)->hitAreas.reset();
	}
	virtual bool nextInfo(int &x, int &y, int &width, int &height,
			std::string &info) const {
		if (iit == childlist.end())
			return false;
		int localx, localy;
		bool status = (*iit)->infos.next(localx, localy, width, height, info);
		if (status) {
			x = localx + (*iit)->x;
			y = localy + (*iit)->y;
			return true;
		} else {
			for (iit++; iit != childlist.end(); iit++) {
				(*iit)->infos.reset();
				if ((*iit)->infos.next(localx, localy, width, height, info)) {
					x = localx + (*iit)->x;
					y = localy + (*iit)->y;
					return true;
				}
			}
			return false;
		}
	}
	virtual void findInfoReset() const {
		iit = childlist.begin();
		(*iit)->infos.reset();
	}
	bool getPointValue(int x, int y, bool &found) const {
		for (dvec::const_reverse_iterator it = childlist.rbegin(); it
				!= childlist.rend(); it++) {
			int cx = (*it)->x;
			int cy = (*it)->y;
			int cw = (*it)->width();
			int ch = (*it)->height();
			if (cx <= x && cy <= y && cx + cw - 1 >= x && cy + ch - 1 >= y) {
				found = true;
				return (**it)(x - cx, y - cy);
			}
		}
		found = false;
		return false;
	}
public:
	Container() :
		sizetype(dynamic), w(0), h(0), hit(childlist.begin()), iit(childlist.begin()) {
	}
	Container(int width, int height) :
		sizetype(fixed), w(width), h(height), hit(childlist.begin()), iit(childlist.begin()) {
	}
	virtual ~Container() {
	}
	virtual int width() const {
		int ww = 0;
		switch (sizetype) {
		case fixed:
			ww = w;
			break;
		case dynamic:
			for (dvec::const_iterator it = childlist.begin(); it
					!= childlist.end(); it++) {
				int www = (*it)->x + (*it)->width();
				if (www > ww)
					ww = www;
			}
			break;
		}
		return ww;
	}
	virtual int height() const {
		int hh = 0;
		switch (sizetype) {
		case fixed:
			hh = h;
			break;
		case dynamic:
			for (dvec::const_iterator it = childlist.begin(); it
					!= childlist.end(); it++) {
				int hhh = (*it)->y + (*it)->height();
				if (hhh > hh)
					hh = hhh;
			}
			break;
		}
		return hh;
	}
	virtual void drawTo(unsigned char *dest) const {
		drawToByPoint(dest, 0, 0, width(), height());//TODO optimize
	}
	virtual void drawTo(unsigned char *dest, int x, int y, int width,
			int height) const {
		drawToByPoint(dest, x, y, width, height);//TODO optimize
	}
	virtual bool operator()(int x, int y) const {
		bool notused;
		return getPointValue(x, y, notused);
	}
	virtual void addChild(DisplayObject *c) {
		if (c->parent != 0)
			return;
		childlist.push_back(c);
		setParent(c, this);
	}
	virtual void addChild(DisplayObject *c, int index) {
		if (c->parent != 0)
			return;
		if (index < 0)
			index = 0;
		int clsize = childlist.size();
		if (index > clsize)
			index = clsize;
		childlist.insert(childlist.begin() + index, c);
		setParent(c, this);
	}
	virtual void removeChild(DisplayObject *c) {
		for (dvec::iterator it = childlist.begin(); it != childlist.end(); it++)
			if (*it == c) {
				childlist.erase(it);
				setParent(c, 0);
				break;
			}
	}
	virtual void removeChild(int index) {
		if (index < 0 || index >= (int) childlist.size())
			return;
		dvec::iterator it = childlist.begin() + index;
		DisplayObject *c = *it;
		childlist.erase(it);
		setParent(c, 0);
	}
	virtual void deleteChild(DisplayObject *c) {
		removeChild(c);
		delete c;
	}
	virtual void deleteChild(int index) {
		if (index < 0 || index >= (int) childlist.size())
			return;
		dvec::iterator it = childlist.begin() + index;
		DisplayObject *c = *it;
		childlist.erase(it);
		delete c;
	}
	virtual DisplayObject *childAt(int index) {
		if (index < 0 || index >= (int) childlist.size())
			return 0;
		return childlist[index];
	}
	virtual void clear(){
		childlist.clear();
	}
	virtual void clearAndDestroy(){
		for(dvec::iterator it=childlist.begin();it!=childlist.end();it++)
			delete *it;
		childlist.clear();
	}
};
////////////////////////////////////////////////////////////////////////////////////
//class Sprite are extended from class Container and class Canvas,so it support
//both drawing graphics and containing DisplayObjects. The canvas is the bottom
//layer so anything you draw will be covered by Sprite's children if overlapping
class Sprite: virtual public Container, virtual public Canvas {
protected:
	virtual bool nextHitArea(int &x, int &y, int &width, int &height,
			std::string &id) const {
		return Container::nextHitArea(x, y, width, height, id);
	}
	virtual void findHitAreaReset() const {
		Container::findHitAreaReset();
	}
	virtual bool nextInfo(int &x, int &y, int &width, int &height,
			std::string &info) const {
		return Container::nextInfo(x, y, width, height, info);
	}
	virtual void findInfoReset() const {
		Container::findInfoReset();
	}
public:
	Sprite(int width, int height) :
		Container(width, height), Canvas(width, height) {
	}
	virtual ~Sprite() {
	}
	virtual int width() const {
		return Canvas::width();
	}
	virtual int height() const {
		return Canvas::height();
	}
	virtual void drawTo(unsigned char *dest) const {
		Canvas::drawTo(dest);
		Container::drawTo(dest);
	}
	virtual void drawTo(unsigned char *dest, int x, int y, int width,
			int height) const {
		Canvas::drawTo(dest, x, y, width, height);
		Container::drawTo(dest, x, y, width, height);
	}
	virtual bool operator()(int x, int y) const {
		bool found;
		bool retval = getPointValue(x, y, found);
		if (found)
			return retval;
		else
			return Canvas::operator()(x, y);
	}
};
////////////////////////////////////////////////////////////////////////////////////
//Class TextArea is a class to display text in the screen. There are three mode,
//which is determined in constructor.In the dynamic mode, both width and height
//are dynamic, the text will come to a new line until it comes up with a newline
//character. To create a dynamic mode TextArea, please use the constructor with
//no parameter In the wfixed mode, width is fixed, the text will be put into a
//new line when there is a newline character or it reaches the border specified
//by width. To wfixed mode TextArea, use the constructor with one parameter width.
//The fixed mode TextArea's both width and height are fixed, it change line like
//the wfixed mode ones but the down border are fixed. That means if the real height
//are not as large as the height given, the left will be filled blank, and if the
//real height are larger than given, the exceeding part will not display. To create
//a fixed TextArea, please use the constructor with two parameters, width and height
template <braille_string (*const Translate)(std::string),std::string (*const BackTranslate)(braille_string)>
class TextArea: virtual public DisplayObject {
	const enum {
		dynamic, wfixed, fixed
	} mode;
	const int w, h;
	mutable bool info;
	braille_string _btext;
	std::string _text;
protected:
	virtual bool nextHitArea(int &x, int &y, int &width, int &height, std::string &id) const {
		return false;
	}
	virtual void findHitAreaReset() const {
	}
	virtual bool nextInfo(int &x, int &y, int &width, int &height, std::string &info) const {
		if(this->info){
			x = this->x;
			y = this->y;
			width = this->width();
			height = this->height();
			info = this->getText();
			this->info = false;
			return true;
		}
		return false;
	}
	virtual void findInfoReset() const {
		info = true;
	}
public:
	TextArea() :
		mode(dynamic), w(0), h(0), info(true), letterSpacing(0), lineSpacing(0) {
	}
	TextArea(int width) :
		mode(wfixed), w(width), h(0), info(true), letterSpacing(0), lineSpacing(0) {
	}
	TextArea(int width, int height) :
		mode(wfixed), w(width), h(height), letterSpacing(0), lineSpacing(0) {
	}
	virtual int width() const {
		int return_value;
		switch (mode) {
		case dynamic: {
			braille_string::const_iterator it;
			int m = 0;
			int count = 0;
			for (it = _btext.begin(); it != _btext.end(); it++) {
				if (*it == Braille::NEWLINE) {
					if (count > m)
						m = count;
					count = 0;
				}
				if (!it->isInv())
					count++;
			}
			if (count > m)
				m = count;
			return_value = (m > 0 ? Braille::width * m + letterSpacing
					* (m - 1) : 0);
			break;
		}
		case wfixed:
		case fixed:
			return_value = w;
			break;
		}
		return return_value;
	}
	virtual int height() const {
		int return_value;
		switch (mode) {
		case dynamic: {
			braille_string::const_iterator it;
			int count = 1;
			for (it = _btext.begin(); it != _btext.end(); it++)
				if (*it == Braille::NEWLINE)
					count++;
			return_value = Braille::height * count + lineSpacing * (count - 1);
			break;
		}
		case wfixed: {
			braille_string::const_iterator it;
			unsigned char nl = Braille::NEWLINE;
			int cw = 0;
			int count = 1;
			for (it = _btext.begin(); it != _btext.end(); it++) {
				if (!it->isInv())
					cw += (cw != 0 ? letterSpacing : 0) + Braille::width;
				if (cw > w) {
					count++;
					it--;
					cw = 0;
					continue;
				}
				if (*it == nl) {
					count++;
					cw = 0;
				}
			}
			return_value = Braille::height * count + lineSpacing * (count - 1);
			break;
		}
		case fixed:
			return_value = h;
			break;
		}
		return return_value;
	}
	virtual void drawTo(unsigned char *dest) const {
		drawToByPoint(dest, 0, 0, width(), height());//TODO optimize
	}
	virtual void drawTo(unsigned char *dest, int x, int y, int width,
			int height) const {
		drawToByPoint(dest, x, y, width, height);//TODO optimize
	}
	virtual bool operator()(int x, int y) const {
		int lh = Braille::height + lineSpacing;
		int lw = Braille::width + letterSpacing;
		int ln = y / lh, col = x / lw, ly = y % lh, lx = x % lw;
		if (ly >= Braille::height || lx >= Braille::width)
			return false;
		braille_string::const_iterator it = _btext.begin();
		for (int cln = 0; cln < ln; cln++)
			for (int cw = 0;; it++) {
				if (it == _btext.end() && mode == fixed)
					return false;
				if (*it == Braille::NEWLINE) {
					it++;
					break;
				}
				if (it->isInv())
					continue;
				cw += Braille::width;
				if (mode == wfixed || mode == fixed) {
					if (cw > w)
						break;
					else if ((cw += letterSpacing) > w)
						continue;
				}
			}
		for (int ccol = 0; ccol < col; it++) {
			if (it == _btext.end() || *it == Braille::NEWLINE)
				return false;
			if (it->isInv())
				continue;
			ccol++;
		}
		return (*it)(lx, ly);
	}
	int letterSpacing;
	int lineSpacing;
	braille_string getBText() const {
		return _btext;
	}
	std::string getText() const {
		return _text;
	}
	void setText(std::string text){
		_text = text;
		_btext = (*Translate)(text);
	}
	void setBText(braille_string btext){
		_btext = btext;
		_text = (*BackTranslate)(btext);
	}
};
////////////////////////////////////////////////////////////////////////////////////
//Class TextButton
template <braille_string (*const Translate)(std::string),std::string (*const BackTranslate)(braille_string)>
class TextButton: virtual public TextArea<Translate,BackTranslate> {
	mutable bool findHitAreaEnd;
	std::string _id;
protected:
	virtual bool nextHitArea(int &x, int &y, int &width, int &height, std::string &id) const {
		if (!findHitAreaEnd) {
			x = 0;
			y = 0;
			width = this->width();
			height = this->height();
			id = this->_id;
			findHitAreaEnd = true;
			return true;
		}
		return false;
	}
	virtual void findHitAreaReset() const {
		findHitAreaEnd = false;
	}
	virtual bool nextInfo(int &x, int &y, int &width, int &height, std::string &info) const {
		TextArea<Translate,BackTranslate>::nextInfo(x,y,width,height,info);
	}
	virtual void findInfoReset() const {
		TextArea<Translate,BackTranslate>::findInfoReset();
	}
public:
	TextButton(std::string id) :
		TextArea<Translate,BackTranslate>() , findHitAreaEnd(false), _id(id) {
	}
	TextButton(std::string id, int width) :
		TextArea<Translate,BackTranslate>(width), findHitAreaEnd(false), _id(id) {
	}
	TextButton(std::string id, int width, int height) :
		TextArea<Translate,BackTranslate>(width, height), findHitAreaEnd(false), _id(id) {
	}
	std::string id() const{
		return _id;
	}
};
////////////////////////////////////////////////////////////////////////////////////
//events
class Event {
public:
	enum EventType {
		Click = 3, KeyPressed = 2, Exit = 1, ForceExit = 0
	} type;
	std::string targetID;
	int code;
	Event() {
	}
	Event(EventType type, std::string target, int code) :
		type(type), targetID(target), code(code) {
	}
	bool operator<(const Event &event)const{
		if(type!=event.type)
			return type<event.type;
		if(targetID!=event.targetID)
			return targetID!=event.targetID;
		if(code!=event.code)
			return code<event.code;
		return false;
	}
};
////////////////////////////////////////////////////////////////////////////////////
//Class Application
class Application {
public:
	typedef void *(*ListenerFunctionType)(const Event *, void *);
private:
	class ListenFunc {
	public:
		ListenFunc(ListenerFunctionType func, void *param, void **retval) :
			func(func), param(param), return_value(retval) {
		}
		ListenerFunctionType func;
		void *param;
		void **return_value;
	};
	typedef boost::asio::io_service io_service;
	typedef boost::asio::ip::tcp::socket socket;
	typedef boost::asio::ip::tcp::endpoint endpoint;
	typedef boost::system::error_code error_code;
	typedef boost::asio::ip::address_v4 address_v4;
	typedef std::multimap<Event, ListenFunc> lltype;
	typedef lltype::const_iterator cllit;
	typedef lltype::iterator llit;
	static const int idMaxLen;
	mutable io_service *iosev;
	mutable socket *data_sock;
	mutable socket *demo_sock;
	mutable lltype listenerList;
	static const int len = 16;
	DisplayObject *windows[len];
	static bool inited;
	std::string name;
	static Application *app;
	void read_data(socket *sock,unsigned char *buf, int size) const{
		boost::asio::read(*sock, boost::asio::buffer(buf, size));
	}
	void write_data(socket *sock,unsigned char *buf, int size) const {
		boost::asio::write(*sock, boost::asio::buffer(buf, size));
	}
	void initConnection() {
		iosev = new io_service;
		data_sock = new socket(*iosev);
		demo_sock = new socket(*iosev);
		endpoint data_ep(address_v4::from_string("127.0.0.1"), 40000);
		endpoint demo_ep(address_v4::from_string("127.0.0.1"), 40001);
		error_code ec;
		data_sock->connect(data_ep, ec);
		if (ec){
			throw DaemonConnectException();
		}else{
			pid_t pid = getpid();
			write_data(data_sock,(unsigned char *)&pid,sizeof(pid_t));
		}
		demo_sock->connect(demo_ep, ec);
		if (ec){
			demo_sock = 0;
		}else{
			pid_t pid = getpid();
			write_data(demo_sock,(unsigned char *)&pid,sizeof(pid_t));
		}
	}
	bool getEvent(Event &event) const {
		//type(4),code(4),size(4),id(?<idMaxLen)
		const int size1 = sizeof(int)*3;
		unsigned char buf1[size1];
		read_data(data_sock,buf1,size1);
		int type,code,size2;
		unsigned char *bufptr = buf1;
		type = *(int *) bufptr;
		bufptr += sizeof(int);
		code = *(int *) bufptr;
		bufptr += sizeof(int);
		size2 = *(int *) bufptr;
		unsigned char buf2[size2+1];
		read_data(data_sock,buf2,size2);
		buf2[size2] = 0;
		event.type = Event::EventType(type);
		event.code = code;
		event.targetID = std::string((char *)buf2);
		return type;
	}
	void dispatchEvent(const Event &event) {
		std::pair<cllit, cllit> range = listenerList.equal_range(event);
		for (cllit i = range.first; i != range.second; ++i)
			if (i->second.return_value != 0)
				(*(i->second.return_value)) = (*(i->second.func))(&event,
						i->second.param);
			else
				(*(i->second.func))(&event, i->second.param);
	}
	Application(std::string name) :
		data_sock(0),demo_sock(0),name(name)
	{
		for (int i = 1; i < 16; i++)
			windows[i] = 0;
	}
	~Application() {
		delete iosev;
		delete data_sock;
		delete demo_sock;
	}
public:
	static Application *init(std::string name) {
		if(!inited){
			app = new Application(name);
			app->initConnection();
		}
		return app;
	}
	void addEventListener(const Event &event, ListenerFunctionType func,
			void *param=0, void **return_value=0) {
		listenerList.insert(std::pair<Event, ListenFunc>(event,
				ListenFunc(func, param, return_value)));
	}
	void removeEventListener(const Event &event) {
		while (listenerList.count(event) > 0)
			listenerList.erase(listenerList.find(event));
	}
	void removeEventListener(const Event &event, ListenerFunctionType func) {
		beginfind: std::pair<llit, llit> range = listenerList.equal_range(event);
		for (llit i = range.first; i != range.second; ++i) {
			if (i->second.func == func) {
				listenerList.erase(i);
				goto beginfind;
			}
		}
	}
	void removeEventListener(ListenerFunctionType func) {
		beginfind: for (llit i = listenerList.begin(); i != listenerList.end(); ++i) {
			if (i->second.func == func) {
				listenerList.erase(i);
				goto beginfind;
			}
		}
	}
	void removeEventListener(const Event &event, ListenerFunctionType func,
			void *param, void **return_value) {
		beginfind: std::pair<llit, llit> range = listenerList.equal_range(event);
		for (llit i = range.first; i != range.second; ++i) {
			ListenFunc *lfptr = &i->second;
			if (lfptr->func == func && lfptr->param == param
					&& lfptr->return_value == return_value) {
				listenerList.erase(i);
				goto beginfind;
			}
		}
	}
	void exit(int status) {
		delete app;
		std::exit(status);
	}
	void main() {
		Event event;
		while (getEvent(event)) {
			dispatchEvent(event);
			if (event.type == Event::Exit)
				exit(0);
		}
		return;
	}
	DisplayObject *operator[](int i) const {
		return windows[i];
	}
	DisplayObject* &operator[](int i) {
		return windows[i];
	}
	void update() const {
		//[width(4),height(4),data(?)](*),(number)[workspace(4),x(4),y(4),width(4),height(4),size(4),id(?<idMaxLen)](*)
		//calculate size
		int size;
		size = 2*len * sizeof(int); //width and height
		for (DisplayObject * const *i = &windows[0]; i < &windows[len]; i++) {
			if (*i != 0) {
				int width = (*i)->width();
				int height = (*i)->height();
				int bytes = width * height;
				bytes = (bytes >> 3) + (bytes & 0x07 ? 1 : 0);
				size += bytes;
			}
		}
		size += sizeof(int);
		int number = 0;
		for (DisplayObject * const *i = &windows[0]; i < &windows[len]; i++) {
			if (*i != 0) {
				(*i)->hitAreas.reset();
				int bytes = 0;
				int x, y, width, height, length;
				std::string id;
				while ((*i)->hitAreas.next(x, y, width, height, id)) {
					number++;
					length = id.length();
					if (length > idMaxLen)
						length = idMaxLen;
					bytes += sizeof(int) * 6 + length;
				}
				(*i)->hitAreas.reset();
				size += bytes;
			}
		}
		//write data to buf
		unsigned char buf[size + 1];
		for (int i = 0; i < size; i++)
			buf[i] = 0;
		unsigned char *bufptr = &buf[0];
		for (DisplayObject * const *i = &windows[0]; i < &windows[len]; i++) {
			if (*i == 0) {
				bufptr += 2 * sizeof(int);
			} else {
				int width = (*i)->width();
				int height = (*i)->height();
				int bytes = width * height;
				bytes = (bytes >> 3) + (bytes & 0x07 ? 1 : 0);
				*((int *) bufptr) = width;
				bufptr += sizeof(int);
				*((int *) bufptr) = height;
				bufptr += sizeof(int);
				(*i)->drawTo(bufptr);
				bufptr += bytes;
			}
		}
		*((int *) bufptr) = number;
		bufptr += sizeof(int);
		for (DisplayObject * const *i = &windows[0]; i < &windows[len]; i++) {
			if (*i != 0) {
				(*i)->hitAreas.reset();
				int x, y, width, height;
				std::string id;
				while ((*i)->hitAreas.next(x, y, width, height, id)) {
					*((int *) bufptr) = i - windows;
					bufptr += sizeof(int);
					*((int *) bufptr) = x;
					bufptr += sizeof(int);
					*((int *) bufptr) = y;
					bufptr += sizeof(int);
					*((int *) bufptr) = width;
					bufptr += sizeof(int);
					*((int *) bufptr) = height;
					bufptr += sizeof(int);
					int length = id.length();
					if (length > idMaxLen) {
						length = idMaxLen;
						*((int *) bufptr) = length;
						bufptr += sizeof(int);
						for (int i = 0; i < length; i++) {
							bufptr[i] = id.c_str()[i];
						}
					} else {
						*((int *) bufptr) = length;
						bufptr += sizeof(int);
						strcpy((char *) bufptr, id.c_str());
					}
					bufptr += length;
				}
			}
		}
		write_data(data_sock, buf, size);
		//[workspace(4),x(4),y(4),width(4),height(4),size(4),text(?)](*)
		if(demo_sock){
			int size = 0;
			for (DisplayObject * const *i = &windows[0]; i < &windows[len]; i++) {
				if (*i != 0) {
					(*i)->infos.reset();
					int bytes = 0;
					int x, y, width, height, length;
					std::string info;
					while ((*i)->infos.next(x, y, width, height, info))
						bytes += sizeof(int) * 6 + info.length();
					(*i)->infos.reset();
					size += bytes;
				}
			}
			unsigned char buf[size+1];
			unsigned char *bufptr = buf;
			for (DisplayObject * const *i = &windows[0]; i < &windows[len]; i++) {
				if (*i != 0) {
					(*i)->infos.reset();
					int x, y, width, height, length;
					std::string info;
					while ((*i)->infos.next(x, y, width, height, info)){
						*((int *) bufptr) = i - windows;
						bufptr += sizeof(int);
						*((int *) bufptr) = x;
						bufptr += sizeof(int);
						*((int *) bufptr) = y;
						bufptr += sizeof(int);
						*((int *) bufptr) = width;
						bufptr += sizeof(int);
						*((int *) bufptr) = height;
						bufptr += sizeof(int);
						int length = info.length();
						*((int *) bufptr) = length;
						bufptr += sizeof(int);
						strcpy((char *) bufptr, info.c_str());
					}
					(*i)->infos.reset();
				}
			}
			write_data(demo_sock,buf,size);
		}
		return;
	}
};
bool Application::inited = false;
Application *Application::app = 0;
const int Application::idMaxLen = 256;

}

#endif
