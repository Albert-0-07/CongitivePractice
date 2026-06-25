/**
 * @file    app_config.cpp
 * @brief   AppConfig 实现 — 全局配置单例
 *
 * 加载顺序: 默认值 → config.json → 环境变量 → 命令行参数
 */

#include "config/config.hpp"

#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>

namespace merchant {
namespace config {

/* ============================================================
   单例
   ============================================================ */
AppConfig& AppConfig::instance() {
    static AppConfig s_instance;
    return s_instance;
}

/* ============================================================
   从 JSON 文件加载配置
   简易解析: 仅提取顶层 key-value 字符串/数字
   ============================================================ */
bool AppConfig::loadFromFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "[WARN] 无法打开配置文件: " << filePath << std::endl;
        return false;
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();

    // 简易 JSON 字符串值提取
    auto extractStr = [&](const std::string& key) -> std::string {
        auto pos = content.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        pos = content.find(':', pos);
        if (pos == std::string::npos) return "";
        pos = content.find('"', pos);
        if (pos == std::string::npos) return "";
        auto end = content.find('"', pos + 1);
        if (end == std::string::npos) return "";
        return content.substr(pos + 1, end - pos - 1);
    };

    auto extractInt = [&](const std::string& key) -> int {
        auto pos = content.find("\"" + key + "\"");
        if (pos == std::string::npos) return -1;
        pos = content.find(':', pos);
        if (pos == std::string::npos) return -1;
        std::string num;
        for (size_t i = pos + 1; i < content.size() && (isdigit(content[i]) || content[i] == '-'); ++i)
            num += content[i];
        return num.empty() ? -1 : std::stoi(num);
    };

    // database
    std::string dbHost = extractStr("dbHost");
    if (!dbHost.empty()) database.host = dbHost;
    int dbPort = extractInt("dbPort");
    if (dbPort > 0) database.port = static_cast<uint16_t>(dbPort);
    std::string dbUser = extractStr("dbUser");
    if (!dbUser.empty()) database.user = dbUser;
    std::string dbPass = extractStr("dbPassword");
    if (!dbPass.empty()) database.password = dbPass;
    std::string dbName = extractStr("dbName");
    if (!dbName.empty()) database.dbName = dbName;
    int dbPool = extractInt("dbPoolSize");
    if (dbPool > 0) database.poolSize = dbPool;

    // server
    std::string svHost = extractStr("host");
    if (!svHost.empty()) server.host = svHost;
    int svPort = extractInt("port");
    if (svPort > 0) server.port = static_cast<uint16_t>(svPort);
    int svThreads = extractInt("threads");
    if (svThreads > 0) server.threads = svThreads;
    std::string svStatic = extractStr("staticPath");
    if (!svStatic.empty()) server.staticPath = svStatic;

    // merchant
    int mcPageSize = extractInt("defaultPageSize");
    if (mcPageSize > 0) merchant.defaultPageSize = mcPageSize;
    int mcMaxPage = extractInt("maxPageSize");
    if (mcMaxPage > 0) merchant.maxPageSize = mcMaxPage;
    int mcAutoVoid = extractInt("orderAutoVoidMinutes");
    if (mcAutoVoid > 0) merchant.orderAutoVoidMinutes = mcAutoVoid;
    std::string mcUpload = extractStr("uploadPath");
    if (!mcUpload.empty()) merchant.uploadPath = mcUpload;
    std::string mcDefaultPwd = extractStr("defaultPassword");
    if (!mcDefaultPwd.empty()) merchant.defaultPassword = mcDefaultPwd;

    return true;
}

/* ============================================================
   命令行参数覆盖
   支持的参数:
     --port N        服务端口
     --host HOST     绑定地址
     --static PATH   静态文件路径
     --db-host HOST  数据库地址
     --db-port N     数据库端口
     --db-user USER  数据库用户
     --db-pass PASS  数据库密码
     --db-name NAME  数据库名称
   ============================================================ */
void AppConfig::applyCommandLine(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        auto nextVal = [&]() -> std::string {
            if (i + 1 < argc && argv[i + 1][0] != '-') return argv[++i];
            return "";
        };

        if (arg == "--port") {
            std::string v = nextVal();
            if (!v.empty()) server.port = static_cast<uint16_t>(std::stoi(v));
        } else if (arg == "--host") {
            std::string v = nextVal();
            if (!v.empty()) server.host = v;
        } else if (arg == "--static") {
            std::string v = nextVal();
            if (!v.empty()) server.staticPath = v;
        } else if (arg == "--threads") {
            std::string v = nextVal();
            if (!v.empty()) server.threads = std::stoi(v);
        } else if (arg == "--db-host") {
            std::string v = nextVal();
            if (!v.empty()) database.host = v;
        } else if (arg == "--db-port") {
            std::string v = nextVal();
            if (!v.empty()) database.port = static_cast<uint16_t>(std::stoi(v));
        } else if (arg == "--db-user") {
            std::string v = nextVal();
            if (!v.empty()) database.user = v;
        } else if (arg == "--db-pass") {
            std::string v = nextVal();
            if (!v.empty()) database.password = v;
        } else if (arg == "--db-name") {
            std::string v = nextVal();
            if (!v.empty()) database.dbName = v;
        }
    }
}

/* ============================================================
   校验配置有效性
   ============================================================ */
bool AppConfig::validate(std::string& errorMsg) const {
    if (server.port == 0 || server.port > 65535) {
        errorMsg = "无效的服务端口: " + std::to_string(server.port);
        return false;
    }
    if (server.threads < 1 || server.threads > 64) {
        errorMsg = "线程数不合理: " + std::to_string(server.threads);
        return false;
    }
    if (database.poolSize < 1 || database.poolSize > 100) {
        errorMsg = "数据库连接池大小不合理: " + std::to_string(database.poolSize);
        return false;
    }
    if (database.timeout < 1 || database.timeout > 300) {
        errorMsg = "数据库连接超时不合理: " + std::to_string(database.timeout);
        return false;
    }
    return true;
}

} // namespace config
} // namespace merchant
