/**
 * @file    product.cpp
 * @brief   商品模型辅助 — JSON 序列化
 */

#include "models/product.hpp"

namespace merchant {
namespace model {

/* ============================================================
   ProductCategory → JSON
   ============================================================ */
std::string toJson(const ProductCategory& c) {
    std::string json;
    json += "{";
    json += "\"id\":"          + std::to_string(c.id)         + ",";
    json += "\"name\":\""      + c.name                      + "\",";
    json += "\"description\":\"" + c.description             + "\",";
    json += "\"sort\":"        + std::to_string(c.sort)       + ",";
    json += "\"status\":"      + std::to_string(c.status)     + ",";
    json += "\"createdAt\":\"" + c.createdAt                 + "\"";
    json += "}";
    return json;
}

/* ============================================================
   Product → JSON
   ============================================================ */
std::string toJson(const Product& p) {
    std::string json;
    json += "{";
    json += "\"id\":"          + std::to_string(p.id)         + ",";
    json += "\"name\":\""      + p.name                      + "\",";
    json += "\"categoryId\":"  + std::to_string(p.categoryId) + ",";
    json += "\"image\":\""     + p.image                     + "\",";
    json += "\"description\":\"" + p.description             + "\",";
    json += "\"price\":"       + std::to_string(p.price)      + ",";
    json += "\"stock\":"       + std::to_string(p.stock)      + ",";
    json += "\"unit\":\""      + p.unit                      + "\",";
    json += "\"status\":"      + std::to_string(p.status)     + ",";
    json += "\"createdAt\":\"" + p.createdAt                 + "\"";
    json += "}";
    return json;
}

/* ============================================================
   Promotion → JSON
   ============================================================ */
std::string toJson(const Promotion& promo) {
    std::string json;
    json += "{";
    json += "\"id\":"          + std::to_string(promo.id)          + ",";
    json += "\"name\":\""      + promo.name                       + "\",";
    json += "\"type\":\""      + promo.type                       + "\",";
    json += "\"startTime\":\"" + promo.startTime                  + "\",";
    json += "\"endTime\":\""   + promo.endTime                    + "\",";
    json += "\"discount\":"    + std::to_string(promo.discount)    + ",";
    json += "\"description\":\"" + promo.description              + "\",";
    json += "\"status\":\""    + promo.status                     + "\",";

    // productIds array
    json += "\"productIds\":[";
    for (size_t i = 0; i < promo.productIds.size(); ++i) {
        json += std::to_string(promo.productIds[i]);
        if (i + 1 < promo.productIds.size()) json += ",";
    }
    json += "]";

    // conditional fields
    if (!promo.fullDiscountCondition.empty())
        json += ",\"fullDiscountCondition\":\"" + promo.fullDiscountCondition + "\"";
    if (!promo.giftDescription.empty())
        json += ",\"giftDescription\":\"" + promo.giftDescription + "\"";
    if (!promo.couponDescription.empty())
        json += ",\"couponDescription\":\"" + promo.couponDescription + "\"";

    json += "}";
    return json;
}

/* ============================================================
   StoreProduct → JSON
   ============================================================ */
std::string toJson(const StoreProduct& sp) {
    std::string json;
    json += "{";
    json += "\"id\":"        + std::to_string(sp.id)        + ",";
    json += "\"storeId\":"   + std::to_string(sp.storeId)   + ",";
    json += "\"productId\":" + std::to_string(sp.productId) + ",";
    json += "\"stock\":"     + std::to_string(sp.stock)     + ",";
    json += "\"status\":"    + std::to_string(sp.status)    + ",";
    json += "\"bindTime\":\"" + sp.bindTime                + "\"";
    json += "}";
    return json;
}

/* ============================================================
   SalesRankItem → JSON
   ============================================================ */
std::string toJson(const SalesRankItem& s) {
    std::string json;
    json += "{";
    json += "\"id\":"          + std::to_string(s.id)          + ",";
    json += "\"name\":\""      + s.name                       + "\",";
    json += "\"categoryId\":"  + std::to_string(s.categoryId)  + ",";
    json += "\"image\":\""     + s.image                      + "\",";
    json += "\"salesCount\":"  + std::to_string(s.salesCount)  + ",";
    json += "\"salesAmount\":" + std::to_string(s.salesAmount) + ",";
    json += "\"avgPrice\":"    + std::to_string(s.avgPrice);
    json += "}";
    return json;
}

/* ============================================================
   VisitRankItem → JSON
   ============================================================ */
std::string toJson(const VisitRankItem& v) {
    std::string json;
    json += "{";
    json += "\"id\":"             + std::to_string(v.id)             + ",";
    json += "\"name\":\""         + v.name                          + "\",";
    json += "\"categoryId\":"     + std::to_string(v.categoryId)     + ",";
    json += "\"image\":\""        + v.image                         + "\",";
    json += "\"visitCount\":"     + std::to_string(v.visitCount)     + ",";
    json += "\"uniqueVisitors\":" + std::to_string(v.uniqueVisitors) + ",";
    json += "\"conversionRate\":\"" + v.conversionRate              + "\"";
    json += "}";
    return json;
}

/* ============================================================
   ProductQueryParams → URL query string
   ============================================================ */
ProductQueryParams parseProductQuery(const std::string& qs) {
    ProductQueryParams p;
    // Simple key=value parsing
    auto findParam = [&](const std::string& key) -> std::string {
        auto pos = qs.find(key + "=");
        if (pos == std::string::npos) return "";
        pos += key.size() + 1;
        auto end = qs.find("&", pos);
        return qs.substr(pos, end == std::string::npos ? end : end - pos);
    };

    std::string name = findParam("name");
    if (!name.empty()) p.name = name;

    std::string catId = findParam("categoryId");
    if (!catId.empty()) p.categoryId = std::stoll(catId);

    std::string st = findParam("status");
    if (!st.empty()) p.status = std::stoi(st);

    std::string pg = findParam("page");
    if (!pg.empty()) p.page = std::stoi(pg);

    std::string ps = findParam("pageSize");
    if (!ps.empty()) p.pageSize = std::stoi(ps);

    return p;
}

} // namespace model
} // namespace merchant
