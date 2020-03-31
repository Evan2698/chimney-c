

#include "core/g.h"
#include <string>
#include <vector>
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <iostream>
#include "config/config.h"
#include "privacy/privacy.h"
#include "core/func.hpp"
#include "core/address.h"
#include "core/peerfactory.h"
#include "core/socks5server.h"
#include <signal.h>
#include <unistd.h>
#include <memory>
#include "core/g.h"
#include <signal.h>
#include <thread>
#include "udpserver/udpserver.h"

std::string get_my_path();
std::shared_ptr<client> get_local_setting();
Socks5Server *g_p = nullptr;
int main(int argc, char *argv[])
{
    UNREFERENCED_PARAMETER(argc);
    google::InitGoogleLogging(argv[0]);
    FLAGS_colorlogtostderr = true;
    google::SetStderrLogging(google::INFO);

    auto settings = get_local_setting();
    if (!settings)
    {
        LOG(ERROR) << "config file is not exist!!!" << std::endl;
        exit(1);
    }

    std::string whereru = "WhereRU";
    std::vector<unsigned char> msg(whereru.begin(), whereru.end());
    std::vector<unsigned char> keyW(settings->pwd.begin(), settings->pwd.end());
    Address remote(settings->server, settings->server_port, Address::Type::ipv4);
    Address local(settings->local, settings->local_port, Address::Type::ipv4);
    LOG(INFO) << "S address:" << remote.toString() << std::endl;
    LOG(INFO) << "---------------------" << std::endl;
    LOG(INFO) << "L address:" << local.toString() << std::endl;

    auto key = PrivacyBase::make_sha1(keyW).value();
    auto hash = PrivacyBase::make_hmac(key, msg).value();
    LOG(INFO) << "key:" << ToHexEX(key.begin(), key.end()) << std::endl;
    LOG(INFO) << "hash:" << ToHexEX(hash.begin(), hash.end()) << std::endl;

    auto &client = PeerFactory::get_instance();
    client.set_key(key);
    client.set_user_pass(hash, hash);
    client.set_proxy(remote);
    client.set_time_out(settings->timeout);

    signal(SIGPIPE, SIG_IGN);
    Socks5Server server(local, settings->timeout);
    g_p = &server;
    signal(SIGINT, [](int sig) -> void {
        g_p->shutdown();
        LOG(INFO) << "signal " << sig << std::endl;
        exit(3);
    });

    std::thread tt([&settings, &key]()-> void {
        Address localudp(settings->local, settings->local_udp, Address::Type::ipv4);
        Address remoteudp(settings->server, settings->udp_port, Address::Type::ipv4);

        auto ui = PrivacyBase::build_privacy_method(settings->methodName);
        if (!ui.has_value()){
              LOG(INFO) << "build privacy failed !!!!" << std::endl;
              return;
        }

        UDPServer server(localudp, remoteudp, ui.value(), key);
        server.Run();
    });

    auto ret = server.run();
}

std::string get_my_path()
{
    char abs_path[1024] = {0};
    int cnt = readlink("/proc/self/exe", abs_path, 1024);
    if (cnt < 0 || cnt >= 1024)
    {
        return NULL;
    }

    for (int i = cnt; i >= 0; --i)
    {
        if (abs_path[i] == '/')
        {
            abs_path[i + 1] = '\0';
            break;
        }
    }

    return abs_path;
}

std::shared_ptr<client> get_local_setting()
{
    auto path = get_my_path();
    path.append("/config.json");

    if (access(path.c_str(), F_OK) != 0)
    {
        return nullptr;
    }

    std::ifstream ifs(path);
    rapidjson::IStreamWrapper isw(ifs);

    rapidjson::Document d;
    d.ParseStream(isw);

    auto c = std::make_shared<client>();
    c->server_port = d["server_port"].GetInt();
    c->server = d["server"].GetString();
    c->local = d["local"].GetString();
    c->udp_port = d["udp_port"].GetInt();
    c->local_port = d["local_port"].GetInt();
    c->local_udp = d["local_udp_port"].GetInt();
    c->network = d["which"].GetString();
    c->pwd = d["password"].GetString();
    c->timeout = d["timeout"].GetInt();
    c->methodName = d["method"].GetString();
    return c;
}
