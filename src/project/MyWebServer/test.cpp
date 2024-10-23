while (1)
{
    memset(buf, '\0', BUFFER_SIZE);
    int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
    if (ret＜0)
    {
        /*对于非阻塞IO，下面的条件成立表示数据已经全部读取完毕。此后，epoll就能再次
       触发sockfd上的EPOLLIN事件，以驱动下一次读操作*/
        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
        {
            printf("read later\n");
            break;
        }
        close(sockfd);
        break;
    }
    else if (ret == 0)
    {
        close(sockfd);
    }
    else
    {
        printf("get%d bytes of content:%s\n", ret, buf);
    }
}
// }
// else
// {
//     printf("something else happened\n");
// }
// }
// }