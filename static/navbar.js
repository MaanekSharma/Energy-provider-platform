(function() {
  // Helper function to get a cookie by name.
  function getCookie(name) {
    const value = `; ${document.cookie}`;
    const parts = value.split(`; ${name}=`);
    if (parts.length === 2) return parts.pop().split(';').shift();
    return null;
  }

  // Function to show a toast notification.
  function showToast(message) {
    const toast = document.createElement('div');
    toast.className = 'toast';
    toast.textContent = message;
    document.body.appendChild(toast);
    setTimeout(() => {
      document.body.removeChild(toast);
    }, 3000);
  }

  // password toggle
  const togglePassword = document.getElementById('toggle-password');
  const passwordInput = document.getElementById('modal-password');
  if (togglePassword && passwordInput) {
    togglePassword.addEventListener('click', function() {
      if (passwordInput.type === 'password') {
        passwordInput.type = 'text';
        // "closed-eye" icon for "showing" the password.
        togglePassword.innerHTML = `
          <!-- Slash-Eye Icon (closed eye) -->
          <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="#2ecc71" viewBox="0 0 24 24">
            <path d="M2.39 1.73L1.11 3l2.05 2.05A11.897 11.897 0 0 0 0 12s3 7 12 7c2.32 0 4.34-.45 6.03-1.18l2.84 2.84 1.27-1.27-5.44-5.44L2.39 1.73zM12 5c2.61 0 4.83 1 6.5 2.44.9.81 1.67 1.71 2.33 2.56-.78 1.1-1.75 2.16-2.83 2.97l-1.43-1.43c.73-.53 1.39-1.16 1.99-1.84-1.46-1.74-3.33-3-5.56-3-1.15 0-2.25.28-3.23.75L7.8 5.79C8.86 5.28 10.37 5 12 5z"/>
          </svg>
        `;
      } else {
        passwordInput.type = 'password';
        // "open-eye" icon to indicate password is hidden.
        togglePassword.innerHTML = `
          <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="#2ecc71" viewBox="0 0 24 24">
            <path d="M12 5c-7.633 0-11 6.5-11 7s3.367 7 11 7 11-6.5 11-7-3.367-7-11-7zm0 12c-2.761 0-5-2.239-5-5s2.239-5 5-5c2.762 0 5 2.239 5 5s-2.238 5-5 5zm0-8c-1.654 0-3 1.346-3 3s1.346 3 3 3c1.654 0 3-1.346 3-3s-1.346-3-3-3z"/>
          </svg>
        `;
      }
    });
  } else {
    console.error("Toggle password or password input element not found.");
  }

  // Determine if admin is logged in.
  const isLoggedIn = (getCookie("admin_session") === "valid");

  // Get the navbar list element.
  const navbarList = document.getElementById('navbar-list');
  // Clear existing items.
  navbarList.innerHTML = '';

  // Always include the Dashboard link.
  const dashboardItem = document.createElement('li');
  dashboardItem.innerHTML = '<a href="index.html">Dashboard</a>';
  navbarList.appendChild(dashboardItem);

  if (isLoggedIn) {
    // If logged in, add all the nav links.
    const links = [
      { href: 'customers.html', text: 'Customers' },
      { href: 'sales.html', text: 'Sales' },
      { href: 'billing.html', text: 'Billing' },
      { href: 'reports.html', text: 'Reports' },    ];
    links.forEach(link => {
      const li = document.createElement('li');
      li.innerHTML = `<a href="${link.href}">${link.text}</a>`;
      navbarList.appendChild(li);
    });
    
    const adminItem = document.createElement('li');
    adminItem.id = "admin-item";
    adminItem.innerHTML = '<a class="cta-button" href="/admin/logout" id="admin-logout-btn">Logout</a>';
    navbarList.appendChild(adminItem);
  } else {
    // If not logged in, only show the Sign In button.
    const adminItem = document.createElement('li');
    adminItem.id = "admin-item";
    adminItem.innerHTML = '<a class="cta-button" href="#" id="admin-login-btn">Sign In</a>';
    navbarList.appendChild(adminItem);
  }

  // event handler for Sign In.
  const signInBtn = document.getElementById('admin-login-btn');
  if (signInBtn) {
    signInBtn.addEventListener('click', function(e) {
      e.preventDefault();
      document.getElementById('login-modal').style.display = 'block';
    });
  }

  // logout event handler using AJAX.
  const logoutBtn = document.getElementById('admin-logout-btn');
  if (logoutBtn) {
    logoutBtn.addEventListener('click', function(e) {
      e.preventDefault();
      fetch('/admin/logout', { credentials: 'include' })
        .then(response => {
          if (response.ok) {
            // Rebuild navbar for logged-out state.
            navbarList.innerHTML = '';
            const dash = document.createElement('li');
            dash.innerHTML = '<a href="index.html">Dashboard</a>';
            navbarList.appendChild(dash);
            const signIn = document.createElement('li');
            signIn.innerHTML = '<a class="cta-button" href="#" id="admin-login-btn">Sign In</a>';
            navbarList.appendChild(signIn);
            signIn.addEventListener('click', function(e) {
              e.preventDefault();
              document.getElementById('login-modal').style.display = 'block';
            });
            showToast("Logout successful!");
          } else {
            console.error("Logout failed.");
          }
        })
        .catch(err => console.error(err));
    });
  }

  // Modal close handler.
  const modal = document.getElementById('login-modal');
  const closeBtn = document.getElementById('modal-close');
  closeBtn.addEventListener('click', function() {
    modal.style.display = 'none';
  });

  // Handle login form submission.
  const loginForm = document.getElementById('loginForm');
  loginForm.addEventListener('submit', function(e) {
    e.preventDefault();
    const formData = new FormData(loginForm);
    fetch('/admin/login', {
      method: 'POST',
      credentials: 'include',
      headers: { "Content-Type": "application/x-www-form-urlencoded;charset=UTF-8" },
      body: new URLSearchParams(formData)
    })
    .then(response => response.text().then(text => ({ status: response.status, text: text })))
    .then(result => {
      if (result.status === 200) {
        showToast("Login successful!");
        window.location.reload(); // Reload to update navbar state
      } else {
        const loginError = document.getElementById('login-error');
        loginError.style.display = 'block';
      }
    })
    .catch(err => console.error(err));
  });

  // Close modal when clicking outside it.
  window.onclick = function(event) {
    if (event.target == modal) {
      modal.style.display = "none";
    }
  };
})();
