/* ============================================================
   商户管理系统 — Merchant Panel Logic
   Provides: window.App.Merchant
   All data ops go through window.App.API.
   UI helpers (showModal, showToast, showConfirm) from window.App.UI.
   ============================================================ */

(function () {
  'use strict';

  var Merchant = {};

  /* ==========================================================
     HELPERS
     ========================================================== */

  function fmtMoney(val) {
    return '¥' + parseFloat(val || 0).toFixed(2);
  }

  function statusBadge(status, labels) {
    labels = labels || { 1: ['启用', 'badge-success'], 0: ['禁用', 'badge-default'] };
    var pair = labels[status] || ['未知', 'badge-default'];
    return '<span class="badge ' + pair[1] + '">' + pair[0] + '</span>';
  }

  function buildSelectOptions(list, valueKey, labelKey, selectedVal, placeholder) {
    var html = placeholder ? '<option value="">' + placeholder + '</option>' : '';
    for (var i = 0; i < list.length; i++) {
      var v = valueKey ? list[i][valueKey] : list[i];
      var l = labelKey ? list[i][labelKey] : list[i];
      var sel = (String(v) === String(selectedVal)) ? ' selected' : '';
      html += '<option value="' + v + '"' + sel + '>' + l + '</option>';
    }
    return html;
  }

  /* ==========================================================
     1. CATEGORIES (商品分类)
     ========================================================== */
  Merchant.Categories = (function () {
    var self = {};

    // Load categories from API into given container
    self.loadData = function (container) {
      var tableBody = container.querySelector('.js-cat-tbody');
      API.getCategories().then(function (list) {
        var rows = '';
        for (var i = 0; i < list.length; i++) {
          var c = list[i];
          rows += '<tr>'
            + '<td class="cell-number">' + (i + 1) + '</td>'
            + '<td>' + esc(c.name) + '</td>'
            + '<td>' + esc(c.description || '-') + '</td>'
            + '<td class="cell-number">' + c.sort + '</td>'
            + '<td>' + statusBadge(c.status, { 1: ['启用', 'badge-success'], 0: ['禁用', 'badge-default'] }) + '</td>'
            + '<td>' + (c.createdAt || '-') + '</td>'
            + '<td class="cell-actions">'
              + '<a href="javascript:void(0)" class="action-link js-edit" data-id="' + c.id + '">编辑</a>'
              + '<a href="javascript:void(0)" class="action-link action-link-danger js-delete" data-id="' + c.id + '">删除</a>'
            + '</td>'
            + '</tr>';
        }
        if (!list.length) {
          rows = '<tr><td colspan="7" style="text-align:center;padding:32px;color:#bfbfbf;">暂无分类数据</td></tr>';
        }
        tableBody.innerHTML = rows;
        bindEvents(container);
      }).catch(function (err) {
        UI.showToast(err.message || '加载分类失败', 'error');
      });
    };

    function bindEvents(container) {
      var edits = container.querySelectorAll('.js-edit');
      for (var i = 0; i < edits.length; i++) {
        edits[i].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          API.getCategories().then(function (list) {
            var cat = list.find(function (x) { return x.id === id; });
            if (cat) self.showEditModal(cat, container);
          });
        };
      }
      var dels = container.querySelectorAll('.js-delete');
      for (var j = 0; j < dels.length; j++) {
        dels[j].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          self.deleteCategory(id, container);
        };
      }
    }

    self.showAddModal = function (container) {
      var body = ''
        + '<div class="form-row"><label class="form-label">分类名称 <span style="color:#ff4d4f">*</span></label>'
        + '<input class="form-input" id="catName" placeholder="请输入分类名称">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">分类描述</label>'
        + '<textarea class="form-input" id="catDesc" rows="3" placeholder="请输入分类描述"></textarea>'
        + '</div>'
        + '<div class="form-row"><label class="form-label">排序值</label>'
        + '<input class="form-input" id="catSort" type="number" value="0" placeholder="数字越小越靠前">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">状态</label>'
        + '<label class="radio-label"><input type="radio" name="catStatus" value="1" checked> 启用</label>'
        + '<label class="radio-label" style="margin-left:16px"><input type="radio" name="catStatus" value="0"> 禁用</label>'
        + '</div>';
      UI.showModal('新增分类', body, function () {
        var name = document.getElementById('catName').value.trim();
        if (!name) { UI.showToast('请输入分类名称', 'error'); return false; }
        var statusRadio = document.querySelector('input[name="catStatus"]:checked');
        var data = {
          name: name,
          description: document.getElementById('catDesc').value.trim(),
          sort: parseInt(document.getElementById('catSort').value) || 0,
          status: statusRadio ? parseInt(statusRadio.value) : 1
        };
        API.createCategory(data).then(function () {
          UI.showToast('分类创建成功', 'success');
          self.loadData(container);
        }).catch(function (err) {
          UI.showToast(err.message || '创建失败', 'error');
        });
        return true;
      });
    };

    self.showEditModal = function (cat, container) {
      var body = ''
        + '<div class="form-row"><label class="form-label">分类名称 <span style="color:#ff4d4f">*</span></label>'
        + '<input class="form-input" id="catName" value="' + escAttr(cat.name) + '" placeholder="请输入分类名称">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">分类描述</label>'
        + '<textarea class="form-input" id="catDesc" rows="3" placeholder="请输入分类描述">' + esc(cat.description || '') + '</textarea>'
        + '</div>'
        + '<div class="form-row"><label class="form-label">排序值</label>'
        + '<input class="form-input" id="catSort" type="number" value="' + (cat.sort || 0) + '" placeholder="数字越小越靠前">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">状态</label>'
        + '<label class="radio-label"><input type="radio" name="catStatus" value="1"' + (cat.status === 1 ? ' checked' : '') + '> 启用</label>'
        + '<label class="radio-label" style="margin-left:16px"><input type="radio" name="catStatus" value="0"' + (cat.status === 0 ? ' checked' : '') + '> 禁用</label>'
        + '</div>';
      UI.showModal('编辑分类', body, function () {
        var name = document.getElementById('catName').value.trim();
        if (!name) { UI.showToast('请输入分类名称', 'error'); return false; }
        var statusRadio = document.querySelector('input[name="catStatus"]:checked');
        var data = {
          name: name,
          description: document.getElementById('catDesc').value.trim(),
          sort: parseInt(document.getElementById('catSort').value) || 0,
          status: statusRadio ? parseInt(statusRadio.value) : 1
        };
        API.updateCategory(cat.id, data).then(function () {
          UI.showToast('分类更新成功', 'success');
          self.loadData(container);
        }).catch(function (err) {
          UI.showToast(err.message || '更新失败', 'error');
        });
        return true;
      });
    };

    self.deleteCategory = function (id, container) {
      UI.showConfirm('确定要删除该分类吗？删除后不可恢复。').then(function (confirmed) {
        if (!confirmed) return;
        API.deleteCategory(id).then(function () {
          UI.showToast('分类删除成功', 'success');
          self.loadData(container);
        }).catch(function (err) {
          UI.showToast(err.message || '删除失败', 'error');
        });
      });
    };

    self.render = function (container) {
      container.innerHTML = ''
        + '<div class="content-header">'
          + '<div><h2 class="content-title">商品分类</h2><p class="content-subtitle">管理商品的分类体系，用于商品归类与展示</p></div>'
          + '<div><button class="btn btn-primary js-add-cat">新增分类</button></div>'
        + '</div>'
        + '<div class="table-container">'
          + '<table class="data-table">'
            + '<thead><tr><th>序号</th><th>分类名称</th><th>分类描述</th><th>排序</th><th>状态</th><th>创建时间</th><th>操作</th></tr></thead>'
            + '<tbody class="js-cat-tbody"></tbody>'
          + '</table>'
        + '</div>';

      container.querySelector('.js-add-cat').onclick = function () {
        self.showAddModal(container);
      };
      self.loadData(container);
    };

    return self;
  })();

  /* ==========================================================
     2. PRODUCTS (商品信息管理)
     ========================================================== */
  Merchant.Products = (function () {
    var self = {};

    self.loadData = function (container, filters) {
      filters = filters || {};
      var tbody = container.querySelector('.js-prod-tbody');
      API.getProducts({ name: filters.name || '', categoryId: filters.categoryId || '', status: filters.status, page: 1, pageSize: 1000 }).then(function (result) {
        var list = result.list || result;
        var rows = '';
        for (var i = 0; i < list.length; i++) {
          var p = list[i];
          var catName = API.getCategoryName(p.categoryId);
          rows += '<tr>'
            + '<td class="cell-number">' + (i + 1) + '</td>'
            + '<td class="cell-image"><span style="font-size:24px">' + esc(p.image || '📦') + '</span></td>'
            + '<td class="cell-name">' + esc(p.name) + '</td>'
            + '<td>' + esc(catName) + '</td>'
            + '<td class="cell-number">' + fmtMoney(p.price) + '</td>'
            + '<td class="cell-number">' + p.stock + '</td>'
            + '<td>' + statusBadge(p.status, { 1: ['上架', 'badge-success'], 0: ['下架', 'badge-default'] }) + '</td>'
            + '<td>' + (p.createdAt || '-') + '</td>'
            + '<td class="cell-actions">'
              + '<a href="javascript:void(0)" class="action-link js-edit-prod" data-id="' + p.id + '">编辑</a>'
              + '<a href="javascript:void(0)" class="action-link action-link-danger js-del-prod" data-id="' + p.id + '">删除</a>'
            + '</td>'
            + '</tr>';
        }
        if (!list.length) {
          rows = '<tr><td colspan="9" style="text-align:center;padding:32px;color:#bfbfbf;">暂无商品数据</td></tr>';
        }
        tbody.innerHTML = rows;
        bindProductEvents(container);
      }).catch(function (err) {
        UI.showToast(err.message || '加载商品失败', 'error');
      });
    };

    function bindProductEvents(container) {
      var edits = container.querySelectorAll('.js-edit-prod');
      for (var i = 0; i < edits.length; i++) {
        edits[i].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          API.getProducts({ page: 1, pageSize: 1000 }).then(function (result) {
            var list = result.list || result;
            var prod = list.find(function (x) { return x.id === id; });
            if (prod) self.showEditModal(prod, container);
          });
        };
      }
      var dels = container.querySelectorAll('.js-del-prod');
      for (var j = 0; j < dels.length; j++) {
        dels[j].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          UI.showConfirm('确定要删除该商品吗？删除后不可恢复。').then(function (confirmed) {
            if (!confirmed) return;
            API.deleteProduct(id).then(function () {
              UI.showToast('商品删除成功', 'success');
              self.loadData(container, getCurrentFilters(container));
            }).catch(function (err) {
              UI.showToast(err.message || '删除失败', 'error');
            });
          });
        };
      }
    }

    function getCurrentFilters(container) {
      return {
        name: (container.querySelector('#prodFilterName') || {}).value || '',
        categoryId: (container.querySelector('#prodFilterCat') || {}).value || '',
        status: (container.querySelector('#prodFilterStatus') || {}).value || ''
      };
    }

    function buildProductForm(prod) {
      var isEdit = !!prod;
      return ''
        + '<div class="form-row"><label class="form-label">商品名称 <span style="color:#ff4d4f">*</span></label>'
        + '<input class="form-input" id="prodName" value="' + escAttr(prod ? prod.name : '') + '" placeholder="请输入商品名称">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">所属分类 <span style="color:#ff4d4f">*</span></label>'
        + '<select class="form-select" id="prodCat"></select>'
        + '</div>'
        + '<div class="form-row"><label class="form-label">商品图片</label>'
        + '<input class="form-input" id="prodImage" value="' + escAttr(prod ? prod.image || '' : '') + '" placeholder="emoji表情或图片URL">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">商品描述</label>'
        + '<textarea class="form-input" id="prodDesc" rows="3" placeholder="请输入商品描述">' + esc(prod ? prod.description || '' : '') + '</textarea>'
        + '</div>'
        + '<div class="form-row"><label class="form-label">单价 <span style="color:#ff4d4f">*</span></label>'
        + '<input class="form-input" id="prodPrice" type="number" step="0.01" value="' + (prod ? prod.price : '') + '" placeholder="请输入单价">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">库存数量</label>'
        + '<input class="form-input" id="prodStock" type="number" value="' + (prod ? prod.stock : '0') + '" placeholder="请输入库存数量">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">单位</label>'
        + '<input class="form-input" id="prodUnit" value="' + escAttr(prod ? prod.unit || '' : '') + '" placeholder="如：斤、个、箱">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">状态</label>'
        + '<label class="radio-label"><input type="radio" name="prodStatus" value="1"' + (prod && prod.status !== 1 ? '' : ' checked') + '> 上架</label>'
        + '<label class="radio-label" style="margin-left:16px"><input type="radio" name="prodStatus" value="0"' + (prod && prod.status === 0 ? ' checked' : '') + '> 下架</label>'
        + '</div>';
    }

    self.showAddModal = function (container) {
      var body = buildProductForm(null);
      UI.showModal('新增商品', body, function () {
        return saveProduct(null, container);
      });
      // Populate category dropdown
      API.getCategories().then(function (cats) {
        var sel = document.getElementById('prodCat');
        if (sel) sel.innerHTML = buildSelectOptions(cats, 'id', 'name', '', '请选择分类');
      });
    };

    self.showEditModal = function (prod, container) {
      var body = buildProductForm(prod);
      UI.showModal('编辑商品', body, function () {
        return saveProduct(prod.id, container);
      });
      API.getCategories().then(function (cats) {
        var sel = document.getElementById('prodCat');
        if (sel) sel.innerHTML = buildSelectOptions(cats, 'id', 'name', prod.categoryId, '请选择分类');
      });
    };

    function saveProduct(id, container) {
      var name = document.getElementById('prodName').value.trim();
      if (!name) { UI.showToast('请输入商品名称', 'error'); return false; }
      var categoryId = document.getElementById('prodCat').value;
      if (!categoryId) { UI.showToast('请选择所属分类', 'error'); return false; }
      var price = parseFloat(document.getElementById('prodPrice').value);
      if (isNaN(price) || price < 0) { UI.showToast('请输入有效单价', 'error'); return false; }
      var statusRadio = document.querySelector('input[name="prodStatus"]:checked');
      var data = {
        name: name,
        categoryId: parseInt(categoryId),
        image: document.getElementById('prodImage').value.trim(),
        description: document.getElementById('prodDesc').value.trim(),
        price: price,
        stock: parseInt(document.getElementById('prodStock').value) || 0,
        unit: document.getElementById('prodUnit').value.trim(),
        status: statusRadio ? parseInt(statusRadio.value) : 1
      };
      var promise = id ? API.updateProduct(id, data) : API.createProduct(data);
      promise.then(function () {
        UI.showToast(id ? '商品更新成功' : '商品创建成功', 'success');
        self.loadData(container, getCurrentFilters(container));
      }).catch(function (err) {
        UI.showToast(err.message || '保存失败', 'error');
      });
      return true;
    }

    self.render = function (container) {
      container.innerHTML = ''
        + '<div class="content-header">'
          + '<div><h2 class="content-title">商品信息管理</h2><p class="content-subtitle">管理所有商品信息，包括名称、分类、价格、库存等</p></div>'
          + '<div><button class="btn btn-primary js-add-prod">新增商品</button></div>'
        + '</div>'
        + '<div class="filter-bar">'
          + '<div class="form-group">'
            + '<label class="form-label">商品名称</label>'
            + '<input class="form-input" id="prodFilterName" placeholder="请输入商品名称">'
          + '</div>'
          + '<div class="form-group">'
            + '<label class="form-label">所属分类</label>'
            + '<select class="form-select" id="prodFilterCat"><option value="">全部分类</option></select>'
          + '</div>'
          + '<div class="form-group">'
            + '<label class="form-label">状态</label>'
            + '<select class="form-select" id="prodFilterStatus">'
              + '<option value="">全部</option>'
              + '<option value="1">上架</option>'
              + '<option value="0">下架</option>'
            + '</select>'
          + '</div>'
          + '<div class="form-group" style="align-self:flex-end">'
            + '<button class="btn btn-primary js-prod-search">查询</button>'
            + '<button class="btn btn-default js-prod-reset" style="margin-left:8px">重置</button>'
          + '</div>'
        + '</div>'
        + '<div class="table-container">'
          + '<table class="data-table">'
            + '<thead><tr>'
              + '<th>序号</th><th>商品图片</th><th>商品名称</th><th>所属分类</th>'
              + '<th>价格</th><th>库存</th><th>状态</th><th>创建时间</th><th>操作</th>'
            + '</tr></thead>'
            + '<tbody class="js-prod-tbody"></tbody>'
          + '</table>'
        + '</div>';

      // Populate category filter dropdown
      var catSel = container.querySelector('#prodFilterCat');
      API.getCategories().then(function (cats) {
        catSel.innerHTML = '<option value="">全部分类</option>' + buildSelectOptions(cats, 'id', 'name');
      });

      container.querySelector('.js-add-prod').onclick = function () {
        self.showAddModal(container);
      };
      container.querySelector('.js-prod-search').onclick = function () {
        self.loadData(container, getCurrentFilters(container));
      };
      container.querySelector('.js-prod-reset').onclick = function () {
        container.querySelector('#prodFilterName').value = '';
        container.querySelector('#prodFilterCat').value = '';
        container.querySelector('#prodFilterStatus').value = '';
        self.loadData(container, {});
      };

      self.loadData(container);
    };

    return self;
  })();

  /* ==========================================================
     3. PROMOTIONS (促销管理)
     ========================================================== */
  Merchant.Promotions = (function () {
    var self = {};

    var statusLabelMap = {
      '进行中': ['进行中', 'badge-success'],
      '未开始': ['未开始', 'badge-primary'],
      '已结束': ['已结束', 'badge-default']
    };

    self.loadData = function (container) {
      var tbody = container.querySelector('.js-promo-tbody');
      API.getPromotions().then(function (list) {
        var rows = '';
        for (var i = 0; i < list.length; i++) {
          var p = list[i];
          var discountDisplay = p.type === '限时折扣' ? (p.discount || '0') : '0';
          rows += '<tr>'
            + '<td class="cell-number">' + (i + 1) + '</td>'
            + '<td>' + esc(p.name) + '</td>'
            + '<td>' + esc(p.type) + '</td>'
            + '<td>' + (p.startTime || '-') + '</td>'
            + '<td>' + (p.endTime || '-') + '</td>'
            + '<td class="cell-number">' + discountDisplay + '</td>'
            + '<td>' + statusBadgeCustom(p.status, statusLabelMap) + '</td>'
            + '<td class="cell-actions">'
              + '<a href="javascript:void(0)" class="action-link js-edit-promo" data-id="' + p.id + '">编辑</a>'
              + '<a href="javascript:void(0)" class="action-link action-link-danger js-del-promo" data-id="' + p.id + '">删除</a>'
              + '<a href="javascript:void(0)" class="action-link js-bind-promo" data-id="' + p.id + '">绑定商品</a>'
            + '</td>'
            + '</tr>';
        }
        if (!list.length) {
          rows = '<tr><td colspan="8" style="text-align:center;padding:32px;color:#bfbfbf;">暂无促销活动</td></tr>';
        }
        tbody.innerHTML = rows;
        bindPromoEvents(container);
      }).catch(function (err) {
        UI.showToast(err.message || '加载促销失败', 'error');
      });
    };

    function bindPromoEvents(container) {
      var edits = container.querySelectorAll('.js-edit-promo');
      for (var i = 0; i < edits.length; i++) {
        edits[i].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          API.getPromotions().then(function (list) {
            var promo = list.find(function (x) { return x.id === id; });
            if (promo) self.showEditModal(promo, container);
          });
        };
      }
      var dels = container.querySelectorAll('.js-del-promo');
      for (var j = 0; j < dels.length; j++) {
        dels[j].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          UI.showConfirm('确定要删除该促销活动吗？').then(function (confirmed) {
            if (!confirmed) return;
            API.deletePromotion(id).then(function () {
              UI.showToast('促销活动删除成功', 'success');
              self.loadData(container);
            }).catch(function (err) {
              UI.showToast(err.message || '删除失败', 'error');
            });
          });
        };
      }
      var binds = container.querySelectorAll('.js-bind-promo');
      for (var k = 0; k < binds.length; k++) {
        binds[k].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          API.getPromotions().then(function (list) {
            var promo = list.find(function (x) { return x.id === id; });
            if (promo) self.showBindProductsModal(promo, container);
          });
        };
      }
    }

    function buildPromoForm(promo) {
      var isEdit = !!promo;
      return ''
        + '<div class="form-row"><label class="form-label">促销名称 <span style="color:#ff4d4f">*</span></label>'
        + '<input class="form-input" id="promoName" value="' + escAttr(promo ? promo.name : '') + '" placeholder="请输入促销名称">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">促销类型 <span style="color:#ff4d4f">*</span></label>'
        + '<select class="form-select" id="promoType">'
          + '<option value="限时折扣"' + (promo && promo.type === '限时折扣' ? ' selected' : '') + '>限时折扣</option>'
          + '<option value="满减"' + (promo && promo.type === '满减' ? ' selected' : '') + '>满减</option>'
          + '<option value="买赠"' + (promo && promo.type === '买赠' ? ' selected' : '') + '>买赠</option>'
          + '<option value="优惠券"' + (promo && promo.type === '优惠券' ? ' selected' : '') + '>优惠券</option>'
        + '</select>'
        + '</div>'
        + '<div class="form-row"><label class="form-label">开始时间</label>'
        + '<input class="form-input" id="promoStart" type="datetime-local" value="' + (promo ? (promo.startTime || '').replace(' ', 'T') : '') + '">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">结束时间</label>'
        + '<input class="form-input" id="promoEnd" type="datetime-local" value="' + (promo ? (promo.endTime || '').replace(' ', 'T') : '') + '">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">折扣值 (1-10)</label>'
        + '<input class="form-input" id="promoDiscount" type="number" step="0.1" min="1" max="10" value="' + (promo ? promo.discount || '' : '') + '" placeholder="例：8 表示8折">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">促销描述</label>'
        + '<textarea class="form-input" id="promoDesc" rows="3" placeholder="请输入促销描述">' + esc(promo ? promo.description || '' : '') + '</textarea>'
        + '</div>';
    }

    self.showAddModal = function (container) {
      var body = buildPromoForm(null);
      UI.showModal('新增促销', body, function () {
        return savePromotion(null, container);
      });
    };

    self.showEditModal = function (promo, container) {
      var body = buildPromoForm(promo);
      UI.showModal('编辑促销', body, function () {
        return savePromotion(promo.id, container);
      });
    };

    function savePromotion(id, container) {
      var name = document.getElementById('promoName').value.trim();
      if (!name) { UI.showToast('请输入促销名称', 'error'); return false; }
      var type = document.getElementById('promoType').value;
      if (!type) { UI.showToast('请选择促销类型', 'error'); return false; }
      var startVal = document.getElementById('promoStart').value;
      var endVal = document.getElementById('promoEnd').value;
      var data = {
        name: name,
        type: type,
        startTime: startVal ? startVal.replace('T', ' ') + ':00' : '',
        endTime: endVal ? endVal.replace('T', ' ') + ':00' : '',
        discount: parseFloat(document.getElementById('promoDiscount').value) || 0,
        description: document.getElementById('promoDesc').value.trim()
      };
      var promise = id ? API.updatePromotion(id, data) : API.createPromotion(data);
      promise.then(function () {
        UI.showToast(id ? '促销更新成功' : '促销创建成功', 'success');
        self.loadData(container);
      }).catch(function (err) {
        UI.showToast(err.message || '保存失败', 'error');
      });
      return true;
    }

    self.showBindProductsModal = function (promo, container) {
      API.getProducts({ page: 1, pageSize: 1000 }).then(function (result) {
        var allProducts = result.list || result;
        var boundIds = promo.productIds || [];
        var available = allProducts.filter(function (p) { return boundIds.indexOf(p.id) === -1; });
        var bound = allProducts.filter(function (p) { return boundIds.indexOf(p.id) !== -1; });

        var selectedAvail = [];

        function buildLeft() {
          var html = '';
          for (var i = 0; i < available.length; i++) {
            var p = available[i];
            html += '<div class="dual-list-item"><label><input type="checkbox" class="js-avail-check" value="' + p.id + '"> ' + esc(p.name) + ' (' + esc(API.getCategoryName(p.categoryId)) + ')</label></div>';
          }
          if (!available.length) html = '<div class="dual-list-item" style="color:#bfbfbf;text-align:center;">暂无可用商品</div>';
          return html;
        }

        function buildRight() {
          var html = '';
          for (var i = 0; i < bound.length; i++) {
            var p = bound[i];
            html += '<div class="dual-list-item"><label><input type="checkbox" class="js-bound-check" value="' + p.id + '"> ' + esc(p.name) + ' (' + esc(API.getCategoryName(p.categoryId)) + ')</label></div>';
          }
          if (!bound.length) html = '<div class="dual-list-item" style="color:#bfbfbf;text-align:center;">暂未绑定商品</div>';
          return html;
        }

        var body = '<div class="dual-list">'
          + '<div class="dual-list-panel">'
            + '<div class="dual-list-header">可选商品</div>'
            + '<div class="dual-list-body js-left-body">' + buildLeft() + '</div>'
          + '</div>'
          + '<div style="display:flex;flex-direction:column;justify-content:center;gap:8px;padding:0 12px;">'
            + '<button class="btn btn-primary btn-sm js-move-right">→ 绑定</button>'
            + '<button class="btn btn-default btn-sm js-move-left">← 解绑</button>'
          + '</div>'
          + '<div class="dual-list-panel">'
            + '<div class="dual-list-header">已绑定商品</div>'
            + '<div class="dual-list-body js-right-body">' + buildRight() + '</div>'
          + '</div>'
          + '</div>';

        UI.showModal('绑定商品 - ' + esc(promo.name), body, function () {
          var checks = document.querySelectorAll('.js-bound-check');
          var newBoundIds = [];
          for (var i = 0; i < checks.length; i++) {
            newBoundIds.push(parseInt(checks[i].value));
          }
          // Also include the items that weren't checkbox-removed from bound
          return API.updatePromotion(promo.id, { productIds: newBoundIds }).then(function () {
            UI.showToast('商品绑定更新成功', 'success');
            self.loadData(container);
          }).catch(function (err) {
            UI.showToast(err.message || '绑定失败', 'error');
          }).then(function () { return true; });
        });

        // Bind transfer buttons
        setTimeout(function () {
          var leftBody = document.querySelector('.js-left-body');
          var rightBody = document.querySelector('.js-right-body');
          var moveRight = document.querySelector('.js-move-right');
          var moveLeft = document.querySelector('.js-move-left');

          if (moveRight) moveRight.onclick = function () {
            var checks = leftBody.querySelectorAll('.js-avail-check:checked');
            for (var i = 0; i < checks.length; i++) {
              var pid = parseInt(checks[i].value);
              var idx = available.findIndex(function (x) { return x.id === pid; });
              if (idx !== -1) {
                var item = available.splice(idx, 1)[0];
                bound.push(item);
              }
            }
            leftBody.innerHTML = buildLeft();
            rightBody.innerHTML = buildRight();
            rebindCheckEvents(leftBody, rightBody);
          };

          if (moveLeft) moveLeft.onclick = function () {
            var checks = rightBody.querySelectorAll('.js-bound-check:checked');
            for (var i = 0; i < checks.length; i++) {
              var pid = parseInt(checks[i].value);
              var idx = bound.findIndex(function (x) { return x.id === pid; });
              if (idx !== -1) {
                var item = bound.splice(idx, 1)[0];
                available.push(item);
              }
            }
            leftBody.innerHTML = buildLeft();
            rightBody.innerHTML = buildRight();
            rebindCheckEvents(leftBody, rightBody);
          };
        }, 100);
      });
    };

    function rebindCheckEvents(leftBody, rightBody) {
      // Re-bind after DOM refresh — no additional work needed since inline onclick not used
    }

    self.render = function (container) {
      container.innerHTML = ''
        + '<div class="content-header">'
          + '<div><h2 class="content-title">促销管理</h2><p class="content-subtitle">管理限时折扣、满减、买赠、优惠券等促销活动</p></div>'
          + '<div><button class="btn btn-primary js-add-promo">新增促销</button></div>'
        + '</div>'
        + '<div class="table-container">'
          + '<table class="data-table">'
            + '<thead><tr>'
              + '<th>序号</th><th>促销名称</th><th>促销类型</th><th>开始时间</th><th>结束时间</th>'
              + '<th>折扣</th><th>状态</th><th>操作</th>'
            + '</tr></thead>'
            + '<tbody class="js-promo-tbody"></tbody>'
          + '</table>'
        + '</div>';

      container.querySelector('.js-add-promo').onclick = function () {
        self.showAddModal(container);
      };
      self.loadData(container);
    };

    return self;
  })();

  /* ==========================================================
     4. STORES (门店管理)
     ========================================================== */
  Merchant.Stores = (function () {
    var self = {};

    self.loadData = function (container) {
      var tbody = container.querySelector('.js-store-tbody');
      API.getStores().then(function (list) {
        var rows = '';
        for (var i = 0; i < list.length; i++) {
          var s = list[i];
          var areaName = API.getAreaName(s.areaId);
          rows += '<tr>'
            + '<td class="cell-number">' + (i + 1) + '</td>'
            + '<td>' + esc(s.name) + '</td>'
            + '<td>' + esc(areaName) + '</td>'
            + '<td>' + esc(s.openTime) + '-' + esc(s.closeTime) + '</td>'
            + '<td>' + esc(s.phone || '-') + '</td>'
            + '<td>' + esc(s.address || '-') + '</td>'
            + '<td>' + statusBadge(s.status, { 1: ['营业中', 'badge-success'], 0: ['已关闭', 'badge-default'] }) + '</td>'
            + '<td class="cell-actions">'
              + '<a href="javascript:void(0)" class="action-link js-edit-store" data-id="' + s.id + '">编辑</a>'
              + '<a href="javascript:void(0)" class="action-link action-link-danger js-del-store" data-id="' + s.id + '">删除</a>'
            + '</td>'
            + '</tr>';
        }
        if (!list.length) {
          rows = '<tr><td colspan="8" style="text-align:center;padding:32px;color:#bfbfbf;">暂无门店数据</td></tr>';
        }
        tbody.innerHTML = rows;
        bindStoreEvents(container);
      }).catch(function (err) {
        UI.showToast(err.message || '加载门店失败', 'error');
      });
    };

    function bindStoreEvents(container) {
      var edits = container.querySelectorAll('.js-edit-store');
      for (var i = 0; i < edits.length; i++) {
        edits[i].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          API.getStores().then(function (list) {
            var store = list.find(function (x) { return x.id === id; });
            if (store) self.showEditModal(store, container);
          });
        };
      }
      var dels = container.querySelectorAll('.js-del-store');
      for (var j = 0; j < dels.length; j++) {
        dels[j].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          UI.showConfirm('确定要删除该门店吗？删除后不可恢复。').then(function (confirmed) {
            if (!confirmed) return;
            API.deleteStore(id).then(function () {
              UI.showToast('门店删除成功', 'success');
              self.loadData(container);
            }).catch(function (err) {
              UI.showToast(err.message || '删除失败', 'error');
            });
          });
        };
      }
    }

    function buildStoreForm(store) {
      var isEdit = !!store;
      return ''
        + '<div class="form-row"><label class="form-label">门店名称 <span style="color:#ff4d4f">*</span></label>'
        + '<input class="form-input" id="storeName" value="' + escAttr(store ? store.name : '') + '" placeholder="请输入门店名称">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">所属区域 <span style="color:#ff4d4f">*</span></label>'
        + '<select class="form-select" id="storeArea"></select>'
        + '</div>'
        + '<div class="form-row"><label class="form-label">联系人</label>'
        + '<input class="form-input" id="storeContact" value="' + escAttr(store ? store.contact || '' : '') + '" placeholder="请输入联系人">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">联系电话</label>'
        + '<input class="form-input" id="storePhone" value="' + escAttr(store ? store.phone || '' : '') + '" placeholder="请输入联系电话">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">营业开始时间</label>'
        + '<input class="form-input" id="storeOpenTime" type="time" value="' + (store ? store.openTime || '' : '') + '">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">营业结束时间</label>'
        + '<input class="form-input" id="storeCloseTime" type="time" value="' + (store ? store.closeTime || '' : '') + '">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">详细地址</label>'
        + '<textarea class="form-input" id="storeAddress" rows="2" placeholder="请输入详细地址">' + esc(store ? store.address || '' : '') + '</textarea>'
        + '</div>'
        + '<div class="form-row"><label class="form-label">门店图片</label>'
        + '<input class="form-input" id="storeImage" value="' + escAttr(store ? store.image || '🏪' : '🏪') + '" placeholder="emoji表情">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">门店描述</label>'
        + '<textarea class="form-input" id="storeDesc" rows="3" placeholder="请输入门店描述">' + esc(store ? store.description || '' : '') + '</textarea>'
        + '</div>'
        + '<div class="form-row"><label class="form-label">状态</label>'
        + '<label class="radio-label"><input type="radio" name="storeStatus" value="1"' + (store && store.status !== 1 ? '' : ' checked') + '> 营业中</label>'
        + '<label class="radio-label" style="margin-left:16px"><input type="radio" name="storeStatus" value="0"' + (store && store.status === 0 ? ' checked' : '') + '> 已关闭</label>'
        + '</div>';
    }

    self.showAddModal = function (container) {
      var body = buildStoreForm(null);
      UI.showModal('新增门店', body, function () {
        return saveStore(null, container);
      });
      API.getServiceAreas().then(function (areas) {
        var sel = document.getElementById('storeArea');
        if (sel) sel.innerHTML = buildSelectOptions(areas, 'id', 'name', '', '请选择区域');
      });
    };

    self.showEditModal = function (store, container) {
      var body = buildStoreForm(store);
      UI.showModal('编辑门店', body, function () {
        return saveStore(store.id, container);
      });
      API.getServiceAreas().then(function (areas) {
        var sel = document.getElementById('storeArea');
        if (sel) sel.innerHTML = buildSelectOptions(areas, 'id', 'name', store.areaId, '请选择区域');
      });
    };

    function saveStore(id, container) {
      var name = document.getElementById('storeName').value.trim();
      if (!name) { UI.showToast('请输入门店名称', 'error'); return false; }
      var areaId = document.getElementById('storeArea').value;
      if (!areaId) { UI.showToast('请选择所属区域', 'error'); return false; }
      var statusRadio = document.querySelector('input[name="storeStatus"]:checked');
      var data = {
        name: name,
        areaId: parseInt(areaId),
        contact: document.getElementById('storeContact').value.trim(),
        phone: document.getElementById('storePhone').value.trim(),
        openTime: document.getElementById('storeOpenTime').value || '08:00',
        closeTime: document.getElementById('storeCloseTime').value || '22:00',
        address: document.getElementById('storeAddress').value.trim(),
        image: document.getElementById('storeImage').value.trim() || '🏪',
        description: document.getElementById('storeDesc').value.trim(),
        status: statusRadio ? parseInt(statusRadio.value) : 1
      };
      var promise = id ? API.updateStore(id, data) : API.createStore(data);
      promise.then(function () {
        UI.showToast(id ? '门店更新成功' : '门店创建成功', 'success');
        self.loadData(container);
      }).catch(function (err) {
        UI.showToast(err.message || '保存失败', 'error');
      });
      return true;
    }

    self.render = function (container) {
      container.innerHTML = ''
        + '<div class="content-header">'
          + '<div><h2 class="content-title">门店管理</h2><p class="content-subtitle">管理所有门店信息，包括名称、区域、营业时间等</p></div>'
          + '<div><button class="btn btn-primary js-add-store">新增门店</button></div>'
        + '</div>'
        + '<div class="table-container">'
          + '<table class="data-table">'
            + '<thead><tr>'
              + '<th>序号</th><th>门店名称</th><th>所属区域</th><th>营业时间</th>'
              + '<th>联系电话</th><th>地址</th><th>状态</th><th>操作</th>'
            + '</tr></thead>'
            + '<tbody class="js-store-tbody"></tbody>'
          + '</table>'
        + '</div>';

      container.querySelector('.js-add-store').onclick = function () {
        self.showAddModal(container);
      };
      self.loadData(container);
    };

    return self;
  })();

  /* ==========================================================
     5. SERVICE AREAS (服务区域管理)
     ========================================================== */
  Merchant.ServiceAreas = (function () {
    var self = {};

    self.loadData = function (container) {
      var tbody = container.querySelector('.js-area-tbody');
      API.getServiceAreas().then(function (list) {
        var rows = '';
        for (var i = 0; i < list.length; i++) {
          var a = list[i];
          var parentName = a.parentId ? API.getAreaName(a.parentId) : '-';
          rows += '<tr>'
            + '<td class="cell-number">' + (i + 1) + '</td>'
            + '<td>' + esc(a.name) + '</td>'
            + '<td>' + esc(a.code || '-') + '</td>'
            + '<td>' + esc(parentName) + '</td>'
            + '<td class="cell-number">' + (a.sort || 0) + '</td>'
            + '<td>' + (a.createdAt || '-') + '</td>'
            + '<td class="cell-actions">'
              + '<a href="javascript:void(0)" class="action-link js-edit-area" data-id="' + a.id + '">编辑</a>'
              + '<a href="javascript:void(0)" class="action-link action-link-danger js-del-area" data-id="' + a.id + '">删除</a>'
            + '</td>'
            + '</tr>';
        }
        if (!list.length) {
          rows = '<tr><td colspan="7" style="text-align:center;padding:32px;color:#bfbfbf;">暂无区域数据</td></tr>';
        }
        tbody.innerHTML = rows;
        bindAreaEvents(container);
      }).catch(function (err) {
        UI.showToast(err.message || '加载区域失败', 'error');
      });
    };

    function bindAreaEvents(container) {
      var edits = container.querySelectorAll('.js-edit-area');
      for (var i = 0; i < edits.length; i++) {
        edits[i].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          API.getServiceAreas().then(function (list) {
            var area = list.find(function (x) { return x.id === id; });
            if (area) self.showEditModal(area, container);
          });
        };
      }
      var dels = container.querySelectorAll('.js-del-area');
      for (var j = 0; j < dels.length; j++) {
        dels[j].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          UI.showConfirm('确定要删除该区域吗？删除后不可恢复。').then(function (confirmed) {
            if (!confirmed) return;
            API.deleteServiceArea(id).then(function () {
              UI.showToast('区域删除成功', 'success');
              self.loadData(container);
            }).catch(function (err) {
              UI.showToast(err.message || '删除失败', 'error');
            });
          });
        };
      }
    }

    function buildAreaForm(area, areaList) {
      var isEdit = !!area;
      var parentOpts = '<option value="0">无（顶级区域）</option>';
      for (var i = 0; i < areaList.length; i++) {
        var a = areaList[i];
        if (area && a.id === area.id) continue; // cannot be its own parent
        var sel = (area && area.parentId === a.id) ? ' selected' : '';
        parentOpts += '<option value="' + a.id + '"' + sel + '>' + esc(a.name) + '</option>';
      }
      return ''
        + '<div class="form-row"><label class="form-label">区域名称 <span style="color:#ff4d4f">*</span></label>'
        + '<input class="form-input" id="areaName" value="' + escAttr(area ? area.name : '') + '" placeholder="请输入区域名称">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">区域编码</label>'
        + '<input class="form-input" id="areaCode" value="' + escAttr(area ? area.code || '' : '') + '" placeholder="请输入区域编码">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">上级区域</label>'
        + '<select class="form-select" id="areaParent">' + parentOpts + '</select>'
        + '</div>'
        + '<div class="form-row"><label class="form-label">排序值</label>'
        + '<input class="form-input" id="areaSort" type="number" value="' + (area ? area.sort || 0 : 0) + '" placeholder="数字越小越靠前">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">备注</label>'
        + '<textarea class="form-input" id="areaRemark" rows="3" placeholder="请输入备注">' + esc(area ? area.remark || '' : '') + '</textarea>'
        + '</div>';
    }

    self.showAddModal = function (container) {
      API.getServiceAreas().then(function (areas) {
        var body = buildAreaForm(null, areas);
        UI.showModal('新增区域', body, function () {
          return saveArea(null, container);
        });
      });
    };

    self.showEditModal = function (area, container) {
      API.getServiceAreas().then(function (areas) {
        var body = buildAreaForm(area, areas);
        UI.showModal('编辑区域', body, function () {
          return saveArea(area.id, container);
        });
      });
    };

    function saveArea(id, container) {
      var name = document.getElementById('areaName').value.trim();
      if (!name) { UI.showToast('请输入区域名称', 'error'); return false; }
      var data = {
        name: name,
        code: document.getElementById('areaCode').value.trim(),
        parentId: parseInt(document.getElementById('areaParent').value) || 0,
        sort: parseInt(document.getElementById('areaSort').value) || 0,
        remark: document.getElementById('areaRemark').value.trim()
      };
      var promise = id ? API.updateServiceArea(id, data) : API.createServiceArea(data);
      promise.then(function () {
        UI.showToast(id ? '区域更新成功' : '区域创建成功', 'success');
        self.loadData(container);
      }).catch(function (err) {
        UI.showToast(err.message || '保存失败', 'error');
      });
      return true;
    }

    self.render = function (container) {
      container.innerHTML = ''
        + '<div class="content-header">'
          + '<div><h2 class="content-title">服务区域管理</h2><p class="content-subtitle">管理服务覆盖区域，支持多级区域划分</p></div>'
          + '<div><button class="btn btn-primary js-add-area">新增区域</button></div>'
        + '</div>'
        + '<div class="table-container">'
          + '<table class="data-table">'
            + '<thead><tr>'
              + '<th>序号</th><th>区域名称</th><th>区域编码</th><th>上级区域</th>'
              + '<th>排序</th><th>创建时间</th><th>操作</th>'
            + '</tr></thead>'
            + '<tbody class="js-area-tbody"></tbody>'
          + '</table>'
        + '</div>';

      container.querySelector('.js-add-area').onclick = function () {
        self.showAddModal(container);
      };
      self.loadData(container);
    };

    return self;
  })();

  /* ==========================================================
     6. STORE PRODUCTS (注册商品管理)
     ========================================================== */
  Merchant.StoreProducts = (function () {
    var self = {};
    var currentStoreId = null;

    self.loadData = function (container) {
      if (!currentStoreId) {
        container.querySelector('.js-sp-table-wrap').innerHTML = '<div style="text-align:center;padding:48px;color:#bfbfbf;">请先选择门店</div>';
        return;
      }
      var tbody = container.querySelector('.js-sp-tbody');
      API.getStoreProducts(currentStoreId).then(function (list) {
        var rows = '';
        for (var i = 0; i < list.length; i++) {
          var sp = list[i];
          var prodName = API.getProductName(sp.productId);
          var allProds = getAllProductsCache();
          var prod = allProds ? allProds.find(function (p) { return p.id === sp.productId; }) : null;
          var catName = prod ? API.getCategoryName(prod.categoryId) : '-';
          rows += '<tr>'
            + '<td class="cell-number">' + (i + 1) + '</td>'
            + '<td>' + esc(prodName) + '</td>'
            + '<td>' + esc(catName) + '</td>'
            + '<td class="cell-number">' + sp.stock + '</td>'
            + '<td>' + statusBadge(sp.status, { 1: ['已上架', 'badge-success'], 0: ['已下架', 'badge-default'] }) + '</td>'
            + '<td>' + (sp.bindTime || '-') + '</td>'
            + '<td class="cell-actions">'
              + (sp.status === 1
                ? '<a href="javascript:void(0)" class="action-link js-toggle-status" data-id="' + sp.id + '" data-status="0">下架</a>'
                : '<a href="javascript:void(0)" class="action-link js-toggle-status" data-id="' + sp.id + '" data-status="1">上架</a>')
              + '<a href="javascript:void(0)" class="action-link js-adj-stock" data-id="' + sp.id + '" data-stock="' + sp.stock + '">调整库存</a>'
              + '<a href="javascript:void(0)" class="action-link action-link-danger js-remove-sp" data-id="' + sp.id + '">移除</a>'
            + '</td>'
            + '</tr>';
        }
        if (!list.length) {
          rows = '<tr><td colspan="7" style="text-align:center;padding:32px;color:#bfbfbf;">该门店暂未绑定商品</td></tr>';
        }
        tbody.innerHTML = rows;
        bindSPEvents(container);
      }).catch(function (err) {
        UI.showToast(err.message || '加载门店商品失败', 'error');
      });
    };

    var _allProducts = null;
    function getAllProductsCache() {
      return _allProducts;
    }

    function bindSPEvents(container) {
      var toggles = container.querySelectorAll('.js-toggle-status');
      for (var i = 0; i < toggles.length; i++) {
        toggles[i].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          var newStatus = parseInt(this.getAttribute('data-status'));
          API.updateStoreProductStatus(id, newStatus).then(function () {
            UI.showToast(newStatus === 1 ? '商品已上架' : '商品已下架', 'success');
            self.loadData(container);
          }).catch(function (err) {
            UI.showToast(err.message || '操作失败', 'error');
          });
        };
      }
      var adjusts = container.querySelectorAll('.js-adj-stock');
      for (var j = 0; j < adjusts.length; j++) {
        adjusts[j].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          var curStock = parseInt(this.getAttribute('data-stock'));
          self.showAdjustStockInline(id, curStock, container, this);
        };
      }
      var removes = container.querySelectorAll('.js-remove-sp');
      for (var k = 0; k < removes.length; k++) {
        removes[k].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          UI.showConfirm('确定要从该门店移除该商品吗？').then(function (confirmed) {
            if (!confirmed) return;
            API.removeStoreProduct(id).then(function () {
              UI.showToast('商品已从门店移除', 'success');
              self.loadData(container);
            }).catch(function (err) {
              UI.showToast(err.message || '移除失败', 'error');
            });
          });
        };
      }
    }

    self.showAdjustStockInline = function (id, curStock, container, anchorEl) {
      var td = anchorEl.parentElement;
      var originalHtml = td.innerHTML;
      td.innerHTML = ''
        + '<input type="number" class="form-input" style="width:80px;display:inline-block" id="adjStockInput" value="' + curStock + '" min="0">'
        + '<button class="btn btn-primary btn-sm js-save-stock" data-id="' + id + '" style="margin-left:4px">保存</button>'
        + '<button class="btn btn-default btn-sm js-cancel-stock" style="margin-left:4px">取消</button>';

      td.querySelector('.js-save-stock').onclick = function () {
        var newStock = parseInt(document.getElementById('adjStockInput').value) || 0;
        API.updateStoreProductStock(id, newStock).then(function () {
          UI.showToast('库存更新成功', 'success');
          self.loadData(container);
        }).catch(function (err) {
          UI.showToast(err.message || '更新失败', 'error');
        });
      };
      td.querySelector('.js-cancel-stock').onclick = function () {
        self.loadData(container);
      };
    };

    self.showBindProductsModal = function (container) {
      API.getProducts({ page: 1, pageSize: 1000 }).then(function (result) {
        var allProducts = result.list || result;
        _allProducts = allProducts;
        return API.getStoreProducts(currentStoreId).then(function (boundList) {
          var boundProductIds = boundList.map(function (sp) { return sp.productId; });
          var available = allProducts.filter(function (p) { return boundProductIds.indexOf(p.id) === -1; });

          function buildLeft() {
            var html = '';
            for (var i = 0; i < available.length; i++) {
              var p = available[i];
              html += '<div class="dual-list-item">'
                + '<label><input type="checkbox" class="js-bind-check" value="' + p.id + '"> ' + esc(p.name) + ' (' + esc(API.getCategoryName(p.categoryId)) + ')</label>'
                + '<input type="number" class="form-input js-bind-stock-' + p.id + '" value="0" min="0" style="width:70px;margin-left:8px;display:inline-block" placeholder="库存">'
                + '</div>';
            }
            if (!available.length) html = '<div class="dual-list-item" style="color:#bfbfbf;text-align:center;">所有商品已绑定</div>';
            return html;
          }

          function buildRight() {
            var boundProducts = boundList.map(function (sp) {
              var p = allProducts.find(function (x) { return x.id === sp.productId; });
              return p ? { id: p.id, name: p.name, categoryId: p.categoryId, stock: sp.stock } : null;
            }).filter(Boolean);
            var html = '';
            for (var i = 0; i < boundProducts.length; i++) {
              var bp = boundProducts[i];
              html += '<div class="dual-list-item"><label><input type="checkbox" class="js-unbind-check" value="' + bp.id + '"> ' + esc(bp.name) + ' (库存:' + bp.stock + ')</label></div>';
            }
            if (!boundProducts.length) html = '<div class="dual-list-item" style="color:#bfbfbf;text-align:center;">暂未绑定商品</div>';
            return html;
          }

          var body = '<div class="dual-list">'
            + '<div class="dual-list-panel">'
              + '<div class="dual-list-header">可选商品</div>'
              + '<div class="dual-list-body js-bind-left">' + buildLeft() + '</div>'
            + '</div>'
            + '<div style="display:flex;flex-direction:column;justify-content:center;gap:8px;padding:0 12px;">'
              + '<button class="btn btn-primary btn-sm js-bind-confirm">→ 绑定</button>'
              + '<button class="btn btn-default btn-sm js-unbind-confirm">← 解绑</button>'
            + '</div>'
            + '<div class="dual-list-panel">'
              + '<div class="dual-list-header">已绑定商品</div>'
              + '<div class="dual-list-body js-bind-right">' + buildRight() + '</div>'
            + '</div>'
            + '</div>';

          UI.showModal('绑定商品到门店', body, function () {
            var allChecks = document.querySelectorAll('.js-bind-check');
            var bindings = [];
            for (var i = 0; i < allChecks.length; i++) {
              if (allChecks[i].checked) {
                var pid = parseInt(allChecks[i].value);
                var stockEl = document.querySelector('.js-bind-stock-' + pid);
                bindings.push({ productId: pid, stock: stockEl ? parseInt(stockEl.value) || 0 : 0 });
              }
            }
            if (bindings.length === 0) { UI.showToast('请选择要绑定的商品', 'error'); return false; }
            return API.bindProductToStore(currentStoreId, bindings).then(function () {
              UI.showToast('商品绑定成功', 'success');
              self.loadData(container);
            }).catch(function (err) {
              UI.showToast(err.message || '绑定失败', 'error');
            }).then(function () { return true; });
          });

          // Transfer buttons
          setTimeout(function () {
            var leftBody = document.querySelector('.js-bind-left');
            var rightBody = document.querySelector('.js-bind-right');
            var bindBtn = document.querySelector('.js-bind-confirm');
            var unbindBtn = document.querySelector('.js-unbind-confirm');

            if (bindBtn) bindBtn.onclick = function (e) {
              e.preventDefault();
              // just close modal and let onSave handle it via the form data
              var checks = leftBody.querySelectorAll('.js-bind-check:checked');
              for (var i = 0; i < checks.length; i++) {
                checks[i].setAttribute('data-selected', '1');
              }
              // Trigger the modal save button
              var saveBtn = document.querySelector('.modal-footer .btn-primary');
              if (saveBtn) saveBtn.click();
            };

            if (unbindBtn) unbindBtn.onclick = function () {
              var checks = rightBody.querySelectorAll('.js-unbind-check:checked');
              var toRemove = [];
              for (var i = 0; i < checks.length; i++) {
                toRemove.push(parseInt(checks[i].value));
              }
              if (toRemove.length === 0) { UI.showToast('请选择要解绑的商品', 'error'); return; }
              var p = Promise.resolve();
              for (var j = 0; j < toRemove.length; j++) {
                // Find store product id
                (function (productId) {
                  p = p.then(function () {
                    return API.getStoreProducts(currentStoreId).then(function (bList) {
                      var sp = bList.find(function (x) { return x.productId === productId; });
                      if (sp) return API.removeStoreProduct(sp.id);
                    });
                  });
                })(toRemove[j]);
              }
              p.then(function () {
                UI.showToast('商品解绑成功', 'success');
                self.loadData(container);
                // Close modal
                var overlay = document.querySelector('.modal-overlay');
                if (overlay) overlay.click();
              }).catch(function (err) {
                UI.showToast(err.message || '解绑失败', 'error');
              });
            };
          }, 100);
        });
      });
    };

    self.render = function (container) {
      container.innerHTML = ''
        + '<div class="content-header">'
          + '<div><h2 class="content-title">注册商品管理</h2><p class="content-subtitle">管理各门店注册的商品及库存信息</p></div>'
          + '<div>'
            + '<label style="font-size:13px;margin-right:8px">选择门店</label>'
            + '<select class="form-select js-store-selector" style="width:200px"><option value="">请选择门店</option></select>'
            + '<button class="btn btn-primary js-bind-sp" style="margin-left:12px;display:none">绑定商品</button>'
          + '</div>'
        + '</div>'
        + '<div class="js-sp-table-wrap"></div>'
        + '<div class="js-sp-table" style="display:none">'
          + '<div class="table-container">'
            + '<table class="data-table">'
              + '<thead><tr>'
                + '<th>序号</th><th>商品名称</th><th>所属分类</th><th>门店库存</th>'
                + '<th>上架状态</th><th>绑定时间</th><th>操作</th>'
              + '</tr></thead>'
              + '<tbody class="js-sp-tbody"></tbody>'
            + '</table>'
          + '</div>'
        + '</div>';

      // Populate store selector
      var storeSel = container.querySelector('.js-store-selector');
      API.getStores().then(function (stores) {
        storeSel.innerHTML = '<option value="">请选择门店</option>' + buildSelectOptions(stores, 'id', 'name');
      });

      storeSel.onchange = function () {
        currentStoreId = this.value ? parseInt(this.value) : null;
        var tableDiv = container.querySelector('.js-sp-table');
        var wrapDiv = container.querySelector('.js-sp-table-wrap');
        var bindBtn = container.querySelector('.js-bind-sp');
        if (currentStoreId) {
          tableDiv.style.display = '';
          wrapDiv.style.display = 'none';
          bindBtn.style.display = '';
          self.loadData(container);
        } else {
          tableDiv.style.display = 'none';
          wrapDiv.style.display = '';
          wrapDiv.innerHTML = '<div style="text-align:center;padding:48px;color:#bfbfbf;">请先选择门店</div>';
          bindBtn.style.display = 'none';
        }
      };

      container.querySelector('.js-bind-sp').onclick = function () {
        self.showBindProductsModal(container);
      };
    };

    return self;
  })();

  /* ==========================================================
     7. STATISTICS (数据统计)
     ========================================================== */
  Merchant.Statistics = (function () {
    var self = {};
    var currentTab = 'sales';

    self.loadData = function (container) {
      if (currentTab === 'sales') {
        API.getSalesRanking({}).then(function (list) {
          renderSalesRanking(container, list);
        }).catch(function (err) {
          UI.showToast(err.message || '加载销售排行失败', 'error');
        });
      } else {
        API.getVisitRanking({}).then(function (list) {
          renderVisitRanking(container, list);
        }).catch(function (err) {
          UI.showToast(err.message || '加载访问排行失败', 'error');
        });
      }
    };

    function renderSalesRanking(container, list) {
      var panel = container.querySelector('.js-stat-panel');
      if (!list || !list.length) {
        panel.innerHTML = '<div style="text-align:center;padding:48px;color:#bfbfbf;">暂无数据</div>';
        return;
      }
      var maxAmount = list[0].salesAmount || 1;

      var tableRows = '';
      for (var i = 0; i < list.length; i++) {
        var item = list[i];
        var catName = API.getCategoryName(item.categoryId);
        tableRows += '<tr>'
          + '<td class="cell-number">' + (i + 1) + '</td>'
          + '<td>' + esc(item.name) + '</td>'
          + '<td>' + esc(catName) + '</td>'
          + '<td class="cell-number">' + item.salesCount + '</td>'
          + '<td class="cell-number">' + fmtMoney(item.salesAmount) + '</td>'
          + '<td class="cell-number">' + fmtMoney(item.avgPrice) + '</td>'
          + '</tr>';
      }

      var chartRows = '';
      for (var j = 0; j < list.length; j++) {
        var it = list[j];
        var pct = Math.round((it.salesAmount / maxAmount) * 100);
        chartRows += '<div class="bar-chart-row">'
          + '<div class="bar-chart-label">' + esc(it.name) + '</div>'
          + '<div class="bar-chart-track"><div class="bar-chart-fill" style="width:' + pct + '%"></div></div>'
          + '<div class="bar-chart-value">' + fmtMoney(it.salesAmount) + '</div>'
          + '</div>';
      }

      panel.innerHTML = ''
        + '<div class="table-container">'
          + '<table class="data-table">'
            + '<thead><tr><th>排名</th><th>商品名称</th><th>所属分类</th><th>销量</th><th>销售额</th><th>均价</th></tr></thead>'
            + '<tbody>' + tableRows + '</tbody>'
          + '</table>'
        + '</div>'
        + '<div style="margin-top:24px">'
          + '<h3 style="font-size:16px;margin-bottom:16px;font-weight:600">销售额排行</h3>'
          + '<div class="bar-chart">' + chartRows + '</div>'
        + '</div>';
    }

    function renderVisitRanking(container, list) {
      var panel = container.querySelector('.js-stat-panel');
      if (!list || !list.length) {
        panel.innerHTML = '<div style="text-align:center;padding:48px;color:#bfbfbf;">暂无数据</div>';
        return;
      }
      var maxVisits = list[0].visitCount || 1;

      var tableRows = '';
      for (var i = 0; i < list.length; i++) {
        var item = list[i];
        var catName = API.getCategoryName(item.categoryId);
        tableRows += '<tr>'
          + '<td class="cell-number">' + (i + 1) + '</td>'
          + '<td>' + esc(item.name) + '</td>'
          + '<td>' + esc(catName) + '</td>'
          + '<td class="cell-number">' + item.visitCount + '</td>'
          + '<td class="cell-number">' + item.uniqueVisitors + '</td>'
          + '<td class="cell-number">' + esc(item.conversionRate) + '</td>'
          + '</tr>';
      }

      var chartRows = '';
      for (var j = 0; j < list.length; j++) {
        var it = list[j];
        var pct = Math.round((it.visitCount / maxVisits) * 100);
        chartRows += '<div class="bar-chart-row">'
          + '<div class="bar-chart-label">' + esc(it.name) + '</div>'
          + '<div class="bar-chart-track"><div class="bar-chart-fill" style="width:' + pct + '%"></div></div>'
          + '<div class="bar-chart-value">' + it.visitCount + ' 次</div>'
          + '</div>';
      }

      panel.innerHTML = ''
        + '<div class="table-container">'
          + '<table class="data-table">'
            + '<thead><tr><th>排名</th><th>商品名称</th><th>所属分类</th><th>访问量</th><th>独立访客</th><th>转化率</th></tr></thead>'
            + '<tbody>' + tableRows + '</tbody>'
          + '</table>'
        + '</div>'
        + '<div style="margin-top:24px">'
          + '<h3 style="font-size:16px;margin-bottom:16px;font-weight:600">访问量排行</h3>'
          + '<div class="bar-chart">' + chartRows + '</div>'
        + '</div>';
    }

    self.render = function (container) {
      container.innerHTML = ''
        + '<div class="content-header">'
          + '<div><h2 class="content-title">数据统计</h2><p class="content-subtitle">查看商品销售与流量数据统计</p></div>'
          + '<div></div>'
        + '</div>'
        + '<div class="filter-bar">'
          + '<div class="form-group">'
            + '<label class="form-label">开始日期</label>'
            + '<input class="form-input" id="statStartDate" type="date">'
          + '</div>'
          + '<div class="form-group">'
            + '<label class="form-label">结束日期</label>'
            + '<input class="form-input" id="statEndDate" type="date">'
          + '</div>'
          + '<div class="form-group" style="align-self:flex-end">'
            + '<button class="btn btn-primary js-stat-search">查询</button>'
          + '</div>'
        + '</div>'
        + '<div class="tabs js-stat-tabs">'
          + '<div class="tab-item active js-tab-sales" data-tab="sales">商品销售排行</div>'
          + '<div class="tab-item js-tab-visits" data-tab="visits">商品访问排行</div>'
        + '</div>'
        + '<div class="js-stat-panel"></div>';

      // Tab switching
      container.querySelector('.js-tab-sales').onclick = function () {
        currentTab = 'sales';
        container.querySelector('.js-tab-sales').classList.add('active');
        container.querySelector('.js-tab-visits').classList.remove('active');
        self.loadData(container);
      };
      container.querySelector('.js-tab-visits').onclick = function () {
        currentTab = 'visits';
        container.querySelector('.js-tab-visits').classList.add('active');
        container.querySelector('.js-tab-sales').classList.remove('active');
        self.loadData(container);
      };
      container.querySelector('.js-stat-search').onclick = function () {
        self.loadData(container);
      };

      self.loadData(container);
    };

    return self;
  })();

  /* ==========================================================
     8. ORDERS (订单管理)
     ========================================================== */
  Merchant.Orders = (function () {
    var self = {};

    var orderStatusMap = {
      '待付款': ['待付款', 'badge-warning'],
      '待发货': ['待发货', 'badge-primary'],
      '已发货': ['已发货', 'badge-info'],
      '已完成': ['已完成', 'badge-success'],
      '已取消': ['已取消', 'badge-default']
    };

    self.loadData = function (container, filters) {
      filters = filters || {};
      var tbody = container.querySelector('.js-order-tbody');
      API.getOrders({ orderNo: filters.orderNo || '', status: filters.status || '', page: 1, pageSize: 1000 }).then(function (result) {
        var list = result.list || result;
        var rows = '';
        for (var i = 0; i < list.length; i++) {
          var o = list[i];
          rows += '<tr>'
            + '<td class="cell-number">' + (i + 1) + '</td>'
            + '<td>' + esc(o.orderNo) + '</td>'
            + '<td>' + esc(o.userName) + '</td>'
            + '<td class="cell-number">' + o.productCount + '</td>'
            + '<td class="cell-number">' + fmtMoney(o.totalAmount) + '</td>'
            + '<td>' + statusBadgeCustom(o.status, orderStatusMap) + '</td>'
            + '<td>' + (o.createTime || '-') + '</td>'
            + '<td class="cell-actions">'
              + '<a href="javascript:void(0)" class="action-link js-order-detail" data-id="' + o.id + '">查看详情</a>'
              + (o.status === '待发货' ? '<a href="javascript:void(0)" class="action-link js-order-ship" data-id="' + o.id + '">发货</a>' : '')
              + (o.status === '待付款' || o.status === '待发货' ? '<a href="javascript:void(0)" class="action-link action-link-danger js-order-void" data-id="' + o.id + '">作废</a>' : '')
            + '</td>'
            + '</tr>';
        }
        if (!list.length) {
          rows = '<tr><td colspan="8" style="text-align:center;padding:32px;color:#bfbfbf;">暂无订单数据</td></tr>';
        }
        tbody.innerHTML = rows;
        bindOrderEvents(container);
      }).catch(function (err) {
        UI.showToast(err.message || '加载订单失败', 'error');
      });
    };

    function bindOrderEvents(container) {
      var details = container.querySelectorAll('.js-order-detail');
      for (var i = 0; i < details.length; i++) {
        details[i].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          self.showDetailModal(id, container);
        };
      }
      var ships = container.querySelectorAll('.js-order-ship');
      for (var j = 0; j < ships.length; j++) {
        ships[j].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          self.showShipModal(id, container);
        };
      }
      var voids = container.querySelectorAll('.js-order-void');
      for (var k = 0; k < voids.length; k++) {
        voids[k].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          self.voidOrder(id, container);
        };
      }
    }

    function getCurrentOrderFilters(container) {
      return {
        orderNo: (container.querySelector('#orderFilterNo') || {}).value || '',
        status: (container.querySelector('#orderFilterStatus') || {}).value || ''
      };
    }

    self.showDetailModal = function (orderId, container) {
      API.getOrderDetail(orderId).then(function (order) {
        var itemsHtml = '';
        for (var i = 0; i < order.items.length; i++) {
          var it = order.items[i];
          itemsHtml += '<tr>'
            + '<td>' + esc(it.name) + '</td>'
            + '<td class="cell-number">' + fmtMoney(it.price) + '</td>'
            + '<td class="cell-number">' + it.qty + '</td>'
            + '<td class="cell-number">' + fmtMoney(it.subtotal) + '</td>'
            + '</tr>';
        }

        var timelineHtml = '';
        for (var t = 0; t < order.timeline.length; t++) {
          var tl = order.timeline[t];
          var isLast = t === order.timeline.length - 1;
          timelineHtml += '<div class="timeline-item">'
            + '<div class="timeline-dot"></div>'
            + '<div class="timeline-content">'
              + '<span>' + esc(tl.status) + '</span>'
              + '<span class="time">' + esc(tl.time) + '</span>'
            + '</div>'
            + '</div>';
        }

        var body = ''
          + '<div style="display:flex;gap:24px;margin-bottom:24px;flex-wrap:wrap">'
            + '<div style="flex:1;min-width:250px">'
              + '<h4 style="margin-bottom:12px;font-size:15px;font-weight:600">订单信息</h4>'
              + '<table class="data-table"><tbody>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">订单编号</td><td style="padding:6px 12px">' + esc(order.orderNo) + '</td></tr>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">下单时间</td><td style="padding:6px 12px">' + esc(order.createTime) + '</td></tr>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">订单状态</td><td style="padding:6px 12px">' + statusBadgeCustom(order.status, orderStatusMap) + '</td></tr>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">支付方式</td><td style="padding:6px 12px">' + esc(order.payMethod || '-') + '</td></tr>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">支付时间</td><td style="padding:6px 12px">' + esc(order.payTime || '-') + '</td></tr>'
              + '</tbody></table>'
            + '</div>'
            + '<div style="flex:1;min-width:250px">'
              + '<h4 style="margin-bottom:12px;font-size:15px;font-weight:600">客户信息</h4>'
              + '<table class="data-table"><tbody>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">用户名</td><td style="padding:6px 12px">' + esc(order.userName) + '</td></tr>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">电话</td><td style="padding:6px 12px">' + esc(order.phone) + '</td></tr>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">地址</td><td style="padding:6px 12px">' + esc(order.address) + '</td></tr>'
              + '</tbody></table>'
            + '</div>'
          + '</div>'
          + '<h4 style="margin-bottom:8px;font-size:15px;font-weight:600">商品清单</h4>'
          + '<div class="table-container" style="margin-bottom:16px">'
            + '<table class="data-table">'
              + '<thead><tr><th>商品名称</th><th>单价</th><th>数量</th><th>小计</th></tr></thead>'
              + '<tbody>' + itemsHtml + '</tbody>'
            + '</table>'
          + '</div>'
          + '<div style="text-align:right;font-size:16px;font-weight:600;margin-bottom:16px">'
            + '订单总额：' + fmtMoney(order.totalAmount)
          + '</div>'
          + '<h4 style="margin-bottom:8px;font-size:15px;font-weight:600">状态时间线</h4>'
          + '<div class="order-timeline">' + timelineHtml + '</div>';

        UI.showModal('订单详情 - ' + esc(order.orderNo), body, null, true);
      }).catch(function (err) {
        UI.showToast(err.message || '加载订单详情失败', 'error');
      });
    };

    self.showShipModal = function (orderId, container) {
      var body = ''
        + '<div class="form-row"><label class="form-label">快递单号</label>'
        + '<input class="form-input" id="shipTrackingNo" placeholder="请输入快递单号">'
        + '</div>'
        + '<div class="form-row"><label class="form-label">快递公司</label>'
        + '<input class="form-input" id="shipCourier" placeholder="如：顺丰速运">'
        + '</div>';
      UI.showModal('订单发货', body, function () {
        var trackingNo = document.getElementById('shipTrackingNo').value.trim();
        var courier = document.getElementById('shipCourier').value.trim();
        if (!trackingNo) { UI.showToast('请输入快递单号', 'error'); return false; }
        return API.shipOrder(orderId, { trackingNo: trackingNo, courier: courier }).then(function () {
          UI.showToast('发货成功', 'success');
          self.loadData(container, getCurrentOrderFilters(container));
        }).catch(function (err) {
          UI.showToast(err.message || '发货失败', 'error');
        }).then(function () { return true; });
      });
    };

    self.voidOrder = function (orderId, container) {
      UI.showConfirm('确定要作废该订单吗？作废后不可恢复。').then(function (confirmed) {
        if (!confirmed) return;
        API.voidOrder(orderId).then(function () {
          UI.showToast('订单已作废', 'success');
          self.loadData(container, getCurrentOrderFilters(container));
        }).catch(function (err) {
          UI.showToast(err.message || '作废失败', 'error');
        });
      });
    };

    self.render = function (container) {
      container.innerHTML = ''
        + '<div class="content-header">'
          + '<div><h2 class="content-title">订单管理</h2><p class="content-subtitle">管理所有订单，支持查看详情、发货、作废等操作</p></div>'
          + '<div></div>'
        + '</div>'
        + '<div class="filter-bar">'
          + '<div class="form-group">'
            + '<label class="form-label">订单编号</label>'
            + '<input class="form-input" id="orderFilterNo" placeholder="请输入订单编号">'
          + '</div>'
          + '<div class="form-group">'
            + '<label class="form-label">订单状态</label>'
            + '<select class="form-select" id="orderFilterStatus">'
              + '<option value="">全部</option>'
              + '<option value="待付款">待付款</option>'
              + '<option value="待发货">待发货</option>'
              + '<option value="已发货">已发货</option>'
              + '<option value="已完成">已完成</option>'
              + '<option value="已取消">已取消</option>'
            + '</select>'
          + '</div>'
          + '<div class="form-group">'
            + '<label class="form-label">开始日期</label>'
            + '<input class="form-input" id="orderStartDate" type="date">'
          + '</div>'
          + '<div class="form-group">'
            + '<label class="form-label">结束日期</label>'
            + '<input class="form-input" id="orderEndDate" type="date">'
          + '</div>'
          + '<div class="form-group" style="align-self:flex-end">'
            + '<button class="btn btn-primary js-order-search">查询</button>'
            + '<button class="btn btn-default js-order-reset" style="margin-left:8px">重置</button>'
          + '</div>'
        + '</div>'
        + '<div class="table-container">'
          + '<table class="data-table">'
            + '<thead><tr>'
              + '<th>序号</th><th>订单编号</th><th>用户名</th><th>商品数量</th>'
              + '<th>订单金额</th><th>订单状态</th><th>下单时间</th><th>操作</th>'
            + '</tr></thead>'
            + '<tbody class="js-order-tbody"></tbody>'
          + '</table>'
        + '</div>';

      container.querySelector('.js-order-search').onclick = function () {
        self.loadData(container, getCurrentOrderFilters(container));
      };
      container.querySelector('.js-order-reset').onclick = function () {
        container.querySelector('#orderFilterNo').value = '';
        container.querySelector('#orderFilterStatus').value = '';
        container.querySelector('#orderStartDate').value = '';
        container.querySelector('#orderEndDate').value = '';
        self.loadData(container, {});
      };

      self.loadData(container);
    };

    return self;
  })();

  /* ==========================================================
     9. MEMBERS (会员管理)
     ========================================================== */
  Merchant.Members = (function () {
    var self = {};

    self.loadData = function (container, filters) {
      filters = filters || {};
      var tbody = container.querySelector('.js-member-tbody');
      API.getMembers({ keyword: filters.keyword || '', page: 1, pageSize: 1000 }).then(function (result) {
        var list = result.list || result;
        var rows = '';
        for (var i = 0; i < list.length; i++) {
          var m = list[i];
          rows += '<tr>'
            + '<td class="cell-number">' + (i + 1) + '</td>'
            + '<td><span style="font-size:20px">' + esc(m.avatar || '👤') + '</span> ' + esc(m.username) + '</td>'
            + '<td>' + esc(m.phone) + '</td>'
            + '<td>' + esc(m.email || '-') + '</td>'
            + '<td>' + esc(m.gender || '-') + '</td>'
            + '<td>' + (m.createdAt || '-') + '</td>'
            + '<td>' + (m.lastLogin || '-') + '</td>'
            + '<td class="cell-number">' + m.orderCount + '</td>'
            + '<td class="cell-actions">'
              + '<a href="javascript:void(0)" class="action-link js-member-detail" data-id="' + m.id + '">查看详情</a>'
            + '</td>'
            + '</tr>';
        }
        if (!list.length) {
          rows = '<tr><td colspan="9" style="text-align:center;padding:32px;color:#bfbfbf;">暂无会员数据</td></tr>';
        }
        tbody.innerHTML = rows;
        bindMemberEvents(container);
      }).catch(function (err) {
        UI.showToast(err.message || '加载会员失败', 'error');
      });
    };

    function bindMemberEvents(container) {
      var details = container.querySelectorAll('.js-member-detail');
      for (var i = 0; i < details.length; i++) {
        details[i].onclick = function () {
          var id = parseInt(this.getAttribute('data-id'));
          self.showDetailModal(id, container);
        };
      }
    }

    function getCurrentMemberFilters(container) {
      return {
        keyword: (container.querySelector('#memberFilterKeyword') || {}).value || ''
      };
    }

    self.showDetailModal = function (memberId, container) {
      API.getMemberDetail(memberId).then(function (m) {
        var addressesHtml = '';
        for (var i = 0; i < m.addresses.length; i++) {
          addressesHtml += '<div style="padding:6px 0;border-bottom:1px solid var(--color-border-light)">'
            + '<span style="font-size:16px;margin-right:8px">📍</span> '
            + esc(m.addresses[i])
            + '</div>';
        }
        var body = ''
          + '<div style="text-align:center;margin-bottom:24px">'
            + '<span style="font-size:56px;display:block;margin-bottom:8px">' + esc(m.avatar || '👤') + '</span>'
            + '<h3 style="font-size:20px;margin-bottom:4px">' + esc(m.username) + '</h3>'
            + '<p style="color:#8c8c8c;font-size:13px">' + esc(m.realName || '') + '</p>'
          + '</div>'
          + '<div style="display:flex;gap:24px;flex-wrap:wrap;margin-bottom:16px">'
            + '<div style="flex:1;min-width:200px">'
              + '<h4 style="font-size:14px;font-weight:600;margin-bottom:12px">基本信息</h4>'
              + '<table class="data-table"><tbody>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">性别</td><td style="padding:6px 12px">' + esc(m.gender || '-') + '</td></tr>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">手机号</td><td style="padding:6px 12px">' + esc(m.phone) + '</td></tr>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">邮箱</td><td style="padding:6px 12px">' + esc(m.email || '-') + '</td></tr>'
              + '</tbody></table>'
            + '</div>'
            + '<div style="flex:1;min-width:200px">'
              + '<h4 style="font-size:14px;font-weight:600;margin-bottom:12px">消费统计</h4>'
              + '<table class="data-table"><tbody>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">总订单数</td><td style="padding:6px 12px;font-weight:600">' + m.orderCount + '</td></tr>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">总消费金额</td><td style="padding:6px 12px;font-weight:600;color:var(--color-primary)">' + fmtMoney(m.totalSpent) + '</td></tr>'
                + '<tr><td style="padding:6px 12px;color:#8c8c8c">客单价</td><td style="padding:6px 12px">' + (m.orderCount > 0 ? fmtMoney(m.totalSpent / m.orderCount) : fmtMoney(0)) + '</td></tr>'
              + '</tbody></table>'
            + '</div>'
          + '</div>'
          + '<h4 style="font-size:14px;font-weight:600;margin-bottom:8px">收货地址 (' + m.addresses.length + ')</h4>'
          + '<div style="margin-bottom:16px">' + addressesHtml + '</div>'
          + '<div style="display:flex;gap:24px">'
            + '<div style="flex:1"><span style="color:#8c8c8c">注册时间：</span>' + esc(m.createdAt || '-') + '</div>'
            + '<div style="flex:1"><span style="color:#8c8c8c">最后登录：</span>' + esc(m.lastLogin || '-') + '</div>'
          + '</div>';

        UI.showModal('会员详情 - ' + esc(m.username), body, null, true);
      }).catch(function (err) {
        UI.showToast(err.message || '加载会员详情失败', 'error');
      });
    };

    self.render = function (container) {
      container.innerHTML = ''
        + '<div class="content-header">'
          + '<div><h2 class="content-title">会员管理</h2><p class="content-subtitle">管理所有注册会员信息</p></div>'
          + '<div></div>'
        + '</div>'
        + '<div class="filter-bar">'
          + '<div class="form-group">'
            + '<label class="form-label">用户名/手机号</label>'
            + '<input class="form-input" id="memberFilterKeyword" placeholder="请输入用户名或手机号">'
          + '</div>'
          + '<div class="form-group">'
            + '<label class="form-label">注册开始日期</label>'
            + '<input class="form-input" id="memberStartDate" type="date">'
          + '</div>'
          + '<div class="form-group">'
            + '<label class="form-label">注册结束日期</label>'
            + '<input class="form-input" id="memberEndDate" type="date">'
          + '</div>'
          + '<div class="form-group" style="align-self:flex-end">'
            + '<button class="btn btn-primary js-member-search">查询</button>'
            + '<button class="btn btn-default js-member-reset" style="margin-left:8px">重置</button>'
          + '</div>'
        + '</div>'
        + '<div class="table-container">'
          + '<table class="data-table">'
            + '<thead><tr>'
              + '<th>序号</th><th>用户名</th><th>手机号</th><th>邮箱</th>'
              + '<th>性别</th><th>注册时间</th><th>最后登录</th><th>订单数</th><th>操作</th>'
            + '</tr></thead>'
            + '<tbody class="js-member-tbody"></tbody>'
          + '</table>'
        + '</div>';

      container.querySelector('.js-member-search').onclick = function () {
        self.loadData(container, getCurrentMemberFilters(container));
      };
      container.querySelector('.js-member-reset').onclick = function () {
        container.querySelector('#memberFilterKeyword').value = '';
        container.querySelector('#memberStartDate').value = '';
        container.querySelector('#memberEndDate').value = '';
        self.loadData(container, {});
      };

      self.loadData(container);
    };

    return self;
  })();

  /* ==========================================================
     SHARED UTILITY FUNCTIONS
     ========================================================== */

  function esc(str) {
    if (!str) return '';
    return String(str).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
  }

  function escAttr(str) {
    if (!str) return '';
    return String(str).replace(/&/g, '&amp;').replace(/"/g, '&quot;').replace(/'/g, '&#39;').replace(/</g, '&lt;').replace(/>/g, '&gt;');
  }

  function statusBadgeCustom(status, map) {
    var pair = map[status] || ['未知', 'badge-default'];
    return '<span class="badge ' + pair[1] + '">' + pair[0] + '</span>';
  }

  // Shortcuts to globals
  var API = window.App.API;
  var UI = window.App.UI;

  /* ==========================================================
     EXPORT
     ========================================================== */

  window.App = window.App || {};
  window.App.Merchant = Merchant;
})();
