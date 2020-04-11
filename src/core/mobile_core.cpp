#include "core/mobile_core.h"
#include "core/socks5server.h"
#include "udpserver/udpserver.h"
#include <thread>
#include "core/g.h"
#include "core/func.hpp"
#include "core/peerfactory.h"
#include "core/address.h"

static Socks5Server *g_tcpServer = nullptr;
static UDPServer *g_udpServer = nullptr;
static std::thread *g_tcpRoutine = nullptr;
static std::thread *g_udpRoutine = nullptr;
INITIALIZE_EASYLOGGINGPP
int start_server(client *settings)
{
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

    Address localudp(settings->local, settings->local_udp, Address::Type::ipv4);
    Address remoteudp(settings->server, settings->udp_port, Address::Type::ipv4);
    auto ui = PrivacyBase::build_privacy_method(settings->methodName);
    if (!ui.has_value())
    {
        LOG(INFO) << "build privacy failed !!!!" << std::endl;
        return -1;
    }

    g_tcpServer = new Socks5Server(local, settings->timeout);
    g_udpServer = new UDPServer(localudp, remoteudp, ui.value(), key);

    g_tcpRoutine = new std::thread([]()->void{
        g_tcpServer->run();

    });

    g_udpRoutine = new std::thread([]()->void{
        g_udpServer->Run();

    });

    return 0;
}

int stop_server()
{
    if (g_tcpServer != nullptr) {
        g_tcpServer->shutdown();       
        LOG(INFO) << "g_tcpServer stop"<< std::endl;
    }

    if (g_udpServer != nullptr) {
        g_udpServer->stop_server();
         LOG(INFO) << "g_udpServer stop"<< std::endl;
    }

    

    if (g_tcpServer != nullptr){
        if (g_tcpRoutine != nullptr) {
            g_tcpRoutine->join();
            delete g_tcpRoutine;
            g_tcpRoutine = nullptr;
        }
        delete g_tcpServer;
        g_tcpServer = nullptr;
    }

     LOG(INFO) << "safe delete g_tcpServer"<< std::endl;


     if (g_udpServer != nullptr) {
         if (g_udpRoutine != nullptr) {
             g_udpRoutine->join();
             delete g_udpRoutine;
             g_udpRoutine = nullptr;
         }
         g_udpServer = nullptr;
     }

    LOG(INFO) << "safe delete g_udpServer"<< std::endl;

     return 0;
}

int init_log()
{
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.setGlobally(
        el::ConfigurationType::Format, "%datetime[%thread]%file:%line  %msg");

    //defaultConf.setGlobally(
    //    el::ConfigurationType::Enabled, "false");
    el::Loggers::reconfigureLogger("default", defaultConf);

    return 0;
}
