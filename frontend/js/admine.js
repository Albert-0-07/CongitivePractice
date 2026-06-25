/**
 * Admin Panel Module for Chinese Retail Management System
 * IIFE pattern defining window.App.Admin
 * Dependencies: window.App.API, window.App.UI
 */
(function () {
  'use strict';

  var API = window.App.API;
  var UI = window.App.UI;

  // ============================================================
  // Helper utilities
  // ============================================================

  function escapeHTML(str) {
    if (!str) return '';
    return String(str)
      .replace(/&/g, '&amp;')
      .replace(/</g, '&lt;')
      .replace(/>/g, '&gt;')
      .replace(/"/g, '&quot;')
      .replace(/'/g, '&#39;');
  }

  function formatDateTime(dt) {
    if (!dt) return '-';
    var d = new Date(dt);
    if (isNaN(d.getTime())) return '-';
    var pad = function (n) { return n < 10 ? '0' + n : '' + n; };
    return (
      d.getFullYear() +
      '-' + pad(d.getMonth() + 1) +
      '-' + pad(d.getDate()) +
      ' ' + pad(d.getHours()) +
      ':' + pad(d.getMinutes()) +
      ':' + pad(d.getSeconds())
    );
  }

  function formatDate(dt) {
    if (!dt) return '';
    var d = new Date(dt);
    if (isNaN(d.getTime())) return '';
    var pad = function (n) { return n < 10 ? '0' + n : '' + n; };
    return d.getFullYear() + '-' + pad(d.getMonth() + 1) + '-' + pad(d.getDate());
  }

  function roleName(roleCode) {
    var map = {
      super_admin: '超级管理员',
      admin: '管理员',
      operator: '运营',
      service: '客服',
      merchant: '商户'
    };
    return map[roleCode] || roleCode || '-';
  }

  function roleBadgeClass(roleCode) {
    var map = {
      super_admin: 'badge-danger',
      admin: 'badge-primary',
      operator: 'badge-warning',
      service: 'badge-info',
      merchant: 'badge-default'
    };
    return map[roleCode] || 'badge-default';
  }

  function statusBadge(status) {
    if (status === 1 || status === '1' || status === true || status === 'enabled') {
      return '<span class="badge badge-success">启用</span>';
    }
    return '<span class="badge badge-danger">禁用</span>';
  }

  function logTypeBadge(type) {
    var map = {
      '登录': 'badge-primary',
      '新增': 'badge-success',
      '修改': 'badge-warning',
      '删除': 'badge-danger',
      '导出': 'badge-info',
      '作废': 'badge-default'
    };
    var cls = map[type] || 'badge-default';
    return '<span class="badge ' + cls + '">' + escapeHTML(type) + '</span>';
  }

  function logModuleBadge(module) {
    return '<span class="badge badge-info">' + escapeHTML(module) + '</span>';
  }

  function buildActionButtons(actions) {
    var html = '<div class="action-buttons">';
    actions.forEach(function (btn) {
      html +=
        '<button class="btn btn-sm ' +
        (btn.cls || 'btn-default') +
        '" ' +
        (btn.onclick ? 'onclick="' + btn.onclick + '"' : '') +
        '>' +
        escapeHTML(btn.label) +
        '</button>';
    });
    html += '</div>';
    return html;
  }

  // ============================================================
  // 1. Users Module (用户管理)
  // ============================================================

  var Users = {
    container: null,
    currentPage: 1,
    pageSize: 15,
    total: 0,

    render: function (container) {
      this.container = container;
      this.currentPage = 1;
      this.loadData();
    },

    loadData: function () {
      var self = this;
      if (!this.container) return;

      this.container.innerHTML =
        '<div style="text-align:center;padding:40px;"><i class="fa fa-spinner fa-spin"></i> 加载中...</div>';

      API.getUsers({ page: this.currentPage, pageSize: this.pageSize })
        .then(function (res) {
          var data = res.data || res;
          var list = data.list || data.records || data || [];
          if (!Array.isArray(list)) list = [];
          self.total = data.total || list.length;
          self.renderPanel(list);
        })
        .catch(function (err) {
          console.error('加载用户列表失败:', err);
          self.container.innerHTML =
            '<div style="text-align:center;padding:40px;color:#999;">加载失败，请刷新重试</div>';
        });
    },

    renderPanel: function (list) {
      var self = this;
      var html = '';

      // Header
      html += '<div class="content-header">';
      html += '<div><h2 class="content-title">用户管理</h2></div>';
      html += '<div>';
      html += '<button class="btn btn-primary" id="btn-add-user"><i class="fa fa-plus"></i> 新增用户</button>';
      html += '</div>';
      html += '</div>';

      // Table
      html += '<div class="table-container">';
      html += '<table class="data-table">';
      html += '<thead><tr>';
      html += '<th>序号</th>';
      html += '<th>用户名</th>';
      html += '<th>真实姓名</th>';
      html += '<th>角色</th>';
      html += '<th>手机号</th>';
      html += '<th>邮箱</th>';
      html += '<th>状态</th>';
      html += '<th>创建时间</th>';
      html += '<th>操作</th>';
      html += '</tr></thead>';
      html += '<tbody>';

      if (list.length === 0) {
        html += '<tr><td colspan="9" style="text-align:center;padding:30px;color:#999;">暂无数据</td></tr>';
      } else {
        list.forEach(function (u, idx) {
          var num = (self.currentPage - 1) * self.pageSize + idx + 1;
          html += '<tr>';
          html += '<td>' + num + '</td>';
          html += '<td>' + escapeHTML(u.username) + '</td>';
          html += '<td>' + escapeHTML(u.realname || u.real_name || '-') + '</td>';
          html += '<td><span class="badge ' + roleBadgeClass(u.role) + '">' + roleName(u.role) + '</span></td>';
          html += '<td>' + escapeHTML(u.phone || u.mobile || '-') + '</td>';
          html += '<td>' + escapeHTML(u.email || '-') + '</td>';
          html += '<td>' + statusBadge(u.status) + '</td>';
          html += '<td>' + formatDateTime(u.created_at || u.create_time) + '</td>';
          html += '<td>';
          html += '<button class="btn btn-sm btn-primary btn-edit-user" data-id="' + u.id + '">编辑</button> ';
          html += '<button class="btn btn-sm btn-danger btn-del-user" data-id="' + u.id + '">删除</button> ';
          html += '<button class="btn btn-sm btn-warning btn-reset-pwd" data-id="' + u.id + '" data-username="' + escapeHTML(u.username) + '">重置密码</button>';
          html += '</td>';
          html += '</tr>';
        });
      }

      html += '</tbody></table>';
      html += '</div>';

      // Pagination
      html += self.renderPagination();

      this.container.innerHTML = html;
      this.bindEvents();
    },

    renderPagination: function () {
      var self = this;
      var totalPages = Math.ceil(this.total / this.pageSize);
      if (totalPages <= 1) return '';

      var html = '<div class="pagination">';
      html += '<button class="btn btn-sm btn-default page-btn" data-page="' + (this.currentPage - 1) + '" ' + (this.currentPage <= 1 ? 'disabled' : '') + '>上一页</button>';
      html += '<span class="page-info">第 ' + this.currentPage + ' / ' + totalPages + ' 页，共 ' + this.total + ' 条</span>';
      html += '<button class="btn btn-sm btn-default page-btn" data-page="' + (this.currentPage + 1) + '" ' + (this.currentPage >= totalPages ? 'disabled' : '') + '>下一页</button>';
      html += '</div>';
      return html;
    },

    bindEvents: function () {
      var self = this;

      // Add user
      var addBtn = document.getElementById('btn-add-user');
      if (addBtn) {
        addBtn.addEventListener('click', function () {
          self.showAddModal();
        });
      }

      // Edit user
      var editBtns = this.container.querySelectorAll('.btn-edit-user');
      editBtns.forEach(function (btn) {
        btn.addEventListener('click', function () {
          var id = this.getAttribute('data-id');
          self.showEditModal(id);
        });
      });

      // Delete user
      var delBtns = this.container.querySelectorAll('.btn-del-user');
      delBtns.forEach(function (btn) {
        btn.addEventListener('click', function () {
          var id = this.getAttribute('data-id');
          self.deleteUser(id);
        });
      });

      // Reset password
      var pwdBtns = this.container.querySelectorAll('.btn-reset-pwd');
      pwdBtns.forEach(function (btn) {
        btn.addEventListener('click', function () {
          var id = this.getAttribute('data-id');
          var username = this.getAttribute('data-username');
          self.resetPassword(id, username);
        });
      });

      // Pagination
      var pageBtns = this.container.querySelectorAll('.page-btn');
      pageBtns.forEach(function (btn) {
        btn.addEventListener('click', function () {
          var page = parseInt(this.getAttribute('data-page'));
          if (!isNaN(page) && page >= 1) {
            self.currentPage = page;
            self.loadData();
          }
        });
      });
    },

    showAddModal: function () {
      var self = this;
      var bodyHTML =
        '<form id="form-user-add">' +
        '<div class="form-group">' +
        '<label>用户名 <span class="required">*</span></label>' +
        '<input type="text" class="form-control" name="username" required placeholder="请输入用户名">' +
        '</div>' +
        '<div class="form-group">' +
        '<label>真实姓名</label>' +
        '<input type="text" class="form-control" name="realname" placeholder="请输入真实姓名">' +
        '</div>' +
        '<div class="form-group">' +
        '<label>角色</label>' +
        '<select class="form-control" name="role">' +
        '<option value="super_admin">超级管理员</option>' +
        '<option value="operator">运营</option>' +
        '<option value="service">客服</option>' +
        '<option value="merchant">商户</option>' +
        '</select>' +
        '</div>' +
        '<div class="form-group">' +
        '<label>手机号</label>' +
        '<input type="text" class="form-control" name="phone" placeholder="请输入手机号">' +
        '</div>' +
        '<div class="form-group">' +
        '<label>邮箱</label>' +
        '<input type="text" class="form-control" name="email" placeholder="请输入邮箱">' +
        '</div>' +
        '<div class="form-group">' +
        '<label>密码 <span class="required">*</span></label>' +
        '<input type="password" class="form-control" name="password" required placeholder="请输入密码">' +
        '</div>' +
        '</form>';

      UI.showModal('新增用户', bodyHTML, function (modal) {
        var form = document.getElementById('form-user-add');
        if (!form) return;
        var username = form.querySelector('[name="username"]').value.trim();
        var password = form.querySelector('[name="password"]').value.trim();
        if (!username) { UI.showToast('请输入用户名', 'warning'); return false; }
        if (!password) { UI.showToast('请输入密码', 'warning'); return false; }
        var payload = {
          username: username,
          realname: form.querySelector('[name="realname"]').value.trim(),
          role: form.querySelector('[name="role"]').value,
          phone: form.querySelector('[name="phone"]').value.trim(),
          email: form.querySelector('[name="email"]').value.trim(),
          password: password
        };
        API.createUser(payload)
          .then(function () {
            UI.showToast('用户创建成功', 'success');
            ;
            self.loadData();
          })
          .catch(function (err) {
            UI.showToast('创建失败: ' + (err.message || err.msg || '未知错误'), 'error');
          });
      });
    },

    showEditModal: function (id) {
      var self = this;

      API.getUser(id)
        .then(function (res) {
          var u = res.data || res;
          var bodyHTML =
            '<form id="form-user-edit">' +
            '<input type="hidden" name="id" value="' + escapeHTML(u.id) + '">' +
            '<div class="form-group">' +
            '<label>用户名 <span class="required">*</span></label>' +
            '<input type="text" class="form-control" name="username" required value="' + escapeHTML(u.username) + '" placeholder="请输入用户名">' +
            '</div>' +
            '<div class="form-group">' +
            '<label>真实姓名</label>' +
            '<input type="text" class="form-control" name="realname" value="' + escapeHTML(u.realname || u.real_name || '') + '" placeholder="请输入真实姓名">' +
            '</div>' +
            '<div class="form-group">' +
            '<label>角色</label>' +
            '<select class="form-control" name="role">' +
            '<option value="super_admin" ' + (u.role === 'super_admin' ? 'selected' : '') + '>超级管理员</option>' +
            '<option value="operator" ' + (u.role === 'operator' ? 'selected' : '') + '>运营</option>' +
            '<option value="service" ' + (u.role === 'service' ? 'selected' : '') + '>客服</option>' +
            '<option value="merchant" ' + (u.role === 'merchant' ? 'selected' : '') + '>商户</option>' +
            '</select>' +
            '</div>' +
            '<div class="form-group">' +
            '<label>手机号</label>' +
            '<input type="text" class="form-control" name="phone" value="' + escapeHTML(u.phone || u.mobile || '') + '" placeholder="请输入手机号">' +
            '</div>' +
            '<div class="form-group">' +
            '<label>邮箱</label>' +
            '<input type="text" class="form-control" name="email" value="' + escapeHTML(u.email || '') + '" placeholder="请输入邮箱">' +
            '</div>' +
            '<div class="form-group">' +
            '<label>密码</label>' +
            '<input type="password" class="form-control" name="password" placeholder="留空则不修改密码">' +
            '<small class="form-text text-muted">如需修改密码请在此输入新密码，留空则保持原密码不变</small>' +
            '</div>' +
            '</form>';

          UI.showModal('编辑用户', bodyHTML, function (modal) {
            var form = document.getElementById('form-user-edit');
            if (!form) return;
            var username = form.querySelector('[name="username"]').value.trim();
            if (!username) { UI.showToast('请输入用户名', 'warning'); return false; }
            var payload = {
              id: form.querySelector('[name="id"]').value,
              username: username,
              realname: form.querySelector('[name="realname"]').value.trim(),
              role: form.querySelector('[name="role"]').value,
              phone: form.querySelector('[name="phone"]').value.trim(),
              email: form.querySelector('[name="email"]').value.trim()
            };
            var password = form.querySelector('[name="password"]').value.trim();
            if (password) {
              payload.password = password;
            }
            API.updateUser(payload)
              .then(function () {
                UI.showToast('用户更新成功', 'success');
                ;
                self.loadData();
              })
              .catch(function (err) {
                UI.showToast('更新失败: ' + (err.message || err.msg || '未知错误'), 'error');
              });
          });
        })
        .catch(function (err) {
          UI.showToast('获取用户信息失败', 'error');
        });
    },

    deleteUser: function (id) {
      var self = this;
      UI.showConfirm('确定要删除该用户吗？此操作不可恢复。')
        .then(function (confirmed) {
          if (!confirmed) return;
          API.deleteUser(id)
            .then(function () {
              UI.showToast('用户已删除', 'success');
              self.loadData();
            })
            .catch(function (err) {
              UI.showToast('删除失败: ' + (err.message || err.msg || '未知错误'), 'error');
            });
        });
    },

    resetPassword: function (id, username) {
      var self = this;
      UI.showConfirm('确定要重置用户 "' + username + '" 的密码吗？密码将重置为默认密码。')
        .then(function (confirmed) {
          if (!confirmed) return;
          API.resetUserPassword(id)
            .then(function (res) {
              var defaultPwd = (res && (res.data || res.default_password)) || '123456';
              UI.showToast('密码已重置，默认密码为: ' + defaultPwd, 'success');
              self.loadData();
            })
            .catch(function (err) {
              UI.showToast('重置失败: ' + (err.message || err.msg || '未知错误'), 'error');
            });
        });
    }
  };

  // ============================================================
  // 2. Permissions Module (权限管理)
  // ============================================================

  var Permissions = {
    container: null,
    roles: [],
    selectedRoleId: null,
    permissions: {},

    // Permission matrix definition
    permissionGroups: [
      {
        module: '商品管理',
        key: 'product',
        permissions: [
          { key: 'view', label: '查看' },
          { key: 'add', label: '新增' },
          { key: 'edit', label: '修改' },
          { key: 'del', label: '删除' }
        ]
      },
      {
        module: '订单管理',
        key: 'order',
        permissions: [
          { key: 'view', label: '查看' },
          { key: 'ship', label: '发货' },
          { key: 'void', label: '作废' }
        ]
      },
      {
        module: '会员管理',
        key: 'member',
        permissions: [
          { key: 'view', label: '查看' }
        ]
      },
      {
        module: '门店管理',
        key: 'store',
        permissions: [
          { key: 'view', label: '查看' },
          { key: 'add', label: '新增' },
          { key: 'edit', label: '修改' },
          { key: 'del', label: '删除' }
        ]
      },
      {
        module: '营销管理',
        key: 'marketing',
        permissions: [
          { key: 'view', label: '查看' },
          { key: 'add', label: '新增' },
          { key: 'edit', label: '修改' },
          { key: 'del', label: '删除' }
        ]
      },
      {
        module: '数据统计',
        key: 'statistics',
        permissions: [
          { key: 'view', label: '查看' }
        ]
      },
      {
        module: '用户管理',
        key: 'user',
        permissions: [
          { key: 'view', label: '查看' },
          { key: 'add', label: '新增' },
          { key: 'edit', label: '修改' },
          { key: 'del', label: '删除' }
        ]
      },
      {
        module: '权限管理',
        key: 'permission',
        permissions: [
          { key: 'view', label: '查看' },
          { key: 'edit', label: '修改' }
        ]
      },
      {
        module: '操作日志',
        key: 'log',
        permissions: [
          { key: 'view', label: '查看' }
        ]
      }
    ],

    render: function (container) {
      this.container = container;
      var self = this;
      this.container.innerHTML =
        '<div style="text-align:center;padding:40px;"><i class="fa fa-spinner fa-spin"></i> 加载中...</div>';

      API.getRoles()
        .then(function (res) {
          var data = res.data || res;
          self.roles = data.list || data.records || data || [];
          if (!Array.isArray(self.roles)) self.roles = [];
          self.renderPanel();
          if (self.roles.length > 0) {
            self.selectRole(self.roles[0].id);
          }
        })
        .catch(function (err) {
          console.error('加载角色列表失败:', err);
          self.container.innerHTML =
            '<div style="text-align:center;padding:40px;color:#999;">加载失败，请刷新重试</div>';
        });
    },

    renderPanel: function () {
      var self = this;
      var html = '';

      // Header
      html += '<div class="content-header">';
      html += '<div><h2 class="content-title">权限管理</h2></div>';
      html += '</div>';

      // Two-column layout
      html += '<div class="permission-layout">';

      // Left: Role list
      html += '<div class="permission-left">';
      html += '<h4>角色列表</h4>';
      html += '<div class="role-list-cards">';
      this.roles.forEach(function (role) {
        html += '<div class="role-card" data-role-id="' + role.id + '">';
        html += '<span class="badge ' + roleBadgeClass(role.code || role.role) + '">' + escapeHTML(role.name || roleName(role.code || role.role)) + '</span>';
        html += '<span class="role-desc">' + escapeHTML(role.description || role.desc || '') + '</span>';
        html += '</div>';
      });
      html += '</div>';
      html += '</div>';

      // Right: Permission matrix
      html += '<div class="permission-right" id="permission-matrix-container">';
      html += '<p style="text-align:center;color:#999;padding:40px;">请选择左侧角色以查看权限</p>';
      html += '</div>';

      html += '</div>'; // .permission-layout

      this.container.innerHTML = html;
      this.bindRoleEvents();
    },

    bindRoleEvents: function () {
      var self = this;
      var cards = this.container.querySelectorAll('.role-card');
      cards.forEach(function (card) {
        card.addEventListener('click', function () {
          var roleId = this.getAttribute('data-role-id');
          self.selectRole(roleId);
        });
      });
    },

    selectRole: function (roleId) {
      this.selectedRoleId = roleId;

      // Update active card
      var cards = this.container.querySelectorAll('.role-card');
      cards.forEach(function (card) {
        if (card.getAttribute('data-role-id') === String(roleId)) {
          card.classList.add('active');
        } else {
          card.classList.remove('active');
        }
      });

      // Load permissions
      this.loadPermissions(roleId);
    },

    loadPermissions: function (roleId) {
      var self = this;
      var container = document.getElementById('permission-matrix-container');
      if (!container) return;
      container.innerHTML =
        '<div style="text-align:center;padding:40px;"><i class="fa fa-spinner fa-spin"></i> 加载权限数据...</div>';

      API.getRolePermissions(roleId)
        .then(function (res) {
          var data = res.data || res;
          self.permissions = (data && data.permissions) ? data.permissions : (data || {});
          self.renderMatrix(container);
        })
        .catch(function (err) {
          console.error('加载权限失败:', err);
          self.permissions = {};
          self.renderMatrix(container);
        });
    },

    renderMatrix: function (container) {
      var self = this;
      var html = '';

      html += '<h4>权限设置</h4>';
      html += '<div class="table-container">';
      html += '<table class="data-table permission-matrix">';
      html += '<thead><tr>';
      html += '<th>功能模块</th>';
      html += '<th>查看</th>';
      html += '<th>新增</th>';
      html += '<th>修改</th>';
      html += '<th>删除</th>';
      html += '<th>发货</th>';
      html += '<th>作废</th>';
      html += '</tr></thead>';
      html += '<tbody>';

      this.permissionGroups.forEach(function (group) {
        html += '<tr>';
        html += '<td><strong>' + escapeHTML(group.module) + '</strong></td>';

        // All possible column keys in order
        var columnKeys = ['view', 'add', 'edit', 'del', 'ship', 'void'];

        columnKeys.forEach(function (colKey) {
          var permDef = group.permissions.find(function (p) { return p.key === colKey; });
          if (permDef) {
            var permKey = group.key + '_' + colKey;
            var checked = self.permissions[permKey] ? 'checked' : '';
            html += '<td><input type="checkbox" class="perm-checkbox" data-perm-key="' + permKey + '" ' + checked + '></td>';
          } else {
            html += '<td></td>';
          }
        });

        html += '</tr>';
      });

      html += '</tbody></table>';
      html += '</div>';
      html += '<div style="margin-top:15px;text-align:right;">';
      html += '<button class="btn btn-primary" id="btn-save-permissions">保存权限</button>';
      html += '</div>';

      container.innerHTML = html;

      // Bind save button
      var saveBtn = document.getElementById('btn-save-permissions');
      if (saveBtn) {
        saveBtn.addEventListener('click', function () {
          self.savePermissions();
        });
      }
    },

    savePermissions: function () {
      var self = this;
      if (!this.selectedRoleId) {
        UI.showToast('请先选择一个角色', 'warning');
        return;
      }

      var checkboxes = document.querySelectorAll('.perm-checkbox');
      var permData = {};
      checkboxes.forEach(function (cb) {
        permData[cb.getAttribute('data-perm-key')] = cb.checked ? 1 : 0;
      });

      var payload = {
        roleId: this.selectedRoleId,
        permissions: permData
      };

      API.saveRolePermissions(payload)
        .then(function () {
          UI.showToast('权限保存成功', 'success');
          // Refresh local cache
          self.permissions = permData;
        })
        .catch(function (err) {
          UI.showToast('保存失败: ' + (err.message || err.msg || '未知错误'), 'error');
        });
    }
  };

  // ============================================================
  // 3. Logs Module (操作日志)
  // ============================================================

  var Logs = {
    container: null,
    currentPage: 1,
    pageSize: 20,
    total: 0,
    filters: {
      operator: '',
      type: '',
      dateFrom: '',
      dateTo: ''
    },

    logTypes: ['全部', '登录', '新增', '修改', '删除', '导出', '作废'],

    render: function (container) {
      this.container = container;
      this.currentPage = 1;
      this.filters = { operator: '', type: '', dateFrom: '', dateTo: '' };
      this.loadData();
    },

    loadData: function () {
      var self = this;
      if (!this.container) return;

      this.container.innerHTML =
        '<div style="text-align:center;padding:40px;"><i class="fa fa-spinner fa-spin"></i> 加载中...</div>';

      var params = {
        page: this.currentPage,
        pageSize: this.pageSize
      };
      if (this.filters.operator) params.operator = this.filters.operator;
      if (this.filters.type && this.filters.type !== '全部') params.type = this.filters.type;
      if (this.filters.dateFrom) params.dateFrom = this.filters.dateFrom;
      if (this.filters.dateTo) params.dateTo = this.filters.dateTo;

      API.getLogs(params)
        .then(function (res) {
          var data = res.data || res;
          var list = data.list || data.records || data || [];
          if (!Array.isArray(list)) list = [];
          self.total = data.total || list.length;
          self.renderPanel(list);
        })
        .catch(function (err) {
          console.error('加载操作日志失败:', err);
          self.container.innerHTML =
            '<div style="text-align:center;padding:40px;color:#999;">加载失败，请刷新重试</div>';
        });
    },

    renderPanel: function (list) {
      var self = this;
      var html = '';

      // Header
      html += '<div class="content-header">';
      html += '<div><h2 class="content-title">操作日志</h2></div>';
      html += '</div>';

      // Filter bar
      html += '<div class="filter-bar">';
      html += '<form class="form-inline" id="form-log-filter">';
      html += '<div class="form-group">';
      html += '<label>操作人</label>';
      html += '<input type="text" class="form-control" name="operator" value="' + escapeHTML(self.filters.operator) + '" placeholder="操作人">';
      html += '</div>';
      html += '<div class="form-group">';
      html += '<label>操作类型</label>';
      html += '<select class="form-control" name="type">';
      self.logTypes.forEach(function (t) {
        html += '<option value="' + t + '" ' + (self.filters.type === t ? 'selected' : '') + '>' + t + '</option>';
      });
      html += '</select>';
      html += '</div>';
      html += '<div class="form-group">';
      html += '<label>操作时间</label>';
      html += '<input type="date" class="form-control" name="dateFrom" value="' + escapeHTML(self.filters.dateFrom) + '" placeholder="开始日期">';
      html += '<span> 至 </span>';
      html += '<input type="date" class="form-control" name="dateTo" value="' + escapeHTML(self.filters.dateTo) + '" placeholder="结束日期">';
      html += '</div>';
      html += '<div class="form-group">';
      html += '<button type="button" class="btn btn-primary" id="btn-log-search">查询</button>';
      html += '<button type="button" class="btn btn-default" id="btn-log-reset">重置</button>';
      html += '</div>';
      html += '</form>';
      html += '</div>';

      // Table
      html += '<div class="table-container">';
      html += '<table class="data-table">';
      html += '<thead><tr>';
      html += '<th>序号</th>';
      html += '<th>操作人</th>';
      html += '<th>操作类型</th>';
      html += '<th>操作模块</th>';
      html += '<th>操作描述</th>';
      html += '<th>IP地址</th>';
      html += '<th>操作时间</th>';
      html += '<th>操作</th>';
      html += '</tr></thead>';
      html += '<tbody>';

      if (list.length === 0) {
        html += '<tr><td colspan="8" style="text-align:center;padding:30px;color:#999;">暂无数据</td></tr>';
      } else {
        list.forEach(function (log, idx) {
          var num = (self.currentPage - 1) * self.pageSize + idx + 1;
          html += '<tr>';
          html += '<td>' + num + '</td>';
          html += '<td>' + escapeHTML(log.operator || log.username || log.user || '-') + '</td>';
          html += '<td>' + logTypeBadge(log.type || log.action_type || log.action) + '</td>';
          html += '<td>' + logModuleBadge(log.module || log.target_module || '-') + '</td>';
          html += '<td>' + escapeHTML(log.description || log.desc || log.content || '-') + '</td>';
          html += '<td>' + escapeHTML(log.ip || log.ip_address || '-') + '</td>';
          html += '<td>' + formatDateTime(log.created_at || log.create_time || log.time) + '</td>';
          html += '<td>';
          html += '<button class="btn btn-sm btn-info btn-log-detail" data-log-id="' + log.id + '">查看详情</button>';
          html += '</td>';
          html += '</tr>';
        });
      }

      html += '</tbody></table>';
      html += '</div>';

      // Pagination
      html += self.renderPagination();

      this.container.innerHTML = html;
      this.bindEvents();
    },

    renderPagination: function () {
      var self = this;
      var totalPages = Math.ceil(this.total / this.pageSize);
      if (totalPages <= 1) return '';

      var html = '<div class="pagination">';
      html += '<button class="btn btn-sm btn-default page-btn" data-page="' + (this.currentPage - 1) + '" ' + (this.currentPage <= 1 ? 'disabled' : '') + '>上一页</button>';
      html += '<span class="page-info">第 ' + this.currentPage + ' / ' + totalPages + ' 页，共 ' + this.total + ' 条</span>';
      html += '<button class="btn btn-sm btn-default page-btn" data-page="' + (this.currentPage + 1) + '" ' + (this.currentPage >= totalPages ? 'disabled' : '') + '>下一页</button>';
      html += '</div>';
      return html;
    },

    bindEvents: function () {
      var self = this;

      // Search
      var searchBtn = document.getElementById('btn-log-search');
      if (searchBtn) {
        searchBtn.addEventListener('click', function () {
          var form = document.getElementById('form-log-filter');
          if (!form) return;
          self.filters.operator = form.querySelector('[name="operator"]').value.trim();
          self.filters.type = form.querySelector('[name="type"]').value;
          self.filters.dateFrom = form.querySelector('[name="dateFrom"]').value;
          self.filters.dateTo = form.querySelector('[name="dateTo"]').value;
          self.currentPage = 1;
          self.loadData();
        });
      }

      // Reset
      var resetBtn = document.getElementById('btn-log-reset');
      if (resetBtn) {
        resetBtn.addEventListener('click', function () {
          var form = document.getElementById('form-log-filter');
          if (!form) return;
          form.querySelector('[name="operator"]').value = '';
          form.querySelector('[name="type"]').value = '全部';
          form.querySelector('[name="dateFrom"]').value = '';
          form.querySelector('[name="dateTo"]').value = '';
          self.filters = { operator: '', type: '', dateFrom: '', dateTo: '' };
          self.currentPage = 1;
          self.loadData();
        });
      }

      // Detail
      var detailBtns = this.container.querySelectorAll('.btn-log-detail');
      detailBtns.forEach(function (btn) {
        btn.addEventListener('click', function () {
          var logId = this.getAttribute('data-log-id');
          self.showDetail(logId);
        });
      });

      // Pagination
      var pageBtns = this.container.querySelectorAll('.page-btn');
      pageBtns.forEach(function (btn) {
        btn.addEventListener('click', function () {
          var page = parseInt(this.getAttribute('data-page'));
          if (!isNaN(page) && page >= 1) {
            self.currentPage = page;
            self.loadData();
          }
        });
      });
    },

    showDetail: function (logId) {
      API.getLogDetail(logId)
        .then(function (res) {
          var log = res.data || res;
          var bodyHTML =
            '<div class="log-detail">' +
            '<table class="info-table">' +
            '<tr><td class="info-label">操作人</td><td>' + escapeHTML(log.operator || log.username || '-') + '</td></tr>' +
            '<tr><td class="info-label">操作类型</td><td>' + logTypeBadge(log.type || log.action_type || '-') + '</td></tr>' +
            '<tr><td class="info-label">操作模块</td><td>' + logModuleBadge(log.module || log.target_module || '-') + '</td></tr>' +
            '<tr><td class="info-label">操作描述</td><td>' + escapeHTML(log.description || log.desc || log.content || '-') + '</td></tr>' +
            '<tr><td class="info-label">IP地址</td><td>' + escapeHTML(log.ip || log.ip_address || '-') + '</td></tr>' +
            '<tr><td class="info-label">操作时间</td><td>' + formatDateTime(log.created_at || log.create_time || log.time) + '</td></tr>' +
            '<tr><td class="info-label">请求参数</td><td><pre>' + escapeHTML(JSON.stringify(log.params || log.request_params || {}, null, 2)) + '</pre></td></tr>' +
            '<tr><td class="info-label">User-Agent</td><td>' + escapeHTML(log.user_agent || log.ua || '-') + '</td></tr>' +
            '</table>' +
            '</div>';
          UI.showModal('操作日志详情', bodyHTML, null);
        })
        .catch(function (err) {
          UI.showToast('获取日志详情失败', 'error');
        });
    }
  };

  // ============================================================
  // 4. AdminUsers Module (管理员用户管理)
  // ============================================================

  var AdminUsers = {
    container: null,
    currentPage: 1,
    pageSize: 15,
    total: 0,

    render: function (container) {
      this.container = container;
      this.currentPage = 1;
      this.loadData();
    },

    loadData: function () {
      var self = this;
      if (!this.container) return;

      this.container.innerHTML =
        '<div style="text-align:center;padding:40px;"><i class="fa fa-spinner fa-spin"></i> 加载中...</div>';

      API.getAdminUsers({ page: this.currentPage, pageSize: this.pageSize })
        .then(function (res) {
          var data = res.data || res;
          var list = data.list || data.records || data || [];
          if (!Array.isArray(list)) list = [];
          self.total = data.total || list.length;
          self.renderPanel(list);
        })
        .catch(function (err) {
          console.error('加载管理员列表失败:', err);
          self.container.innerHTML =
            '<div style="text-align:center;padding:40px;color:#999;">加载失败，请刷新重试</div>';
        });
    },

    renderPanel: function (list) {
      var self = this;
      var html = '';

      // Header
      html += '<div class="content-header">';
      html += '<div><h2 class="content-title">管理员用户管理</h2></div>';
      html += '<div>';
      html += '<button class="btn btn-primary" id="btn-add-admin"><i class="fa fa-plus"></i> 新增管理员</button>';
      html += '</div>';
      html += '</div>';

      // Table
      html += '<div class="table-container">';
      html += '<table class="data-table">';
      html += '<thead><tr>';
      html += '<th>序号</th>';
      html += '<th>用户名</th>';
      html += '<th>真实姓名</th>';
      html += '<th>角色</th>';
      html += '<th>手机号</th>';
      html += '<th>邮箱</th>';
      html += '<th>状态</th>';
      html += '<th>最后登录IP</th>';
      html += '<th>登录次数</th>';
      html += '<th>创建时间</th>';
      html += '<th>操作</th>';
      html += '</tr></thead>';
      html += '<tbody>';

      if (list.length === 0) {
        html += '<tr><td colspan="11" style="text-align:center;padding:30px;color:#999;">暂无数据</td></tr>';
      } else {
        list.forEach(function (u, idx) {
          var num = (self.currentPage - 1) * self.pageSize + idx + 1;
          html += '<tr>';
          html += '<td>' + num + '</td>';
          html += '<td>' + escapeHTML(u.username) + '</td>';
          html += '<td>' + escapeHTML(u.realname || u.real_name || '-') + '</td>';
          html += '<td><span class="badge ' + roleBadgeClass(u.role) + '">' + roleName(u.role) + '</span></td>';
          html += '<td>' + escapeHTML(u.phone || u.mobile || '-') + '</td>';
          html += '<td>' + escapeHTML(u.email || '-') + '</td>';
          html += '<td>' + statusBadge(u.status) + '</td>';
          html += '<td>' + escapeHTML(u.last_login_ip || u.login_ip || '-') + '</td>';
          html += '<td>' + (u.login_count || u.login_times || 0) + '</td>';
          html += '<td>' + formatDateTime(u.created_at || u.create_time) + '</td>';
          html += '<td>';
          html += '<button class="btn btn-sm btn-primary btn-edit-admin" data-id="' + u.id + '">编辑</button> ';
          html += '<button class="btn btn-sm btn-danger btn-del-admin" data-id="' + u.id + '">删除</button> ';
          html += '<button class="btn btn-sm btn-warning btn-reset-admin-pwd" data-id="' + u.id + '" data-username="' + escapeHTML(u.username) + '">重置密码</button>';
          html += '</td>';
          html += '</tr>';
        });
      }

      html += '</tbody></table>';
      html += '</div>';

      // Pagination
      html += self.renderPagination();

      this.container.innerHTML = html;
      this.bindEvents();
    },

    renderPagination: function () {
      var self = this;
      var totalPages = Math.ceil(this.total / this.pageSize);
      if (totalPages <= 1) return '';

      var html = '<div class="pagination">';
      html += '<button class="btn btn-sm btn-default page-btn" data-page="' + (this.currentPage - 1) + '" ' + (this.currentPage <= 1 ? 'disabled' : '') + '>上一页</button>';
      html += '<span class="page-info">第 ' + this.currentPage + ' / ' + totalPages + ' 页，共 ' + this.total + ' 条</span>';
      html += '<button class="btn btn-sm btn-default page-btn" data-page="' + (this.currentPage + 1) + '" ' + (this.currentPage >= totalPages ? 'disabled' : '') + '>下一页</button>';
      html += '</div>';
      return html;
    },

    bindEvents: function () {
      var self = this;

      // Add admin
      var addBtn = document.getElementById('btn-add-admin');
      if (addBtn) {
        addBtn.addEventListener('click', function () {
          self.showAddModal();
        });
      }

      // Edit admin
      var editBtns = this.container.querySelectorAll('.btn-edit-admin');
      editBtns.forEach(function (btn) {
        btn.addEventListener('click', function () {
          var id = this.getAttribute('data-id');
          self.showEditModal(id);
        });
      });

      // Delete admin
      var delBtns = this.container.querySelectorAll('.btn-del-admin');
      delBtns.forEach(function (btn) {
        btn.addEventListener('click', function () {
          var id = this.getAttribute('data-id');
          self.deleteAdmin(id);
        });
      });

      // Reset password
      var pwdBtns = this.container.querySelectorAll('.btn-reset-admin-pwd');
      pwdBtns.forEach(function (btn) {
        btn.addEventListener('click', function () {
          var id = this.getAttribute('data-id');
          var username = this.getAttribute('data-username');
          self.resetAdminPassword(id, username);
        });
      });

      // Pagination
      var pageBtns = this.container.querySelectorAll('.page-btn');
      pageBtns.forEach(function (btn) {
        btn.addEventListener('click', function () {
          var page = parseInt(this.getAttribute('data-page'));
          if (!isNaN(page) && page >= 1) {
            self.currentPage = page;
            self.loadData();
          }
        });
      });
    },

    showAddModal: function () {
      var self = this;
      var bodyHTML =
        '<form id="form-admin-add">' +
        '<div class="form-group">' +
        '<label>用户名 <span class="required">*</span></label>' +
        '<input type="text" class="form-control" name="username" required placeholder="请输入用户名">' +
        '</div>' +
        '<div class="form-group">' +
        '<label>真实姓名</label>' +
        '<input type="text" class="form-control" name="realname" placeholder="请输入真实姓名">' +
        '</div>' +
        '<div class="form-group">' +
        '<label>角色</label>' +
        '<select class="form-control" name="role">' +
        '<option value="super_admin">超级管理员</option>' +
        '<option value="admin">管理员</option>' +
        '<option value="operator">运营</option>' +
        '<option value="service">客服</option>' +
        '</select>' +
        '</div>' +
        '<div class="form-group">' +
        '<label>手机号</label>' +
        '<input type="text" class="form-control" name="phone" placeholder="请输入手机号">' +
        '</div>' +
        '<div class="form-group">' +
        '<label>邮箱</label>' +
        '<input type="text" class="form-control" name="email" placeholder="请输入邮箱">' +
        '</div>' +
        '<div class="form-group">' +
        '<label>密码 <span class="required">*</span></label>' +
        '<input type="password" class="form-control" name="password" required placeholder="请输入密码">' +
        '</div>' +
        '</form>';

      UI.showModal('新增管理员', bodyHTML, function (modal) {
        var form = document.getElementById('form-admin-add');
        if (!form) return;
        var username = form.querySelector('[name="username"]').value.trim();
        var password = form.querySelector('[name="password"]').value.trim();
        if (!username) { UI.showToast('请输入用户名', 'warning'); return false; }
        if (!password) { UI.showToast('请输入密码', 'warning'); return false; }
        var payload = {
          username: username,
          realname: form.querySelector('[name="realname"]').value.trim(),
          role: form.querySelector('[name="role"]').value,
          phone: form.querySelector('[name="phone"]').value.trim(),
          email: form.querySelector('[name="email"]').value.trim(),
          password: password
        };
        API.createAdminUser(payload)
          .then(function () {
            UI.showToast('管理员创建成功', 'success');
            ;
            self.loadData();
          })
          .catch(function (err) {
            UI.showToast('创建失败: ' + (err.message || err.msg || '未知错误'), 'error');
          });
      });
    },

    showEditModal: function (id) {
      var self = this;

      API.getAdminUser(id)
        .then(function (res) {
          var u = res.data || res;
          var bodyHTML =
            '<form id="form-admin-edit">' +
            '<input type="hidden" name="id" value="' + escapeHTML(u.id) + '">' +
            '<div class="form-group">' +
            '<label>用户名 <span class="required">*</span></label>' +
            '<input type="text" class="form-control" name="username" required value="' + escapeHTML(u.username) + '" placeholder="请输入用户名">' +
            '</div>' +
            '<div class="form-group">' +
            '<label>真实姓名</label>' +
            '<input type="text" class="form-control" name="realname" value="' + escapeHTML(u.realname || u.real_name || '') + '" placeholder="请输入真实姓名">' +
            '</div>' +
            '<div class="form-group">' +
            '<label>角色</label>' +
            '<select class="form-control" name="role">' +
            '<option value="super_admin" ' + (u.role === 'super_admin' ? 'selected' : '') + '>超级管理员</option>' +
            '<option value="admin" ' + (u.role === 'admin' ? 'selected' : '') + '>管理员</option>' +
            '<option value="operator" ' + (u.role === 'operator' ? 'selected' : '') + '>运营</option>' +
            '<option value="service" ' + (u.role === 'service' ? 'selected' : '') + '>客服</option>' +
            '</select>' +
            '</div>' +
            '<div class="form-group">' +
            '<label>手机号</label>' +
            '<input type="text" class="form-control" name="phone" value="' + escapeHTML(u.phone || u.mobile || '') + '" placeholder="请输入手机号">' +
            '</div>' +
            '<div class="form-group">' +
            '<label>邮箱</label>' +
            '<input type="text" class="form-control" name="email" value="' + escapeHTML(u.email || '') + '" placeholder="请输入邮箱">' +
            '</div>' +
            '<div class="form-group">' +
            '<label>密码</label>' +
            '<input type="password" class="form-control" name="password" placeholder="留空则不修改密码">' +
            '<small class="form-text text-muted">如需修改密码请在此输入新密码，留空则保持原密码不变</small>' +
            '</div>' +
            '</form>';

          UI.showModal('编辑管理员', bodyHTML, function (modal) {
            var form = document.getElementById('form-admin-edit');
            if (!form) return;
            var username = form.querySelector('[name="username"]').value.trim();
            if (!username) { UI.showToast('请输入用户名', 'warning'); return false; }
            var payload = {
              id: form.querySelector('[name="id"]').value,
              username: username,
              realname: form.querySelector('[name="realname"]').value.trim(),
              role: form.querySelector('[name="role"]').value,
              phone: form.querySelector('[name="phone"]').value.trim(),
              email: form.querySelector('[name="email"]').value.trim()
            };
            var password = form.querySelector('[name="password"]').value.trim();
            if (password) {
              payload.password = password;
            }
            API.updateAdminUser(payload)
              .then(function () {
                UI.showToast('管理员更新成功', 'success');
                ;
                self.loadData();
              })
              .catch(function (err) {
                UI.showToast('更新失败: ' + (err.message || err.msg || '未知错误'), 'error');
              });
          });
        })
        .catch(function (err) {
          UI.showToast('获取管理员信息失败', 'error');
        });
    },

    deleteAdmin: function (id) {
      var self = this;
      UI.showConfirm('确定要删除该管理员吗？此操作不可恢复。')
        .then(function (confirmed) {
          if (!confirmed) return;
          API.deleteAdminUser(id)
            .then(function () {
              UI.showToast('管理员已删除', 'success');
              self.loadData();
            })
            .catch(function (err) {
              UI.showToast('删除失败: ' + (err.message || err.msg || '未知错误'), 'error');
            });
        });
    },

    resetAdminPassword: function (id, username) {
      var self = this;
      UI.showConfirm('确定要重置管理员 "' + username + '" 的密码吗？密码将重置为默认密码。')
        .then(function (confirmed) {
          if (!confirmed) return;
          API.resetAdminPassword(id)
            .then(function (res) {
              var defaultPwd = (res && (res.data || res.default_password)) || '123456';
              UI.showToast('密码已重置，默认密码为: ' + defaultPwd, 'success');
              self.loadData();
            })
            .catch(function (err) {
              UI.showToast('重置失败: ' + (err.message || err.msg || '未知错误'), 'error');
            });
        });
    }
  };

  // ============================================================
  // Export to window.App.Admin
  // ============================================================

  window.App.Admin = {
    Users: Users,
    Permissions: Permissions,
    Logs: Logs,
    AdminUsers: AdminUsers
  };

})();
