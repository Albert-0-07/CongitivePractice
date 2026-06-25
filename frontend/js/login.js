/* ============================================================
   商户管理系统 — Auth Module (Login)
   Provides: window.App.Auth
   ============================================================ */

(function () {
  'use strict';

  var Auth = {
    TOKEN_KEY: 'merchant_admin_token',
    USER_KEY: 'merchant_admin_user',

    /* Check if user is currently logged in */
    isLoggedIn: function () {
      return !!localStorage.getItem(this.TOKEN_KEY);
    },

    /* Get stored auth token */
    getToken: function () {
      return localStorage.getItem(this.TOKEN_KEY);
    },

    /* Get stored user info */
    getUser: function () {
      try {
        var raw = localStorage.getItem(this.USER_KEY);
        return raw ? JSON.parse(raw) : null;
      } catch (e) {
        return null;
      }
    },

    /* Get user role */
    getRole: function () {
      var user = this.getUser();
      return user ? user.role : null;
    },

    /* Login — call API, store token & user on success */
    login: function (username, password) {
      var self = this;
      return App.API.login(username, password).then(function (result) {
        localStorage.setItem(self.TOKEN_KEY, result.token);
        localStorage.setItem(self.USER_KEY, JSON.stringify(result.user));
        return result.user;
      });
    },

    /* Logout — clear storage */
    logout: function () {
      localStorage.removeItem(this.TOKEN_KEY);
      localStorage.removeItem(this.USER_KEY);
    },

    /* Render the login page as a full-viewport overlay.
       The app shell HTML stays intact underneath — it's hidden until login succeeds. */
    renderLoginPage: function () {
      // Ensure app shell is hidden
      var shell = document.getElementById('appShell');
      if (shell) shell.classList.add('hidden');

      // Create login overlay (don't destroy body.innerHTML — the app shell lives underneath)
      var loginPage = document.createElement('div');
      loginPage.id = 'loginPage';
      loginPage.innerHTML = [
        '<div class="login-container">',
          '<div class="login-card">',
            '<div class="login-header">',
              '<div style="font-size:48px;margin-bottom:8px;">🏪</div>',
              '<h1>商户管理系统</h1>',
              '<p>Merchant Management System</p>',
            '</div>',
            '<div class="login-error hidden" id="loginError"></div>',
            '<form class="login-form" id="loginForm">',
              '<div class="form-group">',
                '<label class="form-label" for="loginUsername">用户名</label>',
                '<input class="form-input" type="text" id="loginUsername" placeholder="请输入用户名" autocomplete="username" autofocus>',
              '</div>',
              '<div class="form-group">',
                '<label class="form-label" for="loginPassword">密码</label>',
                '<input class="form-input" type="password" id="loginPassword" placeholder="请输入密码" autocomplete="current-password">',
              '</div>',
              '<button type="submit" class="btn btn-primary" id="loginBtn">登 录</button>',
            '</form>',
            '<div style="text-align:center;margin-top:16px;font-size:12px;color:#8c8c8c;">',
              '测试账号: admin / admin123 &nbsp;|&nbsp; merchant / merchant123',
            '</div>',
          '</div>',
        '</div>'
      ].join('');
      document.body.appendChild(loginPage);

      var form = document.getElementById('loginForm');
      var errorEl = document.getElementById('loginError');
      var btn = document.getElementById('loginBtn');
      var self = this;

      form.addEventListener('submit', function (e) {
        e.preventDefault();
        var username = document.getElementById('loginUsername').value.trim();
        var password = document.getElementById('loginPassword').value;

        if (!username || !password) {
          errorEl.textContent = '请输入用户名和密码';
          errorEl.classList.remove('hidden');
          return;
        }

        btn.disabled = true;
        btn.textContent = '登录中...';
        errorEl.classList.add('hidden');

        self.login(username, password).then(function (user) {
          // Remove login overlay (app shell was preserved underneath)
          var lp = document.getElementById('loginPage');
          if (lp && lp.parentNode) lp.parentNode.removeChild(lp);
          if (window._onLoginSuccess) {
            window._onLoginSuccess(user);
          }
        }).catch(function (err) {
          errorEl.textContent = err.message || '登录失败，请重试';
          errorEl.classList.remove('hidden');
          btn.disabled = false;
          btn.textContent = '登 录';
        });
      });
    }
  };

  window.App = window.App || {};
  window.App.Auth = Auth;
})();
