#include "common/socks5incom.h"
#include "glog/logging.h"
#include "common/minipool.h"
#include "common/g.h"
#include <random>
#include "uv.h"
#include <thread>
#include <chrono>
#include "client/clientfactory.h"

WriteStreamHub::WriteStreamHub(Socks5Income *p) : pwriter(p)
{
    LOG(INFO) << "WriteStreamHub" << std::endl;
}

WriteStreamHub::~WriteStreamHub()
{
    pwriter = nullptr;
    LOG(INFO) << "~WriteStreamHub" << std::endl;
}

int WriteStreamHub::write_to_stream(char *out, unsigned int len)
{
    if (pwriter != nullptr)
    {
        return pwriter->write_to_stream(out, len);
    }
    return 0;
}

static unsigned int random_char()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}

static std::string generate_hex(const unsigned int len)
{
    std::stringstream ss;
    for (auto i = 0; i < len; i++)
    {
        const auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << rc;
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}

std::atomic_uint Socks5Income::object_counter = 0;

Socks5Income::Socks5Income() : parent(nullptr)
{
    memset(&stream, 0, sizeof(stream));
    hub = std::make_shared<WriteStreamHub>(this);
    name = generate_hex(16);
    ++object_counter;
    this->router = ClientFactory::get_instance()->build_client();
    LOG(INFO) << "Socks5Income ( " << name
              << " )is constructed! "
              << "All: " << object_counter << std::endl;
}
Socks5Income::~Socks5Income()
{
    LOG(INFO) << "begin ~Socks5Income" << std::endl;
    router.reset();
    hub.reset();
    parent = nullptr;
    memset(&stream, 0, sizeof(stream));
    --object_counter;
    LOG(INFO) << "Socks5Income ( " << name
              << " )is destoried!"
              << " REST All:  " << object_counter << std::endl;
}

int Socks5Income::on_connect(uv_stream_t *server)
{
    this->parent = server;
    auto r = uv_tcp_init(uv_default_loop(), &stream);
    if (r != 0)
    {
        LOG(ERROR) << "call uv_tcp_init failed! RET=" << r << std::endl;
        return r;
    }

    stream.data = this;

    r = uv_accept(server, (uv_stream_t *)&stream);
    if (r != 0)
    {
        LOG(ERROR) << "call uv_accept failed! RET=" << r << std::endl;
        return r;
    }

    router->register_write_stream(hub);
    r = uv_read_start((uv_stream_t *)&stream, Socks5Income::alloc_cb, Socks5Income::after_read);
    if (r != 0)
    {
        LOG(ERROR) << "call uv_read_start failed! RET=" << r << std::endl;
        return r;
    }

    return 0;
}

void Socks5Income::alloc_cb(uv_handle_t *handle,
                            size_t suggested_size,
                            uv_buf_t *buf)

{
    buf->base = reinterpret_cast<char *>(MiniPool::get_instance().Alloc());
    buf->len = MiniPool::get_instance().size() - 128;
}

void Socks5Income::after_read(uv_stream_t *handle,
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
    Socks5Income *pThis = (Socks5Income *)handle->data;
    if (nread < 0)
    {
        LOG(ERROR) << "socket has an error. " << nread << std::endl;
        pThis->start_shutdown();
        return;
    }

    if (!!(pThis->router))
    {
        auto ret = pThis->router->routing_and_answser(buf->base, nread);
        LOG(INFO) << "routing to socks5 stream  RET=" << ret << std::endl;
        if (ret != 0)
        {
            pThis->start_shutdown();
            LOG(ERROR) << "write error!  " << ret << std::endl;
        }
    }
}

int Socks5Income::write_to_stream(char *out, unsigned int len)
{
    uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
    uv_buf_t *buf = (uv_buf_t *)malloc(sizeof(uv_buf_t));
    buf->len = len;
    buf->base = out;
    req->data = buf;
    auto r = uv_write(req, (uv_stream_t *)&stream, buf, 1, Socks5Income::after_write);
}

void Socks5Income::after_write(uv_write_t *req, int status)
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

void Socks5Income::after_shutdown(uv_shutdown_t *req, int status)
{
    LOG(INFO) << "after_shutdown status=" << status << std::endl;
    uv_close((uv_handle_t *)req->handle, on_close);
    free(req);
}

void Socks5Income::start_shutdown()
{
    uv_read_stop((uv_stream_t *)&stream);
    auto req = (uv_shutdown_t *)malloc(sizeof(uv_shutdown_t));
    auto r = uv_shutdown(req, (uv_stream_t *)&stream, Socks5Income::after_shutdown);
    LOG(INFO) << "uv_shutdown RET=" << r << std::endl;
}

void Socks5Income::on_close(uv_handle_t *handle)
{
    Socks5Income *pThis = (Socks5Income *)handle->data;
    while (pThis->router->query_status() != 0)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    delete pThis;
}