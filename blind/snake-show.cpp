#include <cstdlib>
#include <curses.h>
#include <ctime>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
const int lines = 15;
const int cols = 20;
const int points = lines*cols;
const int bytes = 38;
class snake{
public:
	enum dir{
		up = 1,
		down = -1,
		left = 2,
		right = -2
	};
	static unsigned char scr[bytes];
private:
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
void print(){
	int fd = open("/dev/blindfb",O_WRONLY);
	if(fd!=-1){
		write(fd,snake::scr,bytes);
		close(fd);
	}else{
		printw("error");
		refresh();
	}
	clear();
	const char *pchar = "@";
	snake *snake = &snake::isnake;
	for(int y=0;y<LINES;y++){
		for(int x=0;x<COLS;x++){
			if(snake->val(x,y))
				mvprintw(y,x,pchar);
		}
	}
	move(15,0);
}
				
int main(){
	const int refresh_delay = 10;
	snake *snake = &snake::isnake;
	srand(time(0));
	initscr();
	halfdelay(1);
	keypad(stdscr,TRUE);
	noecho();
	int count = 0;
	while(1){
		if(!count){
			if(!snake->next())
				break;
			print();
			refresh();
		}
		switch(getch()){
			case KEY_LEFT:
				snake->chdir(snake::left);
				break;
			case KEY_RIGHT:
				snake->chdir(snake::right);
				break;
			case KEY_UP:
				snake->chdir(snake::up);
				break;
			case KEY_DOWN:
				snake->chdir(snake::down);
				break;
		}
		if(count!=refresh_delay)
			count++;
		else
			count=0;
	}
	endwin();
}
