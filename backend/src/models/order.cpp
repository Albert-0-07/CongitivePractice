/**
 * @file    order.cpp
 * @brief   订单模型辅助 — JSON 序列化、查询参数解析
 */

#include "models/order.hpp"
#include <sstream>

namespace merchant {
namespace model {

/* ============================================================
   OrderItem → JSON
   ============================================================ */
std::string toJson(const OrderItem& item) {
    std::string json;
    json += "{";
    json += "\"productId\":"  + std::to_string(item.productId)  + ",";
    json += "\"name\":\""     + item.name                      + "\",";
    json += "\"price\":"      + std::to_string(item.price)      + ",";
    json += "\"qty\":"        + std::to_string(item.qty)        + ",";
    json += "\"subtotal\":"   + std::to_string(item.subtotal);
    json += "}";
    return json;
}

/* ============================================================
   OrderTimelineEntry → JSON
   ============================================================ */
std::string toJson(const OrderTimelineEntry& t) {
    std::string json;
    json += "{";
    json += "\"status\":\""  + t.status  + "\",";
    json += "\"time\":\""    + t.time    + "\"";
    json += "}";
    return json;
}

/* ============================================================
   Order → JSON (含嵌套 items 和 timeline)
   ============================================================ */
std::string toJson(const Order& o) {
    std::string json;
    json += "{";
    json += "\"id\":"          + std::to_string(o.id)          + ",";
    json += "\"orderNo\":\""   + o.orderNo                    + "\",";
    json += "\"userId\":"      + std::to_string(o.userId)      + ",";
    json += "\"userName\":\""  + o.userName                   + "\",";
    json += "\"phone\":\""     + o.phone                      + "\",";
    json += "\"address\":\""   + o.address                    + "\",";
    json += "\"productCount\":"+ std::to_string(o.productCount)+ ",";
    json += "\"totalAmount\":" + std::to_string(o.totalAmount) + ",";
    json += "\"status\":\""    + o.status                     + "\",";
    json += "\"payMethod\":\"" + o.payMethod                  + "\",";
    json += "\"payTime\":\""   + o.payTime                    + "\",";
    json += "\"createTime\":\""+ o.createTime                 + "\",";
    json += "\"trackingNo\":\""+ o.trackingNo                 + "\",";
    json += "\"courier\":\""   + o.courier                    + "\",";

    // items array
    json += "\"items\":[";
    for (size_t i = 0; i < o.items.size(); ++i) {
        json += toJson(o.items[i]);
        if (i + 1 < o.items.size()) json += ",";
    }
    json += "],";

    // timeline array
    json += "\"timeline\":[";
    for (size_t i = 0; i < o.timeline.size(); ++i) {
        json += toJson(o.timeline[i]);
        if (i + 1 < o.timeline.size()) json += ",";
    }
    json += "]";

    json += "}";
    return json;
}

/* ============================================================
   OrderQueryParams parsing
   ============================================================ */
OrderQueryParams parseOrderQuery(const std::string& qs) {
    OrderQueryParams p;

    auto findParam = [&](const std::string& key) -> std::string {
        auto pos = qs.find(key + "=");
        if (pos == std::string::npos) return "";
        pos += key.size() + 1;
        auto end = qs.find("&", pos);
        return qs.substr(pos, end == std::string::npos ? end : end - pos);
    };

    std::string orderNo = findParam("orderNo");
    if (!orderNo.empty()) p.orderNo = orderNo;

    std::string status = findParam("status");
    if (!status.empty()) p.status = status;

    std::string userName = findParam("userName");
    if (!userName.empty()) p.userName = userName;

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

} // namespace model
} // namespace merchant
