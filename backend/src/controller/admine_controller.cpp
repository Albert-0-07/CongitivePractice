/**
 * @file    admine_controller.cpp
 * @brief   AdminController 实现 — 用户管理/角色管理/操作日志
 */

#include "controller/admine_controller.hpp"

namespace merchant {
namespace controller {

/* ============================================================
   JSON helpers (same pattern as other controllers)
   ============================================================ */
static std::string jsonEscape(const std::string& s) {
    std::string out; out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break; case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break; case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break; default: out += c;
        }
    }
    return out;
}
static std::string jstr(const std::string& s) { return "\"" + jsonEscape(s) + "\""; }
static std::string jnum(int64_t n)  { return std::to_string(n); }
static std::string jnum32(int32_t n) { return std::to_string(n); }
static std::string jbool(bool b) { return b ? "true" : "false"; }

static std::string jobj(const std::vector<std::pair<std::string, std::string>>& p) {
    std::string o = "{";
    for (size_t i = 0; i < p.size(); ++i) {
        o += jstr(p[i].first) + ":" + p[i].second;
        if (i + 1 < p.size()) o += ",";
    }
    return o + "}";
}

static std::string jerr(const std::string& m, int c) {
    return jobj({{"error","true"},{"message",jstr(m)},{"code",jnum(c)}});
}

static std::string jpage(const std::string& arr, int64_t t, int32_t pg, int32_t ps) {
    return jobj({{"data",arr},{"total",jnum(t)},{"page",jnum32(pg)},{"pageSize",jnum32(ps)},{"error","false"}});
}

static std::unordered_map<std::string,std::string> parseQs(const std::string& qs) {
    std::unordered_map<std::string,std::string> m;
    size_t p = 0;
    while (p < qs.size()) {
        auto eq = qs.find('=',p); auto am = qs.find('&',p);
        if (eq == std::string::npos || eq >= am) break;
        m[qs.substr(p,eq-p)] = qs.substr(eq+1, (am==std::string::npos?qs.size():am)-eq-1);
        p = (am == std::string::npos) ? qs.size() : am+1;
    }
    return m;
}

static std::string extr(const std::string& j, const std::string& k) {
    auto p = j.find("\""+k+"\""); if (p==std::string::npos) return "";
    p = j.find(':',p); if (p==std::string::npos) return "";
    p = j.find('"',p); if (p==std::string::npos) return "";
    auto e = j.find('"',p+1); if (e==std::string::npos) return "";
    return j.substr(p+1, e-p-1);
}

static int32_t extrI32(const std::string& j, const std::string& k) {
    auto v = extr(j,k);
    return v.empty() ? 0 : std::stoi(v);
}

static std::string opName(const std::string& b) {
    auto v = extr(b,"operatorName"); return v.empty()?"system":v;
}
static std::string opIp(const std::string& b) {
    auto v = extr(b,"operatorIp"); return v.empty()?"127.0.0.1":v;
}

/* ============================================================
   serialize User
   ============================================================ */
static std::string serUser(const merchant::model::User& u) {
    std::string addr = "[";
    for (size_t i=0;i<u.addresses.size();++i) {
        if(i>0)addr+=","; addr+=jstr(u.addresses[i]);
    }
    addr+="]";
    return jobj({{"id",jnum(u.id)},{"username",jstr(u.username)},{"realName",jstr(u.realName)},
        {"role",jstr(u.role)},{"phone",jstr(u.phone)},{"email",jstr(u.email)},
        {"gender",jstr(u.gender)},{"avatar",jstr(u.avatar)},{"status",jnum32(u.status)},
        {"createdAt",jstr(u.createdAt)},{"lastLogin",jstr(u.lastLogin)},
        {"addresses",addr},{"orderCount",jnum32(u.orderCount)},
        {"totalSpent",std::to_string(u.totalSpent)},
        {"lastLoginIp",jstr(u.lastLoginIp)},{"loginCount",jnum32(u.loginCount)}});
}

/* ============================================================
   serialize Role
   ============================================================ */
static std::string serRole(const merchant::model::Role& r) {
    std::string perms = "{"; bool first=true;
    for (auto& [mod,mp] : r.permissions) {
        if(!first) perms+=",";
        perms+=jstr(mod)+":"+jobj({{"view",jbool(mp.view)},{"add",jbool(mp.add)},
            {"edit",jbool(mp.edit)},{"del",jbool(mp.del)},{"ship",jbool(mp.ship)},{"void",jbool(mp.voId)}});
        first=false;
    }
    perms+="}";
    return jobj({{"id",jnum(r.id)},{"name",jstr(r.name)},{"permissions",perms}});
}

/* ============================================================
   Constructor
   ============================================================ */
AdminController::AdminController(AdminServicePtr svc) : m_service(std::move(svc)) {}

AdminController::Response AdminController::success(const std::string& j) { return "{\"error\":false,\"data\":"+j+"}"; }
AdminController::Response AdminController::error(const std::string& m, int c) { return jerr(m,c); }
AdminController::Response AdminController::pageResult(const std::string& a, int64_t t, int32_t pg, int32_t ps) { return jpage(a,t,pg,ps); }

/* ============================================================
   GET /api/admin/users
   ============================================================ */
AdminController::Response AdminController::getUsers(const Request& q) {
    try {
        auto ps = parseQs(q); UserQueryParams p;
        auto it=ps.find("username"); if(it!=ps.end())p.username=it->second;
        it=ps.find("role"); if(it!=ps.end())p.role=it->second;
        it=ps.find("status"); if(it!=ps.end())p.status=std::stoi(it->second);
        it=ps.find("page"); if(it!=ps.end())p.page=std::stoi(it->second);
        it=ps.find("pageSize"); if(it!=ps.end())p.pageSize=std::stoi(it->second);
        auto r=m_service->getUsers(p);
        std::string arr="["; for(size_t i=0;i<r.list.size();++i){if(i>0)arr+=",";arr+=serUser(r.list[i]);} arr+="]";
        return jpage(arr,r.total,r.page,r.pageSize);
    }catch(const std::exception& e){return jerr(e.what(),500);}
}

/* ============================================================
   GET /api/admin/users/:id
   ============================================================ */
AdminController::Response AdminController::getUserById(int64_t id) {
    try { return success(serUser(m_service->getUserById(id))); }
    catch(const std::exception& e){return jerr(e.what(),404);}
}

/* ============================================================
   POST /api/admin/users
   ============================================================ */
AdminController::Response AdminController::createUser(const Request& b) {
    try {
        merchant::model::User d;
        d.username=extr(b,"username"); d.realName=extr(b,"realName");
        d.role=extr(b,"role"); d.phone=extr(b,"phone"); d.email=extr(b,"email");
        d.gender=extr(b,"gender"); int32_t st=extrI32(b,"status"); d.status=st?st:1;
        return success(serUser(m_service->createUser(d,opName(b),opIp(b))));
    }catch(const std::exception& e){return jerr(e.what(),400);}
}

/* ============================================================
   PUT /api/admin/users/:id
   ============================================================ */
AdminController::Response AdminController::updateUser(int64_t id, const Request& b) {
    try {
        merchant::model::User d;
        d.username=extr(b,"username"); d.realName=extr(b,"realName");
        d.role=extr(b,"role"); d.phone=extr(b,"phone"); d.email=extr(b,"email");
        d.status=extrI32(b,"status");
        return success(serUser(m_service->updateUser(id,d,opName(b),opIp(b))));
    }catch(const std::exception& e){return jerr(e.what(),400);}
}

/* ============================================================
   DELETE /api/admin/users/:id
   ============================================================ */
AdminController::Response AdminController::deleteUser(int64_t id, const Request& b) {
    try { m_service->deleteUser(id,opName(b),opIp(b)); return "{\"error\":false,\"message\":\"用户已删除\"}"; }
    catch(const std::exception& e){return jerr(e.what(),400);}
}

/* ============================================================
   POST /api/admin/users/:id/reset-pwd
   ============================================================ */
AdminController::Response AdminController::resetUserPassword(int64_t id, const Request& b) {
    try {
        auto pwd=m_service->resetUserPassword(id,opName(b),opIp(b));
        return success(jobj({{"message",jstr("密码已重置为: "+pwd)},{"defaultPassword",jstr(pwd)}}));
    }catch(const std::exception& e){return jerr(e.what(),400);}
}

/* ============================================================
   GET /api/admin/roles
   ============================================================ */
AdminController::Response AdminController::getRoles() {
    try {
        auto roles=m_service->getRoles();
        std::string arr="["; for(size_t i=0;i<roles.size();++i){if(i>0)arr+=",";arr+=serRole(roles[i]);} arr+="]";
        return "{\"error\":false,\"data\":"+arr+"}";
    }catch(const std::exception& e){return jerr(e.what(),500);}
}

/* ============================================================
   POST /api/admin/roles
   ============================================================ */
AdminController::Response AdminController::createRole(const Request& b) {
    try {
        std::string name=extr(b,"name");
        std::unordered_map<std::string,merchant::model::ModulePermissions> perms;
        auto r=m_service->createRole(name,perms,opName(b),opIp(b));
        return success(serRole(r));
    }catch(const std::exception& e){return jerr(e.what(),400);}
}

/* ============================================================
   PUT /api/admin/roles/:id
   ============================================================ */
AdminController::Response AdminController::updateRolePermissions(int64_t id, const Request& b) {
    try {
        std::unordered_map<std::string,merchant::model::ModulePermissions> perms;
        auto r=m_service->updateRolePermissions(id,perms,opName(b),opIp(b));
        return success(serRole(r));
    }catch(const std::exception& e){return jerr(e.what(),400);}
}

/* ============================================================
   DELETE /api/admin/roles/:id
   ============================================================ */
AdminController::Response AdminController::deleteRole(int64_t id, const Request& b) {
    try { m_service->deleteRole(id,opName(b),opIp(b)); return "{\"error\":false,\"message\":\"角色已删除\"}"; }
    catch(const std::exception& e){return jerr(e.what(),400);}
}

/* ============================================================
   GET /api/admin/operation-logs
   ============================================================ */
AdminController::Response AdminController::getOperationLogs(const Request& qs) {
    try {
        auto ps=parseQs(qs); LogQueryParams p;
        auto it=ps.find("operator"); if(it!=ps.end())p.operator_=it->second;
        it=ps.find("type"); if(it!=ps.end())p.type=it->second;
        it=ps.find("startDate"); if(it!=ps.end())p.startDate=it->second;
        it=ps.find("endDate"); if(it!=ps.end())p.endDate=it->second;
        it=ps.find("page"); if(it!=ps.end())p.page=std::stoi(it->second);
        it=ps.find("pageSize"); if(it!=ps.end())p.pageSize=std::stoi(it->second);
        auto r=m_service->getOperationLogs(p);
        std::string arr="["; for(size_t i=0;i<r.list.size();++i){
            auto& l=r.list[i]; if(i>0)arr+=",";
            arr+=jobj({{"id",jnum(l.id)},{"operator",jstr(l.operator_)},{"type",jstr(l.type)},
                {"module",jstr(l.module)},{"description",jstr(l.description)},
                {"ip",jstr(l.ip)},{"time",jstr(l.time)},
                {"userAgent",jstr(l.userAgent)},{"requestParams",jstr(l.requestParams)}});
        } arr+="]";
        return jpage(arr,r.total,r.page,r.pageSize);
    }catch(const std::exception& e){return jerr(e.what(),500);}
}

/* ============================================================
   GET /api/admin/operation-logs/:id
   ============================================================ */
AdminController::Response AdminController::getOperationLogDetail(int64_t id) {
    try {
        auto l=m_service->getOperationLogDetail(id);
        return success(jobj({{"id",jnum(l.id)},{"operator",jstr(l.operator_)},{"type",jstr(l.type)},
            {"module",jstr(l.module)},{"description",jstr(l.description)},
            {"ip",jstr(l.ip)},{"time",jstr(l.time)},
            {"userAgent",jstr(l.userAgent)},{"requestParams",jstr(l.requestParams)}}));
    }catch(const std::exception& e){return jerr(e.what(),404);}
}

} // namespace controller
} // namespace merchant
