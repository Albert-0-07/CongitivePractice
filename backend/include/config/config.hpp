/**
 * @file    config.hpp
 * @brief   应用配置 — 数据库连接、服务器参数、商户相关配置
 *
 * 全局配置单例, 从 config.json / 环境变量 / 启动参数加载。
 * 本文件定义配置结构, 具体加载逻辑见 src/config/ 实现。
 */

#pragma once

#include <string>
#include <cstdint>
#include <memory>

namespace merchant {
namespace config {

/**
 * @brief 数据库连接配置
 */
struct DatabaseConfig {
    std::string host     = "127.0.0.1";
    uint16_t    port     = 3306;
    std::string user     = "root";
    std::string password = "";
    std::string dbName   = "merchant_system";
    std::string charset  = "utf8mb4";
    int32_t     poolSize = 10;             ///< 连接池大小
    int32_t     timeout  = 30;             ///< 连接超时 (秒)

    DatabaseConfig() = default;
};

/**
 * @brief HTTP 服务器配置
 */
struct ServerConfig {
    std::string host  = "0.0.0.0";
    uint16_t    port  = 8080;
    int32_t     threads = 4;               ///< 工作线程数
    std::string staticPath = "../../frontend"; ///< 前端静态文件路径 (从 build/ 运行)

    ServerConfig() = default;
};

/**
 * @brief 商户系统业务配置
 */
struct MerchantConfig {
    int32_t     defaultPageSize   = 10;    ///< 默认每页条数
    int32_t     maxPageSize       = 100;   ///< 最大每页条数
    int32_t     orderAutoVoidMinutes = 30; ///< 未支付订单自动取消时间 (分钟)
    std::string uploadPath        = "./uploads"; ///< 商品图片上传路径
    std::string defaultPassword   = "123456";    ///< 新建用户默认密码

    MerchantConfig() = default;
};

/**
 * @brief 全局应用配置
 */
class AppConfig {
public:
    static AppConfig& instance();

    DatabaseConfig   database;
    ServerConfig     server;
    MerchantConfig   merchant;

    /// 从 JSON 文件加载配置
    bool loadFromFile(const std::string& filePath);

    /// 从命令行参数覆盖
    void applyCommandLine(int argc, char* argv[]);

    /// 校验配置有效性
    bool validate(std::string& errorMsg) const;

private:
    AppConfig() = default;
    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;
};

} // namespace config
} // namespace merchant
