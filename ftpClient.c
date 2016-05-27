/* ftpClient.c 	*/

#ifdef	WIN32
  #include	<windows.h>
  #include	"winsock.h"
#else
  #define	closesocket close
  #include	<sys/socket.h>
  #include	<netinet/in.h>
  #include	<arpa/inet.h>
  #include	<netdb.h>
#endif

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<io.h>

#define       BUFSIZE 999
#define		FTPPORT	21           /*ftp server control port*/

extern  int  error;
char    localhost[] = "192.168.70.1";	/* local host name			*/
/*-----------------------------------------------
* Program: ftpclient
*
* Purpose:
*
* Syntax:
*
*
*
*
* Notes:
*
*
*-------------------------------------------------------
*/
/*函数声明*/
int send_recv(SOCKET sd, char *input, char *res, int *len); //向socket发送命令并接收响应
int nlst_list(SOCKET sd);                                   //列出文件列表
int pasv_connect(SOCKET sd);                          //进入pasv连接
int  make_socket(char *host,int port);          //创建socket连接
int down_file(SOCKET sd, char *sourcefile, char *localfile, int * size);    //下载文件
int up_file(SOCKET sd, char *sourcefile, char *localfile, int * size);          //上传文件

int  main(int   argc, char  *argv[])
{
  int port , port_data;				/* protocol prot number					*/
  int n,size;					/* number to characters read			*/
  int res_num,ret;
  int timeout;
  char *command;
  char buf[BUFSIZE] , host[30] ,input[30];			/* buffer for data from the server		*/
  char *sourcefile,*aimfile;
  char *space = " ";
  SOCKET sd;

    /*socket连接到服务器*/
    port = FTPPORT;
    while(1){
         fprintf(stderr,"MyFTP>");
         gets(input);
         if  (strlen(input)==0)  continue;

         command = strtok(input,space); //取用户输入指令的第一个字段（以空格隔开）

         if(strcmp(command,"OPEN")==0){
            command = strtok(NULL,space);   //取用户输入指令的第二个字段，即ip地址
            strcpy(host,command);

            /* Create a socket.	*/
            sd = make_socket(host,port);
            if  (sd<0)  {
              fprintf(stderr,"Socket creation failed\n");
            }else{
                n = recv(sd, buf, BUFSIZE, 0);
                printf("\n 连接服务器成功！ \n");
                printf("输入? 可查看命令列表\n");
                break;  //连接成功，跳出循环
            }

         }else{
            printf("请先用“OPEN [ip]”命令连接到ftp服务器\n");
         }
    }
    /*socket连接到服务器 END*/

    /*登录到ftp */
    while(1){
         fprintf(stderr,"MyFTP>");
         gets(input);
         if  (strlen(input)==0)  continue;

         if(strcmp(input,"?")==0)
         {
                printf("USER [username]  用户名登录\n");
                //printf("PASS [password]  密码\n");
                printf("NLST 列出文件目录\n");
                printf("RETR [file]  下载文件\n");
                printf("STOR [file] 上传文件\n");
                printf("QUIT 离开\n");
         }else{
                command = strtok(input,space); //取用户输入指令的第一个字段（以空格隔开）
                if(strcmp(command,"USER")==0){
                        command = strtok(NULL,space);   //取用户输入指令的第二个字段，即用户名
                        sprintf(input, "USER %s\r\n",command);

                        if(send_recv(sd, input, buf, &n)==331){
                                printf("请输入密码:");
                                gets(buf);
                                sprintf(input, "PASS %s\r\n",buf);

                                if( send_recv(sd, input, buf, &n) ==230){
                                        fprintf(stderr,"%s\n",buf);
                                        break;
                                }
                                else{
                                        printf("登录失败!\n");
                                        return -1;
                                }
                        }else{
                                printf("用户名无效!\n");
                                return -1;
                        }
                }else{
                    printf("请先登录!\n");
                }
        }
    }   /*while end*/

    /*获取用户输入指令*/
  while (1) {
    fprintf(stderr,"MyFTP>");
    gets(input);
	if  (strlen(input)==0)  continue;

    if(strcmp(input,"?")==0)
    {
        //printf("USER [username]  登录的用户名\n");
        //printf("PASS [password]  密码\n");
        printf("NLST 列出文件目录\n");
        printf("RETR [file]  下载文件\n");
        printf("STOR [file] 上传文件\n");
        printf("QUIT 离开\n");
    }
    else if(strcmp(input,"NLST")==0)
    {
        n = nlst_list(sd);
        if(n<0){
            printf("\n获取文件列表失败！\n");
        }
    }
    else if(strcmp(command,"OPEN")==0){
        command = strtok(NULL,space);   //取用户输入指令的第二个字段，即ip地址
        strcpy(host,command);
        /* Create a socket.	*/
        sd = make_socket(host,port);
        if  (sd<0)  {
          fprintf(stderr,"Socket creation failed\n");
        }else{
            if(n = recv(sd, buf, BUFSIZE, 0)>0){
                //sscanf(buf, "%d", &n);
                printf("\n connect the ftp server successfully \n");
            }
            break;  //连接成功，跳出循环
        }

     }
    else{   //有多个参数用空格隔开的指令
        command = strtok(input,space);  //strtok函数用于截取输入指令以空格为间隔的第一个词，strtok(NULL,space)可获取第二个词

        if(strcmp(command,"RETR")==0){
            sourcefile = strtok(NULL,space);
            if(sourcefile){
                aimfile = strtok(NULL,space);
                if(!aimfile){   //若没有指定第三个参数就让源文件和目标文件同名
                    aimfile = sourcefile;
                }
                if(res_num = down_file(sd,sourcefile,aimfile,&size) != 0 )
                    printf("\n获取远程文件失败！\n");
                else{
                    fprintf(stderr,"\n%s文件下载成功！共下载%d字节\n",sourcefile,size);
                }
            }else{
                printf("请输入文件名！\n");
            }
        }
        else if(strcmp(command,"STOR")==0){
            sourcefile = strtok(NULL,space);
            if(sourcefile){
                aimfile = strtok(NULL,space);
                if(!aimfile){   //若没有指定第三个参数就让源文件和目标文件同名
                    aimfile = sourcefile;
                }
                if(res_num = up_file(sd,sourcefile,aimfile,&size) != 0 )
                    printf("\n上传文件失败！\n");
                else{
                    fprintf(stderr,"\n%s文件上传成功！共上传%d字节\n",sourcefile,size);
                }
            }else{
                printf("请输入文件名！\n");
            }
        }
        else if(strcmp(command,"OPEN")==0){
            //
        }
        else if(strcmp(command,"QUIT")==0){
            send_recv(sd, "QUIT\r\n",buf,&n);
            printf("\nBye~\n");
            closesocket(sd);
            break;
        }
        else{
            printf("错误指令!\n");
        }
    }
  }  //  while

   return 0;
}  // main

/*——函数实现——*/

/*通过host和端口port创建socket连接*/
int  make_socket(char *host,int port)
{
  struct     hostent   *ptrh;	/* pointer to a host table entry		*/
  struct     protoent  *ptrp;	/* pointer to a protocol table entry	*/
  struct     sockaddr_in sad;	/* structre to hold server's address	*/
  SOCKET        sd;				/* socket descriptor's address			*/
  int       timeout ; //复杂的网络环境要设置超时判断
#ifdef WIN32
  WSADATA wsaData;
  WSAStartup(0x0101, &wsaData);
#endif

  memset( (char *) &sad, 0, sizeof(sad));	/* clear sockaddr structure	*/
  sad.sin_family = AF_INET;					/* set family to internet	*/
  sad.sin_port = htons((u_short)port);

  if ( (ptrh = gethostbyname(host)) == NULL )  {
    fprintf(stderr,"invalid host: %s \n",host);
    return -1;
  }  //  if

  memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

  if ( (ptrp = getprotobyname("tcp")) == NULL ) {
    fprintf(stderr,"cannot map \"tcp\" to protocol number.\n");
    return -1;
  }  //  if


/* Create a socket.	*/
    sd=socket(AF_INET,SOCK_STREAM,ptrp->p_proto);
    if  (sd<0)  {
      fprintf(stderr,"Socket creation failed\n");
      return -1;
    }  //  if

    //设置超时连接
    timeout = 3000;
    setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout));
    setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout));

/* Connect the socket to the specified server.	*/
    if (connect(sd, (struct sockaddr *)&sad, sizeof(sad))<0) {
      fprintf(stderr,"Connecting failed! Please try late!\n");
  	  closesocket(sd);
  	   return -1;
    }
    return sd;
}

/*向socket发送命令input并接收响应到res，len为res的长度*/
int send_recv(SOCKET sd, char *input, char *res, int *len)
{
    char buf[BUFSIZE];
    int n , res_num;
    if(send(sd, input, strlen(input), 0) == -1)   return -1;

    n = recv(sd, buf, BUFSIZE, 0);
    if(n < 1)
        return -1;
    buf[n]=0;
    if(NULL != len)
        *len = n;
    if(NULL != res)
        sprintf(res, "%s", buf);

    sscanf(buf, "%d", &res_num);    //截取服务器返回的响应码
    return res_num;
}

/*连接到PASV接口
 *       PASV（被动）方式的连接过程是：
 *       客户端向服务器的FTP端口（默认是21）发送连接请求，
 *       服务器接受连接，建立一条命令链路。
*/
int pasv_connect(SOCKET sd)
{
    SOCKET sd_pasv;
    int send_result;
    int result;
    int len;
    int port;
    int addr[6];
    char input[BUFSIZE];
    char buf[BUFSIZE];

    //设置PASV被动模式
    memset(input,sizeof(input),0);
    sprintf(input, "PASV\r\n");
    result = send_recv(sd,input, buf,  &len);
    if(result >= 0)
    {
        sscanf(buf, "%*[^(](%d,%d,%d,%d,%d,%d)",
               &addr[0],&addr[1],&addr[2],&addr[3],
               &addr[4],&addr[5]);
    }   //提取服务器监听的ip和端口号

    //连接PASV端口
    memset(buf, sizeof(buf), 0);
    port = addr[4]*256+addr[5];          //端口计算
    sprintf(buf, "%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);
    sd_pasv = make_socket(buf,port);

    if(-1 == sd_pasv)
        return -1;
    return sd_pasv;
}

/*列出文件目录*/
int nlst_list(SOCKET sd)
{
    SOCKET sd_pasv;
    char buf[BUFSIZE];
    int send_re;
    int result;
    int len;

    //连接到PASV接口
    sd_pasv = pasv_connect(sd);
    if(-1 == sd_pasv)
    {
        return -1;
    }
    //发送NLST命令
    memset(buf,sizeof(buf),0);
    send_re = send_recv(sd, "NLST\r\n",buf,&len);
    if(send_re >= 300 || send_re < 0)
        return -1;
    /*从PASV接口获取数据*/
    if( (len = recv(sd_pasv,buf,BUFSIZE,0)) > 0)
    {
        buf[len] = 0;
        printf("文件目录：\n");
        printf("%s",buf);
        printf("\n目录传输完毕\n");
    }
    closesocket(sd_pasv);

    //向服务器接收返回值
    memset(buf, sizeof(buf), 0);
    len = recv(sd, buf, BUFSIZE, 0);
    buf[len] = 0;
    sscanf(buf, "%d", &result);
    if(result != 226)
        return -1;

    return 0;
}

/*
 * 作用: 从服务器下载文件到本地 RETR
 * 参数: SOCKET，源文件名，目标文件名，文件大小
 * 返回 0 表示列表成功  result>0 表示其他错误响应码
 *         -1:文件创建失败  -2 pasv接口错误
  */
int down_file(SOCKET sd, char *sourcefile, char *aimfile, int * size)
{
    SOCKET sd_data;
    int len,write_len;
    char buf[BUFSIZE];
    char res[BUFSIZE];
    int result;
    *size=0;
    //打开本地文件
    FILE * fp = fopen(aimfile, "wb");
    if(NULL == fp)
    {
        printf("Can't Open the file.\n");
        return -1;
    }

    //连接到PASV接口 用于传输文件
    sd_data = pasv_connect(sd);

    if(-1 == sd_data)
    {
        fclose(fp); //关闭文件
        return -2;
    }

    //发送RETR命令
    memset(buf, sizeof(buf), 0);
    sprintf(buf, "RETR %s\r\n", sourcefile);

    result = send_recv(sd,buf,res,&len);

    if(result >= 300 || result < 0) //响应码
    {
        fclose(fp);
        return result;
    }

    //开始向PASV读取所要下载文件的数据
    memset(buf, sizeof(buf), 0);
    while((len = recv(sd_data, buf, BUFSIZE, 0)) > 0 )
    {
        write_len = fwrite(&buf, len, 1, fp);
        if(write_len != 1) //写入文件不完整
        {
            closesocket(sd_data); //关闭套接字
            fclose(fp); //关闭文件
            return -1;
        }
        if(NULL != size)
        {
            *size += len;
        }
    }

    //下载完成
    closesocket(sd_data);
    fclose(fp);

    //向服务器接收返回值
    memset(buf, sizeof(buf), 0);
    len = recv(sd, buf, BUFSIZE, 0);
    buf[len] = 0;
    printf("%s\n",buf);
    sscanf(buf, "%d", &result);
    if(result >= 300)
        return result;

    return 0;
}

/*
 * 作用: 上传文件到服务器 STOR
 * 参数: SOCKET，源文件名，目标文件名，文件大小
 * 返回0 表示列表成功  result>0 表示其他错误响应码
 *  -1:文件创建失败  -2 pasv接口错误
  */
int up_file(SOCKET sd, char *sourcefile, char *aimfile, int * size)
{
    SOCKET sd_data;
    int len,send_len;
    char buf[BUFSIZE];
    char res[BUFSIZE];
    FILE * fp;
    int result;

    //打开本地文件
    fp = fopen(sourcefile, "rb");
    if(NULL == fp)
    {
        printf("Can't Not Open the file.\n");
        return -1;
    }

    //连接到PASV接口
    sd_data = pasv_connect(sd);
    if(sd_data == -1)
    {
        fclose(fp);
        return -2;
    }

    //发送STOR命令
    memset(buf, sizeof(buf), 0);
    sprintf(buf, "STOR %s\r\n", aimfile);
    result = send_recv(sd,buf,res,&len);

    if(result >= 300 || result == 0)
    {
        fclose(fp);
        return result;
    }

    //开始向PASV通道写数据
    memset(buf, sizeof(buf), 0);
    while( (len = fread(buf, 1, BUFSIZE, fp)) > 0)
    {
        send_len = send(sd_data, buf, len, 0);
        if(send_len != len)
        {
            closesocket(sd_data);
            fclose(fp);
            return -1;
        }
        if(NULL != size)
        {
            *size += send_len;
        }
    }
    //完成上传
    closesocket(sd_data);
    fclose(fp);

    //向服务器接收响应码
    memset(buf, sizeof(buf), 0);
    len = recv(sd, buf, BUFSIZE, 0);
    buf[len] = 0;
    sscanf(buf, "%d", &result);
    if(result >= 300)
        return result;

    return 0;
}
