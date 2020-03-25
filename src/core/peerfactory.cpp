#include "core/peerfactory.h"
#include "core/socket.h"
#include "core/func.hpp"
#include <optional>
#include "core/g.h"
PeerFactory::PeerFactory()
{
}

PeerFactory *PeerFactory::get_instance()
{
    static PeerFactory one;
    return &one;
}

int PeerFactory::sayHello(Stream * sp)
{
    unsigned char szBuffer[3] = {0x5, 0x1, 0x2};
    std::vector<unsigned char> out(std::begin(szBuffer), std::end(szBuffer));
    int n = sp->Write(out);
    if (n <= 0)
    {
        LOG_S(ERROR) << "say hello failed! " << std::endl;
        return -1;
    }

    out.resize(300);
    n = sp->Read(out);
    if (n <= 0)
    {
        LOG_S(ERROR) << "read hello failed! " << std::endl;
        return -1;
    }

    if (out[0] != 0x5 || out[1] != 2 || out[2] > 250 || out[2] + 3 != out.size())
    {
        LOG_S(ERROR) << "server response format error "
                   << ToHexEX(out.begin(), out.end())
                   << "LEN=" << out.size() << std::endl;
        return -1;
    }
    std::vector<unsigned char> ls(out.begin() + 3, out.begin() + 3 + out[2]);
    auto op = PrivacyBase::build_privacy_method(ls);
    if (!op.has_value())
    {
        LOG_S(ERROR) << "parse method failed!!" << std::endl;
        return -1;
    }
    method = op.value();

    return 0;
}

int PeerFactory::VerifyAuth(Stream *sp)
{
    std::vector<unsigned char> out;
    auto ret = this->method->Compress(this->pass, this->key, out);
    if (ret != 0)
    {
        LOG_S(ERROR) << "Compressed pass failed " << std::endl;
        return -1;
    }

    auto wcnt = 2 + this->user.size() + 1 + out.size();
    std::vector<unsigned char> tmp(wcnt, 0);
    unsigned char *szBuffer = tmp.data();

    szBuffer[0] = 0x5;
    szBuffer[1] = this->user.size();
    std::copy(this->user.begin(), this->user.end(), szBuffer + 2);
    szBuffer[2 + this->user.size()] = out.size();
    std::copy(out.begin(), out.end(), szBuffer + 2 + this->user.size() + 1);
    ret = sp->Write(tmp);
    if (ret <= 0)
    {
        LOG_S(ERROR) << "send user & pass failed: " << ret << std::endl;
        return -1;
    }
    ret = sp->Read(tmp);
    if (ret <= 0)
    {
        LOG_S(ERROR) << "Server response about user & pass. " << ret << std::endl;
        return -1;
    }

    if (tmp.size() < 2 || tmp[0] != 5 || tmp[1] != 0)
    {
        LOG_S(ERROR) << "Verify user failed. " << ToHexEX(tmp.begin(), tmp.end()) << std::endl;
        return -1;
    }

    return 0;
}

std::shared_ptr<Address> PeerFactory::doConnect(Stream *sp, const Address &target)
{
    std::shared_ptr<Address> bound;
    auto bytes = target.PackSocks5Address();
    bytes[1] = 0x1; //connect command
    auto tmp = std::vector<unsigned char>(bytes.begin() + 4, bytes.end());
    std::vector<unsigned char> out;
    auto ret = this->method->Compress(tmp, this->key, out);
    if (ret != 0)
    {
        LOG_S(ERROR) << "Compress target failed. " << std::endl;
        return bound;
    }
    auto wcnt = 4 + 1 + out.size();
    tmp.resize(wcnt);
    std::copy(bytes.begin(), bytes.begin() + 4, tmp.begin());
    tmp[4] = out.size();
    std::copy(out.begin(), out.end(), tmp.begin() + 5);
    auto writen = sp->Write(tmp);
    if (writen <= 0)
    {
        LOG_S(ERROR) << "connect failed " << writen << std::endl;
        return bound;
    }

    tmp.resize(300);
    auto rcnt = sp->Read(tmp);
    if (rcnt <= 0)
    {
        LOG_S(ERROR) << "read bound address failed. " << rcnt << std::endl;
        return bound;
    }

    if (tmp[0] != 0x5 || tmp[1] != 0 || tmp.size() < 10)
    {
        LOG_S(ERROR) << "bound address format is invalid. " << ToHexEX(tmp.begin(), tmp.end()) << std::endl;
        return bound;
    }
    
    std::vector<unsigned char> cmds(tmp.begin() + 3, tmp.end());
    return Address::FromSocks5CommandStream(cmds);
}

Stream * PeerFactory::build_peer_with_target(const std::shared_ptr<Address> &target)
{
   
    LOG_S(INFO) << "target address: " << target->toString() << std::endl;

    auto fd = SocketBuilder::create_socket(proxy, "tcp");
    if (fd == -1)
    {
        LOG_S(ERROR) << "create socket failed: " << fd << std::endl;
        return nullptr;
    }
    auto out = new Stream(fd);
    StreamHodler holder(out);
    
    if (sayHello(out) != 0)
    {
        LOG_S(ERROR) << "say hello failed: " << std::endl;       
        return nullptr;
    }

    if (VerifyAuth(out) != 0)
    {
        LOG_S(ERROR) << "verify user and pass failed " << std::endl;
        return nullptr;
    }
    auto bound = doConnect(out, *target.get());
    if (!bound)
    {
        LOG_S(ERROR) << "connect failed>>>>>>" << target->toString() << std::endl; 
        return nullptr;
    }

    out->set_local(*bound.get());
    out->set_remote(*target.get());

    return holder.detach();
}

void PeerFactory::set_proxy(const Address &r)
{
    this->proxy = r;
}

void PeerFactory::set_key(const std::vector<unsigned char> &k)
{
    this->key.resize(k.size());
    std::copy(k.begin(), k.end(), key.begin());
}

void PeerFactory::set_user_pass(const std::vector<unsigned char> &u, const std::vector<unsigned char> &p)
{
    this->user.resize(u.size());
    std::copy(u.begin(), u.end(), user.begin());

    this->pass.resize(p.size());
    std::copy(p.begin(), p.end(), pass.begin());
}

std::shared_ptr<Privacy>  & PeerFactory::getMethod()
{
    return method;
}

 std::vector<unsigned char > & PeerFactory::get_key()
 {
     return key;
 }