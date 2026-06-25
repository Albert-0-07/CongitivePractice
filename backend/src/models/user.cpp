/**
 * @file    user.cpp
 * @brief   用户/会员模型辅助 — JSON 序列化、查询参数解析
 */

#include "models/user.hpp"

namespace merchant {
namespace model {

/* ============================================================
   User (Member) → JSON
   ============================================================ */
std::string toJson(const User& u) {
    std::string json;
    json += "{";
    json += "\"id\":"         + std::to_string(u.id)         + ",";
    json += "\"username\":\"" + u.username                  + "\",";
    json += "\"realName\":\"" + u.realName                  + "\",";
    json += "\"gender\":\""   + u.gender                    + "\",";
    json += "\"phone\":\""    + u.phone                     + "\",";
    json += "\"email\":\""    + u.email                     + "\",";
    json += "\"avatar\":\""   + u.avatar                    + "\",";
    json += "\"role\":\""     + u.role                      + "\",";
    json += "\"status\":"     + std::to_string(u.status)     + ",";
    json += "\"createdAt\":\""+ u.createdAt                 + "\",";
    json += "\"lastLogin\":\""+ u.lastLogin                 + "\",";

    // addresses array
    json += "\"addresses\":[";
    for (size_t i = 0; i < u.addresses.size(); ++i) {
        json += "\"" + u.addresses[i] + "\"";
        if (i + 1 < u.addresses.size()) json += ",";
    }
    json += "],";

    json += "\"orderCount\":" + std::to_string(u.orderCount) + ",";
    json += "\"totalSpent\":" + std::to_string(u.totalSpent);

    // admin extension fields (only when non-empty)
    if (!u.lastLoginIp.empty())
        json += ",\"lastLoginIp\":\"" + u.lastLoginIp + "\"";
    if (u.loginCount > 0)
        json += ",\"loginCount\":" + std::to_string(u.loginCount);

    json += "}";
    return json;
}

/* ============================================================
   User → JSON (without password — for admin user management)
   ============================================================ */
std::string toJsonUserAdmin(const User& u) {
    std::string json;
    json += "{";
    json += "\"id\":"         + std::to_string(u.id)         + ",";
    json += "\"username\":\"" + u.username                  + "\",";
    json += "\"realName\":\"" + u.realName                  + "\",";
    json += "\"role\":\""     + u.role                      + "\",";
    json += "\"phone\":\""    + u.phone                     + "\",";
    json += "\"email\":\""    + u.email                     + "\",";
    json += "\"status\":"     + std::to_string(u.status)     + ",";
    json += "\"createdAt\":\""+ u.createdAt                 + "\",";
    json += "\"lastLoginIp\":\"" + u.lastLoginIp            + "\",";
    json += "\"loginCount\":" + std::to_string(u.loginCount);
    json += "}";
    return json;
}

/* ============================================================
   MemberQueryParams parsing
   ============================================================ */
MemberQueryParams parseMemberQuery(const std::string& qs) {
    MemberQueryParams p;

    auto findParam = [&](const std::string& key) -> std::string {
        auto pos = qs.find(key + "=");
        if (pos == std::string::npos) return "";
        pos += key.size() + 1;
        auto end = qs.find("&", pos);
        return qs.substr(pos, end == std::string::npos ? end : end - pos);
    };

    std::string kw = findParam("keyword");
    if (!kw.empty()) p.keyword = kw;

    std::string startDate = findParam("startDate");
    if (!startDate.empty()) p.startDate = startDate;

    std::string endDate = findParam("endDate");
    if (!endDate.empty()) p.endDate = endDate;

    std::string pg = findParam("page");
    if (!pg.empty()) p.page = std::stoi(pg);

    std::string ps = findParam("pageSize");
    if (!ps.empty()) p.pageSize = std::stoi(ps);

    return p;
}

/* ============================================================
   OperationLog → JSON
   ============================================================ */
std::string toJson(const OperationLog& log) {
    std::string json;
    json += "{";
    json += "\"id\":"          + std::to_string(log.id)      + ",";
    json += "\"operator\":\""  + log.operator_               + "\",";
    json += "\"type\":\""      + log.type                    + "\",";
    json += "\"module\":\""    + log.module                  + "\",";
    json += "\"description\":\""+ log.description            + "\",";
    json += "\"ip\":\""        + log.ip                      + "\",";
    json += "\"time\":\""      + log.time                    + "\"";
    if (!log.userAgent.empty())
        json += ",\"userAgent\":\"" + log.userAgent + "\"";
    if (!log.requestParams.empty())
        json += ",\"requestParams\":\"" + log.requestParams + "\"";
    json += "}";
    return json;
}

} // namespace model
} // namespace merchant
