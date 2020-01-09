#include <QtCore>
#include <QWidget>
#include <QMutex>
#include <iostream>
#include <pcap.h>
#include <linux/ip.h>           //iphdr
#include <linux/tcp.h>          //tcphdr
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <map>
#include <string>
#include <QString>
#include <mutex>
#include <set>
#include "header.h"
#include "mythread.h"
#include <QMessageBox>


mythread::mythread(QObject *parent) :
    QThread(parent)
{
}

using namespace std;


static map<string,struct map_value>m;
//static map<string,string>reassem_map;
static map<string,struct map_value>reassem_map;
static set<string>sld;
static mutex mutx;

string sld_process(bool,string);
string domain_check(string);
string data(string,string::size_type);
string make_key(struct iphdr *,struct tcphdr *);
void tcp_reassembly(const u_char *http,struct iphdr * ip_header, struct tcphdr * tcp_header,unsigned int http_len);



string sld_process(bool ox,string host_name)
{
    string tmp=host_name;                       //Host name copy
    tmp = tmp.substr(0,tmp.rfind("."));         //ex)tmp : test.gilgil
    if(ox) tmp = tmp.substr(0,tmp.rfind("."));
    return host_name=host_name.substr(tmp.rfind(".")+1);    //ex)host_name: gilgil.net
}
string domain_check(string host)
{
    string dase_domain;
    for(auto it=sld.begin();it!=sld.end();it++){
        if(host.find(*it) != string::npos){
            if(*it==".kr"){
                dase_domain = sld_process(true,host);
            }
            else{
                dase_domain = sld_process(false,host);
            }
        }
    }
//    sleep(1);
    return dase_domain;
}


string data(string http,string::size_type it)
{

    string result;
    result.clear();
    if (it != string::npos){ //찾았다면.
        string str2 = http.substr(it);      //cookie or host string extraction
        result=http.substr(it,str2.find("\n"));      //cookie or host ~ \n
        result=result.substr(result.find_first_of(":")+2);
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

    return key;
}

void tcp_reassembly(const u_char *http,struct iphdr * ip_header, struct tcphdr * tcp_header,unsigned int tcp_segment)
{
    struct map_value * http_value = new struct map_value;
    string key;
    //    string http_data;
    key = make_key(ip_header,tcp_header);

    for(unsigned int i=0;i<tcp_segment;i++)
        http_value->http_data+=static_cast<char>(http[i]);

    map<string,struct map_value>::iterator it;
    //next_seq = tcp_header->seq+tcp_segment;
    it = reassem_map.find(key);
    if(tcp_segment!=0)                                                   //tcp_segment 존재
    {
        if(it != reassem_map.end())                                      //Key가 있으면 붙이기
        {
            if(ntohl(tcp_header->seq) == it->second.next_seq){          //현재 패킷의 sequence가 이전에 저장해둔 sequence 값이랑 같으면 reassemble
                it->second.http_data.append(http_value->http_data);
                it->second.next_seq=ntohl(tcp_header->seq)+tcp_segment;
            }
            else{
//                system("clear");
//                cout<<"키는 있고 reassemble 이 끝났는데 또 패킷이 오면 key 뒤에 a를 붙여주기. 기존 패킷과 섞이지 않기위해.\n";    //키는 있고 reassemble 이 끝났는데 또 패킷이 오면 key 뒤에 a를 붙여주기. 기존 패킷과 섞이지 않기위해.
//                sleep(3);
            }
        }
        else if(it == reassem_map.end())                                    //Key 가 없으면 map에 추가하기.
        {
            if(!it->second.reassem_finish){
                http_value->next_seq=ntohl(tcp_header->seq)+tcp_segment;       //next_sequence number
                reassem_map.insert(make_pair(key,*http_value));
            }
        }
    }
    else if((it != reassem_map.end()) && (tcp_segment == 0))                 //Key는 있는데 데이터가 없다 .. 그러면 reassembly가 끝났다는 것.
    {
        if(ntohl(tcp_header->seq) == it->second.next_seq)
        {
            it->second.reassem_finish=true;     //reassemble finish

            string::size_type cookie_it,host_it;
            static struct map_value * value= new struct map_value;

            cookie_it=it->second.http_data.find("Cookie:");                   //cookie의 위치
            host_it=it->second.http_data.find("Host:");                       //Host의 위치
            if(cookie_it == string::npos|| host_it == string::npos) return;

            string host=data(it->second.http_data,host_it);
            string cookie=data(it->second.http_data,cookie_it);


            char src_buf[16];
            inet_ntop(AF_INET,&ip_header->saddr,src_buf,sizeof(src_buf));

            mutx.lock();
            auto iter =m.find(key);
            if(iter == m.end() && (it->second.reassem_finish)){
                value->host=host;
                value->cookie=cookie;
                value->base_domain=domain_check(host);
                m.insert(make_pair(key,*value));
            }
            mutx.unlock();
        }
        else if(it->second.next_seq==static_cast<unsigned int>(NULL))
        {
            // Key는 있는데 데이터,next_sequence number 도 없으면 reassemble 과정에서 문제가 있는걸까
//            system("clear");
//            cout<<"Key는 있는데 데이터,next_sequence number 도 없으면 reassemble 과정에서 문제가 있는걸까 \n";
//            sleep(3);

        }
    }
}

void mythread::run()
{
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    handle = pcap_open_live("wlan0",BUFSIZ,1,100,errbuf);
//    handle = pcap_open_offline("/root/Desktop/Project/naver_packet.pcap",errbuf);

    sld.insert(".kr");
    sld.insert(".com");
    sld.insert(".net");
    sld.insert(".doosan");
    sld.insert(".hyundai");
    sld.insert(".samsung");
    sld.insert(".lotte");
    int numberchanged_i=0;
    map<string,struct map_value>::iterator it=m.begin();
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
        struct iphdr *ip_header = reinterpret_cast<struct iphdr *>((reinterpret_cast<uint8_t *>(llc) + sizeof(llc)));
        struct tcphdr * tcp_header = reinterpret_cast<struct tcphdr *>((reinterpret_cast<uint8_t *>(ip_header)+(ip_header->ihl*4)));
        QMutex qmutex;
        qmutex.lock();
        if(this->Stop) break;
        qmutex.unlock();
        if(subtype == 0x88)
            if(ip_header->protocol == TCP &&(ntohs(tcp_header->dest) == 80 || ntohs(tcp_header->source) == 80 )){
                const u_char * http =(u_char *)((uint8_t*)tcp_header+(tcp_header->doff*4));
                unsigned int tcp_segment=ntohs(ip_header->tot_len) - ip_header->ihl*4 - (tcp_header->doff*4);

                tcp_reassembly(http,ip_header,tcp_header,tcp_segment);

                if(static_cast<int>(m.size())>numberchanged_i)
                {
                    it++;
                    numberchanged_i++;
                    QString pass_host = QString::fromStdString(it->second.host);
                    QString pass_cookie = QString::fromStdString(it->second.cookie);
                    QString pass_basedomain = QString::fromStdString(it->second.base_domain);
                    emit NumberChanged(static_cast<int>(m.size()),pass_host,pass_cookie,pass_basedomain);
                }
            }
    }

}

