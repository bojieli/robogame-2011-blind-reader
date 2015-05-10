#include "libblindgui.h"
#include <fstream>
#include <dirent.h>
#include <stdio.h>
#include "translate.h"

using namespace blindgui;
using namespace std;
const int scr_width = 20;
const char *bkpath = "~/Documents/ebook";
Container menu;
Container filelist;
TextArea<&Translate,&BackTranslate> content(scr_width);
vector<string> fnlist;
Application *application = Application::init("reader");
void *ropenfile(const Event *,void *);
void *rexitfunc(const Event *,void *);
void *rspeakfunc(const Event *,void *);
void *entrychoice(const Event *,void *);
int main(){
	TextButton<&Translate,&BackTranslate> ropen(string("open"),scr_width);
	ropen.setText(string("open"));
	ropen.x = 0;
	ropen.y = 0;
	TextButton<&Translate,&BackTranslate> rspeak(string("speak"),scr_width);
	rspeak.setText(string("speak"));
	rspeak.x = 0;
	rspeak.y = ropen.y + ropen.height();
	TextButton<&Translate,&BackTranslate> rexit(string("exit"),scr_width);
	rexit.setText(string("exit"));
	rexit.x = 0;
	rexit.y = rspeak.y + rspeak.height();
	menu.addChild(&ropen);
	menu.addChild(&rexit);
	(*application)[0] = &menu;
	(*application)[1] = &content;
	application->addEventListener(Event(Event::Click,string("open"),0),&ropenfile);
	application->addEventListener(Event(Event::Click,string("exit"),0),&rexitfunc);
	application->update();
	application->main();
}
void *ropenfile(const Event *event,void *notused){
	dirent *ent = 0;
	DIR *pdir;
	if((pdir=opendir(bkpath))==0){
		content.setText(string("Open ebook dir failed"));
		return 0;
	}
	fnlist.clear();
	filelist.clearAndDestroy();
	int y = 0;
	while((ent=readdir(pdir))!=0){
		string fn(ent->d_name);
		string id_prefix("fn_");
		string id = id_prefix+fn;
		fnlist.push_back(fn);
		TextButton<&Translate,&BackTranslate> *fbtn = new TextButton<&Translate,&BackTranslate>(id,scr_width);
		fbtn->y = y;
		fbtn->setText(fn);
		y += fbtn->height();
		filelist.addChild(fbtn);
		application->addEventListener(Event(Event::Click,id,0),&entrychoice,fbtn);
	}
	(*application)[0] = &filelist;
	closedir(pdir);
	application->update();
}
void *rspeakfunc(const Event *event,void *notused){
}
void *rexitfunc(const Event *event,void *notused){
	application->exit(0);
}
void *entrychoice(const Event *event,void *button){
	string id = event->targetID;
	ifstream in((string(bkpath)+string("/")+id.substr(3,id.length()-3)).c_str());
	content.setText("");
	while(!in.eof()){
		char buf[2];
		buf[0] = in.get();
		buf[1] = 0;
		content.setText(content.getText()+buf);
	}
	(*application)[0] = &menu;
	application->removeEventListener(&entrychoice);
	application->update();
}
