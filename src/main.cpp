

#include "core/g.h"
#include <string>
#include <vector>
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <fstream>
#include <iostream>
#include "config/config.h"
#include <signal.h>
#include <unistd.h>
#include <memory>
#include <signal.h>
#include <thread>
#include "core/mobile_core.h"
#include <iostream>

std::string get_my_path();
std::shared_ptr<client> get_local_setting();
INITIALIZE_EASYLOGGINGPP
void myCrashHandler(int sig) {
    LOG(ERROR) << "Woops! Crashed!";     
    // FOLLOWING LINE IS ABSOLUTELY NEEDED AT THE END IN ORDER TO ABORT APPLICATION
    el::Helpers::crashAbort(sig);
}

int main(int argc, char *argv[])
{
    el::Helpers::setCrashHandler(myCrashHandler); 
    el::Configurations defaultConf;
    defaultConf.setToDefault();
    defaultConf.parseFromText("*GLOBAL:\n ENABLED = false \n TO_FILE =  false \n FORMAT=%datetime{%d/%M} [%thread] %file:%line %msg");
    el::Loggers::reconfigureLogger("default", defaultConf);
  
    UNREFERENCED_PARAMETER(argc);
    auto settings = get_local_setting();
    if (!settings)
    {
        LOG(ERROR) << "config file is not exist!!!" << std::endl;
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);
    start_server(settings.get());

    signal(SIGINT, [](int sig) -> void {
        std::cout << "will exit, please wait!!!!" <<std::endl;
        signal(SIGINT, SIG_IGN);
        stop_server();
        LOG(INFO) << "signal " << sig << std::endl;
        exit(3);
    });

    while (true)
    {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }
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
