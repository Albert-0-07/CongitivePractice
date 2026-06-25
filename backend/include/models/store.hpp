/**
 * @file    store.hpp
 * @brief   门店相关数据模型 — Store, ServiceArea
 *
 * 对应前端商户管理中的:
 *   - 门店管理 (Store)
 *   - 服务区域管理 (ServiceArea)
 *
 * 与 product.hpp 中的 StoreProduct 配合完成 注册商品管理 功能。
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <optional>

namespace merchant {
namespace model {

/* ============================================================
   服务区域 (Service Area)
   对应前端: ServiceAreas CRUD
   支持树形层级结构 (通过 parentId 自引用)
   ============================================================ */
struct ServiceArea {
    int64_t     id          = 0;        ///< 区域ID (主键)
    std::string name;                   ///< 区域名称
    std::string code;                   ///< 区域编码
    int64_t     parentId    = 0;        ///< 上级区域ID: 0 = 顶级区域
    int32_t     sort        = 0;        ///< 排序值
    std::string remark;                 ///< 备注
    std::string createdAt;              ///< 创建时间

    ServiceArea() = default;

    /// 是否为顶级区域
    bool isTopLevel() const { return parentId == 0; }
};

/* ============================================================
   门店 (Store / Pickup Location)
   对应前端: Stores CRUD
   ============================================================ */
struct Store {
    int64_t     id          = 0;        ///< 门店ID (主键)
    std::string name;                   ///< 门店名称
    int64_t     areaId      = 0;        ///< 所属服务区域ID -> ServiceArea::id
    std::string contact;                ///< 联系人姓名
    std::string phone;                  ///< 联系电话
    std::string openTime;               ///< 营业开始时间 (HH:MM)
    std::string closeTime;              ///< 营业结束时间 (HH:MM)
    std::string address;                ///< 详细地址
    std::string image;                  ///< 门店图片 (emoji 或 URL)
    std::string description;            ///< 门店描述
    int32_t     status      = 1;        ///< 状态: 1=营业中, 0=已关闭
    std::string createdAt;              ///< 创建时间

    Store() = default;

    /// 返回格式化营业时间, 如 "08:00-22:00"
    std::string businessHours() const {
        return openTime + "-" + closeTime;
    }
};

/* ============================================================
   门店查询参数
   ============================================================ */
struct StoreQueryParams {
    std::optional<std::string> name;    ///< 按门店名称搜索
    std::optional<int64_t>     areaId;  ///< 按区域筛选
    std::optional<int32_t>     status;  ///< 按状态筛选
    int32_t                    page     = 1;
    int32_t                    pageSize = 10;

    StoreQueryParams() = default;
};

} // namespace model
} // namespace merchant
