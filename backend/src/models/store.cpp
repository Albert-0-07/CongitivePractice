/**
 * @file    store.cpp
 * @brief   门店/区域模型辅助 — JSON 序列化
 */

#include "models/store.hpp"

namespace merchant {
namespace model {

/* ============================================================
   Store → JSON
   ============================================================ */
std::string toJson(const Store& s) {
    std::string json;
    json += "{";
    json += "\"id\":"          + std::to_string(s.id)          + ",";
    json += "\"name\":\""      + s.name                       + "\",";
    json += "\"areaId\":"      + std::to_string(s.areaId)      + ",";
    json += "\"contact\":\""   + s.contact                    + "\",";
    json += "\"phone\":\""     + s.phone                      + "\",";
    json += "\"openTime\":\""  + s.openTime                   + "\",";
    json += "\"closeTime\":\"" + s.closeTime                  + "\",";
    json += "\"address\":\""   + s.address                    + "\",";
    json += "\"image\":\""     + s.image                      + "\",";
    json += "\"description\":\"" + s.description              + "\",";
    json += "\"status\":"      + std::to_string(s.status)      + ",";
    json += "\"createdAt\":\"" + s.createdAt                  + "\"";
    json += "}";
    return json;
}

/* ============================================================
   ServiceArea → JSON
   ============================================================ */
std::string toJson(const ServiceArea& a) {
    std::string json;
    json += "{";
    json += "\"id\":"         + std::to_string(a.id)         + ",";
    json += "\"name\":\""     + a.name                      + "\",";
    json += "\"code\":\""     + a.code                      + "\",";
    json += "\"parentId\":"   + std::to_string(a.parentId)   + ",";
    json += "\"sort\":"       + std::to_string(a.sort)       + ",";
    json += "\"remark\":\""   + a.remark                    + "\",";
    json += "\"createdAt\":\""+ a.createdAt                 + "\"";
    json += "}";
    return json;
}

} // namespace model
} // namespace merchant
