/**
 * @file    merchant_controller.cpp
 * @brief   MerchantController 具体实现
 *
 * 每个 handler 方法:
 *   1. 解析 HTTP 请求参数 (query string 或 JSON body)
 *   2. 调用 MerchantService 对应方法
 *   3. 构造 JSON HTTP 响应
 *   4. 统一 try/catch 错误处理
 */

#include "controller/merchant.hpp"
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <cmath>

namespace merchant {
namespace controller {

using namespace merchant::service;
using namespace merchant::model;

/* ==========================================================
   JSON 构建辅助 (无外部库依赖)
   ========================================================== */

static std::string jsonEscape(const std::string& str) {
    std::string out;
    out.reserve(str.size());
    for (size_t i = 0; i < str.size(); ++i) {
        char c = str[i];
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

static std::string jsonString(const std::string& str) {
    return "\"" + jsonEscape(str) + "\"";
}

static std::string jsonNumber(int64_t n) {
    return std::to_string(n);
}

static std::string jsonNumber(int32_t n) {
    return std::to_string(n);
}

static std::string jsonNumber(double n) {
    if (std::isnan(n) || std::isinf(n)) {
        return "0.0";
    }
    std::ostringstream oss;
    oss << n;
    std::string s = oss.str();
    return s;
}

static std::string jsonBool(bool b) {
    return b ? "true" : "false";
}

/// 将一组已序列化的 json 值拼成数组: items 间用 "," 分隔
static std::string jsonArray_raw(const std::string& joinedItems) {
    return "[" + joinedItems + "]";
}

/// 数组: vector<string> -> "[v0,v1,...]"
static std::string jsonArray(const std::vector<std::string>& elements) {
    std::string out;
    for (size_t i = 0; i < elements.size(); ++i) {
        out += elements[i];
        if (i + 1 < elements.size()) out += ",";
    }
    return "[" + out + "]";
}

/// 对象: pairs = { {"key1", "val1"}, {"key2", "val2"}, ... }
/// 注意: val 必须是已经序列化好的 JSON 片段 (如 jsonString("..."), jsonNumber(...) 的产物)
static std::string jsonObject(const std::vector<std::pair<std::string, std::string>>& pairs) {
    std::string out = "{";
    for (size_t i = 0; i < pairs.size(); ++i) {
        out += jsonString(pairs[i].first) + ":" + pairs[i].second;
        if (i + 1 < pairs.size()) out += ",";
    }
    out += "}";
    return out;
}

/// 错误响应
static std::string jsonError(const std::string& message, int code) {
    return jsonObject({
        {"error",   jsonBool(true)},
        {"message", jsonString(message)},
        {"code",    jsonNumber(code)}
    });
}

/// 分页响应
static std::string jsonPageResult(const std::string& arrayJson,
                                  int64_t total, int32_t page, int32_t pageSize) {
    return jsonObject({
        {"data",     arrayJson},
        {"total",    jsonNumber(total)},
        {"page",     jsonNumber(page)},
        {"pageSize", jsonNumber(pageSize)},
        {"error",    jsonBool(false)}
    });
}

/* ==========================================================
   Query-String 解析器
   ========================================================== */

static std::unordered_map<std::string, std::string> parseQueryString(const std::string& qs) {
    std::unordered_map<std::string, std::string> result;
    if (qs.empty()) return result;

    std::istringstream stream(qs);
    std::string pair;
    while (std::getline(stream, pair, '&')) {
        if (pair.empty()) continue;
        auto eqPos = pair.find('=');
        if (eqPos == std::string::npos) {
            result[pair] = "";
        } else {
            std::string key = pair.substr(0, eqPos);
            std::string val = pair.substr(eqPos + 1);
            result[key] = val;
        }
    }
    return result;
}

/* ==========================================================
   简单的 JSON body 键值提取 (只支持一层)
   从 JSON 字符串中提取 "key":"value" 或 "key":数字
   ========================================================== */

static std::string extractJsonString(const std::string& json, const std::string& key) {
    std::string pattern = "\"" + key + "\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return "";

    pos = json.find(':', pos + pattern.size());
    if (pos == std::string::npos) return "";

    // 跳过空白
    ++pos;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r'))
        ++pos;

    if (pos >= json.size()) return "";

    if (json[pos] == '"') {
        ++pos;
        std::string val;
        while (pos < json.size()) {
            if (json[pos] == '\\') {
                ++pos;
                if (pos < json.size()) {
                    switch (json[pos]) {
                        case '"':  val += '"';  break;
                        case '\\': val += '\\'; break;
                        case 'n':  val += '\n'; break;
                        case 'r':  val += '\r'; break;
                        case 't':  val += '\t'; break;
                        default:   val += json[pos]; break;
                    }
                }
                ++pos;
                continue;
            }
            if (json[pos] == '"') break;
            val += json[pos];
            ++pos;
        }
        return val;
    }
    // 非字符串值 (数字、布尔): 读到 , 或 } 或 ]
    std::string val;
    while (pos < json.size() && json[pos] != ',' && json[pos] != '}' && json[pos] != ']' && json[pos] != ' ' && json[pos] != '\t' && json[pos] != '\n' && json[pos] != '\r') {
        val += json[pos];
        ++pos;
    }
    return val;
}

static int64_t extractJsonInt64(const std::string& json, const std::string& key) {
    std::string v = extractJsonString(json, key);
    if (v.empty()) return 0;
    try { return std::stoll(v); } catch (...) { return 0; }
}

static int32_t extractJsonInt32(const std::string& json, const std::string& key) {
    std::string v = extractJsonString(json, key);
    if (v.empty()) return 0;
    try { return std::stoi(v); } catch (...) { return 0; }
}

static double extractJsonDouble(const std::string& json, const std::string& key) {
    std::string v = extractJsonString(json, key);
    if (v.empty()) return 0.0;
    try { return std::stod(v); } catch (...) { return 0.0; }
}

/// 从 JSON 中提取数组 (如 "[1,2,3]") 并解析为 vector<int64_t>
static std::vector<int64_t> extractJsonIntArray(const std::string& json, const std::string& key) {
    std::vector<int64_t> result;
    std::string pattern = "\"" + key + "\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return result;

    pos = json.find('[', pos + pattern.size());
    if (pos == std::string::npos) return result;

    std::string num;
    for (size_t i = pos + 1; i < json.size(); ++i) {
        char c = json[i];
        if (c == ']') {
            if (!num.empty()) {
                try { result.push_back(std::stoll(num)); } catch (...) {}
                num.clear();
            }
            break;
        }
        if (c == ',') {
            if (!num.empty()) {
                try { result.push_back(std::stoll(num)); } catch (...) {}
                num.clear();
            }
            continue;
        }
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') continue;
        num += c;
    }
    return result;
}

/* ==========================================================
   Success / Error / PageResult (静态成员)
   ========================================================== */

MerchantController::Response MerchantController::success(const std::string& jsonBody) {
    return jsonBody;
}

MerchantController::Response MerchantController::error(const std::string& message, int code) {
    return jsonError(message, code);
}

MerchantController::Response MerchantController::pageResult(const std::string& jsonArray,
                                        int64_t total, int32_t page, int32_t pageSize) {
    return jsonPageResult(jsonArray, total, page, pageSize);
}

/* ==========================================================
   构造函数
   ========================================================== */

MerchantController::MerchantController(MerchantServicePtr service)
    : m_service(std::move(service)) {
}

/* ==========================================================
   模型 -> JSON 序列化辅助
   ========================================================== */

/* ---- ProductCategory ---- */
static std::string serializeCategory(const model::ProductCategory& c) {
    return jsonObject({
        {"id",          jsonNumber(c.id)},
        {"name",        jsonString(c.name)},
        {"description", jsonString(c.description)},
        {"sort",        jsonNumber(c.sort)},
        {"status",      jsonNumber(c.status)},
        {"createdAt",   jsonString(c.createdAt)}
    });
}

/* ---- Product ---- */
static std::string serializeProduct(const model::Product& p) {
    return jsonObject({
        {"id",          jsonNumber(p.id)},
        {"name",        jsonString(p.name)},
        {"categoryId",  jsonNumber(p.categoryId)},
        {"image",       jsonString(p.image)},
        {"description", jsonString(p.description)},
        {"price",       jsonNumber(p.price)},
        {"stock",       jsonNumber(p.stock)},
        {"unit",        jsonString(p.unit)},
        {"status",      jsonNumber(p.status)},
        {"createdAt",   jsonString(p.createdAt)}
    });
}

/* ---- Promotion ---- */
static std::string serializePromotion(const model::Promotion& p) {
    // serialize productIds array
    std::string ids;
    for (size_t i = 0; i < p.productIds.size(); ++i) {
        ids += jsonNumber(p.productIds[i]);
        if (i + 1 < p.productIds.size()) ids += ",";
    }
    return jsonObject({
        {"id",                   jsonNumber(p.id)},
        {"name",                 jsonString(p.name)},
        {"type",                 jsonString(p.type)},
        {"startTime",            jsonString(p.startTime)},
        {"endTime",              jsonString(p.endTime)},
        {"discount",             jsonNumber(p.discount)},
        {"description",          jsonString(p.description)},
        {"status",               jsonString(p.status)},
        {"productIds",           jsonArray_raw(ids)},
        {"fullDiscountCondition",jsonString(p.fullDiscountCondition)},
        {"giftDescription",      jsonString(p.giftDescription)},
        {"couponDescription",    jsonString(p.couponDescription)}
    });
}

/* ---- Store ---- */
static std::string serializeStore(const model::Store& s) {
    return jsonObject({
        {"id",          jsonNumber(s.id)},
        {"name",        jsonString(s.name)},
        {"areaId",      jsonNumber(s.areaId)},
        {"contact",     jsonString(s.contact)},
        {"phone",       jsonString(s.phone)},
        {"openTime",    jsonString(s.openTime)},
        {"closeTime",   jsonString(s.closeTime)},
        {"address",     jsonString(s.address)},
        {"image",       jsonString(s.image)},
        {"description", jsonString(s.description)},
        {"status",      jsonNumber(s.status)},
        {"createdAt",   jsonString(s.createdAt)}
    });
}

/* ---- ServiceArea ---- */
static std::string serializeServiceArea(const model::ServiceArea& a) {
    return jsonObject({
        {"id",        jsonNumber(a.id)},
        {"name",      jsonString(a.name)},
        {"code",      jsonString(a.code)},
        {"parentId",  jsonNumber(a.parentId)},
        {"sort",      jsonNumber(a.sort)},
        {"remark",    jsonString(a.remark)},
        {"createdAt", jsonString(a.createdAt)}
    });
}

/* ---- StoreProduct ---- */
static std::string serializeStoreProduct(const model::StoreProduct& sp) {
    return jsonObject({
        {"id",        jsonNumber(sp.id)},
        {"storeId",   jsonNumber(sp.storeId)},
        {"productId", jsonNumber(sp.productId)},
        {"stock",     jsonNumber(sp.stock)},
        {"status",    jsonNumber(sp.status)},
        {"bindTime",  jsonString(sp.bindTime)}
    });
}

/* ---- OrderItem ---- */
static std::string serializeOrderItem(const model::OrderItem& oi) {
    return jsonObject({
        {"productId", jsonNumber(oi.productId)},
        {"name",      jsonString(oi.name)},
        {"price",     jsonNumber(oi.price)},
        {"qty",       jsonNumber(oi.qty)},
        {"subtotal",  jsonNumber(oi.subtotal)}
    });
}

/* ---- OrderTimelineEntry ---- */
static std::string serializeTimelineEntry(const model::OrderTimelineEntry& te) {
    return jsonObject({
        {"status", jsonString(te.status)},
        {"time",   jsonString(te.time)}
    });
}

/* ---- Order ---- */
static std::string serializeOrder(const model::Order& o) {
    std::string items;
    for (size_t i = 0; i < o.items.size(); ++i) {
        items += serializeOrderItem(o.items[i]);
        if (i + 1 < o.items.size()) items += ",";
    }

    std::string tl;
    for (size_t i = 0; i < o.timeline.size(); ++i) {
        tl += serializeTimelineEntry(o.timeline[i]);
        if (i + 1 < o.timeline.size()) tl += ",";
    }

    return jsonObject({
        {"id",           jsonNumber(o.id)},
        {"orderNo",      jsonString(o.orderNo)},
        {"userId",       jsonNumber(o.userId)},
        {"userName",     jsonString(o.userName)},
        {"phone",        jsonString(o.phone)},
        {"address",      jsonString(o.address)},
        {"productCount", jsonNumber(o.productCount)},
        {"totalAmount",  jsonNumber(o.totalAmount)},
        {"status",       jsonString(o.status)},
        {"payMethod",    jsonString(o.payMethod)},
        {"payTime",      jsonString(o.payTime)},
        {"createTime",   jsonString(o.createTime)},
        {"trackingNo",   jsonString(o.trackingNo)},
        {"courier",      jsonString(o.courier)},
        {"items",        jsonArray_raw(items)},
        {"timeline",     jsonArray_raw(tl)}
    });
}

/* ---- SalesRankItem ---- */
static std::string serializeSalesRankItem(const model::SalesRankItem& s) {
    return jsonObject({
        {"id",          jsonNumber(s.id)},
        {"name",        jsonString(s.name)},
        {"categoryId",  jsonNumber(s.categoryId)},
        {"image",       jsonString(s.image)},
        {"salesCount",  jsonNumber(s.salesCount)},
        {"salesAmount", jsonNumber(s.salesAmount)},
        {"avgPrice",    jsonNumber(s.avgPrice)}
    });
}

/* ---- VisitRankItem ---- */
static std::string serializeVisitRankItem(const model::VisitRankItem& v) {
    return jsonObject({
        {"id",              jsonNumber(v.id)},
        {"name",            jsonString(v.name)},
        {"categoryId",      jsonNumber(v.categoryId)},
        {"image",           jsonString(v.image)},
        {"visitCount",      jsonNumber(v.visitCount)},
        {"uniqueVisitors",  jsonNumber(v.uniqueVisitors)},
        {"conversionRate",  jsonString(v.conversionRate)}
    });
}

/* ---- User / Member ---- */
static std::string serializeUser(const model::User& u) {
    std::string addrs;
    for (size_t i = 0; i < u.addresses.size(); ++i) {
        addrs += jsonString(u.addresses[i]);
        if (i + 1 < u.addresses.size()) addrs += ",";
    }

    return jsonObject({
        {"id",         jsonNumber(u.id)},
        {"username",   jsonString(u.username)},
        {"realName",   jsonString(u.realName)},
        {"gender",     jsonString(u.gender)},
        {"phone",      jsonString(u.phone)},
        {"email",      jsonString(u.email)},
        {"avatar",     jsonString(u.avatar)},
        {"role",       jsonString(u.role)},
        {"status",     jsonNumber(u.status)},
        {"createdAt",  jsonString(u.createdAt)},
        {"lastLogin",  jsonString(u.lastLogin)},
        {"addresses",  jsonArray_raw(addrs)},
        {"orderCount", jsonNumber(u.orderCount)},
        {"totalSpent", jsonNumber(u.totalSpent)}
    });
}

/* ==========================================================
   辅助: 将 vector<T> 序列化为 JSON 数组字符串
   ========================================================== */

template <typename T>
static std::string serializeArray(const std::vector<T>& vec,
                                  std::string (*serializer)(const T&)) {
    std::string out;
    for (size_t i = 0; i < vec.size(); ++i) {
        out += serializer(vec[i]);
        if (i + 1 < vec.size()) out += ",";
    }
    return jsonArray_raw(out);
}


/* ==================================================================
   1-4. 商品分类 (Product Categories)
   ================================================================== */

MerchantController::Response MerchantController::getCategories() {
    try {
        auto categories = m_service->getCategories();
        std::string data = serializeArray(categories, serializeCategory);
        return jsonObject({
            {"data",  data},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::createCategory(const MerchantController::Request& body) {
    try {
        model::ProductCategory data;
        data.name        = extractJsonString(body, "name");
        data.description = extractJsonString(body, "description");
        data.sort        = extractJsonInt32(body, "sort");
        data.status      = extractJsonInt32(body, "status");

        auto result = m_service->createCategory(data);
        return jsonObject({
            {"data",  serializeCategory(result)},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::updateCategory(int64_t id, const Request& body) {
    try {
        model::ProductCategory data;
        data.name        = extractJsonString(body, "name");
        data.description = extractJsonString(body, "description");
        data.sort        = extractJsonInt32(body, "sort");
        data.status      = extractJsonInt32(body, "status");

        auto result = m_service->updateCategory(id, data);
        return jsonObject({
            {"data",  serializeCategory(result)},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::deleteCategory(int64_t id) {
    try {
        m_service->deleteCategory(id);
        return jsonObject({
            {"data",  jsonString("ok")},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}


/* ==================================================================
   5-8. 商品信息 (Products)
   ================================================================== */

MerchantController::Response MerchantController::getProducts(const MerchantController::Request& queryParams) {
    try {
        auto params = parseQueryString(queryParams);
        model::ProductQueryParams q;

        auto it = params.find("name");
        if (it != params.end() && !it->second.empty()) q.name = it->second;

        it = params.find("categoryId");
        if (it != params.end() && !it->second.empty())
            q.categoryId = std::stoll(it->second);

        it = params.find("status");
        if (it != params.end() && !it->second.empty())
            q.status = std::stoi(it->second);

        it = params.find("page");
        if (it != params.end() && !it->second.empty())
            q.page = std::stoi(it->second);

        it = params.find("pageSize");
        if (it != params.end() && !it->second.empty())
            q.pageSize = std::stoi(it->second);

        auto result = m_service->getProducts(q);
        std::string data = serializeArray(result.list, serializeProduct);
        return jsonPageResult(data, result.total, result.page, result.pageSize);
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::createProduct(const MerchantController::Request& body) {
    try {
        model::Product data;
        data.name        = extractJsonString(body, "name");
        data.categoryId  = extractJsonInt64(body, "categoryId");
        data.image       = extractJsonString(body, "image");
        data.description = extractJsonString(body, "description");
        data.price       = extractJsonDouble(body, "price");
        data.stock       = extractJsonInt32(body, "stock");
        data.unit        = extractJsonString(body, "unit");
        data.status      = extractJsonInt32(body, "status");

        auto result = m_service->createProduct(data);
        return jsonObject({
            {"data",  serializeProduct(result)},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::updateProduct(int64_t id, const Request& body) {
    try {
        model::Product data;
        data.name        = extractJsonString(body, "name");
        data.categoryId  = extractJsonInt64(body, "categoryId");
        data.image       = extractJsonString(body, "image");
        data.description = extractJsonString(body, "description");
        data.price       = extractJsonDouble(body, "price");
        data.stock       = extractJsonInt32(body, "stock");
        data.unit        = extractJsonString(body, "unit");
        data.status      = extractJsonInt32(body, "status");

        auto result = m_service->updateProduct(id, data);
        return jsonObject({
            {"data",  serializeProduct(result)},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::deleteProduct(int64_t id) {
    try {
        m_service->deleteProduct(id);
        return jsonObject({
            {"data",  jsonString("ok")},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}


/* ==================================================================
   9-13. 促销管理 (Promotions)
   ================================================================== */

MerchantController::Response MerchantController::getPromotions() {
    try {
        auto promotions = m_service->getPromotions();
        std::string data = serializeArray(promotions, serializePromotion);
        return jsonObject({
            {"data",  data},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::createPromotion(const MerchantController::Request& body) {
    try {
        model::Promotion data;
        data.name                 = extractJsonString(body, "name");
        data.type                 = extractJsonString(body, "type");
        data.startTime            = extractJsonString(body, "startTime");
        data.endTime              = extractJsonString(body, "endTime");
        data.discount             = extractJsonDouble(body, "discount");
        data.description          = extractJsonString(body, "description");
        data.status               = extractJsonString(body, "status");
        data.productIds           = extractJsonIntArray(body, "productIds");
        data.fullDiscountCondition= extractJsonString(body, "fullDiscountCondition");
        data.giftDescription      = extractJsonString(body, "giftDescription");
        data.couponDescription    = extractJsonString(body, "couponDescription");

        auto result = m_service->createPromotion(data);
        return jsonObject({
            {"data",  serializePromotion(result)},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::updatePromotion(int64_t id, const Request& body) {
    try {
        model::Promotion data;
        data.name                 = extractJsonString(body, "name");
        data.type                 = extractJsonString(body, "type");
        data.startTime            = extractJsonString(body, "startTime");
        data.endTime              = extractJsonString(body, "endTime");
        data.discount             = extractJsonDouble(body, "discount");
        data.description          = extractJsonString(body, "description");
        data.status               = extractJsonString(body, "status");
        data.productIds           = extractJsonIntArray(body, "productIds");
        data.fullDiscountCondition= extractJsonString(body, "fullDiscountCondition");
        data.giftDescription      = extractJsonString(body, "giftDescription");
        data.couponDescription    = extractJsonString(body, "couponDescription");

        auto result = m_service->updatePromotion(id, data);
        return jsonObject({
            {"data",  serializePromotion(result)},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::deletePromotion(int64_t id) {
    try {
        m_service->deletePromotion(id);
        return jsonObject({
            {"data",  jsonString("ok")},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::bindProductsToPromotion(int64_t id, const Request& body) {
    try {
        auto productIds = extractJsonIntArray(body, "productIds");
        m_service->bindProductsToPromotion(id, productIds);
        return jsonObject({
            {"data",  jsonString("ok")},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}


/* ==================================================================
   14-17. 门店管理 (Stores)
   ================================================================== */

MerchantController::Response MerchantController::getStores() {
    try {
        auto stores = m_service->getStores();
        std::string data = serializeArray(stores, serializeStore);
        return jsonObject({
            {"data",  data},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::createStore(const MerchantController::Request& body) {
    try {
        model::Store data;
        data.name        = extractJsonString(body, "name");
        data.areaId      = extractJsonInt64(body, "areaId");
        data.contact     = extractJsonString(body, "contact");
        data.phone       = extractJsonString(body, "phone");
        data.openTime    = extractJsonString(body, "openTime");
        data.closeTime   = extractJsonString(body, "closeTime");
        data.address     = extractJsonString(body, "address");
        data.image       = extractJsonString(body, "image");
        data.description = extractJsonString(body, "description");
        data.status      = extractJsonInt32(body, "status");

        auto result = m_service->createStore(data);
        return jsonObject({
            {"data",  serializeStore(result)},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::updateStore(int64_t id, const Request& body) {
    try {
        model::Store data;
        data.name        = extractJsonString(body, "name");
        data.areaId      = extractJsonInt64(body, "areaId");
        data.contact     = extractJsonString(body, "contact");
        data.phone       = extractJsonString(body, "phone");
        data.openTime    = extractJsonString(body, "openTime");
        data.closeTime   = extractJsonString(body, "closeTime");
        data.address     = extractJsonString(body, "address");
        data.image       = extractJsonString(body, "image");
        data.description = extractJsonString(body, "description");
        data.status      = extractJsonInt32(body, "status");

        auto result = m_service->updateStore(id, data);
        return jsonObject({
            {"data",  serializeStore(result)},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::deleteStore(int64_t id) {
    try {
        m_service->deleteStore(id);
        return jsonObject({
            {"data",  jsonString("ok")},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}


/* ==================================================================
   18-21. 服务区域管理 (Service Areas)
   ================================================================== */

MerchantController::Response MerchantController::getServiceAreas() {
    try {
        auto areas = m_service->getServiceAreas();
        std::string data = serializeArray(areas, serializeServiceArea);
        return jsonObject({
            {"data",  data},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::createServiceArea(const MerchantController::Request& body) {
    try {
        model::ServiceArea data;
        data.name   = extractJsonString(body, "name");
        data.code   = extractJsonString(body, "code");
        data.parentId = extractJsonInt64(body, "parentId");
        data.sort   = extractJsonInt32(body, "sort");
        data.remark = extractJsonString(body, "remark");

        auto result = m_service->createServiceArea(data);
        return jsonObject({
            {"data",  serializeServiceArea(result)},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::updateServiceArea(int64_t id, const Request& body) {
    try {
        model::ServiceArea data;
        data.name   = extractJsonString(body, "name");
        data.code   = extractJsonString(body, "code");
        data.parentId = extractJsonInt64(body, "parentId");
        data.sort   = extractJsonInt32(body, "sort");
        data.remark = extractJsonString(body, "remark");

        auto result = m_service->updateServiceArea(id, data);
        return jsonObject({
            {"data",  serializeServiceArea(result)},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::deleteServiceArea(int64_t id) {
    try {
        m_service->deleteServiceArea(id);
        return jsonObject({
            {"data",  jsonString("ok")},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}


/* ==================================================================
   22-26. 注册商品管理 (Store-Product Binding)
   ================================================================== */

MerchantController::Response MerchantController::getStoreProducts(int64_t storeId) {
    try {
        auto products = m_service->getStoreProducts(storeId);
        std::string data = serializeArray(products, serializeStoreProduct);
        return jsonObject({
            {"data",  data},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::bindProductsToStore(const MerchantController::Request& body) {
    try {
        int64_t storeId = extractJsonInt64(body, "storeId");

        std::vector<model::StoreProduct> bindings;

        auto bindPos = body.find("\"bindings\"");
        if (bindPos != std::string::npos) {
            auto arrStart = body.find('[', bindPos);
            if (arrStart != std::string::npos) {
                size_t i = arrStart + 1;
                while (i < body.size() && body[i] != ']') {
                    if (body[i] == '{') {
                        size_t objEnd = body.find('}', i);
                        if (objEnd == std::string::npos) break;
                        std::string objStr = body.substr(i, objEnd - i + 1);

                        model::StoreProduct sp;
                        sp.productId = extractJsonInt64(objStr, "productId");
                        sp.stock     = extractJsonInt32(objStr, "stock");
                        bindings.push_back(sp);

                        i = objEnd + 1;
                    } else {
                        ++i;
                    }
                }
            }
        }

        m_service->bindProductsToStore(storeId, bindings);
        return jsonObject({
            {"data",  jsonString("ok")},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::updateStoreProductStatus(int64_t id, const Request& body) {
    try {
        int32_t status = extractJsonInt32(body, "status");
        m_service->updateStoreProductStatus(id, status);
        return jsonObject({
            {"data",  jsonString("ok")},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::updateStoreProductStock(int64_t id, const Request& body) {
    try {
        int32_t stock = extractJsonInt32(body, "stock");
        m_service->updateStoreProductStock(id, stock);
        return jsonObject({
            {"data",  jsonString("ok")},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::removeStoreProduct(int64_t id) {
    try {
        m_service->removeStoreProduct(id);
        return jsonObject({
            {"data",  jsonString("ok")},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}


/* ==================================================================
   27-28. 数据统计 (Statistics)
   ================================================================== */

MerchantController::Response MerchantController::getSalesRanking(const MerchantController::Request& queryParams) {
    try {
        auto params = parseQueryString(queryParams);
        model::StatQueryParams q;

        auto it = params.find("startDate");
        if (it != params.end() && !it->second.empty()) q.startDate = it->second;
        it = params.find("endDate");
        if (it != params.end() && !it->second.empty()) q.endDate = it->second;
        it = params.find("categoryId");
        if (it != params.end() && !it->second.empty())
            q.categoryId = std::stoll(it->second);

        auto result = m_service->getSalesRanking(q);
        std::string data = serializeArray(result, serializeSalesRankItem);
        return jsonObject({
            {"data",  data},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::getVisitRanking(const MerchantController::Request& queryParams) {
    try {
        auto params = parseQueryString(queryParams);
        model::StatQueryParams q;

        auto it = params.find("startDate");
        if (it != params.end() && !it->second.empty()) q.startDate = it->second;
        it = params.find("endDate");
        if (it != params.end() && !it->second.empty()) q.endDate = it->second;
        it = params.find("categoryId");
        if (it != params.end() && !it->second.empty())
            q.categoryId = std::stoll(it->second);

        auto result = m_service->getVisitRanking(q);
        std::string data = serializeArray(result, serializeVisitRankItem);
        return jsonObject({
            {"data",  data},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}


/* ==================================================================
   29-32. 订单管理 (Orders)
   ================================================================== */

MerchantController::Response MerchantController::getOrders(const MerchantController::Request& queryParams) {
    try {
        auto params = parseQueryString(queryParams);
        model::OrderQueryParams q;

        auto it = params.find("orderNo");
        if (it != params.end() && !it->second.empty()) q.orderNo = it->second;
        it = params.find("status");
        if (it != params.end() && !it->second.empty()) q.status = it->second;
        it = params.find("userName");
        if (it != params.end() && !it->second.empty()) q.userName = it->second;
        it = params.find("startDate");
        if (it != params.end() && !it->second.empty()) q.startDate = it->second;
        it = params.find("endDate");
        if (it != params.end() && !it->second.empty()) q.endDate = it->second;
        it = params.find("page");
        if (it != params.end() && !it->second.empty())
            q.page = std::stoi(it->second);
        it = params.find("pageSize");
        if (it != params.end() && !it->second.empty())
            q.pageSize = std::stoi(it->second);

        auto result = m_service->getOrders(q);
        std::string data = serializeArray(result.list, serializeOrder);
        return jsonPageResult(data, result.total, result.page, result.pageSize);
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::getOrderDetail(int64_t id) {
    try {
        auto order = m_service->getOrderDetail(id);
        return jsonObject({
            {"data",  serializeOrder(order)},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::shipOrder(int64_t id, const Request& body) {
    try {
        model::ShipRequest req;
        req.orderId    = id;
        req.trackingNo = extractJsonString(body, "trackingNo");
        req.courier    = extractJsonString(body, "courier");

        m_service->shipOrder(id, req);
        return jsonObject({
            {"data",  jsonString("ok")},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::voidOrder(int64_t id) {
    try {
        m_service->voidOrder(id);
        return jsonObject({
            {"data",  jsonString("ok")},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}


/* ==================================================================
   33-34. 会员管理 (Members)
   ================================================================== */

MerchantController::Response MerchantController::getMembers(const MerchantController::Request& queryParams) {
    try {
        auto params = parseQueryString(queryParams);
        model::MemberQueryParams q;

        auto it = params.find("keyword");
        if (it != params.end() && !it->second.empty()) q.keyword = it->second;
        it = params.find("startDate");
        if (it != params.end() && !it->second.empty()) q.startDate = it->second;
        it = params.find("endDate");
        if (it != params.end() && !it->second.empty()) q.endDate = it->second;
        it = params.find("page");
        if (it != params.end() && !it->second.empty())
            q.page = std::stoi(it->second);
        it = params.find("pageSize");
        if (it != params.end() && !it->second.empty())
            q.pageSize = std::stoi(it->second);

        auto result = m_service->getMembers(q);
        std::string data = serializeArray(result.list, serializeUser);
        return jsonPageResult(data, result.total, result.page, result.pageSize);
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

MerchantController::Response MerchantController::getMemberDetail(int64_t id) {
    try {
        auto member = m_service->getMemberDetail(id);
        return jsonObject({
            {"data",  serializeUser(member)},
            {"error", jsonBool(false)}
        });
    } catch (const std::exception& e) {
        return jsonError(e.what(), 500);
    }
}

} // namespace controller
} // namespace merchant
