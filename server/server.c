#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/ip.h>

#include <sys/types.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>

#define SERVER_PORT 5015
#define BACKLOG 16
#define MAX_USER 32

extern int errno;

struct client{
	pthread_t tid;
	sem_t mutex;
	int fd;
	struct sockaddr_in addr;
};

/* global variable */
sem_t user_lock;
struct client *user_list[MAX_USER];
int user_cnt = 0;

//client_get_time 00, client_get_name 01, client_get_list 02, client_send_message 03

void print_client_addr(struct client *cptr) {
	printf("%s:%d", inet_ntoa(cptr->addr.sin_addr), ntohs(cptr->addr.sin_port));
}

void client_destroy(struct client* cptr) {
	//关闭socket句柄，但考虑socket连接失败的特殊情况
	if (cptr->fd != -1)
		close(cptr->fd);
	
	sem_destroy(&(cptr->mutex)); 	//释放信号量
	
	free(cptr);
}

int user_find(struct client* cptr) {
    //this function should be used when have an semaphore
    int user_id = -1;
    for (int i = 0; i < user_cnt; i++) {
        if (user_list[i] == cptr) {
            user_id = i;
            break;
        }
    }

    return user_id;
}

void print() {
	/* 
	printf("user_cnt = %d\n", user_cnt);
	for (int i = 0; i < user_cnt; i++) {
		printf("%d: %p\n", i, user_list[i]);
	} */
}

void user_delete(struct client* cptr) {
    sem_wait(&user_lock);
	print();
    int id = user_find(cptr);
    if (id != -1) {
        for (int i = id; i < user_cnt - 1; i++)
            user_list[i] = user_list[i+1];
        user_cnt--;
    } else {
		printf("In user_delete(), this user hasn't been created \n");
	}
	print();
    sem_post(&user_lock);

    client_destroy(cptr);
}

void time2str(char *str) {
	time_t t;
	struct tm tt;

	t = time(NULL);
	localtime_r(&t, &tt);

	strftime(str, 20, "%F %T", &tt);
}

int send_time(struct client *cptr) {
	char mesg[32];
	char *p = mesg;

	*(int *)p = 20;
	p = p + sizeof(int);
	*(p++) = 0;
	time2str(p);

	sem_wait(&(cptr->mutex));
	int ret = (int)send(cptr->fd, mesg, 24, 0);
	sem_post(&(cptr->mutex));
	
	print_client_addr(cptr);
	printf(" get time\n");
	return ret;
}

int send_name(struct client *cptr) {
	char mesg[512];
	char *hostname = mesg + 5;

	FILE *fp = fopen("/etc/hostname", "r");
	if (fp == NULL) {
		strcpy(hostname, "permission denied");
	} else {
		fscanf(fp, "%s", hostname);
		//fgets(hostname, 256, fp);
	}
	fclose(fp);

	int len = (int)strlen(hostname);
	*(int *)mesg = len + 1;
	mesg[4] = 1;

	sem_wait(&(cptr->mutex));
	int ret = (int)send(cptr->fd, mesg, (size_t)(len + 5), 0);
	sem_post(&(cptr->mutex));
	
	print_client_addr(cptr);
	printf(" get host name\n");
	return ret;
}

int send_list(struct client *cptr, struct client *list[], int *total) {
	char mesg[5 + MAX_USER * 6];
	char *info = mesg + 5;

	sem_wait(&user_lock);
	*total = user_cnt;
	for (int i = 0; i < user_cnt; i++)
		list[i] = user_list[i];
	*(int *)mesg = *total * 6 + 1;
	mesg[4] = 2;
	for (int i = 0; i < *total; i++) {
		*(int *)(info + i * 6) = list[i]->addr.sin_addr.s_addr;
        *(short *)(info + i * 6 + 4) = list[i]->addr.sin_port;
	}
	sem_post(&user_lock);

	sem_wait(&(cptr->mutex));
	int ret = (int)send(cptr->fd, mesg, 5 + (size_t)*total * 6, 0);
	sem_post(&(cptr->mutex));
	
	print_client_addr(cptr);
	printf(" get client list\n");
	return ret;
}

int send_message(struct client *cptr, short list_id, char *str, int len,
                 struct client *list[], int list_cnt) {
    if (list_id < 0 || list_id >= list_cnt) {
        return 1;
    }

    struct client *dest_ptr = list[list_id];

    sem_wait(&user_lock);
    int user_id = user_find(dest_ptr);
    sem_post(&user_lock);

    if (user_id == -1) {
        return 2;
    }

    if (len > 256) {
        return 3;
    }

    char mesg[512];
    *(int *)mesg = 7 + len;
    mesg[4] = 3;
    *(int *)(mesg + 5) = cptr->addr.sin_addr.s_addr;
    *(short *)(mesg + 9) = cptr->addr.sin_port;
    for (int i = 0; i < len; i++)
        mesg[i + 11] = str[i];

    sem_wait(&(cptr->mutex));
    int ret = (int)send(dest_ptr->fd, mesg, (size_t)(len + 11), 0);
    sem_post(&(cptr->mutex));

    if (ret == -1) {
        return 4;
    }
	
	print_client_addr(cptr);
	printf(" send message '%s' to ", str);
	print_client_addr(dest_ptr);
	printf("\n");

    return 0;
}

int send_message_response(struct client *cptr, int v) {
    char mesg[6];
    *(int *)mesg = 2;
    mesg[4] = 4;
    switch (v) {
        case 0:
            mesg[5] = 21;
            break;
        case 1:
            mesg[5] = 22;
            break;
        case 2:
            mesg[5] = 23;
            break;
        case 3:
            mesg[5] = 24;
            break;
        default:
            mesg[5] = 25;
            break;
    }

    sem_wait(&(cptr->mutex));
    int ret = (int)send(cptr->fd, mesg, 6, 0);
    sem_post(&(cptr->mutex));

    return ret;
}

//return value: >0 receive length, =0 disconnect, <0 errno
int myrecv(int clientfd, char *mesg, unsigned int len) {
	int ret = 0;
    unsigned int left;
	left = len;
	while (left > 0) {
		ret = (int)recv(clientfd, mesg, left, 0);
		if (ret <= 0)
			break;
		left = left - ret;
		mesg = mesg + ret;
	}
	if (ret > 0)
		ret = len;
	return ret;
}

void* thread(void *para) {
	struct client *cptr = (struct client*)para;
	struct client *list[MAX_USER];
	int list_cnt = 0;

	printf("user ");
	print_client_addr(cptr);
	printf(" connected\n");
	
	sem_wait(&user_lock);
	user_list[user_cnt] = cptr;
	user_cnt++;
	sem_post(&user_lock);
	
	while (1) {
		int ret;
		unsigned int len;
		char *buf;
		
		ret = myrecv(cptr->fd, (char *)&len, sizeof(int));
		if (ret <= 0) {
			printf("In receive(), err = %d\n", errno);
			user_delete(cptr);
			break;
		}
		
		buf = (char *)malloc(len);
		
		ret = myrecv(cptr->fd, buf, len);
		if (ret <= 0) {
			printf("In receive(), err = %d\n", errno);
			free(buf);
			user_delete(cptr);
			break;
		}

		switch (buf[0]) {
			case 0:
				ret = send_time(cptr);
				break;
			case 1:
				ret = send_name(cptr);
				break;
			case 2:
				ret = send_list(cptr, list, &list_cnt);
				break;
			case 3:
                ret = send_message(cptr, *(short *)(buf + 1), buf + 3, len - 3, list, list_cnt);
                /* in test begin --> */
                printf("send message:");
                for (int i = 0; i < len - 3; i++) {
                    putchar(buf[i + 3]);
                }
                printf("\n");
                /* in test end <--- */
                ret = send_message_response(cptr, ret);
				break;
			case 4:
                ret = -1;
				break;
			default:
				printf("Invalid package.\n");
				break;
		}

        if (ret == -1) {
            printf("In send(), err = %d\n", errno);
            free(buf);
            user_delete(cptr);
            break;
        }

		free(buf);
	}

	return NULL;
}

int main() {
	int sockfd;
	struct sockaddr_in server_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("In socket(), can't get fd : %d\n", errno);
		return -1;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in)) == -1) {
		printf("In bind(), err = %d \n", errno);
		close(sockfd);
		return -1;
	}

	if (listen(sockfd, BACKLOG) == -1) {
		printf("In listen(), err = %d\n", errno);
		close(sockfd);
		return -1;
	}
	
	/* 全局变量的初始化 */
	user_cnt = 0;
	sem_init(&user_lock, 0, 1);

	printf("Start to listen client to connect\n");

	while (1) {
		int has_space = 1;
		struct client *cptr = NULL;

		//分配新的存放用户数据的空间，需要互斥锁
		sem_wait(&user_lock);
		if (user_cnt < MAX_USER) {
			cptr = (struct client*)malloc(sizeof(struct client));
		} else {
			has_space = 0;
		}
		sem_post(&user_lock);
		
		//分配空间失败，重新进入主循环请求分配空间
		if (!has_space)
			continue;
		
		//初始化用户信息的互斥锁
		sem_init(&(cptr->mutex), 0, 1);
		
		//接受用户连接请求
		socklen_t sockaddr_size = sizeof(struct sockaddr_in);
		cptr->fd = accept(sockfd, (struct sockaddr *)&(cptr->addr), &sockaddr_size);
		if (cptr->fd == -1) {
			printf("In accept(), err = %d\n", errno);
			user_delete(cptr);
			break;
		}

		//创建服务进程并交给其运行
		int ret = pthread_create(&(cptr->tid), NULL, thread, cptr);
		if (ret != 0) {
			printf("In pthread_create(), err = %d\n", ret);
			user_delete(cptr);
			break;
		}
		printf("has created a thread\n");
	}

	close(sockfd);

	return 0;
}
