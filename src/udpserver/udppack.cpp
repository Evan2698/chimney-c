#include "udpserver/udppack.h"
#include "core/funs.h"
Address Pack::get_dst()
{
    return this->dst;
}

Address Pack::get_src()
{
    return this->src;
}

unsigned char Pack::get_cmd()
{
    return this->cmd;
}

void Pack::set_src(Address &a)
{
    this->src = a;
}
void Pack::set_dst(Address &a)
{
    this->dst = a;
}
void Pack::set_cmd(unsigned char i)
{
    this->cmd = i;
}

std::vector<unsigned char> &Pack::get_out()
{
    return this->buffer;
}

//---------------------------------------------------------------------------------------
// | 1 cmd| 2(len) | 1 type|  ip(domain) target | 2(len) 1 type| ip(domain) src| (3072)data|
//
//
//-----------------------------------------------------------------------------------------

size_t Pack::packToBytes(unsigned char *buffer, size_t t)
{
    buffer[0] = cmd;
    auto dst = this->dst.PackSocks5Address();
    if (dst.size() == 0)
    {
        return 0;
    }

    auto current = buffer + 1;
    auto srcLen = dst.size() - 4;
    auto uslen = Funcs::U16ToBytes(srcLen);
    std::copy(uslen.begin(), uslen.end(), current);
    current += 2;
    *current = dst[3];
    current++;
    std::copy(dst.begin() + 4, dst.end(), current);
    current += (dst.size() - 4);

    dst = this->src.PackSocks5Address();
    if (dst.size() == 0)
    {
        return 0;
    }

    srcLen = dst.size() - 4;
    uslen = Funcs::U16ToBytes(srcLen);
    std::copy(uslen.begin(), uslen.end(), current);
    current += 2;
    current[0] = dst[3];
    current++;
    std::copy(dst.begin() + 4, dst.end(), current);
    current += (dst.size() - 4);

    std::copy(this->buffer.begin(), this->buffer.end(), current);
    current += this->buffer.size();

    return current - buffer;

}


Pack Pack::ParsePack(unsigned char *buffer, unsigned int size)
{
    Pack pck;
    pck.cmd = buffer[0];
    auto pRead = buffer + 1;
    auto ll = Funcs::ToShort(pRead);
    pRead += 2;
    auto pt = Address::FromSocks5CommandStream(std::vector<unsigned char>(pRead, pRead + ll));
    pck.dst = *pt.get();
    pRead += ll;
    ll = Funcs::ToShort(pRead);
    pRead += 2;
    pt = Address::FromSocks5CommandStream(std::vector<unsigned char>(pRead, pRead + ll));
    pck.src = *pt.get();
    pRead += ll;
    pck.buffer = std::vector<unsigned char>(pRead, buffer + size);
    return pck;
}

void Pack::set_data(unsigned char *buffer, size_t t)
{
    this->buffer = std::vector<unsigned char>(buffer, buffer + t);
}

unsigned char * Pack::get_data()
{
    return this->buffer.data();
}