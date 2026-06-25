/**
 * @file    log.cpp
 * @brief   日志模型实现 — 登录日志、操作日志
 *
 * 提供两类日志的完整支持:
 *   - 管理员/商户登录日志 (AdminLoginLog)
 *   - 超级管理员操作日志 (OperationLog, 包含所有 CUD 操作的审计追踪)
 *
 * 以及用户登录日志 (UserLoginLog), 记录用户端登录时间、IP 等信息。
 */

#include "models/log.hpp"

namespace merchant {
namespace model {

/* ============================================================
   登录日志条目 (LoginLogEntry)
   用于:
     - 商户登录日志 (管理后台登录记录)
     - 用户端登录日志 (会员登录记录)

   注意: LoginLogEntry 结构定义在 service/login_servce.hpp 中。
   本文件仅实现其 JSON 序列化辅助函数。
   ============================================================ */

// 日志类型 → 中文标签映射
const char* logTypeLabel(const std::string& type) {
    if (type == LogType::LOGIN)   return "登录";
    if (type == LogType::CREATE)  return "新增";
    if (type == LogType::UPDATE)  return "修改";
    if (type == LogType::DELETE_) return "删除";
    if (type == LogType::EXPORT)  return "导出";
    if (type == LogType::VOID)    return "作废";
    return "未知";
}

// 日志模块 → 中文标签映射
const char* logModuleLabel(const std::string& module) {
    if (module == LogModule::SYSTEM)      return "系统管理";
    if (module == LogModule::PRODUCTS)    return "商品管理";
    if (module == LogModule::ORDERS)      return "订单管理";
    if (module == LogModule::MEMBERS)     return "会员管理";
    if (module == LogModule::STORES)      return "门店管理";
    if (module == LogModule::MARKETING)   return "营销管理";
    if (module == LogModule::STATISTICS)  return "数据统计";
    if (module == LogModule::USERS)       return "用户管理";
    if (module == LogModule::PERMISSIONS) return "权限管理";
    return "未知模块";
}

} // namespace model
} // namespace merchant
