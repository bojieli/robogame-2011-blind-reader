#include <cstdlib>
#include <ctime>
#include <linux/keyboard.h>
#include <unistd.h>
#include "libblindgui.h"
#include <pthread.h>
using namespace blindgui;
const int lines = 15;
const int cols = 20;
const int points = lines*cols;
const int bytes = 38;
Canvas c(cols,lines);
Application *application = Application::init("snake");
class snake{
public:
	enum dir{
		up = 1,
		down = -1,
		left = 2,
		right = -2
	};
private:
	static unsigned char scr[bytes];
	class point{
	public:
		static int get(unsigned char &mask,int x,int y){
			int bit = y*cols+x;
			int index = bit/8;
			mask = 1<<(bit-8*index);
			return index;
		}
		int x;
		int y;
		void draw(){
			int index;
			unsigned char mask;
			index = get(mask,x,y);
			scr[index] |= mask;
		}
		void erase(){
			int index;
			unsigned char mask;
			index = get(mask,x,y);
			snake::scr[index] &= ~mask;
		}
		void random(){
			x=rand()%cols;
			y=rand()%lines;
		}
	};
	point egg;
	point body[points];
	int head,tail;
	snake():
		head(0),tail(0),direc(right)
	{
		push(0,0);
		push(1,0);
		push(2,0);
		newegg();
		egg.draw();
	}
	dir direc;
	void push(int x,int y){
		body[head].x = x;
		body[head].y = y;
		body[head].draw();
		head++==points-1?head=0:0;
	}
	void pop(){
		body[tail].erase();
		tail++==points-1?tail=0:0;
	}
	bool exist(int x,int y){
		for(int i=tail;i!=head;i++==points-1?i=0:0){
			if(body[i].x==x&&body[i].y==y)
				return true;
		}
		return false;
	}
	bool die(int x,int y){
		return x<0 || y<0 || x>=cols || y>=lines || exist(x,y);
	}
	bool eat(int x,int y){
		return egg.x==x && egg.y==y;
	}
	void get(int &x,int &y){
		int i = head?head-1:points-1;
		x = body[i].x;
		y = body[i].y;
	}
	void newegg(){
		egg.random();
		while(exist(egg.x,egg.y))
			egg.random();
	}
public:
	bool next(){
		int x,y;
		get(x,y);
		switch(direc){
			case up:
				y--;
				break;
			case down:
				y++;
				break;
			case left:
				x--;
				break;
			case right:
				x++;
				break;
		}
		if(die(x,y))
			return false;
		push(x,y);
		if(!eat(x,y)){
			pop();
		}else{
			newegg();
			egg.draw();
		}
		return true;
	}
	void chdir(dir direction){
		if(direction!=-direc)
			direc = direction;
	}
	bool val(int x,int y){
		int index;
		unsigned char mask;
		index = point::get(mask,x,y);
		return scr[index]&mask;
	}
	static snake isnake;
};
snake snake::isnake;
unsigned char snake::scr[bytes] = {0};
void *engine(void *){
	const int delay_us = 700000;
	snake *snake = &snake::isnake;
	while(1){
		snake->next();
		for(int i=0;i<lines;i++){
			for(int j=0;j<cols;j++)
				c.graphics.drawPoint(j,i,snake->val(j,i));
		}
		application->update();
		usleep(delay_us);
	}
}
void *snake_up(const Event *,void*){
	snake::isnake.chdir(snake::up);
}
void *snake_down(const Event *,void*){
	snake::isnake.chdir(snake::down);
}
void *snake_left(const Event *,void *){
	snake::isnake.chdir(snake::left);
}
void *snake_right(const Event *,void *){
	snake::isnake.chdir(snake::right);
}
int main(){
	pthread_t id;
	if(pthread_create(&id,NULL,&engine,NULL)!=0)
		return 1;
	(*application)[0] = &c;
	application->addEventListener(Event(Event::KeyPressed,"",K_UP),&snake_up);
	application->addEventListener(Event(Event::KeyPressed,"",K_DOWN),&snake_down);
	application->addEventListener(Event(Event::KeyPressed,"",K_LEFT),&snake_left);
	application->addEventListener(Event(Event::KeyPressed,"",K_RIGHT),&snake_right);
	application->main();
}
