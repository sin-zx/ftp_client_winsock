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
/*��������*/
int send_recv(SOCKET sd, char *input, char *res, int *len); //��socket�������������Ӧ
int nlst_list(SOCKET sd);                                   //�г��ļ��б�
int pasv_connect(SOCKET sd);                          //����pasv����
int  make_socket(char *host,int port);          //����socket����
int down_file(SOCKET sd, char *sourcefile, char *localfile, int * size);    //�����ļ�
int up_file(SOCKET sd, char *sourcefile, char *localfile, int * size);          //�ϴ��ļ�

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

    /*socket���ӵ�������*/
    port = FTPPORT;
    while(1){
         fprintf(stderr,"MyFTP>");
         gets(input);
         if  (strlen(input)==0)  continue;

         command = strtok(input,space); //ȡ�û�����ָ��ĵ�һ���ֶΣ��Կո������

         if(strcmp(command,"OPEN")==0){
            command = strtok(NULL,space);   //ȡ�û�����ָ��ĵڶ����ֶΣ���ip��ַ
            strcpy(host,command);

            /* Create a socket.	*/
            sd = make_socket(host,port);
            if  (sd<0)  {
              fprintf(stderr,"Socket creation failed\n");
            }else{
                n = recv(sd, buf, BUFSIZE, 0);
                printf("\n ���ӷ������ɹ��� \n");
                printf("����? �ɲ鿴�����б�\n");
                break;  //���ӳɹ�������ѭ��
            }

         }else{
            printf("�����á�OPEN [ip]���������ӵ�ftp������\n");
         }
    }
    /*socket���ӵ������� END*/

    /*��¼��ftp */
    while(1){
         fprintf(stderr,"MyFTP>");
         gets(input);
         if  (strlen(input)==0)  continue;

         if(strcmp(input,"?")==0)
         {
                printf("USER [username]  �û�����¼\n");
                //printf("PASS [password]  ����\n");
                printf("NLST �г��ļ�Ŀ¼\n");
                printf("RETR [file]  �����ļ�\n");
                printf("STOR [file] �ϴ��ļ�\n");
                printf("QUIT �뿪\n");
         }else{
                command = strtok(input,space); //ȡ�û�����ָ��ĵ�һ���ֶΣ��Կո������
                if(strcmp(command,"USER")==0){
                        command = strtok(NULL,space);   //ȡ�û�����ָ��ĵڶ����ֶΣ����û���
                        sprintf(input, "USER %s\r\n",command);

                        if(send_recv(sd, input, buf, &n)==331){
                                printf("����������:");
                                gets(buf);
                                sprintf(input, "PASS %s\r\n",buf);

                                if( send_recv(sd, input, buf, &n) ==230){
                                        fprintf(stderr,"%s\n",buf);
                                        break;
                                }
                                else{
                                        printf("��¼ʧ��!\n");
                                        return -1;
                                }
                        }else{
                                printf("�û�����Ч!\n");
                                return -1;
                        }
                }else{
                    printf("���ȵ�¼!\n");
                }
        }
    }   /*while end*/

    /*��ȡ�û�����ָ��*/
  while (1) {
    fprintf(stderr,"MyFTP>");
    gets(input);
	if  (strlen(input)==0)  continue;

    if(strcmp(input,"?")==0)
    {
        //printf("USER [username]  ��¼���û���\n");
        //printf("PASS [password]  ����\n");
        printf("NLST �г��ļ�Ŀ¼\n");
        printf("RETR [file]  �����ļ�\n");
        printf("STOR [file] �ϴ��ļ�\n");
        printf("QUIT �뿪\n");
    }
    else if(strcmp(input,"NLST")==0)
    {
        n = nlst_list(sd);
        if(n<0){
            printf("\n��ȡ�ļ��б�ʧ�ܣ�\n");
        }
    }
    else if(strcmp(command,"OPEN")==0){
        command = strtok(NULL,space);   //ȡ�û�����ָ��ĵڶ����ֶΣ���ip��ַ
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
            break;  //���ӳɹ�������ѭ��
        }

     }
    else{   //�ж�������ÿո������ָ��
        command = strtok(input,space);  //strtok�������ڽ�ȡ����ָ���Կո�Ϊ����ĵ�һ���ʣ�strtok(NULL,space)�ɻ�ȡ�ڶ�����

        if(strcmp(command,"RETR")==0){
            sourcefile = strtok(NULL,space);
            if(sourcefile){
                aimfile = strtok(NULL,space);
                if(!aimfile){   //��û��ָ����������������Դ�ļ���Ŀ���ļ�ͬ��
                    aimfile = sourcefile;
                }
                if(res_num = down_file(sd,sourcefile,aimfile,&size) != 0 )
                    printf("\n��ȡԶ���ļ�ʧ�ܣ�\n");
                else{
                    fprintf(stderr,"\n%s�ļ����سɹ���������%d�ֽ�\n",sourcefile,size);
                }
            }else{
                printf("�������ļ�����\n");
            }
        }
        else if(strcmp(command,"STOR")==0){
            sourcefile = strtok(NULL,space);
            if(sourcefile){
                aimfile = strtok(NULL,space);
                if(!aimfile){   //��û��ָ����������������Դ�ļ���Ŀ���ļ�ͬ��
                    aimfile = sourcefile;
                }
                if(res_num = up_file(sd,sourcefile,aimfile,&size) != 0 )
                    printf("\n�ϴ��ļ�ʧ�ܣ�\n");
                else{
                    fprintf(stderr,"\n%s�ļ��ϴ��ɹ������ϴ�%d�ֽ�\n",sourcefile,size);
                }
            }else{
                printf("�������ļ�����\n");
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
            printf("����ָ��!\n");
        }
    }
  }  //  while

   return 0;
}  // main

/*��������ʵ�֡���*/

/*ͨ��host�Ͷ˿�port����socket����*/
int  make_socket(char *host,int port)
{
  struct     hostent   *ptrh;	/* pointer to a host table entry		*/
  struct     protoent  *ptrp;	/* pointer to a protocol table entry	*/
  struct     sockaddr_in sad;	/* structre to hold server's address	*/
  SOCKET        sd;				/* socket descriptor's address			*/
  int       timeout ; //���ӵ����绷��Ҫ���ó�ʱ�ж�
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

    //���ó�ʱ����
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

/*��socket��������input��������Ӧ��res��lenΪres�ĳ���*/
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

    sscanf(buf, "%d", &res_num);    //��ȡ���������ص���Ӧ��
    return res_num;
}

/*���ӵ�PASV�ӿ�
 *       PASV����������ʽ�����ӹ����ǣ�
 *       �ͻ������������FTP�˿ڣ�Ĭ����21��������������
 *       �������������ӣ�����һ��������·��
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

    //����PASV����ģʽ
    memset(input,sizeof(input),0);
    sprintf(input, "PASV\r\n");
    result = send_recv(sd,input, buf,  &len);
    if(result >= 0)
    {
        sscanf(buf, "%*[^(](%d,%d,%d,%d,%d,%d)",
               &addr[0],&addr[1],&addr[2],&addr[3],
               &addr[4],&addr[5]);
    }   //��ȡ������������ip�Ͷ˿ں�

    //����PASV�˿�
    memset(buf, sizeof(buf), 0);
    port = addr[4]*256+addr[5];          //�˿ڼ���
    sprintf(buf, "%d.%d.%d.%d",addr[0],addr[1],addr[2],addr[3]);
    sd_pasv = make_socket(buf,port);

    if(-1 == sd_pasv)
        return -1;
    return sd_pasv;
}

/*�г��ļ�Ŀ¼*/
int nlst_list(SOCKET sd)
{
    SOCKET sd_pasv;
    char buf[BUFSIZE];
    int send_re;
    int result;
    int len;

    //���ӵ�PASV�ӿ�
    sd_pasv = pasv_connect(sd);
    if(-1 == sd_pasv)
    {
        return -1;
    }
    //����NLST����
    memset(buf,sizeof(buf),0);
    send_re = send_recv(sd, "NLST\r\n",buf,&len);
    if(send_re >= 300 || send_re < 0)
        return -1;
    /*��PASV�ӿڻ�ȡ����*/
    if( (len = recv(sd_pasv,buf,BUFSIZE,0)) > 0)
    {
        buf[len] = 0;
        printf("�ļ�Ŀ¼��\n");
        printf("%s",buf);
        printf("\nĿ¼�������\n");
    }
    closesocket(sd_pasv);

    //����������շ���ֵ
    memset(buf, sizeof(buf), 0);
    len = recv(sd, buf, BUFSIZE, 0);
    buf[len] = 0;
    sscanf(buf, "%d", &result);
    if(result != 226)
        return -1;

    return 0;
}

/*
 * ����: �ӷ����������ļ������� RETR
 * ����: SOCKET��Դ�ļ�����Ŀ���ļ������ļ���С
 * ���� 0 ��ʾ�б�ɹ�  result>0 ��ʾ����������Ӧ��
 *         -1:�ļ�����ʧ��  -2 pasv�ӿڴ���
  */
int down_file(SOCKET sd, char *sourcefile, char *aimfile, int * size)
{
    SOCKET sd_data;
    int len,write_len;
    char buf[BUFSIZE];
    char res[BUFSIZE];
    int result;
    *size=0;
    //�򿪱����ļ�
    FILE * fp = fopen(aimfile, "wb");
    if(NULL == fp)
    {
        printf("Can't Open the file.\n");
        return -1;
    }

    //���ӵ�PASV�ӿ� ���ڴ����ļ�
    sd_data = pasv_connect(sd);

    if(-1 == sd_data)
    {
        fclose(fp); //�ر��ļ�
        return -2;
    }

    //����RETR����
    memset(buf, sizeof(buf), 0);
    sprintf(buf, "RETR %s\r\n", sourcefile);

    result = send_recv(sd,buf,res,&len);

    if(result >= 300 || result < 0) //��Ӧ��
    {
        fclose(fp);
        return result;
    }

    //��ʼ��PASV��ȡ��Ҫ�����ļ�������
    memset(buf, sizeof(buf), 0);
    while((len = recv(sd_data, buf, BUFSIZE, 0)) > 0 )
    {
        write_len = fwrite(&buf, len, 1, fp);
        if(write_len != 1) //д���ļ�������
        {
            closesocket(sd_data); //�ر��׽���
            fclose(fp); //�ر��ļ�
            return -1;
        }
        if(NULL != size)
        {
            *size += len;
        }
    }

    //�������
    closesocket(sd_data);
    fclose(fp);

    //����������շ���ֵ
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
 * ����: �ϴ��ļ��������� STOR
 * ����: SOCKET��Դ�ļ�����Ŀ���ļ������ļ���С
 * ����0 ��ʾ�б�ɹ�  result>0 ��ʾ����������Ӧ��
 *  -1:�ļ�����ʧ��  -2 pasv�ӿڴ���
  */
int up_file(SOCKET sd, char *sourcefile, char *aimfile, int * size)
{
    SOCKET sd_data;
    int len,send_len;
    char buf[BUFSIZE];
    char res[BUFSIZE];
    FILE * fp;
    int result;

    //�򿪱����ļ�
    fp = fopen(sourcefile, "rb");
    if(NULL == fp)
    {
        printf("Can't Not Open the file.\n");
        return -1;
    }

    //���ӵ�PASV�ӿ�
    sd_data = pasv_connect(sd);
    if(sd_data == -1)
    {
        fclose(fp);
        return -2;
    }

    //����STOR����
    memset(buf, sizeof(buf), 0);
    sprintf(buf, "STOR %s\r\n", aimfile);
    result = send_recv(sd,buf,res,&len);

    if(result >= 300 || result == 0)
    {
        fclose(fp);
        return result;
    }

    //��ʼ��PASVͨ��д����
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
    //����ϴ�
    closesocket(sd_data);
    fclose(fp);

    //�������������Ӧ��
    memset(buf, sizeof(buf), 0);
    len = recv(sd, buf, BUFSIZE, 0);
    buf[len] = 0;
    sscanf(buf, "%d", &result);
    if(result >= 300)
        return result;

    return 0;
}
