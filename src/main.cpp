
#include <glog/logging.h>
#include "common/g.h"
#include <string>
#include <vector>
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <iostream>
#include "config/config.h"
#include "client/clientfactory.h"
#include "privacy/privacy.h"
#include "common/func.hpp"
#include "common/socks5_uv.h"

std::string get_my_path();
client get_local_setting();

int main(int argc, char *argv[])
{

    UNREFERENCED_PARAMETER(argc);
    google::InitGoogleLogging(argv[0]);
    google::SetStderrLogging(google::GLOG_INFO);

    auto settings = get_local_setting();
    std::string whereru = "WhereRU";
    std::vector<unsigned char> msg(whereru.begin(), whereru.end());
    std::vector<unsigned char> keyW(settings.pwd.begin(), settings.pwd.end());
    Address remote(settings.server, settings.server_port, Address::Type::ipv4);
    Address local(settings.local, settings.local_port, Address::Type::ipv4);
    LOG(INFO) << "S address:" << remote.toString() << std::endl;
    LOG(INFO) << "---------------------" << std::endl;
    LOG(INFO) << "L address:" << local.toString() << std::endl;

    auto key = make_sha1(keyW).value();
    auto hash = make_hmac(key, msg).value();
    LOG(INFO) << "key:" << ToHexEX(key.begin(), key.end()) << std::endl;
    LOG(INFO) << "hash:" << ToHexEX(hash.begin(), hash.end()) << std::endl;

    auto client = ClientFactory::get_instance();
    client.set_key(key);
    client.set_profile(hash, hash);
    client.set_remote(remote);

    Socks5_uv server;
    auto ret = server.launch(local);
    LOG(INFO) << "server initialization result: " << ret << std::endl;
    server.run();

    signal(SIGINT, [](int sig) -> void {
        LOG(INFO) << "signal " << sig << std::endl;
        exit(3);
    });
    server.run();
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

client get_local_setting()
{
    auto path = get_my_path();
    path.append("/config.json");

    std::ifstream ifs(path);
    rapidjson::IStreamWrapper isw(ifs);

    rapidjson::Document d;
    d.ParseStream(isw);

    client c;
    c.server_port = d["server_port"].GetInt();
    c.server = d["server"].GetString();
    c.local = d["local"].GetString();
    c.local_port = d["local_port"].GetInt();
    c.network = d["which"].GetString();
    c.pwd = d["password"].GetString();
    return c;
}
