/**
 * @file    mechant_service.cpp
 * @brief   商户服务层 — MerchantService 具体实现
 *
 * 实现 include/service/mechant.hpp 中定义的纯虚接口。
 * 使用内存 std::vector 模拟数据，与前端 requests.js 的 mock 数据完全一致。
 */

#include "service/mechant.hpp"
#include <algorithm>
#include <mutex>
#include <stdexcept>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstdlib>

namespace merchant {
namespace service {

using namespace merchant::model;

/* ============================================================
   辅助函数 — 获取当前时间字符串
   ============================================================ */
static std::string nowStr() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M");
    return oss.str();
}

static std::string fullNowStr() {
    auto t = std::time(nullptr);
    auto tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

/* ============================================================
   MerchantServiceImpl — 具体实现类
   ============================================================ */
class MerchantServiceImpl : public MerchantService {
private:
    std::mutex _mutex;
    int64_t _nextId = 1000;

    std::vector<ProductCategory> _categories;
    std::vector<Product> _products;
    std::vector<Promotion> _promotions;
    std::vector<Store> _stores;
    std::vector<ServiceArea> _serviceAreas;
    std::vector<StoreProduct> _storeProducts;
    std::vector<Order> _orders;
    std::vector<User> _members;

    /// 线程安全的 ID 生成
    int64_t _genId() { return _nextId++; }

    /// 查找元素在 vector 中的索引, 未找到返回 -1
    template<typename T>
    static int64_t _findIndex(const std::vector<T>& vec, int64_t id) {
        for (size_t i = 0; i < vec.size(); ++i) {
            if (vec[i].id == id) return static_cast<int64_t>(i);
        }
        return -1;
    }

    /// 按 sort 字段排序的辅助函数
    static bool _bySort(const ProductCategory& a, const ProductCategory& b) { return a.sort < b.sort; }
    static bool _bySortArea(const ServiceArea& a, const ServiceArea& b) { return a.sort < b.sort; }

    /// 通用的分页辅助
    template<typename T>
    static PageResult<T> _paginate(const std::vector<T>& src, int32_t page, int32_t pageSize) {
        PageResult<T> result;
        result.total = static_cast<int64_t>(src.size());
        result.page = page;
        result.pageSize = pageSize;

        int64_t start = (static_cast<int64_t>(page) - 1) * pageSize;
        if (start < 0) start = 0;
        int64_t end = start + pageSize;
        if (end > static_cast<int64_t>(src.size())) end = static_cast<int64_t>(src.size());

        for (int64_t i = start; i < end; ++i) {
            result.list.push_back(src[static_cast<size_t>(i)]);
        }
        return result;
    }

public:
    /* ==========================================================
       CONSTRUCTOR — 初始化 mock 数据 (与前端 requests.js 完全一致)
       ========================================================== */
    MerchantServiceImpl() {
        _nextId = 1000;

        /* --- Categories (8) --- */
        _categories = {
            {1, "新鲜水果", "时令新鲜水果", 1, 1, "2025-03-15 10:00"},
            {2, "蔬菜菌菇", "有机蔬菜及菌菇类", 2, 1, "2025-03-15 10:30"},
            {3, "肉禽蛋奶", "鲜肉、禽类、蛋品及乳制品", 3, 1, "2025-03-16 09:00"},
            {4, "海鲜水产", "新鲜及冷冻海鲜", 4, 1, "2025-03-16 09:30"},
            {5, "粮油调味", "米面粮油及调味品", 5, 1, "2025-03-17 14:00"},
            {6, "零食饮料", "休闲零食及饮料", 6, 1, "2025-03-17 14:30"},
            {7, "日用百货", "日常家居用品", 7, 1, "2025-03-18 11:00"},
            {8, "熟食烘焙", "熟食及现烤烘焙产品", 8, 0, "2025-03-18 11:30"},
        };

        /* --- Products (15) --- */
        _products = {
            {1,  "红富士苹果",  1, "🍎", "山东烟台红富士，脆甜多汁",               12.80, 500,  "斤", 1, "2025-04-01 09:00"},
            {2,  "进口香蕉",    1, "🍌", "菲律宾进口香蕉，香甜软糯",               6.80,  800,  "斤", 1, "2025-04-01 09:30"},
            {3,  "巨峰葡萄",    1, "🍇", "新鲜巨峰葡萄，果粒饱满",                 15.80, 300,  "斤", 1, "2025-04-02 10:00"},
            {4,  "有机西兰花",  2, "🥦", "有机种植西兰花，营养丰富",               8.80,  200,  "颗", 1, "2025-04-02 10:30"},
            {5,  "新鲜番茄",    2, "🍅", "本地温室番茄，鲜红饱满",                 5.80,  600,  "斤", 1, "2025-04-03 08:00"},
            {6,  "金针菇",      2, "🍄", "新鲜金针菇，火锅必备",                   4.50,  400,  "包", 1, "2025-04-03 08:30"},
            {7,  "黑猪五花肉",  3, "🥩", "生态黑猪五花肉，肥瘦相间",               38.80, 150,  "斤", 1, "2025-04-04 09:00"},
            {8,  "土鸡蛋",      3, "🥚", "散养土鸡蛋，蛋黄饱满",                   28.80, 1000, "盒", 1, "2025-04-04 09:30"},
            {9,  "伊利纯牛奶",  3, "🥛", "伊利纯牛奶250ml*24盒",                  68.00, 200,  "箱", 1, "2025-04-05 10:00"},
            {10, "鲜活基围虾",  4, "🦐", "当日鲜活基围虾，肉质弹嫩",               45.80, 80,   "斤", 1, "2025-04-05 10:30"},
            {11, "东北大米",    5, "🍚", "黑龙江五常大米10kg",                     89.90, 300,  "袋", 1, "2025-04-06 08:00"},
            {12, "金龙鱼调和油",5, "🫒", "金龙鱼1:1:1调和油5L",                   79.90, 250,  "桶", 1, "2025-04-06 08:30"},
            {13, "百事可乐",    6, "🥤", "百事可乐330ml*24罐",                    49.90, 400,  "箱", 1, "2025-04-07 09:00"},
            {14, "三只松鼠坚果",6, "🥜", "三只松鼠每日坚果750g",                  89.00, 180,  "盒", 1, "2025-04-07 09:30"},
            {15, "新鲜三文鱼",  4, "🐟", "挪威进口三文鱼，当天直送",               128.00,30,   "份", 0, "2025-04-08 10:00"},
        };

        /* --- Promotions (5) --- */
        _promotions = {
            {1, "618年中大促",  "限时折扣", "2025-06-01 00:00", "2025-06-30 23:59", 8.0, "全场指定商品8折优惠",         "进行中", {1, 2, 5, 7}, "", "", ""},
            {2, "新用户专享",   "满减",     "2025-01-01 00:00", "2025-12-31 23:59", 0,   "满99减20，新用户首单专享",       "进行中", {1, 3, 8, 11}, "", "", ""},
            {3, "端午粽享好礼", "买赠",     "2025-06-20 00:00", "2025-06-25 23:59", 0,   "买粽子礼盒赠送咸鸭蛋一盒",        "未开始", {}, "", "", ""},
            {4, "双十一狂欢",   "优惠券",   "2025-11-01 00:00", "2025-11-30 23:59", 0,   "领取满200减50优惠券",           "未开始", {6, 9, 12, 14}, "", "", ""},
            {5, "周末特惠",     "限时折扣", "2025-04-01 00:00", "2025-08-31 23:59", 9.5, "每周末精选商品9.5折",            "已结束", {4, 10}, "", "", ""},
        };

        /* --- Service Areas (8) --- */
        _serviceAreas = {
            {1, "朝阳区",  "010",    0, 1, "北京市朝阳区",       "2025-02-01 09:00"},
            {2, "海淀区",  "020",    0, 2, "北京市海淀区",       "2025-02-01 09:30"},
            {3, "西城区",  "030",    0, 3, "北京市西城区",       "2025-02-02 10:00"},
            {4, "东城区",  "040",    0, 4, "北京市东城区",       "2025-02-02 10:30"},
            {5, "丰台区",  "050",    0, 5, "北京市丰台区",       "2025-02-03 11:00"},
            {6, "望京街道","010001", 1, 1, "朝阳区望京街道",     "2025-02-03 11:30"},
            {7, "三里屯街道","010002",1, 2, "朝阳区三里屯街道",   "2025-02-04 08:00"},
            {8, "中关村街道","020001",2, 1, "海淀区中关村街道",   "2025-02-04 08:30"},
        };

        /* --- Stores (5) --- */
        _stores = {
            {1, "朝阳旗舰店", 1, "李明", "010-88886666", "08:00", "22:00", "北京市朝阳区建国路88号",       "🏪", "旗舰店，产品最全",             1, "2025-01-10 09:00"},
            {2, "海淀体验店", 2, "王芳", "010-66668888", "09:00", "21:00", "北京市海淀区中关村大街15号",   "🏪", "体验店，支持到店自提",         1, "2025-01-15 10:00"},
            {3, "西城便民店", 3, "张伟", "010-12345678", "07:30", "21:30", "北京市西城区复兴门外大街3号",   "🏪", "社区便民服务店",               1, "2025-02-01 14:00"},
            {4, "东城社区店", 4, "刘洋", "010-87654321", "08:30", "20:30", "北京市东城区王府井大街201号",    "🏪", "东城社区配送点",               0, "2025-02-10 11:00"},
            {5, "丰台仓储店", 5, "陈静", "010-55667788", "07:00", "23:00", "北京市丰台区南三环西路16号",     "🏪", "仓储式门店，库存充足",         1, "2025-03-01 08:00"},
        };

        /* --- Store Products (10) --- */
        _storeProducts = {
            {1,  1, 1,  200, 1, "2025-04-10 10:00"},
            {2,  1, 2,  300, 1, "2025-04-10 10:30"},
            {3,  1, 3,  100, 1, "2025-04-10 11:00"},
            {4,  1, 7,  50,  1, "2025-04-11 09:00"},
            {5,  1, 11, 120, 0, "2025-04-11 09:30"},
            {6,  2, 4,  80,  1, "2025-04-12 10:00"},
            {7,  2, 5,  250, 1, "2025-04-12 10:30"},
            {8,  2, 8,  400, 1, "2025-04-12 11:00"},
            {9,  3, 6,  150, 1, "2025-04-13 08:00"},
            {10, 3, 9,  60,  0, "2025-04-13 08:30"},
        };

        /* --- Orders (5) --- */
        _orders = {
            {1, "ORD20260601001", 1, "张三", "13800138001", "朝阳区望京街道花家地小区3号楼201",  3, 156.80,  "已完成", "微信支付", "2025-06-01 10:30", "2025-06-01 10:25", "SF12345678", "顺丰速运"},
            {2, "ORD20260602002", 2, "李四", "13800138002", "海淀区中关村街道知春路56号院1号楼", 2, 298.70,  "待发货", "支付宝",   "2025-06-02 14:20", "2025-06-02 14:15", "", ""},
            {3, "ORD20260603003", 3, "王五", "13800138003", "西城区复兴门外大街7号院2单元502",   1, 68.00,   "待付款", "",          "",                  "2025-06-03 09:00", "", ""},
            {4, "ORD20260604004", 4, "赵六", "13800138004", "东城区王府井大街霞公府街3号",        4, 452.40,  "已发货", "微信支付", "2025-06-04 16:45", "2025-06-04 16:30", "YT98765432", "圆通速递"},
            {5, "ORD20260605005", 5, "孙七", "13800138005", "丰台区南三环西路天创盛方中心B座",    2, 174.80,  "已取消", "",          "",                  "2025-06-05 11:00", "", ""},
        };

        // Order 1 items & timeline
        _orders[0].items = {
            {1, "红富士苹果", 12.80, 5, 64.00},
            {7, "黑猪五花肉", 38.80, 2, 77.60},
            {5, "新鲜番茄",   5.80,  3, 17.40},
        };
        _orders[0].timeline = {
            {"已下单", "2025-06-01 10:25"},
            {"已支付", "2025-06-01 10:30"},
            {"已发货", "2025-06-01 15:00"},
            {"已完成", "2025-06-03 18:00"},
        };

        // Order 2 items & timeline
        _orders[1].items = {
            {15, "新鲜三文鱼", 128.00, 1, 128.00},
            {11, "东北大米",   89.90,  1, 89.90},
            {8,  "土鸡蛋",    28.80,  2, 57.60},
            {4,  "有机西兰花", 8.80,  3, 26.40},
        };
        _orders[1].timeline = {
            {"已下单", "2025-06-02 14:15"},
            {"已支付", "2025-06-02 14:20"},
        };

        // Order 3 items & timeline
        _orders[2].items = {
            {9, "伊利纯牛奶", 68.00, 1, 68.00},
        };
        _orders[2].timeline = {
            {"已下单", "2025-06-03 09:00"},
        };

        // Order 4 items & timeline
        _orders[3].items = {
            {1,  "红富士苹果", 12.80, 10, 128.00},
            {3,  "巨峰葡萄",   15.80, 5,  79.00},
            {10, "鲜活基围虾", 45.80, 3,  137.40},
            {13, "百事可乐",   49.90, 2,  99.80},
        };
        _orders[3].timeline = {
            {"已下单", "2025-06-04 16:30"},
            {"已支付", "2025-06-04 16:45"},
            {"已发货", "2025-06-05 10:00"},
        };

        // Order 5 items & timeline
        _orders[4].items = {
            {14, "三只松鼠坚果", 89.00, 1, 89.00},
            {12, "金龙鱼调和油", 79.90, 1, 79.90},
        };
        _orders[4].timeline = {
            {"已下单", "2025-06-05 11:00"},
            {"已取消", "2025-06-05 11:30"},
        };

        /* --- Members (7) --- */
        _members = {
            {1, "张三", "", "张建国", "男", "13800138001", "zhangsan@email.com", "👨", "会员", 1, "2025-01-15 08:30", "2025-06-24 09:15", {}, 28, 5230.50, "", 0},
            {2, "李四", "", "李美玲", "女", "13800138002", "lisi@email.com",   "👩", "会员", 1, "2025-02-20 14:00", "2025-06-23 18:30", {}, 15, 2180.00, "", 0},
            {3, "王五", "", "王建国", "男", "13800138003", "wangwu@email.com", "👨", "会员", 1, "2025-03-10 11:20", "2025-06-22 12:00", {}, 5,  620.00,  "", 0},
            {4, "赵六", "", "赵丽华", "女", "13800138004", "zhaoliu@email.com","👩", "会员", 1, "2025-01-05 09:00", "2025-06-24 07:30", {}, 42, 8960.80, "", 0},
            {5, "孙七", "", "孙志强", "男", "13800138005", "sunqi@email.com",  "👨", "会员", 1, "2025-04-18 16:45", "2025-06-20 20:00", {}, 8,  1320.50, "", 0},
            {6, "周八", "", "周晓燕", "女", "13800138006", "zhouba@email.com", "👩", "会员", 1, "2025-05-22 10:00", "2025-06-18 15:20", {}, 3,  340.00,  "", 0},
            {7, "吴九", "", "吴明辉", "男", "13800138007", "wujiu@email.com",  "👨", "会员", 1, "2025-02-08 13:10", "2025-06-24 08:00", {}, 19, 4510.00, "", 0},
        };

        // Member addresses
        _members[0].addresses = {"朝阳区望京街道花家地小区3号楼201"};
        _members[1].addresses = {"海淀区中关村街道知春路56号院1号楼", "西城区某地址"};
        _members[2].addresses = {"西城区复兴门外大街7号院2单元502"};
        _members[3].addresses = {"东城区王府井大街霞公府街3号"};
        _members[4].addresses = {"丰台区南三环西路天创盛方中心B座"};
        _members[5].addresses = {"朝阳区三里屯街道幸福一村5号"};
        _members[6].addresses = {"海淀区学院路街道清华园1号"};
    }

    /* ==========================================================
       商品分类 (Product Categories)
       ========================================================== */

    std::vector<ProductCategory> getCategories() override {
        std::lock_guard<std::mutex> lock(_mutex);
        auto result = _categories;
        std::sort(result.begin(), result.end(), _bySort);
        return result;
    }

    ProductCategory createCategory(const ProductCategory& data) override {
        std::lock_guard<std::mutex> lock(_mutex);
        ProductCategory cat = data;
        cat.id = _genId();
        cat.createdAt = nowStr();
        _categories.push_back(cat);
        return cat;
    }

    ProductCategory updateCategory(int64_t id, const ProductCategory& data) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_categories, id);
        if (idx < 0) throw std::runtime_error("分类不存在");

        auto& cat = _categories[static_cast<size_t>(idx)];
        cat.name = data.name;
        cat.description = data.description;
        cat.sort = data.sort;
        cat.status = data.status;
        return cat;
    }

    void deleteCategory(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_categories, id);
        if (idx < 0) throw std::runtime_error("分类不存在");

        // 校验该分类下无商品
        for (const auto& p : _products) {
            if (p.categoryId == id) {
                throw std::runtime_error("该分类下还有商品，无法删除");
            }
        }

        _categories.erase(_categories.begin() + idx);
    }

    /* ==========================================================
       商品信息 (Products)
       ========================================================== */

    PageResult<Product> getProducts(const ProductQueryParams& params) override {
        std::lock_guard<std::mutex> lock(_mutex);

        std::vector<Product> filtered;
        for (const auto& p : _products) {
            // name filter (substring)
            if (params.name.has_value() && !params.name->empty()) {
                if (p.name.find(params.name.value()) == std::string::npos)
                    continue;
            }
            // categoryId filter
            if (params.categoryId.has_value()) {
                if (p.categoryId != params.categoryId.value())
                    continue;
            }
            // status filter
            if (params.status.has_value()) {
                if (p.status != params.status.value())
                    continue;
            }
            filtered.push_back(p);
        }

        // 按 id 降序 (最新在前)
        std::sort(filtered.begin(), filtered.end(),
                  [](const Product& a, const Product& b) { return a.id > b.id; });

        return _paginate(filtered, params.page, params.pageSize);
    }

    Product getProductById(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_products, id);
        if (idx < 0) throw std::runtime_error("商品不存在");
        return _products[static_cast<size_t>(idx)];
    }

    Product createProduct(const Product& data) override {
        std::lock_guard<std::mutex> lock(_mutex);
        Product prod = data;
        prod.id = _genId();
        prod.createdAt = nowStr();
        _products.push_back(prod);
        return prod;
    }

    Product updateProduct(int64_t id, const Product& data) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_products, id);
        if (idx < 0) throw std::runtime_error("商品不存在");

        auto& prod = _products[static_cast<size_t>(idx)];
        if (!data.name.empty()) prod.name = data.name;
        if (data.categoryId != 0) prod.categoryId = data.categoryId;
        if (!data.image.empty()) prod.image = data.image;
        if (!data.description.empty()) prod.description = data.description;
        if (data.price != 0.0) prod.price = data.price;
        if (data.stock != 0) prod.stock = data.stock;
        if (!data.unit.empty()) prod.unit = data.unit;
        prod.status = data.status;
        return prod;
    }

    void deleteProduct(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_products, id);
        if (idx < 0) throw std::runtime_error("商品不存在");
        _products.erase(_products.begin() + idx);
    }

    /* ==========================================================
       促销管理 (Promotions)
       ========================================================== */

    std::vector<Promotion> getPromotions() override {
        std::lock_guard<std::mutex> lock(_mutex);
        auto result = _promotions;
        std::reverse(result.begin(), result.end());
        return result;
    }

    Promotion createPromotion(const Promotion& data) override {
        std::lock_guard<std::mutex> lock(_mutex);
        Promotion promo = data;
        promo.id = _genId();
        if (promo.status.empty()) promo.status = "未开始";
        _promotions.push_back(promo);
        return promo;
    }

    Promotion updatePromotion(int64_t id, const Promotion& data) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_promotions, id);
        if (idx < 0) throw std::runtime_error("促销不存在");

        auto& promo = _promotions[static_cast<size_t>(idx)];
        if (!data.name.empty()) promo.name = data.name;
        if (!data.type.empty()) promo.type = data.type;
        if (!data.startTime.empty()) promo.startTime = data.startTime;
        if (!data.endTime.empty()) promo.endTime = data.endTime;
        if (data.discount != 0.0) promo.discount = data.discount;
        if (!data.description.empty()) promo.description = data.description;
        if (!data.productIds.empty()) promo.productIds = data.productIds;
        return promo;
    }

    void deletePromotion(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_promotions, id);
        if (idx < 0) throw std::runtime_error("促销不存在");
        _promotions.erase(_promotions.begin() + idx);
    }

    void bindProductsToPromotion(int64_t promotionId,
                                 const std::vector<int64_t>& productIds) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_promotions, promotionId);
        if (idx < 0) throw std::runtime_error("促销不存在");

        // 替换 productIds 向量
        _promotions[static_cast<size_t>(idx)].productIds = productIds;
    }

    std::vector<int64_t> getPromotionProductIds(int64_t promotionId) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_promotions, promotionId);
        if (idx < 0) throw std::runtime_error("促销不存在");

        return _promotions[static_cast<size_t>(idx)].productIds;
    }

    /* ==========================================================
       门店管理 (Stores)
       ========================================================== */

    std::vector<Store> getStores() override {
        std::lock_guard<std::mutex> lock(_mutex);
        auto result = _stores;
        std::reverse(result.begin(), result.end());
        return result;
    }

    Store createStore(const Store& data) override {
        std::lock_guard<std::mutex> lock(_mutex);
        Store store = data;
        store.id = _genId();
        store.createdAt = nowStr();
        _stores.push_back(store);
        return store;
    }

    Store updateStore(int64_t id, const Store& data) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_stores, id);
        if (idx < 0) throw std::runtime_error("门店不存在");

        auto& store = _stores[static_cast<size_t>(idx)];
        if (!data.name.empty()) store.name = data.name;
        if (data.areaId != 0) store.areaId = data.areaId;
        if (!data.contact.empty()) store.contact = data.contact;
        if (!data.phone.empty()) store.phone = data.phone;
        if (!data.openTime.empty()) store.openTime = data.openTime;
        if (!data.closeTime.empty()) store.closeTime = data.closeTime;
        if (!data.address.empty()) store.address = data.address;
        if (!data.image.empty()) store.image = data.image;
        if (!data.description.empty()) store.description = data.description;
        store.status = data.status;
        return store;
    }

    void deleteStore(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_stores, id);
        if (idx < 0) throw std::runtime_error("门店不存在");
        _stores.erase(_stores.begin() + idx);
    }

    /* ==========================================================
       服务区域管理 (Service Areas)
       ========================================================== */

    std::vector<ServiceArea> getServiceAreas() override {
        std::lock_guard<std::mutex> lock(_mutex);
        auto result = _serviceAreas;
        std::sort(result.begin(), result.end(), _bySortArea);
        return result;
    }

    ServiceArea createServiceArea(const ServiceArea& data) override {
        std::lock_guard<std::mutex> lock(_mutex);
        ServiceArea area = data;
        area.id = _genId();
        area.createdAt = nowStr();
        _serviceAreas.push_back(area);
        return area;
    }

    ServiceArea updateServiceArea(int64_t id, const ServiceArea& data) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_serviceAreas, id);
        if (idx < 0) throw std::runtime_error("区域不存在");

        auto& area = _serviceAreas[static_cast<size_t>(idx)];
        if (!data.name.empty()) area.name = data.name;
        if (!data.code.empty()) area.code = data.code;
        area.parentId = data.parentId;
        area.sort = data.sort;
        if (!data.remark.empty()) area.remark = data.remark;
        return area;
    }

    void deleteServiceArea(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_serviceAreas, id);
        if (idx < 0) throw std::runtime_error("区域不存在");

        // 校验该区域下无子区域
        for (const auto& a : _serviceAreas) {
            if (a.parentId == id) {
                throw std::runtime_error("该区域下还有子区域，无法删除");
            }
        }

        // 校验该区域下无门店
        for (const auto& s : _stores) {
            if (s.areaId == id) {
                throw std::runtime_error("该区域下还有门店，无法删除");
            }
        }

        _serviceAreas.erase(_serviceAreas.begin() + idx);
    }

    /* ==========================================================
       注册商品管理 (Store-Product Binding)
       ========================================================== */

    std::vector<StoreProduct> getStoreProducts(int64_t storeId) override {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<StoreProduct> result;
        for (const auto& sp : _storeProducts) {
            if (sp.storeId == storeId) {
                result.push_back(sp);
            }
        }
        return result;
    }

    void bindProductsToStore(int64_t storeId,
                             const std::vector<StoreProduct>& bindings) override {
        std::lock_guard<std::mutex> lock(_mutex);
        for (const auto& b : bindings) {
            // 跳过已绑定的商品
            bool exists = false;
            for (const auto& sp : _storeProducts) {
                if (sp.storeId == storeId && sp.productId == b.productId) {
                    exists = true;
                    break;
                }
            }
            if (exists) continue;

            StoreProduct sp;
            sp.id = _genId();
            sp.storeId = storeId;
            sp.productId = b.productId;
            sp.stock = b.stock;
            sp.status = 1;
            sp.bindTime = nowStr();
            _storeProducts.push_back(sp);
        }
    }

    void updateStoreProductStatus(int64_t id, int32_t status) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_storeProducts, id);
        if (idx < 0) throw std::runtime_error("记录不存在");
        _storeProducts[static_cast<size_t>(idx)].status = status;
    }

    void updateStoreProductStock(int64_t id, int32_t stock) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_storeProducts, id);
        if (idx < 0) throw std::runtime_error("记录不存在");
        _storeProducts[static_cast<size_t>(idx)].stock = stock;
    }

    void removeStoreProduct(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_storeProducts, id);
        if (idx < 0) throw std::runtime_error("记录不存在");
        _storeProducts.erase(_storeProducts.begin() + idx);
    }

    /* ==========================================================
       数据统计 (Statistics)
       ========================================================== */

    std::vector<SalesRankItem> getSalesRanking(const StatQueryParams& params) override {
        std::lock_guard<std::mutex> lock(_mutex);

        std::vector<SalesRankItem> result;
        for (const auto& p : _products) {
            // 分类筛选
            if (params.categoryId.has_value() && p.categoryId != params.categoryId.value()) {
                continue;
            }

            SalesRankItem item;
            item.id = p.id;
            item.name = p.name;
            item.categoryId = p.categoryId;
            item.image = p.image;
            // 使用确定性伪随机数据 (基于 product id)
            item.salesCount = static_cast<int32_t>(((p.id * 137 + 59) % 2000) + 50);
            item.salesAmount = std::round(item.salesCount * p.price * 100.0) / 100.0;
            item.avgPrice = p.price;
            result.push_back(item);
        }

        // 按销售额降序
        std::sort(result.begin(), result.end(),
                  [](const SalesRankItem& a, const SalesRankItem& b) {
                      return a.salesAmount > b.salesAmount;
                  });

        // 只返回前 10
        if (static_cast<int64_t>(result.size()) > 10) {
            result.resize(10);
        }
        return result;
    }

    std::vector<VisitRankItem> getVisitRanking(const StatQueryParams& params) override {
        std::lock_guard<std::mutex> lock(_mutex);

        std::vector<VisitRankItem> result;
        for (const auto& p : _products) {
            // 分类筛选
            if (params.categoryId.has_value() && p.categoryId != params.categoryId.value()) {
                continue;
            }

            VisitRankItem item;
            item.id = p.id;
            item.name = p.name;
            item.categoryId = p.categoryId;
            item.image = p.image;
            // 使用确定性伪随机数据
            int32_t visits = static_cast<int32_t>(((p.id * 241 + 131) % 5000) + 200);
            item.visitCount = visits;
            item.uniqueVisitors = static_cast<int32_t>(visits * 0.65);
            // 转化率
            double convRate = ((p.id * 17 + 7) % 40) + 1.0 + (((p.id * 3) % 10) / 10.0);
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(1) << convRate << "%";
            item.conversionRate = oss.str();
            result.push_back(item);
        }

        // 按访问量降序
        std::sort(result.begin(), result.end(),
                  [](const VisitRankItem& a, const VisitRankItem& b) {
                      return a.visitCount > b.visitCount;
                  });

        // 只返回前 10
        if (static_cast<int64_t>(result.size()) > 10) {
            result.resize(10);
        }
        return result;
    }

    /* ==========================================================
       订单管理 (Orders)
       ========================================================== */

    PageResult<Order> getOrders(const OrderQueryParams& params) override {
        std::lock_guard<std::mutex> lock(_mutex);

        std::vector<Order> filtered;
        for (const auto& o : _orders) {
            // orderNo filter (substring)
            if (params.orderNo.has_value() && !params.orderNo->empty()) {
                if (o.orderNo.find(params.orderNo.value()) == std::string::npos)
                    continue;
            }
            // status filter
            if (params.status.has_value() && !params.status->empty()) {
                if (o.status != params.status.value())
                    continue;
            }
            // userName filter (substring)
            if (params.userName.has_value() && !params.userName->empty()) {
                if (o.userName.find(params.userName.value()) == std::string::npos)
                    continue;
            }
            // startDate filter
            if (params.startDate.has_value() && !params.startDate->empty()) {
                if (o.createTime < params.startDate.value())
                    continue;
            }
            // endDate filter
            if (params.endDate.has_value() && !params.endDate->empty()) {
                if (o.createTime > params.endDate.value())
                    continue;
            }
            filtered.push_back(o);
        }

        // 按 id 降序
        std::sort(filtered.begin(), filtered.end(),
                  [](const Order& a, const Order& b) { return a.id > b.id; });

        return _paginate(filtered, params.page, params.pageSize);
    }

    Order getOrderDetail(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_orders, id);
        if (idx < 0) throw std::runtime_error("订单不存在");
        return _orders[static_cast<size_t>(idx)];
    }

    void shipOrder(int64_t id, const ShipRequest& req) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_orders, id);
        if (idx < 0) throw std::runtime_error("订单不存在");

        auto& order = _orders[static_cast<size_t>(idx)];
        // 校验状态必须是 "待发货"
        if (order.status != "待发货") {
            throw std::runtime_error("只能对待发货订单执行发货操作");
        }

        order.status = "已发货";
        order.trackingNo = req.trackingNo;
        order.courier = req.courier;
        order.timeline.push_back({"已发货", nowStr()});
    }

    void voidOrder(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_orders, id);
        if (idx < 0) throw std::runtime_error("订单不存在");

        auto& order = _orders[static_cast<size_t>(idx)];
        // 校验状态必须是 待付款 或 待发货
        if (order.status != "待付款" && order.status != "待发货") {
            throw std::runtime_error("只能对待付款或待发货订单执行作废操作");
        }

        order.status = "已取消";
        order.timeline.push_back({"已取消", nowStr()});
    }

    /* ==========================================================
       会员管理 (Members)
       ========================================================== */

    PageResult<User> getMembers(const MemberQueryParams& params) override {
        std::lock_guard<std::mutex> lock(_mutex);

        std::vector<User> filtered;
        for (const auto& m : _members) {
            // keyword filter (username or phone)
            if (params.keyword.has_value() && !params.keyword->empty()) {
                const std::string& kw = params.keyword.value();
                if (m.username.find(kw) == std::string::npos &&
                    m.phone.find(kw) == std::string::npos) {
                    continue;
                }
            }
            // startDate filter
            if (params.startDate.has_value() && !params.startDate->empty()) {
                if (m.createdAt < params.startDate.value())
                    continue;
            }
            // endDate filter
            if (params.endDate.has_value() && !params.endDate->empty()) {
                if (m.createdAt > params.endDate.value())
                    continue;
            }
            filtered.push_back(m);
        }

        // 按 id 降序
        std::sort(filtered.begin(), filtered.end(),
                  [](const User& a, const User& b) { return a.id > b.id; });

        return _paginate(filtered, params.page, params.pageSize);
    }

    User getMemberDetail(int64_t id) override {
        std::lock_guard<std::mutex> lock(_mutex);
        int64_t idx = _findIndex(_members, id);
        if (idx < 0) throw std::runtime_error("会员不存在");
        return _members[static_cast<size_t>(idx)];
    }
};

/* ============================================================
   工厂函数
   ============================================================ */
MerchantServicePtr createMerchantService() {
    return std::make_shared<MerchantServiceImpl>();
}

} // namespace service
} // namespace merchant
