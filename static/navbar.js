// navbar.js
(function() {

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
  
    // Toggle password visibility using the eye icon.
    const togglePassword = document.getElementById('toggle-password');
    const passwordInput = document.getElementById('modal-password');
    if (togglePassword && passwordInput) {
      togglePassword.addEventListener('click', function() {
        if (passwordInput.type === 'password') {
          passwordInput.type = 'text';
          togglePassword.innerHTML = `
            <!-- Slash-Eye Icon (closed eye) -->
            <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="#2ecc71" viewBox="0 0 24 24">
              <path d="M2.39 1.73L1.11 3l2.05 2.05A11.897 11.897 0 0 0 0 12s3 7 12 7c2.32 0 4.34-.45 6.03-1.18l2.84 2.84 1.27-1.27-5.44-5.44L2.39 1.73zM12 5c2.61 0 4.83 1 6.5 2.44.9.81 1.67 1.71 2.33 2.56-.78 1.1-1.75 2.16-2.83 2.97l-1.43-1.43c.73-.53 1.39-1.16 1.99-1.84-1.46-1.74-3.33-3-5.56-3-1.15 0-2.25.28-3.23.75L7.8 5.79C8.86 5.28 10.37 5 12 5zm0 11c-2.61 0-4.83-1-6.5-2.44-.9-.81-1.67-1.71-2.33-2.56.52-.73 1.11-1.44 1.77-2.09l1.6 1.6c-.04.16-.04.33-.04.49 0 2.48 2.02 4.5 4.5 4.5.16 0 .33 0 .49-.04l1.6 1.6c-.64.31-1.33.54-2.09.68-.36.07-.73.11-1.1.11z"/>
            </svg>
          `;
        } else {
          passwordInput.type = 'password';
          togglePassword.innerHTML = `
            <svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" fill="#2ecc71" viewBox="0 0 24 24">
              <path d="M12 5c-7.633 0-11 6.5-11 7s3.367 7 11 7 11-6.5 11-7-3.367-7-11-7zm0 12c-2.761 0-5-2.239-5-5s2.239-5 5-5c2.762 0 5 2.239 5 5s-2.238 5-5 5zm0-8c-1.654 0-3 1.346-3 3s1.346 3 3 3c1.654 0 3-1.346 3-3s-1.346-3-3-3z"/>
            </svg>
          `;
        }
      });
    } else {
      console.error("Toggle password or password input not found.");
    }
    
    // Setup admin button on the navbar.
    const navbarList = document.getElementById('navbar-list');
    const adminItem = document.createElement('li');
    adminItem.id = "admin-item";
    if (document.cookie.indexOf("admin_session=valid") !== -1) {
      adminItem.innerHTML = '<a class="cta-button" href="#" id="admin-logout-btn">Logout</a>';
    } else {
      adminItem.innerHTML = '<a class="cta-button" href="#" id="admin-login-btn">Sign In</a>';
    }
    navbarList.appendChild(adminItem);
    
    // Attach event handler for Sign In.
    const signInBtn = document.getElementById('admin-login-btn');
    if (signInBtn) {
      signInBtn.addEventListener('click', function(e) {
        e.preventDefault();
        document.getElementById('login-modal').style.display = 'block';
      });
    }
    
    // Attach event handler for Logout.
    const logoutBtn = document.getElementById('admin-logout-btn');
    if (logoutBtn) {
      logoutBtn.addEventListener('click', function(e) {
        e.preventDefault();
        fetch('/admin/logout', { credentials: 'include' })
          .then(response => {
            if (response.ok) {
              const adminLink = logoutBtn.parentElement;
              adminLink.innerHTML = '<a class="cta-button" href="#" id="admin-login-btn">Sign In</a>';
              document.getElementById('admin-login-btn').addEventListener('click', function(e) {
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
    
    // Handle login form submission via AJAX.
    const loginForm = document.getElementById('loginForm');
    loginForm.addEventListener('submit', function(e) {
      e.preventDefault();
      const formData = new FormData(loginForm);
      fetch('/admin/login', {
        method: 'POST',
        credentials: 'include',
        headers: {
          "Content-Type": "application/x-www-form-urlencoded;charset=UTF-8"
        },
        body: new URLSearchParams(formData)
      })
      .then(response => response.text().then(text => ({ status: response.status, text: text })))
      .then(result => {
        if (result.status === 200) {
          const adminLink = document.getElementById('admin-login-btn').parentElement;
          adminLink.innerHTML = '<a class="cta-button" href="#" id="admin-logout-btn">Logout</a>';
          modal.style.display = 'none';
          document.getElementById('admin-logout-btn').addEventListener('click', function(e) {
            e.preventDefault();
            fetch('/admin/logout', { credentials: 'include' })
              .then(response => {
                if (response.ok) {
                  const adminLink = e.target.parentElement;
                  adminLink.innerHTML = '<a class="cta-button" href="#" id="admin-login-btn">Sign In</a>';
                  document.getElementById('admin-login-btn').addEventListener('click', function(e) {
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
          showToast("Login successful!");
        } else {
          const loginError = document.getElementById('login-error');
          loginError.style.display = 'block';
        }
      })
      .catch(err => console.error(err));
    });
    
    window.onclick = function(event) {
      if (event.target == modal) {
        modal.style.display = "none";
      }
    };
  })();
  