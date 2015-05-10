#define MAXLEN  32000

/*
参数str：为纯汉字的字符串，且编码格式为GBK
返回值：
-1：表示语音库文件打开错误
-2：表示合成语音文件 打开/生成错误
其它：函数执行成功
*/
int   wav(char *str) 
{
 FILE * fpf,*fpt;   //文件指针
 int qm,wm;      //汉字区、位码
 int re;               //函数返回值
 long fileleng=0;   //文件长度 后面修改WAV格式时有用  
 if((fpf=fopen("ddd.wav","rb+"))==NULL)    //打开语音库文件
  return -1;
 
 if((fpt=fopen("china.wav","wb+"))==NULL)  //打开或生成合成后的语音文件，用来播放的
  return -2;

 char head[46];                       //WAV 文件头
 char buffer[MAXLEN];           //发音数据BUFF
 memset(buffer,0,MAXLEN);  //置0
 
 fread(head,sizeof(head),1,fpf);     //读语音库文件头
 fwrite(head,sizeof(head),1,fpt);    //写入合成语音文件
 
 int l=strlen(str);
 char *s=str;
 for(int i=0;i<=l;i=i+2)
 {
  qm=(unsigned char)*(s+i);      //取汉字的区码
  wm=(unsigned char)*(s+1+i);      //取汉字的位码

  if (qm<176||qm>215)   //判断是否在汉字字符集中         
   continue;
    
  if (wm<161||wm>254)  //判断是否在汉字字符集中
    continue;
  

  int position =(qm-176)*94+wm-160;        
  int offset=(position-1)*MAXLEN+46;     //定位
  fseek(fpf,offset,0);
  fread(buffer,sizeof(buffer),1,fpf);     //取发音数据
  fwrite(buffer,sizeof(buffer),1,fpt);     //写入合成文件
  fileleng++;                                          //合成文件长度增加
  
 }   //end for
 
 re =fileleng;
 fileleng=fileleng*MAXLEN;
 fseek(fpt,42,SEEK_SET);
 fwrite(&fileleng,sizeof(long),1,fpt);    //修改合成文件的WAV格式，主要是修改文件大小，具体请看WAV格式表
 
 fileleng+=44;
 
 fseek(fpt,4,SEEK_SET);
 fwrite(&fileleng,sizeof(long),1,fpt);  //修改合成文件的WAV格式，主要是修改文件大小，具体请看WAV格式表
 
 fclose(fpf);     //关闭文件
 fclose(fpt);
 return re;
}

void trans(char *str)
{
 int i = 0, j = 0;
 while( str[i] != '/0' )
 {
  if ( str[i] < 0 )
      {
   str[j++] = str[i++];
   str[j++] = str[i++];
     }
  else
     i++;
 
 }   //end while
 str[j] = '/0'; 
}
