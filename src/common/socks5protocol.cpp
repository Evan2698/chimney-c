#include "common/socks5protocol.h"
#include "common/minipool.h"
#include "glog/logging.h"
#include "common/address.h"
#include "common/func.hpp"

Socks5Protocol::Socks5Protocol() : state(enum_Fresh), peer_state(enum_Fresh), can_close(0)
{
    memset(&client, 0, sizeof(client));
    LOG(INFO) << "Socks5Protocol" << std::endl;
}

Socks5Protocol::~Socks5Protocol()
{
    memset(&client, 0, sizeof(client));
    LOG(INFO) << "~Socks5Protocol" << std::endl;
}

static std::string format_enum(int a)
{
    switch (a)
    {
    case Socks5Protocol::enum_Fresh:
        return "Fresh";
        break;

    case Socks5Protocol::enum_Hello:
        return "Hello";
        break;

    case Socks5Protocol::enum_Connect:
        return "Connect";
        break;

    case Socks5Protocol::enum_Auth:
        return "Auth";
        break;

    case Socks5Protocol::enum_Normal:
        return "Normal";
        break;

    default:
        break;
    }

    return "xxxxx";
}

int Socks5Protocol::routing_and_answser(char *income, unsigned int len)
{

    if (enum_Normal != state)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (enum_Fresh == state)
        {
            BufferHolder holder;
            auto p = (unsigned char *)holder;
            p[0] = 0x5;
            p[1] = 0xFF;
            if (income[0] == 0x5)
            {
                p[1] = 0x00;
            }
            state = enum_Hello;

            auto s = writer.lock();
            if (!!s)
            {
                int ret = s->write_to_stream((char *)holder.detach(), 2);
                LOG(INFO) << "dispatch write RET=" << ret << std::endl;
            }
            return 0;
        }
        else if (enum_Hello == state)
        {
            if (len < 4 || income[0] != 0x5 || income[1] != 1)
            {
                BackError(0xA);
                return -1;
            }

            std::vector<unsigned char> cmd(income + 3, income + len);
            target = Address::FromSocks5CommandStream(cmd);
            if (!target)
            {
                LOG(ERROR) << "parse socks5 target address faied!" << std::endl;
                BackError(0xC);
                writer.reset();
                return -1;
            }
            return init_client();
        }
    }
    else
    {
        LOG(INFO) << "Current state: " << format_enum(state) << std::endl;
        std::vector<unsigned char> in(income, income + len);
        std::vector<unsigned char> out;
        this->method->Compress(in, this->key, out);

        BufferHolder holder;
        if (holder.size() < out.size() + 4)
        {
            LOG(ERROR) << "BUFFER TOO SMALL!  " << out.size() + 4 << std::endl;
            return -1;
        }
        auto p = holder.detach();
        auto n = ToBytes(out.size());
        std::copy(n.begin(), n.begin() + 4, p);
        std::copy(out.begin(), out.end(), p + 4);

        uv_buf_t *buf = (uv_buf_t *)malloc(sizeof(uv_buf_t));
        buf->base = (char *)p;
        buf->len = 4 + out.size();

        uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
        write_req->data = buf;
        uv_write(write_req, (uv_stream_t*)(&this->client), buf, 1, after_write_once);
    }

    return 0;
}

void Socks5Protocol::BackError(unsigned char erorr_code)
{
    unsigned char rsu[10] = {0x05, erorr_code, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    BufferHolder holder;
    auto p = (unsigned char *)holder;
    std::copy(std::begin(rsu), std::end(rsu), p);
    auto s = writer.lock();
    if (!!s)
    {
        int ret = s->write_to_stream((char *)holder.detach(), 2);
        LOG(INFO) << "dispatch write RET=" << ret << std::endl;
        writer.reset();
        return;
    }
}

void Socks5Protocol::register_write_stream(std::weak_ptr<WriteStream> wp)
{
    writer = wp;
}

int Socks5Protocol::init_client()
{
    auto ret = uv_tcp_init(uv_default_loop(), &client);
    if (ret != 0)
    {
        LOG(ERROR) << "initialize peer failed, RET= " << ret << std::endl;
        return ret;
    }

    struct sockaddr_in req_addr = {0};
    ret = uv_ip4_addr(remote.host().c_str(), remote.port(), &req_addr);
    if (ret != 0)
    {
        LOG(ERROR) << "initialize peer socket address failed, RET= " << ret << std::endl;
        return ret;
    }

    memset(&connect_req, 0, sizeof(uv_connect_t));
    connect_req.data = this;
    client.data = this;
    this->can_close = 1;

    return uv_tcp_connect(&connect_req, &client, (struct sockaddr *)&req_addr, Socks5Protocol::on_connect);
}

void Socks5Protocol::on_connect(uv_connect_t *req, int status)
{
    BufferHolder holder;

    uv_buf_t *buf = (uv_buf_t *)malloc(sizeof(uv_buf_t));
    buf->base = (char *)holder.detach();
    buf->len = 3;
    char szBuffer[3] = {0x5, 0x1, 0x2};
    std::copy(std::begin(szBuffer), std::end(szBuffer), buf->base);

    uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
    write_req->data = buf;
    uv_write(write_req, req->handle, buf, 1, after_write_once);
    auto pThis = (Socks5Protocol *)req->data;
    pThis->peer_state = enum_Hello;
}

void Socks5Protocol::after_write_once(uv_write_t *req, int status)
{
    if (req->data != nullptr)
    {
        uv_buf_t *buf = (uv_buf_t *)req->data;
        if (buf->base != nullptr)
        {
            MiniPool::get_instance().Free((unsigned char *)buf->base);
        }
        free(buf);
    }
    uv_read_start(req->handle, alloc_cb, after_read);
    free(req);
}

void Socks5Protocol::alloc_cb(uv_handle_t *handle,
                              size_t suggested_size,
                              uv_buf_t *buf)

{
    buf->base = reinterpret_cast<char *>(MiniPool::get_instance().Alloc());
    buf->len = MiniPool::get_instance().size() - 128;
}

void Socks5Protocol::after_read(uv_stream_t *handle,
                                ssize_t nread,
                                const uv_buf_t *buf)
{
    BufferHolder holder(true);
    if (buf->base != NULL)
    {
        LOG(ERROR) << "NO data, but have space on buf!" << std::endl;
        holder.attach(buf->base);
    }

    if (nread == 0)
    {
        LOG(ERROR) << "nread is zero. unknown" << std::endl;
        return;
    }
    Socks5Protocol *pThis = (Socks5Protocol *)handle->data;
    if (nread < 0)
    {
        LOG(ERROR) << "socket has an error. " << nread << std::endl;
        pThis->start_shutdown();
        return;
    }
    //--------------------------------------------------------------
    // nread > 0
    //--------------------------------------------------------------

    if (pThis->peer_state != enum_Normal)
    {
        std::lock_guard<std::mutex> lock(pThis->_peer);
        if (enum_Hello == pThis->peer_state)
        {
            if (nread < 10 || buf->base[0] != 0x5 || buf->base[1] != 0x2 || buf->base[2] != nread - 3)
            {
                LOG(ERROR) << "socks5 server format is incorrect! " << ToHexEX(buf->base, buf->base + 3) << std::endl;
                pThis->BackError(0xB);
                pThis->start_shutdown();
                return;
            }
            auto szBuffer = buf->base;
            auto start = szBuffer + 3;
            auto end = start + szBuffer[2];
            std::vector<unsigned char> ls(start, end);
            auto sp = build_privacy_method(ls);
            if (!sp.has_value())
            {
                LOG(ERROR) << "bytes 2 method failed" << ToHexEX(szBuffer, szBuffer + nread) << std::endl;
                pThis->BackError(0xD);
                pThis->start_shutdown();
                return;
            }
            pThis->method = sp.value();
            pThis->send_user_pass();
            pThis->peer_state = enum_Auth;
        }
        else if (enum_Auth == pThis->peer_state)
        {
            auto szBuffer = buf->base;
            if (nread < 2 || szBuffer[0] != 5 || szBuffer[1] != 0)
            {
                pThis->BackError(0xE);
                pThis->start_shutdown();
                return;
            }
            pThis->send_target_to_remote();
            pThis->peer_state = enum_Connect;
        }
        else if (enum_Connect == pThis->peer_state)
        {
            auto szBuffer = buf->base;
            if (nread < 10 || szBuffer[0] != 0x5 || szBuffer[1] != 0)
            {
                pThis->BackError(0xF);
                LOG(ERROR) << "connect failed " << ToHexEX(szBuffer, szBuffer + 20) << " - " << nread << std::endl;
                return;
            }
            std::vector<unsigned char> cmds(szBuffer + 3, szBuffer + nread);
            auto bound = Address::FromSocks5CommandStream(cmds);
            pThis->Back_Local_Bound_Address(bound);
            pThis->peer_state = enum_Normal;
        }
    }
    else
    {
        std::lock_guard<std::mutex> lock(pThis->_peer);
        if (pThis->incoming.size() > 4)
        {
            auto &tmp = pThis->incoming;
            auto len = ToInt(tmp.c_str());
            if (tmp.size() >= len + 4)
            {
                std::vector<unsigned char> ll(tmp.begin() + 4, tmp.begin() + 4 + len);
                auto xx = std::string(tmp.begin() + 4 + len, tmp.end());
                pThis->incoming = xx;
                std::vector<unsigned char> out;
                auto ret = pThis->method->UnCompress(ll, pThis->key, out);
                if (ret == 0)
                {
                    pThis->Back_Write(out);
                }
            }
        }

        auto szBuffer = buf->base;
        if (nread < 4)
        {
            pThis->incoming.append(szBuffer, szBuffer + nread);
            LOG(INFO) << "RECV BYTES: APPEND1 " << nread << std::endl;
        }
        else
        {
            auto len = ToInt(szBuffer);
            LOG(INFO) << "require len: " << len << " bytes" << std::endl;
            if (len + 4 <= nread)
            {
                std::vector<unsigned char> zip(szBuffer + 4, szBuffer + 4 + len);
                std::vector<unsigned char> out;
                auto ret = pThis->method->UnCompress(zip, pThis->key, out);
                if (ret == 0)
                {
                    pThis->Back_Write(out);
                }
                if (nread > len + 4)
                {
                    pThis->incoming.append(szBuffer + 4 + len, szBuffer + nread);
                    LOG(INFO) << "RECV BYTES: APPEND3 " << nread << std::endl;
                }
            }
            else
            {
                pThis->incoming.append(szBuffer, szBuffer + nread);
                LOG(INFO) << "RECV BYTES: APPEND2 " << nread << std::endl;
            }
        }
    }
}

void Socks5Protocol::Back_Write(std::vector<unsigned char> &out)
{
    BufferHolder holder;

    if (out.size() > holder.size())
    {
        LOG(ERROR) << "out put buffer to long!!!! REQ="
                   << out.size() << " RAL=" << holder.size() << std::endl;
    }
    else
    {
        auto p = (char *)holder.detach();
        std::copy(out.begin(), out.end(), p);
        auto sp = writer.lock();
        if (!!sp)
        {
            sp->write_to_stream(p, out.size());
        }
    }
}

unsigned int Socks5Protocol::ToInt(const char *sz)
{
    unsigned int hi = sz[0];
    hi = hi << 24;

    unsigned tmp = sz[1];
    tmp = tmp << 16;
    hi = hi | tmp;

    tmp = sz[2];
    tmp = tmp << 8;
    hi = hi | tmp;

    hi = hi | sz[3];
}
std::string Socks5Protocol::ToBytes(unsigned int v)
{
    std::string out;

    char tmp = (v >> 24) & 0xff;
    out.push_back(tmp);

    tmp = (v >> 16) & 0xff;
    out.push_back(tmp);

    tmp = (v >> 8) & 0xff;
    out.push_back(tmp);

    tmp = v & 0xff;
    out.push_back(tmp);

    return out;
}

void Socks5Protocol::send_user_pass()
{
    std::vector<unsigned char> out;
    auto ret = this->method->Compress(this->pass, this->key, out);
    BufferHolder holder;
    auto szBuffer = holder.detach();
    szBuffer[0] = 0x5;
    szBuffer[1] = this->user.size();
    std::copy(this->user.begin(), this->user.end(), szBuffer + 2);
    szBuffer[2 + this->user.size()] = out.size();
    std::copy(out.begin(), out.end(), szBuffer + 2 + this->user.size() + 1);
    auto wcnt = 2 + this->user.size() + 1 + out.size();

    uv_buf_t *buf = (uv_buf_t *)malloc(sizeof(uv_buf_t));
    buf->base = (char *)szBuffer;
    buf->len = wcnt;

    uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
    write_req->data = buf;
    uv_write(write_req, (uv_stream_t *)(&this->client), buf, 1, afterr_write);
}
void Socks5Protocol::afterr_write(uv_write_t *req, int status)
{
    if (req->data != nullptr)
    {
        uv_buf_t *buf = (uv_buf_t *)req->data;
        if (buf->base != nullptr)
        {
            MiniPool::get_instance().Free((unsigned char *)buf->base);
        }
        free(buf);
    }

    LOG(INFO) << "Current write after status: " << status << std::endl;
    if (UV_EPIPE == status)
    {
        uv_close((uv_handle_t *)req->handle, on_close);
    }

    if (req != nullptr)
    {
        free(req);
    }
}

void Socks5Protocol::send_target_to_remote()
{
    BufferHolder holder;
    auto szBuffer = holder.detach();
    auto bytes = target->PackSocks5Address();
    LOG(INFO) << "connect  target : " << target->toString() << std::endl;
    std::copy(bytes.begin(), bytes.begin() + 4, szBuffer);
    szBuffer[1] = 0x1; //Connect
    auto tmp = std::vector<unsigned char>(bytes.begin() + 4, bytes.end());
    std::vector<unsigned char> out;
    this->method->Compress(tmp, this->key, out);
    auto wcnt = 4 + 1 + out.size();
    szBuffer[4] = out.size();
    std::copy(out.begin(), out.end(), szBuffer + 5);

    uv_buf_t *buf = (uv_buf_t *)malloc(sizeof(uv_buf_t));
    buf->base = (char *)szBuffer;
    buf->len = wcnt;

    uv_write_t *write_req = (uv_write_t *)malloc(sizeof(uv_write_t));
    write_req->data = buf;
    uv_write(write_req, (uv_stream_t *)(&this->client), buf, 1, afterr_write);
}

void Socks5Protocol::Back_Local_Bound_Address(std::shared_ptr<Address> bound)
{
    if (!!bound)
    {
        std::lock_guard<std::mutex> lcl(_mutex);
        state = enum_Normal;

        auto ss = bound->PackSocks5Address();
        BufferHolder holder;
        auto p = (unsigned char *)holder;
        std::copy(ss.begin(), ss.end(), p);
        auto s = writer.lock();
        if (!!s)
        {
            s->write_to_stream((char *)holder.detach(), ss.size());
        }
    }
    else
    {
        BackError(0x1F);
        start_shutdown();
        LOG(ERROR) << "bound address failed!!!---------------------";
    }
}

void Socks5Protocol::after_shutdown(uv_shutdown_t *req, int status)
{
    LOG(INFO) << "after_shutdown status=" << status << std::endl;
    uv_close((uv_handle_t *)req->handle, on_close);
    free(req);
}

void Socks5Protocol::start_shutdown()
{
    uv_read_stop((uv_stream_t *)&client);
    auto req = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
    auto r = uv_shutdown(req, (uv_stream_t *)&client, Socks5Protocol::after_shutdown);
    LOG(INFO) << "uv_shutdown RET=" << r << std::endl;
}

void Socks5Protocol::on_close(uv_handle_t *handle)
{
    Socks5Protocol *pThis = (Socks5Protocol *)handle->data;
    pThis->can_close = 0;
}

int Socks5Protocol::query_status()
{
    return can_close;
}