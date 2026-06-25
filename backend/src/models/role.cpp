/**
 * @file    role.cpp
 * @brief   角色实例工厂 — 创建、初始化角色对象
 *
 * 提供预置角色的工厂方法。
 * 每个方法返回一个完整初始化的 Role 实例, 可直接用于:
 *   - 内存存储 (开发/测试阶段)
 *   - SQL INSERT (生产阶段)
 */

#include "models/role.hpp"
#include <stdexcept>

namespace merchant {
namespace model {

/* ============================================================
   预置角色工厂
   每个工厂方法确保:
     - 角色名称与 RoleNames 常量一致
     - 权限配置符合业务规则
   ============================================================ */

/// 超级管理员 — 全模块全权限
Role createSuperAdminRole() {
    return makeSuperAdminRole();  // 委托 role.hpp 中的 inline 实现
}

/// 运营角色 — 商品/订单/会员/门店/营销/统计可查看和操作
Role createOperatorRole() {
    Role r;
    r.id   = 2;
    r.name = RoleNames::OPERATOR;

    ModulePermissions full{true, true, true, true, true, true};  // view/add/edit/del/ship/void 全开
    ModulePermissions readOnly{true, false, false, false, false, false};
    ModulePermissions orderPerms{true, false, false, false, true, true};  // view + ship + void

    r.permissions[PermissionModules::PRODUCTS]    = {true, true, true, false, false, false};
    r.permissions[PermissionModules::ORDERS]      = {true, false, false, false, true, true};
    r.permissions[PermissionModules::MEMBERS]     = readOnly;
    r.permissions[PermissionModules::STORES]      = readOnly;
    r.permissions[PermissionModules::MARKETING]   = {true, true, true, true, false, false};
    r.permissions[PermissionModules::STATISTICS]  = readOnly;
    r.permissions[PermissionModules::USERS]       = {false, false, false, false, false, false};
    r.permissions[PermissionModules::PERMISSIONS] = {false, false, false, false, false, false};
    r.permissions[PermissionModules::LOGS]        = {false, false, false, false, false, false};

    return r;
}

/// 客服角色 — 仅查看权限
Role createServiceRole() {
    Role r;
    r.id   = 3;
    r.name = RoleNames::SERVICE;

    ModulePermissions viewOnly{true, false, false, false, false, false};

    r.permissions[PermissionModules::PRODUCTS]    = viewOnly;
    r.permissions[PermissionModules::ORDERS]      = viewOnly;
    r.permissions[PermissionModules::MEMBERS]     = viewOnly;
    r.permissions[PermissionModules::STORES]      = viewOnly;
    r.permissions[PermissionModules::MARKETING]   = viewOnly;
    r.permissions[PermissionModules::STATISTICS]  = {false, false, false, false, false, false};
    r.permissions[PermissionModules::USERS]       = {false, false, false, false, false, false};
    r.permissions[PermissionModules::PERMISSIONS] = {false, false, false, false, false, false};
    r.permissions[PermissionModules::LOGS]        = {false, false, false, false, false, false};

    return r;
}

/// 商户角色 — 管理自己的商品/订单/门店/营销/统计
Role createMerchantRole() {
    Role r;
    r.id   = 4;
    r.name = RoleNames::MERCHANT;

    r.permissions[PermissionModules::PRODUCTS]    = {true, true, true, true, false, false};
    r.permissions[PermissionModules::ORDERS]      = {true, false, false, false, true, true};
    r.permissions[PermissionModules::MEMBERS]     = {true, false, false, false, false, false};
    r.permissions[PermissionModules::STORES]      = {true, true, true, true, false, false};
    r.permissions[PermissionModules::MARKETING]   = {true, true, true, true, false, false};
    r.permissions[PermissionModules::STATISTICS]  = {true, false, false, false, false, false};
    r.permissions[PermissionModules::USERS]       = {false, false, false, false, false, false};
    r.permissions[PermissionModules::PERMISSIONS] = {false, false, false, false, false, false};
    r.permissions[PermissionModules::LOGS]        = {false, false, false, false, false, false};

    return r;
}

/// 根据名称创建对应角色
Role createRoleByName(const std::string& name) {
    if (name == RoleNames::SUPER_ADMIN) return createSuperAdminRole();
    if (name == RoleNames::OPERATOR)    return createOperatorRole();
    if (name == RoleNames::SERVICE)     return createServiceRole();
    if (name == RoleNames::MERCHANT)    return createMerchantRole();
    throw std::invalid_argument("Unknown role name: " + name);
}

/* ============================================================
   权限工具
   ============================================================ */

/// 将 ModulePermissions 序列化为 JSON 字符串
std::string toJson(const ModulePermissions& mp) {
    return "{"
           "\"view\":"  + std::string(mp.view  ? "true" : "false") + ","
           "\"add\":"   + std::string(mp.add   ? "true" : "false") + ","
           "\"edit\":"  + std::string(mp.edit  ? "true" : "false") + ","
           "\"del\":"   + std::string(mp.del   ? "true" : "false") + ","
           "\"ship\":"  + std::string(mp.ship  ? "true" : "false") + ","
           "\"void\":"  + std::string(mp.voId  ? "true" : "false") +
           "}";
}

/// 将 Role 序列化为 JSON 字符串 (含权限矩阵)
std::string toJson(const Role& role) {
    std::string json;
    json += "{";
    json += "\"id\":"   + std::to_string(role.id)   + ",";
    json += "\"name\":\"" + role.name + "\",";
    json += "\"permissions\":{";

    bool firstModule = true;
    for (const auto& [module, perms] : role.permissions) {
        if (!firstModule) json += ",";
        json += "\"" + module + "\":" + toJson(perms);
        firstModule = false;
    }

    json += "}}";
    return json;
}

} // namespace model
} // namespace merchant
