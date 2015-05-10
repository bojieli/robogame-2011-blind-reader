#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <iostream>
#include "libblindgui.h"
using namespace std;
using namespace blindgui;
void print(DisplayObject *p){
	for(int i=0;i<p->height();i++){
		for(int j=0;j<p->width();j++){
			if((*p)(j,i))
				cout<<"@";
			else
				cout<<" ";
		}
		cout<<endl;
	}
}
#endif
