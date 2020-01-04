#include <stdint.h>
#include <string>


#ifndef HEADER_H
#define HEADER_H

#endif // HEADER_H

#define TCP 6

struct uint24{
    unsigned long v:24;
}__attribute__((packed));           // and push pop

void *plog(void *arg);


struct radiotap_header
{
    uint8_t header_revision;
    uint8_t header_pad;
    uint16_t header_length;
    uint32_t present_flags;
    uint8_t flags;
    uint16_t channel_frequency;
    uint16_t channel_flags;
    uint8_t antenna_signal;
    uint8_t antenna;
    uint16_t rx_flags;
    uint8_t mcs_info1;
    uint8_t mcs_info2;
    uint8_t mcs_info3;

};

struct llc_header
{
    uint8_t dsap;
    uint8_t snap;
    uint8_t control_field;
    uint24 organization_code;
    uint16_t type;
};

struct ip_header
{
    uint8_t version;
    uint8_t differentiated;
    uint16_t total_length;
    uint16_t identification;
    uint16_t flags;
    uint8_t ttl;
    uint8_t protocol;
    uint16_t header_checksum;
    uint32_t source_ip;
    uint32_t destination_ip;
};


struct map_value
{
    std::string host;
    std::string cookie;
};

struct data
{
    std::string host;
    std::string cookie;
    std::string base_domain;
};

