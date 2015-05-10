
#define  MAXLEN  32000
int HandingWav()
{ 
 FILE * fpf,*fpt; //文件操作指针 
 if((fpf=fopen("gbhz.wav","rb+"))==NULL)   //gbhz.wav为处理前的语音文件
     return -1;
 if((fpt=fopen("ddd.wav","rb+"))==NULL)  //ddd.wav为合成的新的语音文件
     return -1;
 
 char head[46];     //wav文件的文件头长度
 char data[100];    //用来加速文件处理的数组
 char buffer[MAXLEN];
 memset(buffer,0,MAXLEN);
 fread(head,sizeof(head),1,fpf);    //head of wav 
 fwrite(head,sizeof(head),1,fpt);
 while(!feof(fpf))
 {
        fread(buffer,MAXLEN,1,fpf);  //读一个字的发音
        fwrite(buffer,MAXLEN,1,fpt);  //写一个字
        memset(buffer,0,MAXLEN); 
        fread(data,1,1,fpf); //读一个字节
        while(data[0]==char(0x80)) //判断是否为0x80
         {
           if(fread(data,100,1,fpf)==-1) //每次取100个字节，但只判断第一个字节，这样可以加速文件处理
                {
                   fclose(fpf);
                   fclose(fpt);
                   return -1;
                 }  //end if
           }  //end while  判断是否为0x80
 }  // end while(!feof(fpf))

 fclose(fpf);  //关闭文件
 fclose(fpt);
 return 0; 
}
