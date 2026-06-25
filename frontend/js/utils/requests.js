/* ============================================================
   商户管理系统 — API Layer (with Mock Data)
   Provides: window.App.API
   All data operations go through this module.
   Swap mock bodies for fetch() calls when backend is ready.
   ============================================================ */

(function () {
  'use strict';

  /* ==========================================================
     MOCK DATA — seeded with realistic Chinese retail data
     ========================================================== */

  let _idCounter = 100;

  function nextId(prefix) {
    _idCounter++;
    return prefix ? prefix + _idCounter : _idCounter;
  }

  /* --- Categories --- */
  let _categories = [
    { id: 1,  name: '新鲜水果', description: '时令新鲜水果', sort: 1,  status: 1, createdAt: '2025-03-15 10:00' },
    { id: 2,  name: '蔬菜菌菇', description: '有机蔬菜及菌菇类', sort: 2,  status: 1, createdAt: '2025-03-15 10:30' },
    { id: 3,  name: '肉禽蛋奶', description: '鲜肉、禽类、蛋品及乳制品', sort: 3,  status: 1, createdAt: '2025-03-16 09:00' },
    { id: 4,  name: '海鲜水产', description: '新鲜及冷冻海鲜', sort: 4,  status: 1, createdAt: '2025-03-16 09:30' },
    { id: 5,  name: '粮油调味', description: '米面粮油及调味品', sort: 5,  status: 1, createdAt: '2025-03-17 14:00' },
    { id: 6,  name: '零食饮料', description: '休闲零食及饮料', sort: 6,  status: 1, createdAt: '2025-03-17 14:30' },
    { id: 7,  name: '日用百货', description: '日常家居用品', sort: 7,  status: 1, createdAt: '2025-03-18 11:00' },
    { id: 8,  name: '熟食烘焙', description: '熟食及现烤烘焙产品', sort: 8,  status: 0, createdAt: '2025-03-18 11:30' },
  ];

  /* --- Products --- */
  let _products = [
    { id: 1,  name: '红富士苹果',    categoryId: 1, image: '🍎', description: '山东烟台红富士，脆甜多汁',                   price: 12.80, stock: 500, unit: '斤',   status: 1, createdAt: '2025-04-01 09:00' },
    { id: 2,  name: '进口香蕉',      categoryId: 1, image: '🍌', description: '菲律宾进口香蕉，香甜软糯',                   price: 6.80,  stock: 800, unit: '斤',   status: 1, createdAt: '2025-04-01 09:30' },
    { id: 3,  name: '巨峰葡萄',      categoryId: 1, image: '🍇', description: '新鲜巨峰葡萄，果粒饱满',                     price: 15.80, stock: 300, unit: '斤',   status: 1, createdAt: '2025-04-02 10:00' },
    { id: 4,  name: '有机西兰花',    categoryId: 2, image: '🥦', description: '有机种植西兰花，营养丰富',                   price: 8.80,  stock: 200, unit: '颗',   status: 1, createdAt: '2025-04-02 10:30' },
    { id: 5,  name: '新鲜番茄',      categoryId: 2, image: '🍅', description: '本地温室番茄，鲜红饱满',                     price: 5.80,  stock: 600, unit: '斤',   status: 1, createdAt: '2025-04-03 08:00' },
    { id: 6,  name: '金针菇',        categoryId: 2, image: '🍄', description: '新鲜金针菇，火锅必备',                       price: 4.50,  stock: 400, unit: '包',   status: 1, createdAt: '2025-04-03 08:30' },
    { id: 7,  name: '黑猪五花肉',    categoryId: 3, image: '🥩', description: '生态黑猪五花肉，肥瘦相间',                   price: 38.80, stock: 150, unit: '斤',   status: 1, createdAt: '2025-04-04 09:00' },
    { id: 8,  name: '土鸡蛋',        categoryId: 3, image: '🥚', description: '散养土鸡蛋，蛋黄饱满',                       price: 28.80, stock: 1000,unit: '盒',   status: 1, createdAt: '2025-04-04 09:30' },
    { id: 9,  name: '伊利纯牛奶',    categoryId: 3, image: '🥛', description: '伊利纯牛奶250ml*24盒',                      price: 68.00, stock: 200, unit: '箱',   status: 1, createdAt: '2025-04-05 10:00' },
    { id: 10, name: '鲜活基围虾',    categoryId: 4, image: '🦐', description: '当日鲜活基围虾，肉质弹嫩',                   price: 45.80, stock: 80,  unit: '斤',   status: 1, createdAt: '2025-04-05 10:30' },
    { id: 11, name: '东北大米',      categoryId: 5, image: '🍚', description: '黑龙江五常大米10kg',                         price: 89.90, stock: 300, unit: '袋',   status: 1, createdAt: '2025-04-06 08:00' },
    { id: 12, name: '金龙鱼调和油',  categoryId: 5, image: '🫒', description: '金龙鱼1:1:1调和油5L',                       price: 79.90, stock: 250, unit: '桶',   status: 1, createdAt: '2025-04-06 08:30' },
    { id: 13, name: '百事可乐',      categoryId: 6, image: '🥤', description: '百事可乐330ml*24罐',                        price: 49.90, stock: 400, unit: '箱',   status: 1, createdAt: '2025-04-07 09:00' },
    { id: 14, name: '三只松鼠坚果',  categoryId: 6, image: '🥜', description: '三只松鼠每日坚果750g',                      price: 89.00, stock: 180, unit: '盒',   status: 1, createdAt: '2025-04-07 09:30' },
    { id: 15, name: '新鲜三文鱼',    categoryId: 4, image: '🐟', description: '挪威进口三文鱼，当天直送',                   price: 128.00,stock: 30,  unit: '份',   status: 0, createdAt: '2025-04-08 10:00' },
  ];

  /* --- Promotions --- */
  let _promotions = [
    { id: 1, name: '618年中大促',    type: '限时折扣', startTime: '2025-06-01 00:00', endTime: '2025-06-30 23:59', discount: 8.0, description: '全场指定商品8折优惠',          status: '进行中', productIds: [1, 2, 5, 7] },
    { id: 2, name: '新用户专享',     type: '满减',     startTime: '2025-01-01 00:00', endTime: '2025-12-31 23:59', discount: 0,   description: '满99减20，新用户首单专享',        status: '进行中', productIds: [1, 3, 8, 11] },
    { id: 3, name: '端午粽享好礼',   type: '买赠',     startTime: '2025-06-20 00:00', endTime: '2025-06-25 23:59', discount: 0,   description: '买粽子礼盒赠送咸鸭蛋一盒',         status: '未开始', productIds: [] },
    { id: 4, name: '双十一狂欢',     type: '优惠券',   startTime: '2025-11-01 00:00', endTime: '2025-11-30 23:59', discount: 0,   description: '领取满200减50优惠券',            status: '未开始', productIds: [6, 9, 12, 14] },
    { id: 5, name: '周末特惠',       type: '限时折扣', startTime: '2025-04-01 00:00', endTime: '2025-08-31 23:59', discount: 9.5, description: '每周末精选商品9.5折',             status: '已结束', productIds: [4, 10] },
  ];

  /* --- Service Areas --- */
  let _serviceAreas = [
    { id: 1,  name: '朝阳区',  code: '010', parentId: 0, sort: 1,  remark: '北京市朝阳区',              createdAt: '2025-02-01 09:00' },
    { id: 2,  name: '海淀区',  code: '020', parentId: 0, sort: 2,  remark: '北京市海淀区',              createdAt: '2025-02-01 09:30' },
    { id: 3,  name: '西城区',  code: '030', parentId: 0, sort: 3,  remark: '北京市西城区',              createdAt: '2025-02-02 10:00' },
    { id: 4,  name: '东城区',  code: '040', parentId: 0, sort: 4,  remark: '北京市东城区',              createdAt: '2025-02-02 10:30' },
    { id: 5,  name: '丰台区',  code: '050', parentId: 0, sort: 5,  remark: '北京市丰台区',              createdAt: '2025-02-03 11:00' },
    { id: 6,  name: '望京街道',code: '010001', parentId: 1, sort: 1, remark: '朝阳区望京街道',          createdAt: '2025-02-03 11:30' },
    { id: 7,  name: '三里屯街道',code:'010002', parentId: 1, sort: 2, remark: '朝阳区三里屯街道',        createdAt: '2025-02-04 08:00' },
    { id: 8,  name: '中关村街道',code:'020001', parentId: 2, sort: 1, remark: '海淀区中关村街道',        createdAt: '2025-02-04 08:30' },
  ];

  /* --- Stores --- */
  let _stores = [
    { id: 1, name: '朝阳旗舰店',    areaId: 1, contact: '李明',   phone: '010-88886666', openTime: '08:00', closeTime: '22:00', address: '北京市朝阳区建国路88号',        image: '🏪', description: '旗舰店，产品最全',               status: 1, createdAt: '2025-01-10 09:00' },
    { id: 2, name: '海淀体验店',    areaId: 2, contact: '王芳',   phone: '010-66668888', openTime: '09:00', closeTime: '21:00', address: '北京市海淀区中关村大街15号',    image: '🏪', description: '体验店，支持到店自提',           status: 1, createdAt: '2025-01-15 10:00' },
    { id: 3, name: '西城便民店',    areaId: 3, contact: '张伟',   phone: '010-12345678', openTime: '07:30', closeTime: '21:30', address: '北京市西城区复兴门外大街3号',    image: '🏪', description: '社区便民服务店',                 status: 1, createdAt: '2025-02-01 14:00' },
    { id: 4, name: '东城社区店',    areaId: 4, contact: '刘洋',   phone: '010-87654321', openTime: '08:30', closeTime: '20:30', address: '北京市东城区王府井大街201号',     image: '🏪', description: '东城社区配送点',                 status: 0, createdAt: '2025-02-10 11:00' },
    { id: 5, name: '丰台仓储店',    areaId: 5, contact: '陈静',   phone: '010-55667788', openTime: '07:00', closeTime: '23:00', address: '北京市丰台区南三环西路16号',      image: '🏪', description: '仓储式门店，库存充足',           status: 1, createdAt: '2025-03-01 08:00' },
  ];

  /* --- Store Products (binding products to stores with stock) --- */
  let _storeProducts = [
    { id: 1,  storeId: 1, productId: 1,  stock: 200, status: 1, bindTime: '2025-04-10 10:00' },
    { id: 2,  storeId: 1, productId: 2,  stock: 300, status: 1, bindTime: '2025-04-10 10:30' },
    { id: 3,  storeId: 1, productId: 3,  stock: 100, status: 1, bindTime: '2025-04-10 11:00' },
    { id: 4,  storeId: 1, productId: 7,  stock: 50,  status: 1, bindTime: '2025-04-11 09:00' },
    { id: 5,  storeId: 1, productId: 11, stock: 120, status: 0, bindTime: '2025-04-11 09:30' },
    { id: 6,  storeId: 2, productId: 4,  stock: 80,  status: 1, bindTime: '2025-04-12 10:00' },
    { id: 7,  storeId: 2, productId: 5,  stock: 250, status: 1, bindTime: '2025-04-12 10:30' },
    { id: 8,  storeId: 2, productId: 8,  stock: 400, status: 1, bindTime: '2025-04-12 11:00' },
    { id: 9,  storeId: 3, productId: 6,  stock: 150, status: 1, bindTime: '2025-04-13 08:00' },
    { id: 10, storeId: 3, productId: 9,  stock: 60,  status: 0, bindTime: '2025-04-13 08:30' },
  ];

  /* --- Orders --- */
  let _orders = [
    { id: 1, orderNo: 'ORD20260601001', userId: 1, userName: '张三', phone: '13800138001', address: '朝阳区望京街道花家地小区3号楼201', productCount: 3, totalAmount: 156.80, status: '已完成', payMethod: '微信支付', payTime: '2025-06-01 10:30', createTime: '2025-06-01 10:25', items: [{productId:1,name:'红富士苹果',price:12.80,qty:5,subtotal:64.00},{productId:7,name:'黑猪五花肉',price:38.80,qty:2,subtotal:77.60},{productId:5,name:'新鲜番茄',price:5.80,qty:3,subtotal:17.40}], timeline: [{status:'已下单',time:'2025-06-01 10:25'},{status:'已支付',time:'2025-06-01 10:30'},{status:'已发货',time:'2025-06-01 15:00'},{status:'已完成',time:'2025-06-03 18:00'}] },
    { id: 2, orderNo: 'ORD20260602002', userId: 2, userName: '李四', phone: '13800138002', address: '海淀区中关村街道知春路56号院1号楼',         productCount: 2, totalAmount: 298.70, status: '待发货', payMethod: '支付宝',   payTime: '2025-06-02 14:20', createTime: '2025-06-02 14:15', items: [{productId:15,name:'新鲜三文鱼',price:128.00,qty:1,subtotal:128.00},{productId:11,name:'东北大米',price:89.90,qty:1,subtotal:89.90},{productId:8,name:'土鸡蛋',price:28.80,qty:2,subtotal:57.60},{productId:4,name:'有机西兰花',price:8.80,qty:3,subtotal:26.40}], timeline: [{status:'已下单',time:'2025-06-02 14:15'},{status:'已支付',time:'2025-06-02 14:20'}] },
    { id: 3, orderNo: 'ORD20260603003', userId: 3, userName: '王五', phone: '13800138003', address: '西城区复兴门外大街7号院2单元502',         productCount: 1, totalAmount: 68.00,  status: '待付款', payMethod: '',         payTime: '',                   createTime: '2025-06-03 09:00', items: [{productId:9,name:'伊利纯牛奶',price:68.00,qty:1,subtotal:68.00}], timeline: [{status:'已下单',time:'2025-06-03 09:00'}] },
    { id: 4, orderNo: 'ORD20260604004', userId: 4, userName: '赵六', phone: '13800138004', address: '东城区王府井大街霞公府街3号',               productCount: 4, totalAmount: 452.40, status: '已发货', payMethod: '微信支付', payTime: '2025-06-04 16:45', createTime: '2025-06-04 16:30', items: [{productId:1,name:'红富士苹果',price:12.80,qty:10,subtotal:128.00},{productId:3,name:'巨峰葡萄',price:15.80,qty:5,subtotal:79.00},{productId:10,name:'鲜活基围虾',price:45.80,qty:3,subtotal:137.40},{productId:13,name:'百事可乐',price:49.90,qty:2,subtotal:99.80}], timeline: [{status:'已下单',time:'2025-06-04 16:30'},{status:'已支付',time:'2025-06-04 16:45'},{status:'已发货',time:'2025-06-05 10:00'}] },
    { id: 5, orderNo: 'ORD20260605005', userId: 5, userName: '孙七', phone: '13800138005', address: '丰台区南三环西路天创盛方中心B座',           productCount: 2, totalAmount: 174.80, status: '已取消', payMethod: '',         payTime: '',                   createTime: '2025-06-05 11:00', items: [{productId:14,name:'三只松鼠坚果',price:89.00,qty:1,subtotal:89.00},{productId:12,name:'金龙鱼调和油',price:79.90,qty:1,subtotal:79.90}], timeline: [{status:'已下单',time:'2025-06-05 11:00'},{status:'已取消',time:'2025-06-05 11:30'}] },
  ];
  let _nextOrderNum = 6;

  /* --- Members --- */
  let _members = [
    { id: 1, username: '张三', realName: '张建国', gender: '男', phone: '13800138001', email: 'zhangsan@email.com', avatar: '👨', addresses: ['朝阳区望京街道花家地小区3号楼201'], orderCount: 28, totalSpent: 5230.50, createdAt: '2025-01-15 08:30', lastLogin: '2025-06-24 09:15' },
    { id: 2, username: '李四', realName: '李美玲', gender: '女', phone: '13800138002', email: 'lisi@email.com',   avatar: '👩', addresses: ['海淀区中关村街道知春路56号院1号楼','西城区某地址'], orderCount: 15, totalSpent: 2180.00, createdAt: '2025-02-20 14:00', lastLogin: '2025-06-23 18:30' },
    { id: 3, username: '王五', realName: '王建国', gender: '男', phone: '13800138003', email: 'wangwu@email.com', avatar: '👨', addresses: ['西城区复兴门外大街7号院2单元502'], orderCount: 5,  totalSpent: 620.00,  createdAt: '2025-03-10 11:20', lastLogin: '2025-06-22 12:00' },
    { id: 4, username: '赵六', realName: '赵丽华', gender: '女', phone: '13800138004', email: 'zhaoliu@email.com',avatar: '👩', addresses: ['东城区王府井大街霞公府街3号'], orderCount: 42, totalSpent: 8960.80, createdAt: '2025-01-05 09:00', lastLogin: '2025-06-24 07:30' },
    { id: 5, username: '孙七', realName: '孙志强', gender: '男', phone: '13800138005', email: 'sunqi@email.com',  avatar: '👨', addresses: ['丰台区南三环西路天创盛方中心B座'], orderCount: 8,  totalSpent: 1320.50, createdAt: '2025-04-18 16:45', lastLogin: '2025-06-20 20:00' },
    { id: 6, username: '周八', realName: '周晓燕', gender: '女', phone: '13800138006', email: 'zhouba@email.com', avatar: '👩', addresses: ['朝阳区三里屯街道幸福一村5号'], orderCount: 3,  totalSpent: 340.00,  createdAt: '2025-05-22 10:00', lastLogin: '2025-06-18 15:20' },
    { id: 7, username: '吴九', realName: '吴明辉', gender: '男', phone: '13800138007', email: 'wujiu@email.com', avatar: '👨', addresses: ['海淀区学院路街道清华园1号'], orderCount: 19, totalSpent: 4510.00, createdAt: '2025-02-08 13:10', lastLogin: '2025-06-24 08:00' },
  ];

  /* --- Users (admin management) --- */
  let _users = [
    { id: 1, username: 'admin',     realName: '系统管理员', role: '超级管理员', phone: '13900000001', email: 'admin@company.com',     status: 1, createdAt: '2025-01-01 00:00' },
    { id: 2, username: 'merchant01',realName: '李明',       role: '商户',       phone: '13900000002', email: 'liming@company.com',   status: 1, createdAt: '2025-01-10 09:00' },
    { id: 3, username: 'operator01',realName: '王芳',       role: '运营',       phone: '13900000003', email: 'wangfang@company.com', status: 1, createdAt: '2025-02-01 10:00' },
    { id: 4, username: 'service01', realName: '张伟',       role: '客服',       phone: '13900000004', email: 'zhangwei@company.com', status: 1, createdAt: '2025-02-15 14:00' },
    { id: 5, username: 'merchant02',realName: '刘洋',       role: '商户',       phone: '13900000005', email: 'liuyang@company.com',  status: 0, createdAt: '2025-03-01 08:30' },
  ];

  /* --- Roles & Permissions --- */
  let _roles = [
    { id: 1, name: '超级管理员', permissions: { products:{view:1,add:1,edit:1,del:1}, orders:{view:1,add:0,edit:0,del:0,ship:1,void:1}, members:{view:1,add:0,edit:0,del:0}, stores:{view:1,add:1,edit:1,del:1}, marketing:{view:1,add:1,edit:1,del:1}, statistics:{view:1}, users:{view:1,add:1,edit:1,del:1}, permissions:{view:1,edit:1}, logs:{view:1} } },
    { id: 2, name: '运营',         permissions: { products:{view:1,add:1,edit:1,del:0}, orders:{view:1,add:0,edit:0,del:0,ship:1,void:1}, members:{view:1,add:0,edit:0,del:0}, stores:{view:1,add:0,edit:0,del:0}, marketing:{view:1,add:1,edit:1,del:1}, statistics:{view:1}, users:{view:0,add:0,edit:0,del:0}, permissions:{view:0,edit:0}, logs:{view:0} } },
    { id: 3, name: '客服',         permissions: { products:{view:1,add:0,edit:0,del:0}, orders:{view:1,add:0,edit:0,del:0,ship:0,void:0}, members:{view:1,add:0,edit:0,del:0}, stores:{view:1,add:0,edit:0,del:0}, marketing:{view:1,add:0,edit:0,del:0}, statistics:{view:0}, users:{view:0,add:0,edit:0,del:0}, permissions:{view:0,edit:0}, logs:{view:0} } },
    { id: 4, name: '商户',         permissions: { products:{view:1,add:1,edit:1,del:1}, orders:{view:1,add:0,edit:0,del:0,ship:1,void:1}, members:{view:1,add:0,edit:0,del:0}, stores:{view:1,add:1,edit:1,del:1}, marketing:{view:1,add:1,edit:1,del:1}, statistics:{view:1}, users:{view:0,add:0,edit:0,del:0}, permissions:{view:0,edit:0}, logs:{view:0} } },
  ];

  /* --- Logs --- */
  let _logs = [
    { id: 1, operator: 'admin',     type: '登录',   module: '系统管理', description: '管理员admin登录系统',                            ip: '192.168.1.100', time: '2025-06-24 08:00:00' },
    { id: 2, operator: 'admin',     type: '新增',   module: '商品管理', description: '新增商品 [红富士苹果]',                          ip: '192.168.1.100', time: '2025-06-24 09:15:00' },
    { id: 3, operator: 'merchant01',type: '修改',   module: '门店管理', description: '修改门店 [朝阳旗舰店] 营业时间',                  ip: '192.168.1.101', time: '2025-06-24 10:30:00' },
    { id: 4, operator: 'operator01',type: '删除',   module: '营销管理', description: '删除促销活动 [春季特卖]',                        ip: '192.168.1.102', time: '2025-06-23 16:45:00' },
    { id: 5, operator: 'admin',     type: '导出',   module: '数据统计', description: '导出6月份销售报表',                              ip: '192.168.1.100', time: '2025-06-23 14:20:00' },
    { id: 6, operator: 'service01', type: '修改',   module: '订单管理', description: '订单 [ORD20260602002] 标记为已发货',              ip: '192.168.1.104', time: '2025-06-23 11:10:00' },
    { id: 7, operator: 'merchant01',type: '新增',   module: '商品管理', description: '新增商品分类 [熟食烘焙]',                        ip: '192.168.1.101', time: '2025-06-22 09:00:00' },
    { id: 8, operator: 'operator01',type: '作废',   module: '订单管理', description: '作废订单 [ORD20260605005]',                      ip: '192.168.1.102', time: '2025-06-21 15:30:00' },
    { id: 9, operator: 'admin',     type: '新增',   module: '用户管理', description: '新增用户 [service01 - 张伟]',                    ip: '192.168.1.100', time: '2025-06-20 10:00:00' },
    { id: 10,operator: 'merchant01',type: '登录',   module: '系统管理', description: '商户merchant01登录系统',                         ip: '192.168.1.101', time: '2025-06-20 08:30:00' },
  ];

  /* ==========================================================
     BACKEND API CALLER
     Tries the real C++ backend first; falls back to mock data
     if the server is unreachable (e.g., opened directly from disk).
     ========================================================== */

  var _backendAvailable = true;   // set false on first connection failure
  var _backendBase     = '';      // same-origin when served by merchant_server

  function _callAPI(method, url, body, mockFn) {
    if (!_backendAvailable) {
      // Fast path: backend already known to be down
      return Promise.resolve().then(mockFn);
    }

    var opts = {
      method: method,
      headers: { 'Content-Type': 'application/json' }
    };
    if (body && method !== 'GET') {
      opts.body = (typeof body === 'string') ? body : JSON.stringify(body);
    }

    return fetch(_backendBase + url, opts).then(function (r) {
      // 安全读取 JSON — 空 body 或解析失败时返回 null
      return r.text().then(function (text) {
        if (!text || !text.trim()) {
          if (!r.ok) throw new Error('HTTP ' + r.status + ': 空响应');
          return null; // signal fallback below
        }
        try {
          return JSON.parse(text);
        } catch (e) {
          if (!r.ok) throw new Error('HTTP ' + r.status + ': ' + text.substring(0, 80));
          console.warn('[API] JSON 解析失败: ' + text.substring(0, 80));
          return null;
        }
      });
    }).then(function (json) {
      if (json === null) throw new Error('_FALLBACK_'); // signal fallback
      if (json.error) throw new Error(json.message || 'Unknown error');
      if (json.data !== undefined) {
        if (json.total !== undefined) {
          return { list: json.data, total: json.total, page: json.page, pageSize: json.pageSize };
        }
        return json.data;
      }
      return json;
    }).catch(function (err) {
      // Network error, empty response, or JSON parse failure → fall back to mock
      var msg = (err && err.message) ? err.message : '';
      if (err instanceof TypeError ||
          msg === 'Failed to fetch' ||
          msg === '_FALLBACK_' ||
          msg.indexOf('HTTP') === 0 ||
          (err && err.name === 'TypeError')) {
        if (_backendAvailable && msg !== '_FALLBACK_' && msg.indexOf('HTTP') !== 0) {
          console.warn('[API] 后端不可用, 切换到本地模拟数据');
          _backendAvailable = false;
        }
        return Promise.resolve().then(mockFn);
      }
      throw err;
    });
  }

  /* ==========================================================
     HELPERS
     ========================================================== */

  function delay(ms) {
    return new Promise(function (r) { setTimeout(r, ms || 150); });
  }

  function paginate(list, page, pageSize) {
    page = page || 1;
    pageSize = pageSize || 10;
    var total = list.length;
    var start = (page - 1) * pageSize;
    var end = start + pageSize;
    return { list: list.slice(start, end), total: total, page: page, pageSize: pageSize };
  }

  function toQuery(params) {
    var parts = [];
    for (var k in params) {
      if (params[k] !== undefined && params[k] !== null && params[k] !== '') {
        parts.push(encodeURIComponent(k) + '=' + encodeURIComponent(params[k]));
      }
    }
    return parts.length ? '?' + parts.join('&') : '';
  }

  /* ==========================================================
     API OBJECT
     ========================================================== */

  var API = {

    /* --- Auth --- */
    login: function (username, password) {
      return _callAPI('POST', '/api/login', JSON.stringify({
        username: username, password: password
      }), function () {
        if (!username || !password) throw new Error('用户名和密码不能为空');
        if (username === 'admin' && password === 'admin123')
          return { token: 'tok_admin_mock', user: { username: 'admin', realName: '系统管理员', role: 'admin', avatar: '👤' } };
        if (username === 'merchant' && password === 'merchant123')
          return { token: 'tok_merchant_mock', user: { username: 'merchant01', realName: '李明', role: 'merchant', avatar: '👤' } };
        throw new Error('用户名或密码错误');
      });
    },

    logout: function () {
      return _callAPI('POST', '/api/logout', '', function () { return { success: true }; });
    },

    /* ==========================================================
       PRODUCT CATEGORIES (商品分类)
       ========================================================== */

    getCategories: function () {
      return _callAPI('GET', '/api/merchant/categories', null, function () {
        return _categories.slice().sort(function (a, b) { return a.sort - b.sort; });
      });
    },

    createCategory: function (data) {
      return _callAPI('POST', '/api/merchant/categories', data, function () {
        var cat = { id: nextId(), name: data.name, description: data.description || '',
          sort: parseInt(data.sort) || 0, status: parseInt(data.status) || 1,
          createdAt: new Date().toISOString().replace('T', ' ').substring(0, 16) };
        _categories.push(cat); return cat;
      });
    },

    updateCategory: function (id, data) {
      return _callAPI('PUT', '/api/merchant/categories/' + id, data, function () {
        var idx = _categories.findIndex(function (c) { return c.id === id; });
        if (idx === -1) throw new Error('分类不存在');
        var cat = _categories[idx];
        if (data.name !== undefined) cat.name = data.name;
        if (data.description !== undefined) cat.description = data.description;
        if (data.sort !== undefined) cat.sort = parseInt(data.sort);
        if (data.status !== undefined) cat.status = parseInt(data.status);
        return cat;
      });
    },

    deleteCategory: function (id) {
      return _callAPI('DELETE', '/api/merchant/categories/' + id, null, function () {
        var idx = _categories.findIndex(function (c) { return c.id === id; });
        if (idx === -1) throw new Error('分类不存在');
        _categories.splice(idx, 1); return { success: true };
      });
    },

    /* ==========================================================
       PRODUCTS (商品信息)
       ========================================================== */

    getProducts: function (params) {
      return _callAPI('GET', '/api/merchant/products' + toQuery(params || {}), null, function () {
        var list = _products.slice(); var p = params || {};
        if (p.name) list = list.filter(function (item) { return item.name.indexOf(p.name) !== -1; });
        if (p.categoryId) list = list.filter(function (item) { return item.categoryId === parseInt(p.categoryId); });
        if (p.status !== undefined && p.status !== '' && p.status !== null) list = list.filter(function (item) { return item.status === parseInt(p.status); });
        list.sort(function (a, b) { return b.id - a.id; });
        return paginate(list, p.page, p.pageSize);
      });
    },

    createProduct: function (data) {
      return _callAPI('POST', '/api/merchant/products', data, function () {
        var prod = { id: nextId(), name: data.name, categoryId: parseInt(data.categoryId), image: data.image || '📦',
          description: data.description || '', price: parseFloat(data.price), stock: parseInt(data.stock) || 0,
          unit: data.unit || '个', status: parseInt(data.status) || 1,
          createdAt: new Date().toISOString().replace('T', ' ').substring(0, 16) };
        _products.push(prod); return prod;
      });
    },

    updateProduct: function (id, data) {
      return _callAPI('PUT', '/api/merchant/products/' + id, data, function () {
        var idx = _products.findIndex(function (p) { return p.id === id; });
        if (idx === -1) throw new Error('商品不存在');
        var prod = _products[idx];
        ['name','categoryId','image','description','price','stock','unit','status'].forEach(function (k) {
          if (data[k] !== undefined) {
            if (k === 'categoryId' || k === 'stock' || k === 'status') prod[k] = parseInt(data[k]);
            else if (k === 'price') prod[k] = parseFloat(data[k]); else prod[k] = data[k];
          }
        }); return prod;
      });
    },

    deleteProduct: function (id) {
      return _callAPI('DELETE', '/api/merchant/products/' + id, null, function () {
        var idx = _products.findIndex(function (p) { return p.id === id; });
        if (idx === -1) throw new Error('商品不存在');
        _products.splice(idx, 1); return { success: true };
      });
    },

    /* ==========================================================
       PROMOTIONS (促销管理)
       ========================================================== */

    getPromotions: function () {
      return _callAPI('GET', '/api/merchant/promotions', null, function () { return _promotions.slice().reverse(); });
    },
    createPromotion: function (data) {
      return _callAPI('POST', '/api/merchant/promotions', data, function () {
        var promo = { id: nextId(), name: data.name, type: data.type, startTime: data.startTime, endTime: data.endTime,
          discount: parseFloat(data.discount) || 0, description: data.description || '', status: '未开始',
          productIds: data.productIds || [] };
        _promotions.push(promo); return promo;
      });
    },
    updatePromotion: function (id, data) {
      return _callAPI('PUT', '/api/merchant/promotions/' + id, data, function () {
        var idx = _promotions.findIndex(function (p) { return p.id === id; });
        if (idx === -1) throw new Error('促销不存在');
        var promo = _promotions[idx];
        ['name','type','startTime','endTime','discount','description'].forEach(function (k) {
          if (data[k] !== undefined) { if (k === 'discount') promo[k] = parseFloat(data[k]); else promo[k] = data[k]; }
        });
        if (data.productIds !== undefined) promo.productIds = data.productIds; return promo;
      });
    },
    deletePromotion: function (id) {
      return _callAPI('DELETE', '/api/merchant/promotions/' + id, null, function () {
        var idx = _promotions.findIndex(function (p) { return p.id === id; });
        if (idx === -1) throw new Error('促销不存在');
        _promotions.splice(idx, 1); return { success: true };
      });
    },

    /* ==========================================================
       SERVICE AREAS (服务区域)
       ========================================================== */

    getServiceAreas: function () {
      return _callAPI('GET', '/api/merchant/service-areas', null, function () {
        return _serviceAreas.slice().sort(function (a, b) { return a.sort - b.sort; });
      });
    },
    createServiceArea: function (data) {
      return _callAPI('POST', '/api/merchant/service-areas', data, function () {
        var area = { id: nextId(), name: data.name, code: data.code || '', parentId: parseInt(data.parentId) || 0,
          sort: parseInt(data.sort) || 0, remark: data.remark || '',
          createdAt: new Date().toISOString().replace('T', ' ').substring(0, 16) };
        _serviceAreas.push(area); return area;
      });
    },
    updateServiceArea: function (id, data) {
      return _callAPI('PUT', '/api/merchant/service-areas/' + id, data, function () {
        var idx = _serviceAreas.findIndex(function (a) { return a.id === id; });
        if (idx === -1) throw new Error('区域不存在');
        var area = _serviceAreas[idx];
        ['name','code','parentId','sort','remark'].forEach(function (k) {
          if (data[k] !== undefined) { if (k === 'parentId' || k === 'sort') area[k] = parseInt(data[k]); else area[k] = data[k]; }
        }); return area;
      });
    },
    deleteServiceArea: function (id) {
      return _callAPI('DELETE', '/api/merchant/service-areas/' + id, null, function () {
        var idx = _serviceAreas.findIndex(function (a) { return a.id === id; });
        if (idx === -1) throw new Error('区域不存在');
        _serviceAreas.splice(idx, 1); return { success: true };
      });
    },

    /* ==========================================================
       STORES (门店管理)
       ========================================================== */

    getStores: function () {
      return _callAPI('GET', '/api/merchant/stores', null, function () { return _stores.slice().reverse(); });
    },
    createStore: function (data) {
      return _callAPI('POST', '/api/merchant/stores', data, function () {
        var store = { id: nextId(), name: data.name, areaId: parseInt(data.areaId), contact: data.contact || '',
          phone: data.phone || '', openTime: data.openTime || '08:00', closeTime: data.closeTime || '22:00',
          address: data.address || '', image: data.image || '🏪', description: data.description || '',
          status: parseInt(data.status) || 1, createdAt: new Date().toISOString().replace('T', ' ').substring(0, 16) };
        _stores.push(store); return store;
      });
    },
    updateStore: function (id, data) {
      return _callAPI('PUT', '/api/merchant/stores/' + id, data, function () {
        var idx = _stores.findIndex(function (s) { return s.id === id; });
        if (idx === -1) throw new Error('门店不存在');
        var store = _stores[idx];
        ['name','areaId','contact','phone','openTime','closeTime','address','image','description','status'].forEach(function (k) {
          if (data[k] !== undefined) { if (k === 'areaId' || k === 'status') store[k] = parseInt(data[k]); else store[k] = data[k]; }
        }); return store;
      });
    },
    deleteStore: function (id) {
      return _callAPI('DELETE', '/api/merchant/stores/' + id, null, function () {
        var idx = _stores.findIndex(function (s) { return s.id === id; });
        if (idx === -1) throw new Error('门店不存在');
        _stores.splice(idx, 1); return { success: true };
      });
    },

    /* ==========================================================
       STORE PRODUCTS (注册商品)
       ========================================================== */

    getStoreProducts: function (storeId) {
      return _callAPI('GET', '/api/merchant/store-products?storeId=' + storeId, null, function () {
        return _storeProducts.filter(function (sp) { return sp.storeId === parseInt(storeId); });
      });
    },
    bindProductToStore: function (storeId, bindings) {
      return _callAPI('POST', '/api/merchant/store-products/bind', { storeId: storeId, bindings: bindings }, function () {
        bindings.forEach(function (b) {
          var exists = _storeProducts.find(function (sp) { return sp.storeId === parseInt(storeId) && sp.productId === parseInt(b.productId); });
          if (!exists) { _storeProducts.push({ id: nextId(), storeId: parseInt(storeId), productId: parseInt(b.productId), stock: parseInt(b.stock) || 0, status: 1, bindTime: new Date().toISOString().replace('T', ' ').substring(0, 16) }); }
        }); return { success: true };
      });
    },
    updateStoreProductStatus: function (id, status) {
      return _callAPI('PUT', '/api/merchant/store-products/' + id + '/status', { status: status }, function () {
        var sp = _storeProducts.find(function (x) { return x.id === id; });
        if (!sp) throw new Error('记录不存在'); sp.status = parseInt(status); return sp;
      });
    },
    updateStoreProductStock: function (id, stock) {
      return _callAPI('PUT', '/api/merchant/store-products/' + id + '/stock', { stock: stock }, function () {
        var sp = _storeProducts.find(function (x) { return x.id === id; });
        if (!sp) throw new Error('记录不存在'); sp.stock = parseInt(stock); return sp;
      });
    },
    removeStoreProduct: function (id) {
      return _callAPI('DELETE', '/api/merchant/store-products/' + id, null, function () {
        var idx = _storeProducts.findIndex(function (x) { return x.id === id; });
        if (idx === -1) throw new Error('记录不存在');
        _storeProducts.splice(idx, 1); return { success: true };
      });
    },

    /* ==========================================================
       STATISTICS (数据统计)
       ========================================================== */

    getSalesRanking: function (params) {
      return _callAPI('GET', '/api/merchant/statistics/sales' + toQuery(params || {}), null, function () {
        var list = _products.map(function (p) { var sales = Math.floor(Math.random() * 2000) + 50;
          return { id: p.id, name: p.name, categoryId: p.categoryId, image: p.image, salesCount: sales, salesAmount: Math.round(sales * p.price * 100) / 100, avgPrice: p.price }; });
        list.sort(function (a, b) { return b.salesAmount - a.salesAmount; }); return list.slice(0, 10);
      });
    },
    getVisitRanking: function (params) {
      return _callAPI('GET', '/api/merchant/statistics/visits' + toQuery(params || {}), null, function () {
        var list = _products.map(function (p) { var visits = Math.floor(Math.random() * 5000) + 200;
          return { id: p.id, name: p.name, categoryId: p.categoryId, image: p.image, visitCount: visits,
            uniqueVisitors: Math.floor(visits * (0.5 + Math.random() * 0.4)), conversionRate: Math.round(Math.random() * 40 * 10) / 10 + '%' }; });
        list.sort(function (a, b) { return b.visitCount - a.visitCount; }); return list.slice(0, 10);
      });
    },

    /* ==========================================================
       ORDERS (订单管理)
       ========================================================== */

    getOrders: function (params) {
      return _callAPI('GET', '/api/merchant/orders' + toQuery(params || {}), null, function () {
        var list = _orders.slice(); var p = params || {};
        if (p.orderNo) list = list.filter(function (o) { return o.orderNo.indexOf(p.orderNo) !== -1; });
        if (p.status) list = list.filter(function (o) { return o.status === p.status; });
        if (p.userName) list = list.filter(function (o) { return o.userName.indexOf(p.userName) !== -1; });
        list.sort(function (a, b) { return b.id - a.id; }); return paginate(list, p.page, p.pageSize);
      });
    },
    getOrderDetail: function (id) {
      return _callAPI('GET', '/api/merchant/orders/' + id, null, function () {
        var order = _orders.find(function (o) { return o.id === id; });
        if (!order) throw new Error('订单不存在'); return JSON.parse(JSON.stringify(order));
      });
    },
    shipOrder: function (id, trackingInfo) {
      return _callAPI('POST', '/api/merchant/orders/' + id + '/ship', trackingInfo, function () {
        var order = _orders.find(function (o) { return o.id === id; });
        if (!order) throw new Error('订单不存在');
        if (order.status !== '待发货') throw new Error('只能对待发货订单执行发货操作');
        order.status = '已发货'; order.trackingNo = trackingInfo.trackingNo || ''; order.courier = trackingInfo.courier || '';
        order.timeline.push({ status: '已发货', time: new Date().toISOString().replace('T', ' ').substring(0, 16) }); return order;
      });
    },
    voidOrder: function (id) {
      return _callAPI('POST', '/api/merchant/orders/' + id + '/void', null, function () {
        var order = _orders.find(function (o) { return o.id === id; });
        if (!order) throw new Error('订单不存在');
        if (order.status !== '待付款' && order.status !== '待发货') throw new Error('只能对待付款或待发货订单执行作废操作');
        order.status = '已取消';
        order.timeline.push({ status: '已取消', time: new Date().toISOString().replace('T', ' ').substring(0, 16) }); return order;
      });
    },

    /* ==========================================================
       MEMBERS (会员管理)
       ========================================================== */

    getMembers: function (params) {
      return _callAPI('GET', '/api/merchant/members' + toQuery(params || {}), null, function () {
        var list = _members.slice(); var p = params || {};
        if (p.keyword) { var kw = p.keyword; list = list.filter(function (m) { return m.username.indexOf(kw) !== -1 || m.phone.indexOf(kw) !== -1; }); }
        list.sort(function (a, b) { return b.id - a.id; }); return paginate(list, p.page, p.pageSize);
      });
    },
    getMemberDetail: function (id) {
      return _callAPI('GET', '/api/merchant/members/' + id, null, function () {
        var member = _members.find(function (m) { return m.id === id; });
        if (!member) throw new Error('会员不存在'); return JSON.parse(JSON.stringify(member));
      });
    },

    /* ==========================================================
       USERS (用户管理 — admin only)
       ========================================================== */

    getUsers: function (params) {
      return _callAPI('GET', '/api/admin/users' + toQuery(params || {}), null, function () { return _users.slice(); });
    },
    createUser: function (data) {
      return _callAPI('POST', '/api/admin/users', data, function () {
        var user = { id: nextId(), username: data.username, realName: data.realName || '', role: data.role,
          phone: data.phone || '', email: data.email || '', status: parseInt(data.status) || 1,
          createdAt: new Date().toISOString().replace('T', ' ').substring(0, 16) };
        _users.push(user); return user;
      });
    },
    updateUser: function (id, data) {
      return _callAPI('PUT', '/api/admin/users/' + id, data, function () {
        var idx = _users.findIndex(function (u) { return u.id === id; });
        if (idx === -1) throw new Error('用户不存在');
        var user = _users[idx];
        ['username','realName','role','phone','email','status'].forEach(function (k) { if (data[k] !== undefined) user[k] = data[k]; });
        if (data.status !== undefined) user.status = parseInt(data.status); return user;
      });
    },
    deleteUser: function (id) {
      return _callAPI('DELETE', '/api/admin/users/' + id, null, function () {
        var idx = _users.findIndex(function (u) { return u.id === id; });
        if (idx === -1) throw new Error('用户不存在');
        _users.splice(idx, 1); return { success: true };
      });
    },
    resetUserPassword: function (id) {
      return _callAPI('POST', '/api/admin/users/' + id + '/reset-pwd', null, function () {
        var user = _users.find(function (u) { return u.id === id; });
        if (!user) throw new Error('用户不存在');
        return { success: true, message: '密码已重置为默认密码: 123456' };
      });
    },

    /* ==========================================================
       PERMISSIONS (权限管理 — admin only)
       ========================================================== */

    getRoles: function () {
      return _callAPI('GET', '/api/admin/roles', null, function () { return _roles.slice(); });
    },
    updateRolePermissions: function (roleId, permissions) {
      return _callAPI('PUT', '/api/admin/roles/' + roleId, { permissions: permissions }, function () {
        var role = _roles.find(function (r) { return r.id === roleId; });
        if (!role) throw new Error('角色不存在');
        role.permissions = permissions; return role;
      });
    },

    /* ==========================================================
       LOGS (操作日志 — admin only)
       ========================================================== */

    getLogs: function (params) {
      return _callAPI('GET', '/api/admin/operation-logs' + toQuery(params || {}), null, function () {
        var list = _logs.slice(); var p = params || {};
        if (p.operator) list = list.filter(function (l) { return l.operator.indexOf(p.operator) !== -1; });
        if (p.type) list = list.filter(function (l) { return l.type === p.type; });
        list.sort(function (a, b) { return b.id - a.id; }); return paginate(list, p.page, p.pageSize);
      });
    },

    /* ==========================================================
       LOOKUP HELPERS — resolve IDs to names for display
       ========================================================== */

    getCategoryName: function (categoryId) {
      var cat = _categories.find(function (c) { return c.id === parseInt(categoryId); });
      return cat ? cat.name : '未知分类';
    },

    getAreaName: function (areaId) {
      var area = _serviceAreas.find(function (a) { return a.id === parseInt(areaId); });
      return area ? area.name : '未知区域';
    },

    getStoreName: function (storeId) {
      var store = _stores.find(function (s) { return s.id === parseInt(storeId); });
      return store ? store.name : '未知门店';
    },

    getProductName: function (productId) {
      var prod = _products.find(function (p) { return p.id === parseInt(productId); });
      return prod ? prod.name : '未知商品';
    }
  };

  /* ==========================================================
     EXPORT
     ========================================================== */

  window.App = window.App || {};
  window.App.API = API;
})();
