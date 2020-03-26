#include "core/socks5peer.h"
#include "core/g.h"
#include "core/socket.h"
#include "core/func.hpp"

Socks5Peer::Socks5Peer(const Bytes &k, const Bytes &u, const Bytes &p, const Address &remote) : key(k), user(u), pass(p),
                                                                                                proxy(remote)
{
}

Socks5Peer::~Socks5Peer()
{
}

std::shared_ptr<Stream> Socks5Peer::build_stream(const std::shared_ptr<Address> &target)
{
    LOG(INFO) << "target address: " << target->toString() << std::endl;

    auto fd = SocketBuilder::create_socket(proxy, "tcp");
    if (fd == -1)
    {
        LOG(ERROR) << "create socket failed: " << fd << std::endl;
        return nullptr;
    }

    auto out = std::make_shared<Stream>(fd);
    if (sayHello(out) != 0)
    {
        LOG(ERROR) << "say hello failed: " << std::endl;
        return nullptr;
    }

    if (VerifyAuth(out) != 0)
    {
        LOG(ERROR) << "verify user and pass failed " << std::endl;
        return nullptr;
    }
    auto bound = doConnect(out, target);
    if (!bound)
    {
        LOG(ERROR) << "connect failed>>>>>>" << target->toString() << std::endl;
        return nullptr;
    }

    out->set_local(*bound.get());
    out->set_remote(*target.get());

    return out;
}

int Socks5Peer::sayHello(std::shared_ptr<Stream> &sp)
{
    unsigned char szBuffer[3] = {0x5, 0x1, 0x2};
    std::vector<unsigned char> out(std::begin(szBuffer), std::end(szBuffer));
    int n = sp->Write(out);
    if (n <= 0)
    {
        LOG(ERROR) << "say hello failed! " << std::endl;
        return -1;
    }

    out.resize(300);
    n = sp->Read(out);
    if (n <= 0)
    {
        LOG(ERROR) << "read hello failed! " << std::endl;
        return -1;
    }

    if (out[0] != 0x5 || out[1] != 2 || out[2] > 250 || out[2] + 3 != out.size())
    {
        LOG(ERROR) << "server response format error "
                     << ToHexEX(out.begin(), out.end())
                     << "LEN=" << out.size() << std::endl;
        return -1;
    }
    std::vector<unsigned char> ls(out.begin() + 3, out.begin() + 3 + out[2]);
    auto op = PrivacyBase::build_privacy_method(ls);
    if (!op.has_value())
    {
        LOG(ERROR) << "parse method failed!!" << std::endl;
        return -1;
    }
    method = op.value();

    return 0;
}

int Socks5Peer::VerifyAuth(std::shared_ptr<Stream> &sp)
{
    std::vector<unsigned char> out;
    auto ret = this->method->Compress(this->pass, this->key, out);
    if (ret != 0)
    {
        LOG(ERROR) << "Compressed pass failed " << std::endl;
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
        LOG(ERROR) << "send user & pass failed: " << ret << std::endl;
        return -1;
    }
    ret = sp->Read(tmp);
    if (ret <= 0)
    {
        LOG(ERROR) << "Server response about user & pass. " << ret << std::endl;
        return -1;
    }

    if (tmp.size() < 2 || tmp[0] != 5 || tmp[1] != 0)
    {
        LOG(ERROR) << "Verify user failed. " << ToHexEX(tmp.begin(), tmp.end()) << std::endl;
        return -1;
    }

    return 0;
}

std::shared_ptr<Address> Socks5Peer::doConnect(std::shared_ptr<Stream> &sp, const std::shared_ptr<Address> &target)
{

    auto bytes = target->PackSocks5Address();
    bytes[1] = 0x1; //connect command
    auto tmp = std::vector<unsigned char>(bytes.begin() + 4, bytes.end());
    if (target->type() == Address::domain){
        tmp = std::vector<unsigned char>(bytes.begin() + 5, bytes.end());
    }
    std::vector<unsigned char> out;
    auto ret = this->method->Compress(tmp, this->key, out);
    if (ret != 0)
    {
        LOG(ERROR) << "Compress target failed. " << std::endl;
        return nullptr;
    }
    auto wcnt = 4 + 1 + out.size();
    tmp.resize(wcnt);
    std::copy(bytes.begin(), bytes.begin() + 4, tmp.begin());
    tmp[4] = out.size();
    std::copy(out.begin(), out.end(), tmp.begin() + 5);
    auto writen = sp->Write(tmp);
    if (writen <= 0)
    {
        LOG(ERROR) << "connect failed " << writen << std::endl;
        return nullptr;
    }

    tmp.resize(300);
    auto rcnt = sp->Read(tmp);
    if (rcnt <= 0)
    {
        LOG(ERROR) << "read bound address failed. " << rcnt << std::endl;
        return nullptr;
    }

    if (tmp[0] != 0x5 || tmp[1] != 0 || tmp.size() < 10)
    {
        LOG(ERROR) << "bound address format is invalid. " << ToHexEX(tmp.begin(), tmp.end()) << std::endl;
        return nullptr;
    }

    std::vector<unsigned char> cmds(tmp.begin() + 3, tmp.end());
    return Address::FromSocks5CommandStream(cmds);
}

std::vector<unsigned char> &Socks5Peer::get_key()
{
    return this->key;
}
std::vector<unsigned char> &Socks5Peer::get_user()
{
    return this->user;
}
std::vector<unsigned char> &Socks5Peer::get_pass()
{
    return this->pass;
}
std::shared_ptr<Privacy> &Socks5Peer::get_Method()
{
    return this->method;
}