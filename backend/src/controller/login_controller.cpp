/**
 * @file    login_controller.cpp
 * @brief   LoginController 实现 — 登录/登出/日志查询
 *
 * POST /api/login              — 管理员/商户/会员登录
 * POST /api/logout             — 登出
 * GET  /api/check-token        — Token 校验
 * GET  /api/admin/login-logs   — 管理端登录日志
 * GET  /api/admin/user-logs    — 用户端登录日志
 * GET  /api/admin/operation-logs — 操作日志
 */

#include "controller/login_controller.hpp"
#include "models/log.hpp"

#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace merchant {
namespace controller {

/* ============================================================
   JSON 辅助
   ============================================================ */
static std::string jsonEscape(const std::string& s) {
    std::string out; out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;
        }
    }
    return out;
}
static std::string jsonString(const std::string& s) { return "\"" + jsonEscape(s) + "\""; }
static std::string jsonNumber(int64_t n) { return std::to_string(n); }
static std::string jsonNumber(int32_t n) { return std::to_string(n); }
static std::string jsonBool(bool b) { return b ? "true" : "false"; }

static std::string jsonObject(const std::vector<std::pair<std::string, std::string>>& pairs) {
    std::string out = "{";
    for (size_t i = 0; i < pairs.size(); ++i) {
        out += jsonString(pairs[i].first) + ":" + pairs[i].second;
        if (i + 1 < pairs.size()) out += ",";
    }
    return out + "}";
}

static std::string jsonError(const std::string& msg, int code) {
    return jsonObject({{"error","true"},{"message",jsonString(msg)},{"code",jsonNumber(code)}});
}

static std::string jsonPageResult(const std::string& arrayJson, int64_t total, int32_t page, int32_t pageSize) {
    return jsonObject({{"data",arrayJson},{"total",jsonNumber(total)},
                       {"page",jsonNumber(page)},{"pageSize",jsonNumber(pageSize)},{"error","false"}});
}

static std::unordered_map<std::string, std::string> parseQuery(const std::string& qs) {
    std::unordered_map<std::string, std::string> m;
    size_t pos = 0;
    while (pos < qs.size()) {
        auto eq = qs.find('=', pos);
        auto amp = qs.find('&', pos);
        if (eq == std::string::npos || eq >= amp) break;
        std::string key = qs.substr(pos, eq - pos);
        std::string val = qs.substr(eq + 1, (amp == std::string::npos ? qs.size() : amp) - eq - 1);
        m[key] = val;
        pos = (amp == std::string::npos) ? qs.size() : amp + 1;
    }
    return m;
}

static std::string extractStr(const std::string& json, const std::string& key) {
    auto pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos);
    if (pos == std::string::npos) return "";
    pos = json.find('"', pos);
    if (pos == std::string::npos) return "";
    auto end = json.find('"', pos + 1);
    if (end == std::string::npos) return "";
    return json.substr(pos + 1, end - pos - 1);
}

/* ============================================================
   Constructor
   ============================================================ */
LoginController::LoginController(LoginServicePtr service) : m_service(std::move(service)) {}

LoginController::Response LoginController::success(const std::string& jsonBody) {
    return "{\"error\":false,\"data\":" + jsonBody + "}";
}
LoginController::Response LoginController::error(const std::string& msg, int code) { return jsonError(msg, code); }
LoginController::Response LoginController::pageResult(const std::string& jsonArray, int64_t total, int32_t page, int32_t pageSize) {
    return jsonPageResult(jsonArray, total, page, pageSize);
}

/* ============================================================
   serialize LoginLogEntry
   ============================================================ */
static std::string serialize(const LoginLogEntry& e) {
    return jsonObject({
        {"id", jsonNumber(e.id)}, {"username", jsonString(e.username)},
        {"realName", jsonString(e.realName)}, {"role", jsonString(e.role)},
        {"loginTime", jsonString(e.loginTime)}, {"ip", jsonString(e.ip)},
        {"userAgent", jsonString(e.userAgent)}, {"success", jsonBool(e.success)},
        {"failReason", jsonString(e.failReason)}
    });
}

/* ============================================================
   POST /api/login
   ============================================================ */
LoginController::Response LoginController::login(const Request& body) {
    try {
        std::string username  = extractStr(body, "username");
        std::string password  = extractStr(body, "password");
        std::string ip        = extractStr(body, "ip");
        std::string userAgent = extractStr(body, "userAgent");
        if (username.empty() || password.empty()) return jsonError("用户名和密码不能为空", 400);
        if (ip.empty()) ip = "127.0.0.1";

        auto result = m_service->login(username, password, ip, userAgent);
        if (!result.success) return jsonError(result.errorMessage, 401);

        std::string userJson = jsonObject({
            {"id", jsonNumber(result.user.id)}, {"username", jsonString(result.user.username)},
            {"realName", jsonString(result.user.realName)}, {"role", jsonString(result.user.role)},
            {"avatar", jsonString(result.user.avatar)}, {"phone", jsonString(result.user.phone)},
            {"email", jsonString(result.user.email)}, {"status", jsonNumber(result.user.status)}
        });
        return jsonObject({{"error","false"},{"token",jsonString(result.token)},{"user",userJson}});
    } catch (const std::exception& e) { return jsonError(e.what(), 500); }
}

/* ============================================================
   POST /api/logout
   ============================================================ */
LoginController::Response LoginController::logout(const Request& body) {
    try {
        std::string token = extractStr(body, "token");
        if (!token.empty()) m_service->logout(token);
        return "{\"error\":false,\"message\":\"已登出\"}";
    } catch (const std::exception& e) { return jsonError(e.what(), 500); }
}

/* ============================================================
   GET /api/check-token
   ============================================================ */
LoginController::Response LoginController::checkToken(const Request& queryParams) {
    try {
        auto params = parseQuery(queryParams);
        auto it = params.find("token");
        if (it == params.end()) return jsonError("缺少token参数", 400);
        auto userPtr = m_service->validateToken(it->second);
        if (!userPtr) return "{\"error\":false,\"valid\":false}";
        std::string userJson = jsonObject({
            {"id",jsonNumber(userPtr->id)},{"username",jsonString(userPtr->username)},
            {"realName",jsonString(userPtr->realName)},{"role",jsonString(userPtr->role)}
        });
        return "{\"error\":false,\"valid\":true,\"user\":" + userJson + "}";
    } catch (const std::exception& e) { return jsonError(e.what(), 500); }
}

/* ============================================================
   GET /api/admin/login-logs
   ============================================================ */
LoginController::Response LoginController::getAdminLoginLogs(const Request& queryParams) {
    try {
        auto params = parseQuery(queryParams);
        LogQueryParams q;
        auto it = params.find("operator"); if (it != params.end()) q.operator_ = it->second;
        it = params.find("page"); if (it != params.end()) q.page = std::stoi(it->second);
        it = params.find("pageSize"); if (it != params.end()) q.pageSize = std::stoi(it->second);
        it = params.find("startDate"); if (it != params.end()) q.startDate = it->second;
        it = params.find("endDate"); if (it != params.end()) q.endDate = it->second;

        auto result = m_service->getAdminLoginLogs(q);
        std::string arr = "[";
        for (size_t i = 0; i < result.list.size(); ++i) {
            if (i > 0) arr += ",";
            arr += serialize(result.list[i]);
        }
        arr += "]";
        return jsonPageResult(arr, result.total, result.page, result.pageSize);
    } catch (const std::exception& e) { return jsonError(e.what(), 500); }
}

/* ============================================================
   GET /api/admin/user-logs
   ============================================================ */
LoginController::Response LoginController::getUserLoginLogs(const Request& queryParams) {
    try {
        auto params = parseQuery(queryParams);
        LogQueryParams q;
        auto it = params.find("keyword"); if (it != params.end()) q.operator_ = it->second;
        it = params.find("page"); if (it != params.end()) q.page = std::stoi(it->second);
        it = params.find("pageSize"); if (it != params.end()) q.pageSize = std::stoi(it->second);

        auto result = m_service->getUserLoginLogs(q);
        std::string arr = "[";
        for (size_t i = 0; i < result.list.size(); ++i) {
            if (i > 0) arr += ",";
            arr += serialize(result.list[i]);
        }
        arr += "]";
        return jsonPageResult(arr, result.total, result.page, result.pageSize);
    } catch (const std::exception& e) { return jsonError(e.what(), 500); }
}

/* ============================================================
   GET /api/admin/operation-logs (via LoginService)
   ============================================================ */
LoginController::Response LoginController::getOperationLogs(const Request& queryParams) {
    try {
        auto params = parseQuery(queryParams);
        LogQueryParams q;
        auto it = params.find("operator"); if (it != params.end()) q.operator_ = it->second;
        it = params.find("type"); if (it != params.end()) q.type = it->second;
        it = params.find("page"); if (it != params.end()) q.page = std::stoi(it->second);
        it = params.find("pageSize"); if (it != params.end()) q.pageSize = std::stoi(it->second);

        auto result = m_service->getOperationLogs(q);
        std::string arr = "[";
        for (size_t i = 0; i < result.list.size(); ++i) {
            auto& l = result.list[i];
            if (i > 0) arr += ",";
            arr += jsonObject({{"id",jsonNumber(l.id)},{"operator",jsonString(l.operator_)},
                              {"type",jsonString(l.type)},{"module",jsonString(l.module)},
                              {"description",jsonString(l.description)},{"ip",jsonString(l.ip)},
                              {"time",jsonString(l.time)}});
        }
        arr += "]";
        return jsonPageResult(arr, result.total, result.page, result.pageSize);
    } catch (const std::exception& e) { return jsonError(e.what(), 500); }
}

} // namespace controller
} // namespace merchant
