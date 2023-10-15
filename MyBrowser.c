
/*    THE CLIENT PROCESS */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include<sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <time.h>
#include<fcntl.h>

#define SIZE 8192

void get_ip(char *url, char *ip)
{
	int temp = 0;
	for(int i=7; i<strlen(url); i++)
	{
		if(url[i] == '/')
		{
			ip[temp] = '\0';
			break;
		}
		ip[temp++] = url[i];
	}
}

int get_port(char *url)
{
	int temp;
	for(long int i = strlen(url); i>=0; i--)
	{
		if(url[i] == ':')
		{
			temp = i;
			break;
		}
	}
	temp+=1;
	if(atoi(url+temp) == 0 && strlen(url+temp)>5)
	{
		return 80;
	}
	else
	{
		return atoi(url+temp);
	}
}

int connect_server(char *ip, int port)
{
	//printf("1");
	int	sockfd ;
	struct sockaddr_in	serv_addr;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		perror("Unable to create socket\n");
		return -1;
	}   

	serv_addr.sin_family	= AF_INET;
	//printf("1");
	int s = inet_aton(ip, &serv_addr.sin_addr);
	if (s == 0) 
	{
        printf("Not in correct format\n");
        return -1;
    } else if (s < 0) 
	{
        printf("Unable to convert IP address\n");
        return -1;
    }
	serv_addr.sin_port	= htons(port);

	if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) 
	{
		perror("Unable to connect to server\n");
		return -1;
	}
	else
	{
		printf("Connected to the server\n");
	}
	return sockfd;
}
char* seperate_content(char *request)
{
  const char *delimiter = "\n\n";
  char *token;
  char *p = request;
  
  token = strstr(p, delimiter);
  if(token==NULL) return NULL;
  *token = '\0';
   p = token + strlen(delimiter);
  return p;
}
char* recvf(int sockfd,char* buf,int* clen)
{
      int bytes_recv=0;
      int chunk_size=100;
	  char *content; 
      while(1)
      { 
          char* chunks=malloc(chunk_size*sizeof(char));
          memset(chunks,0,chunk_size);
          int t=recv(sockfd,chunks,chunk_size,0);
          if(t<0) return NULL;
          strcat(buf,chunks);
		  bytes_recv+=t;
		  if((content=seperate_content(buf))!=NULL)
		  {
			  *clen=bytes_recv-strlen(buf)-2;
			  break;
		  }
		  if(buf[bytes_recv-1]=='\0') break;
          if(t==0) break;
      }
     return content;  
}
int headercheck(char **headers,int n,long long int *len,char* type)
{
	 char date[50];
	// char type[20];
	 memset(date,0,50);
	// memset(type,0,20);
	 
     for(int i=1;i<n;i++)
	 {
		char *token=strtok(headers[i]," ");
        if(strcmp(token,"Expires:")==0)
		{
			token=strtok(NULL," ");
			if(token==NULL) return 400;
			continue;
		}
		else if(strcmp(token,"Cache-control:")==0)
		{   
            token=strtok(NULL," ");
			if(token==NULL) return 400;
			continue; 
		}
		else if(strcmp(token,"Last-modified:")==0)
		{ 
			token=strtok(NULL," ");
			if(token==NULL) return 400;
			continue; 
		}
		else if(strcmp(token,"Content-language:")==0)
		{   
			token=strtok(NULL," ");
			if(token==NULL) return 400;
			continue; 
		}
		else if(strcmp(token,"Content-length:")==0)
		{
			token=strtok(NULL," ");
			if(token==NULL) return 400;
			*len=atoi(token);
			continue; 
		}
		else if(strcmp(token,"Content-type:")==0)
		{   
			token=strtok(NULL," ");
			if(token==NULL) return 400;
			else
			{
				if(strcmp(token,type)==0) strcpy(type,token);
				else return 401;
			}
			continue; 
		}
        else continue;
	 }
	
	 return 1;
}
char** split_into_lines(char *request, int *num_lines,int n) {

  char **lines = (char**)malloc( n* sizeof(char*));
  for (int i = 0; i < n; i++) {
    lines[i] = (char*)malloc(80 * sizeof(char));
  }
  char *line = strtok(request, "\n");
  int i = 0;
  while (line != NULL) {
    strcpy(lines[i], line);
    i++;
    line = strtok(NULL, "\n");
  }
  *num_lines = i;
  return lines;   
}
int main()
{
	while(1)
	{
		printf("\nMyOwnBrowser> ");
		long int l = 5;
		char *expr = (char *)malloc(sizeof(char));
		memset(expr,0,sizeof(expr));
		while(1)
		{
			char ch;
			scanf("%c",&ch);
			if(ch == '\n') break;
			l += 1;
			expr = realloc(expr,l*sizeof(char));
			strncat(expr,&ch,1);
		}
		if(l==5) 
		{
			printf("Only GET, PUT, QUIT Command-line operators are supported\n");
			continue;
		}
		int m = strlen(expr);
		expr[m] = '\0';
		
		char* delimiter = " ";
		char* str = strtok(expr,delimiter);
        int gorp=-1;
		int sockfd;
		char buf[SIZE];
		int file_name;
		char path[SIZE];
		int res;
		char type[25];
		memset(type,0,25);
		if(strcmp(str,"GET")==0)
		{
            gorp=0;
			char* url = strtok(NULL,delimiter);
			char *ip = (char *)malloc(20*sizeof(char));
			get_ip(url,ip);
			int port = get_port(url);
		
			memset(buf,0,sizeof(buf));

			sockfd = connect_server(ip,port);
			if(sockfd<0) continue;
			strcat(buf, "GET ");
			send(sockfd,"GET ",4,0);
			int temp;
			for(long int i = strlen(url); i>=0; i--)
			{
				if(url[i] == ':')
				{
					temp = i;
					break;
				}
			}
			char ch = '\n';
			memset(path,0,8192);

			if(atoi(url+temp+1) == 0 && *(url+temp+1)!=0)
			{
				strncpy(path,url+7+strlen(ip),strlen(url)-7-strlen(ip));
				strcat(buf,path);
				send(sockfd,path,strlen(path),0);
				strcat(buf, " HTTP/1.1\n");
				send(sockfd," HTTP/1.1\n",10,0);
				strcat(buf, "Host: ");
				send(sockfd,"Host: ",6,0);
				strcat(buf,ip);
				send(sockfd,ip,strlen(ip),0);
				strcat(buf,":80\n");
				send(sockfd,":80\n",4,0);

			}
			else
			{
				strncpy(path,url+7+strlen(ip),temp-7-strlen(ip));
				strcat(buf,path);
				send(sockfd,path,strlen(path),0);
				strcat(buf, " HTTP/1.1\n");
				send(sockfd," HTTP/1.1\n",10,0);
				strcat(buf, "Host: ");
				send(sockfd,"Host: ",6,0);
				strcat(buf,ip);
				send(sockfd,ip,strlen(ip),0);
				strcat(buf,url+temp);
				send(sockfd,url+temp,strlen(url+temp),0);
				strncat(buf,&ch,1);
				send(sockfd,"\n",1,0);
			}
			
			strcat(buf, "Date: ");
			send(sockfd,"Date: ",6,0);
			time_t current_time = time(NULL);
			struct tm *time_info = gmtime(&current_time);
			char date[80];
			strftime(date, sizeof(date), "%a, %d %b %Y %T GMT", time_info);
			strcat(buf,date);
			send(sockfd,date,strlen(date),0);
			strncat(buf,&ch,1);
			send(sockfd,"\n",1,0);
			strcat(buf, "Accept: ");
			send(sockfd,"Accept: ",8,0);
			for(long int i = strlen(path); i>=0; i--)
			{
				if(path[i] == '.')
				{
					res = i;
					break;
				}
			}
			res+=1;
			for(long int i = strlen(path); i>=0; i--)
			{
				if(path[i] == '/')
				{
					file_name = i;
					break;
				}
			}
			file_name++;
			if(strcmp(path+res,"pdf") == 0)
			{
				strcat(buf,"application/pdf\n");
			    strcat(type,"application/pdf");
				send(sockfd,"application/pdf\n",16,0);
			}
			else if(strcmp(path+res,"html") == 0)
			{
				strcat(buf,"text/html\n");
				strcpy(type,"text/html");
				send(sockfd,"text/html\n",10,0);
			}
			else if(strcmp(path+res,"jpg") == 0)
			{
				strcat(buf,"image/jpeg\n");
				strcpy(type,"image/jpeg");
				send(sockfd,"image/jpeg\n",11,0);
			}
			else
			{
				strcat(buf,"text/*\n");
				strcpy(type,"text/*");
				send(sockfd,"text/*\n",7,0);
			}
			strcat(buf,"Accept-Language: en-US,en;q=0.8\n");
			send(sockfd,"Accept-Language: en-US,en;q=0.8\n",32,0);
			strcat(buf, "If-Modified-Since: ");
			send(sockfd,"If-Modified-Since: ",19,0);
			memset(date,0,sizeof(date));
			time_info->tm_mday -= 2;
			time_t new_time = timegm(time_info);
			time_info = gmtime(&new_time);
			strftime(date, sizeof(date), "%a, %d %b %Y %T GMT", time_info);
			strcat(buf,date);
			send(sockfd,date,strlen(date),0);
			strncat(buf,&ch,1);
			send(sockfd,"\n",1,0);
			strcat(buf, "Connection: close\n");
			send(sockfd,"Connection: close\n",18,0);
			send(sockfd,"\0",1,0);
			 printf("\n\nRequest sent:\n\n");
			printf("%s",buf);
			memset(buf,0,SIZE);
			
		}
		else if(strcmp(str,"PUT") == 0)
		{
            gorp=1;
			char* url = strtok(NULL,delimiter);
			char* filepath = strtok(NULL,delimiter);
			//printf("%s\n", url);
			//printf("%s\n", filepath);
			char *ip = (char *)malloc(20*sizeof(char));
			get_ip(url,ip);
			//printf("%s\n", ip);
			int port = get_port(url);
			//printf("%d\n",port);
			
			memset(buf,0,sizeof(buf));
			//printf("1");
			sockfd = connect_server(ip,port);
			if(sockfd<0) continue;
			strcat(buf, "PUT ");
			send(sockfd,"PUT ",4,0);
			int temp;
			for(long int i = strlen(url); i>=0; i--)
			{
				if(url[i] == ':')
				{
					temp = i;
					break;
				}
			}

			char ch = '\n';
			memset(path,0,8192);
			char c = '/';

			if(atoi(url+temp+1) == 0 && *(url+temp+1)!=0)
			{
				strncpy(path,url+7+strlen(ip),strlen(url)-7-strlen(ip));
				strcat(buf,path);
				send(sockfd,path,strlen(path),0);
				strncat(buf,&c,1);
				send(sockfd,"/",1,0);
				strcat(buf,filepath);
				send(sockfd,filepath,strlen(filepath),0);
				strcat(buf, " HTTP/1.1\n");
				send(sockfd," HTTP/1.1\n",10,0);
				strcat(buf, "Host: ");
				send(sockfd,"Host: ",6,0);
				strcat(buf,ip);
				send(sockfd,ip,strlen(ip),0);
				strcat(buf,":80\n");
				send(sockfd,":80\n",4,0);
			}
			else
			{
				strncpy(path,url+7+strlen(ip),temp-7-strlen(ip));
				strcat(buf,path);
				send(sockfd,path,strlen(path),0);
				strncat(buf,&c,1);
				send(sockfd,"/",1,0);
				strcat(buf,filepath);
				send(sockfd,filepath,strlen(filepath),0);
				strcat(buf, " HTTP/1.1\n");
				send(sockfd," HTTP/1.1\n",10,0);
				strcat(buf, "Host: ");
				send(sockfd,"Host: ",6,0);
				strcat(buf,ip);
				send(sockfd,ip,strlen(ip),0);
				strcat(buf,url+temp);
				send(sockfd,url+temp,strlen(url+temp),0);
				strncat(buf,&ch,1);
				send(sockfd,"\n",1,0);
			}
			strcat(buf, "Date: ");
			send(sockfd,"Date: ",6,0);
			time_t current_time = time(NULL);
			struct tm *time_info = gmtime(&current_time);
			char date[80];
			strftime(date, sizeof(date), "%a, %d %b %Y %T GMT", time_info);
			strcat(buf,date);
			send(sockfd,date,strlen(date),0);
			strncat(buf,&ch,1);
			send(sockfd,"\n",1,0);
			strcat(buf, "Accept: text/*\n");
			send(sockfd,"Accept: text/*\n",15,0);	
			strcat(buf,"Accept-Language: en-US,en;q=0.8\n");
			send(sockfd,"Accept-Language: en-US,en;q=0.8\n",32,0);
			strcat(buf,"Content-language: en-US\n");
			send(sockfd,"Content-language: en-US\n",24,0);
			FILE * fp;
			fp = fopen(filepath,"rb");
			fseek(fp, 0L, SEEK_END);
			long file_size = ftell(fp);
			printf("%ld\n",file_size);
			rewind(fp);

			strcat(buf,"Content-length: ");
			send(sockfd,"Content-length: ",16,0);
			char length[20];
			sprintf(length,"%ld",file_size);
			strcat(buf,length);
			send(sockfd,length,strlen(length),0);
			strncat(buf,&ch,1);
			send(sockfd,"\n",1,0);
			strcat(buf,"Content-type: ");
			send(sockfd,"Content-type: ",14,0);
			int res = 0;
			for(long int i = strlen(filepath); i>=0; i--)
			{
				if(filepath[i] == '.')
				{
					res = i;
					break;
				}
			}
			res+=1;
			if(strcmp(filepath+res,"pdf") == 0)
			{
				strcat(buf,"application/pdf\n");
				send(sockfd,"application/pdf\n",16,0);
			}
			else if(strcmp(filepath+res,"html") == 0)
			{
				strcat(buf,"text/html\n");
				send(sockfd,"text/html\n",10,0);
			}
			else if(strcmp(filepath+res,"jpg") == 0)
			{
				strcat(buf,"image/jpeg\n");
				send(sockfd,"image/jpeg\n",11,0);
			}
			else
			{
				strcat(buf,"text/*\n");
				send(sockfd,"text/*\n",7,0);
			}
			strcat(buf, "Connection: close\n");
			send(sockfd,"Connection: close\n",18,0);
			strncat(buf,&ch,1);
			send(sockfd,"\n",1,0);

			// Allocate memory for the buffer
			printf("\nRequest Sent:\n\n");
		    printf("%s",buf);
			memset(buf,0,SIZE);
			int read_size;
			long long int x=file_size;
			while((read_size=fread(buf,1,SIZE,fp))>0)
			{
				send(sockfd,buf,read_size,0);
				x-=read_size;
				printf("%lld\n",x);
				memset(buf,0,SIZE);
			}
            fclose(fp);

		
		}
		else if(strcmp(str,"QUIT") == 0)
		{
			break;
		}
		else
		{
			printf("Only GET, PUT, QUIT Command-line operators are supported\n");
			continue;
		}

			struct pollfd fdset;
			fdset.fd = sockfd;
			fdset.events = POLLIN;
			int timeout = 3000;
			if(poll(&fdset,1,timeout) == 0)
			{
				printf("Connection Timed Out\n");
				close(sockfd);
				continue;
			}
			else
			{
			  memset(buf,0,SIZE);
              int clen;
			  char* content=recvf(sockfd,buf,&clen);
			  int headerCount;
			  printf("\n\nResponse received:\n\n");
			  printf("%s\n",buf);
			  char** headers=split_into_lines(buf,&headerCount,11);

			   if(strcmp(headers[0],"HTTP/1.1 200 OK")==0)
			   {
				   printf("\nStatus received: OK\n\n");
				   if(gorp==1)
				   {
					  
				   }
				   else if(gorp==0)
				   {
					  int t,content_length;
					  t=headercheck(headers,headerCount,&content_length,type);
					  if(t==1)
					  {
                        FILE *fp=fopen(path+file_name,"wb");
						content_length-=clen;
					    fwrite(content,1,clen,fp);
						while(content_length>0)
						{
							memset(buf,0,8192);
							int bytes=recv(sockfd,buf,8192,0);
							//printf("%d\n",content_length);
							fwrite(buf,1,bytes,fp);
							content_length-=bytes;
						}
                        fclose(fp);
						char *file_path=malloc(100*sizeof(char));
                        memset(file_path,0,100);
						strcat(file_path,path+file_name);
						//printf("%s\n",file_path);
						char chrome[]="/usr/bin/google-chrome";
						if(strcmp(type,"application/pdf")==0)
						{
                           char *args[]={chrome,file_path,NULL};
                           if(fork()==0)
						   {
							execvp(chrome,args);
						   }
						   else wait(NULL);
						}
						else if(strcmp(type,"text/html")==0)
						{
                           char *args[]={chrome,file_path,NULL};
                           if(fork()==0)
						   {
							execvp(chrome,args);
						   }
						   else wait(NULL);
						}
						else if(strcmp(type,"text/*")==0)
						{
                           char *args[]={"gedit",file_path,NULL};
						    if(fork()==0)
						   {
							execvp(args[0],args);
						   }
						   else wait(NULL);
						}
						else
						{
							printf("Not handled\n");
						}
					
					  }
					  else if(t==401)
					  {
						printf("Content-type is not matching with the Accept-type\n");
					  }
					  else
					  {
						printf("Error in headers\n");
					  }

                    
				   }
                    
			   }
			   else if(strcmp(headers[0],"HTTP/1.1 400 Bad Request")==0)
			   {
                     //headercheck(headers,headerCount,gorp);   
					 printf("\n!!!400 Bad Request: Error in the syntax of request!!!\n");
					 printf("\nServer responded with a message:\n\n%s\n",content);            

			   }
			   else if(strcmp(headers[0],"HTTP/1.1 403 Forbidden")==0)
			   {
                     printf("\n!!!403 Forbidden: Can't be accessed!!!\n");  
					  printf("\nServer responded with a message:\n\n%s\n",content);                      
			   }
			   else if(strcmp(headers[0],"HTTP/1.1 404 Not Found")==0)
			   {
                    printf("\n!!!404 Not Found: No such file or directory!!!\n"); 
					 printf("\nServer responded with a message:\n\n%s\n",content);              
			   }
			   else printf("\nUnknown Error\n");


			}
        
	}
	
	return 0;

}