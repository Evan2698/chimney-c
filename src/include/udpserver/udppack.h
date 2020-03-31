#ifndef PACK_H_874455645264345231544523455_321947812374
#define PACK_H_874455645264345231544523455_321947812374
#include "core/address.h"

struct Pack
{

Address get_dst();

Address get_src();

unsigned char get_cmd();

unsigned char * get_data();

void set_src(Address &a);
void set_dst(Address &a);
void set_cmd(unsigned char i);
void set_data(unsigned char *buffer, size_t t);
 
std::vector<unsigned char> & get_out();

size_t packToBytes(unsigned char *buffer, size_t t);
std::vector<unsigned char> packToBytes();

static Pack ParsePack(unsigned char *buffer,  unsigned int size);

private:
    Address dst;
    Address src;
    unsigned char cmd;
    std::vector<unsigned char> buffer;
};

#endif