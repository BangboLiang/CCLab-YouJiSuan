#include <stdio.h>
#include <iostream>
#include <stdlib.h>					
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>					
#include <pthread.h>
#include <iostream>
#include <string>
#include <string.h>
#include <sstream>
#include <fcntl.h>
#include <linux/tcp.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <getopt.h>
#include <stdlib.h>
#include <math.h>
#include <semaphore.h> 
#define MAXTNUM 5000

using namespace std;

pthread_mutex_t mutex;	// 定义互斥锁，全局变量
pthread_mutex_t lock;//calculate the number of masks 
sem_t full;
sem_t empty;

 // 初始化互斥锁，互斥锁默认是打开的

pthread_t thread_id[MAXTNUM];

int total=0;//calculate the number of masks
int thread_num = 1;
string ipchar;
char* inputip="0.0.0.0";

//----------socket-------------------------------------
int sockfd = 0;	// 套接字
int connfd = 0;
int err_log = 0;
struct sockaddr_in my_addr;	// 服务器地址结构体
unsigned short port = 9999; // 监听端口
//----------socket-------------------------------------	





string f2to10(string num2){
	string _res;
	int len = num2.length();
	int base = 1;
	int num10 = 0;;
	for (int i = len-1; i>=0 ;--i){
		num10 += (num2[i] - '0')*base;
		base = base*2;
	}
	//cout << num10 << endl;
	if (num10 == 0){
		return "0";
	}
	int n =0;
	int xnum = num10;
	while(xnum != 0){
		n++;
		xnum = xnum/10;
	}
	//cout << n << endl;
	for (int i = n-1;i >=0; --i){
		_res += (num10 % (int)(pow(10,i+1)))/(pow(10,i))  + '0';
	}
	//cout << "f2to result is " << _res << endl;
	return _res;
}



string intToIp(uint32_t num)
{

    string strRet = "";
    for (int i = 0; i < 4; i++)
    {
        uint32_t tmp = (num >> ((3 - i) * 8)) & 0xFF;
        //cout << "temp is " << tmp << endl;
        string tmpip="00000000";
        //itoa(tmp, chBuf, 10);
        for (int j = 0;j < 8;++j){
        	uint32_t ttmp = (tmp >> (8-(j+1)) & 0x1);
        	//cout << "ttmp is " << ttmp << endl;
        	tmpip[j] = ttmp + '0';
        	//cout << "tmp ip " << tmpip << endl;
        	//tmpip = f2to10(tmpip);
        }
        //cout << "tmp ip before " << tmpip << endl;
        //cout <<"tmpip is " <<tmpip << endl;
        tmpip = f2to10(tmpip);
        //cout << "tmp ip after " << tmpip << endl;
        strRet += tmpip;

        if (i < 3)
        {
            strRet += ".";
        }
    }

    return strRet;
}
void Error_dealer(string method,string url,int tsockfd)
{
	string entity;
	if(method!="GET"&&method!="POST")//GET和POST之外的请求
        {
		entity="<htmL><title>501 Not Implemented</title><body bgcolor=ffffff>\n Not Implemented\n<p>Does not implement this method: "+method+"\n<hr><em>HTTP Web Server </em>\n</body></html>\n";
		int ilen=entity.length();
		stringstream ss;
		ss<<ilen;
		string slen;
		ss>>slen;
		string tmp="HTTP/1.1 501 Not Implemented\r\nServer: Youjisuan Web Server\r\nContent-Type: text/html\r\nContent-Length: "+slen+"\r\nConnection: Keep-Alive\r\n\r\n";
		string message=tmp+entity;
                char send_buf[1024];
                sprintf(send_buf,"%s",message.c_str());
                write(tsockfd,send_buf,strlen(send_buf));
                
	}
	else
        {
		if(method=="GET")//GET请求不满足要求
                {
			entity="<html><title>GET 404 Not Found</title><body bgcolor=ffffff>\n Not Found\n<p>Couldn't find this file: ."+url+"\n<hr><em>HTTP Web Sever</em>\n</body></html>\n";
		}
		else if(method=="POST")//POST请求不满足要求
                {
			entity="<html><title>POST 404 Not Found</title><body bgcolor=ffffff>\n Not Found\n<hr><em>HTTP Web Sever</em>\n</body></html>\n";
		}
		int ilen=entity.length();
		stringstream ss;
		ss<<ilen;
		string slen;
		ss>>slen;
		string tmp="HTTP/1.1 404 Not Found\r\nServer: Youjisuan Web Server\r\nContent-Type: text/html\r\nContent-Length: "+slen+"\r\nConnection: Keep-Alive\r\n\r\n";
		string message=tmp+entity;
		char send_buf[1024];
                sprintf(send_buf,"%s",message.c_str());
                write(tsockfd,send_buf,strlen(send_buf));
	}
}

void Get_dealer(string method,string url,int tsockfd)
{
	int len=url.length();
	string tmp="./src";
	if(url.find(".")==string::npos)//url中不存在‘.’
        {
		if(url[len-1]=='/'||url.length()==0)
                {
			tmp+=url+"index.html";
		}
		else tmp+=url+"./index.html";
	}
	else tmp+=url;
	int fd=open(tmp.c_str(),O_RDONLY);
	if(fd>=0)
        {
		int tlen=tmp.length();
                struct stat stat_buf;
   		fstat(fd,&stat_buf);
		char outstring[1024];
                sprintf(outstring,"HTTP/1.1 200 OK\r\nServer: Youjisuan Web Server\r\nContent-Length: %d\r\nContent-Type: text/html\r\nConnection: Keep-Alive\r\n\r\n",stat_buf.st_size);
		write(tsockfd,outstring,strlen(outstring));
		sendfile(tsockfd,fd,0,stat_buf.st_size);
	}
	else
        {
		Error_dealer(method,url,tsockfd);
	}
}

void Post_dealer(string name,string ID,int tsockfd)
{
	string entity="<html><title>POST Method</title><body bgcolor=ffffff>\nYour Name: "+name+"\nID: "+ID+"\n<hr><em>Http Web server</em>\n</body></html>\n";
	int ilen=entity.length();
	stringstream ss;
	ss<<ilen;
	string slen;
	ss>>slen;
	string tmp="HTTP/1.1 200 OK\r\nServer: Youjisuan Web Server\r\nContent-Type: text/html\r\nContent-Length: "+slen+"\r\nConnection: Keep-Alive\r\n\r\n";
	string message=tmp+entity;
	char send_buf[1024];
        sprintf(send_buf,"%s",message.c_str());
        write(tsockfd,send_buf,strlen(send_buf));
}

void Dealer(char* recv_buf,int tconnfd)
{
	string s_buf;
	bool status;
	status=true;//持久连接标志
	s_buf=string(recv_buf);
	//处理http请求报文
	//while(s_buf.find("HTTP/1.0")!=string::npos)//判断s_buf中有没有完整报文
	while(s_buf.find("HTTP/1.1")!=string::npos)//判断s_buf中有没有完整报文
       {
		int request_end_pos=0;//请求除主体外报文尾部         
		if((request_end_pos=s_buf.find("\r\n\r\n"))!=-1)//判断是否有请求体
                {
                        string request="";//请求报文
			request_end_pos+=4;
			request=s_buf.substr(0,request_end_pos);//复制报文到request
			int head_end_pos=request.find("Content-Length");
			int entity_pos=request.length();//实体主体起始位置
			if(head_end_pos!=-1)//存在请求体
                        {
				string num;
				head_end_pos+=15;
				while(request[head_end_pos]!='\r')
                                {
					num+=request[head_end_pos++];
				}
				int entity_len=atoi(num.c_str());
				if((s_buf.length()-request.length())>=entity_len)//有主体
                                {
					request+=s_buf.substr(request.length(),entity_len);
					request_end_pos+=entity_len;
                                }
                                else continue;
                        }
                        s_buf=s_buf[request_end_pos];//得到完整请求报文
                        string method,url;
                        request_end_pos=0;
                        while(request[request_end_pos]!=' ')
                        {
            	               method+=request[request_end_pos++];
                        }
                        if(method!="GET"&&method!="POST")
                        {
            	               Error_dealer(method,url,tconnfd);
            	               continue;
                        }
                        ++request_end_pos;
                        while(request[request_end_pos]!=' ')
                        {
			       url+=request[request_end_pos++];
			}
			++request_end_pos;//提取URL
			if(method=="GET")
                        {
				Get_dealer(method,url,tconnfd);
			}
			else if(method=="POST")
                        {                                
				if(url!="/Post_show")
                                {
					Error_dealer(method,url,tconnfd);
					continue;
				}
				string entity=request.substr(entity_pos,request.length()-entity_pos);
				int namepos=entity.find("Name="),idpos=entity.find("&ID=");
				if(namepos==-1||idpos==-1||idpos<=namepos)
                                {
					Error_dealer(method,url,tconnfd);
					continue;
				}
				string name,ID;                        
				name=entity.substr(namepos+5,idpos-namepos-5);
				ID=entity.substr(idpos+4);
				Post_dealer(name,ID,tconnfd);
			}
		}
	}
}

/************************************************************************
函数名称：	void *client_process(void *arg)
函数功能：	线程函数,处理客户信息
函数参数：	已连接套接字
函数返回：	无
************************************************************************/
void *client_process(void *arg)
{
	while(1){
		int recv_len = 0;
		char recv_buf[200000] = "";	// 接收缓冲区
		sem_wait(&full);
		pthread_mutex_lock(&mutex); 
		int tconnfd = connfd; // 传过来的已连接套接字
		pthread_mutex_unlock(&mutex); 
		sem_post(&empty);
		// 接收数据
		while((recv_len = recv(tconnfd, recv_buf, sizeof(recv_buf), 0)) > 0)
		{
			Dealer(recv_buf,tconnfd);
	                pthread_mutex_lock(&lock);
	                total++;
	                pthread_mutex_unlock(&lock);
	                printf("\ntotal=%d\n\n",total);
	                //send(connfd, recv_buf, recv_len, 0); // 给客户端回数据
		}
		
		printf("client closed!\n");
		close(tconnfd);//关闭已连接套接字
	}
}

//===============================================================
// 语法格式：	void main(void)
// 实现功能：	主函数，建立一个TCP并发服务器
// 入口参数：	无
// 出口参数：	无
//===============================================================

void* socket_process(void *arg)
{
	printf("TCP Server Started at port %d!\n", port);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);   //创建TCP套接字
	if(sockfd < 0)
	{
		perror("socket error");
		exit(-1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port   = htons(port);
	// if(inputip = "0.0.0.0"){
	// 	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	// }
	// else{
		// my_addr.sin_addr.s_addr=inet_addr(inputip);
	// }
	printf("Binding server to port %d\n", port);
	cout << "Binding server to ip " << intToIp(ntohl(my_addr.sin_addr.s_addr) )<<"\n";

	// 绑定
	err_log = bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr));
	if(err_log != 0)
	{
		perror("bind");
		close(sockfd);		
		exit(-1);
	}
	
	// 监听，套接字变被动
	err_log = listen(sockfd, 10);
	if( err_log != 0)
	{
		perror("listen");
		close(sockfd);		
		exit(-1);
	}
	
	printf("Waiting client...\n");

	while(1)
	{

		char cli_ip[INET_ADDRSTRLEN] = "";	       // 用于保存客户端IP地址
		struct sockaddr_in client_addr;		       // 用于保存客户端地址
		socklen_t cliaddr_len = sizeof(client_addr);   // 必须初始化!!!
		
		sem_wait(&empty);
		// 上锁，在没有解锁之前，pthread_mutex_lock()会阻塞
		pthread_mutex_lock(&mutex);	
		
		//获得一个已经建立的连接	
		connfd = accept(sockfd, (struct sockaddr*)&client_addr, &cliaddr_len);   							
		if(connfd < 0)
		{
			perror("accept this time");
			sem_post(&empty);
			pthread_mutex_unlock(&mutex);	
			continue;
		}
		
		// 打印客户端的 ip 和端口
		//const char *inet_ntop(int af, const void *src, char *dst, socklen_t cat);
		inet_ntop(AF_INET, &client_addr.sin_addr, cli_ip, INET_ADDRSTRLEN);//将数值格式转化为点分十进制的ip地址格式
		printf("inet_ntop:%s\n",cli_ip);
        //返回值：若成功则为指向结构的指针，若出错则为NULL
		printf("----------------------------------------------\n");
		printf("client ip=%s,port=%d\n", cli_ip,ntohs(client_addr.sin_port));
		
		if(connfd > 0)
		{
			pthread_mutex_unlock(&mutex);	
			sem_post(&full);
			//给回调函数传的参数，&connfd，地址传递
			//pthread_create(&thread_id, NULL, client_process, &connfd);  //创建线程
			//pthread_detach(thread_id); // 线程分离，结束时自动回收资源
		}
	}
	close(sockfd);
	return NULL;
}


int main(int argc, char *argv[])
{
	sem_init(&full,0,0);
	sem_init(&empty,0,1);
	pthread_mutex_init(&mutex, NULL);
    int opt;
    int digit_optind = 0;
    int option_index = 0;
    char *string = "a:b:d:";
    bzero(&my_addr, sizeof(my_addr));	   // 初始化服务器地址
    static struct option long_options[] =
        {  
        {  "ip",required_argument,NULL,'r'},
        {"port",required_argument,NULL,'r'},
        {"number-thread",required_argument,NULL,'r'},
        {NULL, 0, 0, 0}
        };
        int i=1;

        while((opt =getopt_long_only(argc,argv,string,long_options,&option_index))!= -1)
        {  
            if(strcmp(argv[i],"--ip")==0)
            {
            	//inputip = optarg;
               my_addr.sin_addr.s_addr=inet_addr(optarg);
               //unsigned int myipaddr = ntohl(my_addr.sin_addr.s_addr);
               cout << "Binding server to ip " << intToIp(ntohl(my_addr.sin_addr.s_addr))<<"\n";
            }
            if(strcmp(argv[i],"--port")==0)
            {
           		
               port=atoi(optarg);
               //my_addr.sin_port   = htons(port);
               //printf("2 Binding server to port %s\n", optarg);
            }
            if(strcmp(argv[i],"--number-thread")==0)
            {
            	//cout << "Found --numbrt-thread.\n";
            	thread_num=atoi(optarg);
            	printf("Thread num is %d\n",thread_num);
            }
            printf("opt = %c\t\t",        opt);
            printf("optarg = %s\t\t",     optarg);
            printf("argv[i] =%s\t\t",argv[i]);
            // if(i==1)
            // {
                i=optind;
            // }
            printf("option_index = %d\n", option_index);
        }
    pthread_t pro;
    pthread_create(&pro,NULL,socket_process,NULL);
    for(int i = 0;i < thread_num; ++i){
    	pthread_create(&thread_id[i],NULL,client_process,NULL);
    }
    pthread_join(pro,NULL);
    for(int i = 0;i < thread_num;++i){
    	pthread_join(thread_id[i],NULL);
    }
	return 0;
}

