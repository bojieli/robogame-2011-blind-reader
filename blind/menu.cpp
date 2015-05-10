#include <string>
#include <vector>

#include <libxml++/libxml++.h>
#include <boost/asio.hpp>

#include <sys/types.h>
#include <unistd.h>

#include "libblindgui.h"
#include "translate.h"

using namespace std;
using namespace xmlpp;
using namespace blindgui;

class menu_item{
public:
	string name;
	string filename;
	pid_t pid;
	menu_item(string name,string filename):name(name),filename(filename),pid(0){}
};
typedef vector<menu_item> mvec;
typedef mvec::iterator mvit;
typedef mvec::const_iterator mvcit;
typedef TextButton<&Translate,&BackTranslate> button;
typedef vector<button *> bvec;
typedef bvec::iterator bvit;
typedef bvec::const_iterator bvcit;

const int width = 20;
mvec menu;
bvec btns;
Application *application = Application::init("menu");
Container c;
boost::asio::io_service *iosev = 0;
boost::asio::ip::tcp::socket *sock = 0;

void read_data(unsigned char *buf, int size){
	boost::asio::read(*sock, boost::asio::buffer(buf, size));
}
void write_data(unsigned char *buf, int size){
	boost::asio::write(*sock, boost::asio::buffer(buf, size));
}
void init_connection() {
	iosev = new boost::asio::io_service;
	sock = new boost::asio::ip::tcp::socket(*iosev);
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::from_string("127.0.0.1"), 40000);
	boost::system::error_code ec;
	sock->connect(ep, ec);
	if (ec){
		throw DaemonConnectException();
	}else{
		pid_t pid = getpid();
		write_data((unsigned char *)&pid,sizeof(pid_t));
	}
}
bool read_menu(){
	DomParser parser("menu.xml");
	if(parser){
		const Node *xroot = parser.get_document()->get_root_node();
		if(xroot->get_name()!="menu")
			return false;
		typedef Node::NodeList nlist;
		typedef nlist::iterator nlit;
		nlist xmenu = xroot->get_children();
		for(nlit inode=xmenu.begin();inode!=xmenu.end();++inode){
			Node *node = *inode;
			const Element *item = dynamic_cast<const Element*>(node);
			if(item){
				string name = item->get_name();
				if(item->has_child_text()){
					const TextNode *xcmd = item->get_child_text();
					string filename =  xcmd->get_content();
					menu.push_back(menu_item(name,filename));
				}
			}
		}
	}
	return true;
}
void open_item(menu_item *item){
	pid_t pid = fork();
	if(pid>0){
		item->pid = pid;
	}else if(pid==0){
		execl(item->filename.c_str(),(char *)NULL);
		exit(0);
	}else{
		exit(1);
	}
}
void switch_item(menu_item *item){
	write_data((unsigned char *)&(item->pid),sizeof(pid_t));
	int state;
	read_data((unsigned char *)&state,sizeof(int));
	if(!state)
		open_item(item);
}
void *button_do(const Event *event,void *item){
	menu_item *_item = (menu_item*)item;
	if(_item->pid==0)
		open_item(_item);
	else
		switch_item(_item);
	return (void *)0;
}
int main(){
	if(!read_menu())
		return 1;
	init_connection();
	int y = 0;
	for(mvit i=menu.begin();i!=menu.end();++i){
		button *bptr = new button(i->name,width);
		bptr->setText(i->name);
		btns.push_back(bptr);
		bptr->x = 0;
		bptr->y = y;
		y += bptr->height();
		c.addChild(bptr);
		application->addEventListener(Event(Event::Click,i->name,0),&button_do,&*i);
	}
	(*application)[0] = &c;
	application->update();
	application->main();
}
