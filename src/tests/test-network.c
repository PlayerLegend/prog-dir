#include "network.h"
#include "print.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

void test_hosting_works()
{
    char * service = "23572";
    int fd = tcp_host(service);
    if(-1 == fd)
    {
	print_error("Couldn't host tcp on '%s'",service);
	exit(1);
    }
    else
    {
	printf("Hosted tcp on '%s'!\n",service);
    }
    close(fd);

    fd = udp_host(service);
    if(-1 == fd)
    {
	print_error("Couldn't host udp on '%s'",service);
	exit(1);
    }
    else
    {
	printf("Hosted udp on '%s'!\n",service);
    }
    close(fd);
}

int main()
{
    test_hosting_works();
    return 0;
}
