/**
 * @file    log.hpp
 * @brief   操作日志模型 — 系统审计追踪
 *
 * 对应前端管理员: 操作日志 (Logs)
 *
 * OperationLog 结构体定义在 user.hpp 中。
 * 本文件提供日志类型常量与辅助构建函数。
 */

#pragma once

#include "user.hpp"  // OperationLog, LogQueryParams
#include <chrono>
#include <sstream>
#include <iomanip>

namespace merchant {
namespace model {

/* ============================================================
   操作类型常量
   ============================================================ */
namespace LogType {
    constexpr const char* LOGIN   = "登录";
    constexpr const char* CREATE  = "新增";
    constexpr const char* UPDATE  = "修改";
    constexpr const char* DELETE_ = "删除";
    constexpr const char* EXPORT  = "导出";
    constexpr const char* VOID    = "作废";
}

/* ============================================================
   常用操作模块名称
   ============================================================ */
namespace LogModule {
    constexpr const char* SYSTEM      = "系统管理";
    constexpr const char* PRODUCTS    = "商品管理";
    constexpr const char* ORDERS      = "订单管理";
    constexpr const char* MEMBERS     = "会员管理";
    constexpr const char* STORES      = "门店管理";
    constexpr const char* MARKETING   = "营销管理";
    constexpr const char* STATISTICS  = "数据统计";
    constexpr const char* USERS       = "用户管理";
    constexpr const char* PERMISSIONS = "权限管理";
}

/* ============================================================
   日志构建辅助
   ============================================================ */

/// 获取当前时间字符串 (ISO-8601 格式)
inline std::string nowTimeStr() {
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

/// 快速构建一条操作日志
inline OperationLog makeLog(const std::string& operator_,
                            const std::string& type,
                            const std::string& module,
                            const std::string& description,
                            const std::string& ip) {
    OperationLog log;
    log.operator_   = operator_;
    log.type        = type;
    log.module      = module;
    log.description = description;
    log.ip          = ip;
    log.time        = nowTimeStr();
    return log;
}

} // namespace model
} // namespace merchant
