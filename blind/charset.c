 //getgbk．cpp一获取GBK汉字码文件
#include <strstream h>  //字符串I/O操作
#include <fstream．h>   //文件I/O操作
 unsigned char oneline[4];
 ofstream ofs("gbhz.txt",ios::binary );
 oneline[2]=163;
 oneline[3]=172;
 int qm;
 int wm;
 for( qm=176;qm<=247;qm++)  //区码0XB0—0XD7 87
  for(wm=161;wm<=254;wm++) //位码0XA1—0XFE
   if(!((qm==247)&&(wm=250)))
    //剔除GBK中没有编码的字位
   {
    oneline[0]=qm; //汉字区码
    oneline[1]=wm; //汉字位码
    ofs.write((char *)&oneline,4); //写一行至gbhz txt
    
   }//if end
