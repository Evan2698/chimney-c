#include "common/socks5_uv.h"
#include "glog/logging.h"
#include "common/socks5incom.h"
#include "common/g.h"
Socks5_uv::Socks5_uv()
{
    memset(&tcp_server, 0, sizeof(tcp_server));
}

Socks5_uv::~Socks5_uv()
{
    uv_loop_close(uv_default_loop());
    memset(&tcp_server, 0, sizeof(tcp_server));
}

int Socks5_uv::run()
{
    auto r = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
    return r;
}

int Socks5_uv::launch(Address local)
{
    struct sockaddr_in addr;
    int r = uv_ip4_addr(local.host().c_str(), local.port(), &addr);
    LOG(INFO) << "uv_ip4_addr RET=" << r << std::endl;
    if (r != 0)
    {
        LOG(ERROR) << "call uv_ip4_addr failed!" << std::endl;
        return r;
    }
    r = uv_tcp_init(uv_default_loop(), &tcp_server);
    if (r != 0)
    {
        LOG(ERROR) << "uv_tcp_init failed! ." << r << std::endl;     
        return r;
    }

    r = uv_tcp_bind(&tcp_server, (const struct sockaddr *)&addr, 0);
    if (r != 0)
    {
        LOG(ERROR) << "uv_tcp_bind failed! ." << r << std::endl;
        return r;
    }

    tcp_server.data = this;  // set this to every stream object!

    r = uv_listen((uv_stream_t *)&tcp_server, SOMAXCONN, Socks5_uv::on_connection);
    if (r != 0)
    {
        LOG(ERROR) << "uv_listen failed! ." << r << std::endl;
        return r;
    }

    return 0;
}

void Socks5_uv::on_connection(uv_stream_t *server, int status)
{
    UNREFERENCED_PARAMETER(status);
    auto p = new Socks5Income();
    p->on_connect(server);   // need to delete, delete by itself.
}

