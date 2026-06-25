/**
 * @file    role.hpp
 * @brief   角色权限模型 — 权限矩阵定义
 *
 * 对应前端管理员: 权限管理 (Permissions)
 *
 * Role / ModulePermissions 结构体定义在 user.hpp 中。
 * 本文件提供预设角色常量与权限校验辅助函数。
 */

#pragma once

#include "user.hpp"

namespace merchant {
namespace model {

/* ============================================================
   预设角色名称常量
   ============================================================ */
namespace RoleNames {
    constexpr const char* SUPER_ADMIN = "超级管理员";
    constexpr const char* OPERATOR    = "运营";
    constexpr const char* SERVICE     = "客服";
    constexpr const char* MERCHANT    = "商户";
    constexpr const char* MEMBER      = "会员";
}

/* ============================================================
   权限模块名称常量 (用于 permissions map 的 key)
   ============================================================ */
namespace PermissionModules {
    constexpr const char* PRODUCTS    = "products";
    constexpr const char* ORDERS      = "orders";
    constexpr const char* MEMBERS     = "members";
    constexpr const char* STORES      = "stores";
    constexpr const char* MARKETING   = "marketing";
    constexpr const char* STATISTICS  = "statistics";
    constexpr const char* USERS       = "users";
    constexpr const char* PERMISSIONS = "permissions";
    constexpr const char* LOGS        = "logs";
}

/* ============================================================
   权限校验辅助
   ============================================================ */

/// 检查角色是否有某模块的某操作权限
inline bool hasPermission(const Role& role,
                          const std::string& module,
                          const std::string& action) {
    auto it = role.permissions.find(module);
    if (it == role.permissions.end()) return false;

    if (action == "view")  return it->second.view;
    if (action == "add")   return it->second.add;
    if (action == "edit")  return it->second.edit;
    if (action == "del")   return it->second.del;
    if (action == "ship")  return it->second.ship;
    if (action == "void")  return it->second.voId;
    return false;
}

/* ============================================================
   角色工厂函数 (定义在 src/models/role.cpp)
   ============================================================ */

Role createSuperAdminRole();
Role createOperatorRole();
Role createServiceRole();
Role createMerchantRole();

/// 根据名称创建对应角色
Role createRoleByName(const std::string& name);

/* ============================================================
   权限工具
   ============================================================ */

/// 将 ModulePermissions 序列化为 JSON 字符串
std::string toJson(const ModulePermissions& mp);

/// 将 Role 序列化为 JSON 字符串
std::string toJson(const Role& role);

/// 构建超级管理员默认权限 (全模块全权限)
inline Role makeSuperAdminRole() {
    Role r;
    r.id   = 1;
    r.name = RoleNames::SUPER_ADMIN;

    ModulePermissions all{true, true, true, true, true, true};

    r.permissions[PermissionModules::PRODUCTS]    = all;
    r.permissions[PermissionModules::ORDERS]      = all;
    r.permissions[PermissionModules::MEMBERS]     = all;
    r.permissions[PermissionModules::STORES]      = all;
    r.permissions[PermissionModules::MARKETING]   = all;
    r.permissions[PermissionModules::STATISTICS]  = all;
    r.permissions[PermissionModules::USERS]       = all;
    r.permissions[PermissionModules::PERMISSIONS] = all;
    r.permissions[PermissionModules::LOGS]        = all;

    return r;
}

} // namespace model
} // namespace merchant
