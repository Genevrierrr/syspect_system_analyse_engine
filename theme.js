(function () {
  const STORAGE_KEY = 'syspect-theme';
  const root = document.documentElement;
  const media = window.matchMedia ? window.matchMedia('(prefers-color-scheme: dark)') : null;

  function savedTheme() {
    try {
      const value = localStorage.getItem(STORAGE_KEY);
      return value === 'dark' || value === 'light' ? value : null;
    } catch (_) { return null; }
  }

  function preferredTheme() {
    return savedTheme() || (media && media.matches ? 'dark' : 'light');
  }

  function applyTheme(theme, persist) {
    root.dataset.theme = theme;
    if (persist) {
      try { localStorage.setItem(STORAGE_KEY, theme); } catch (_) {}
    }
    const button = document.querySelector('.theme-toggle');
    if (button) {
      const next = theme === 'dark' ? 'light' : 'dark';
      button.setAttribute('aria-label', `Switch to ${next} mode`);
      button.setAttribute('title', `Switch to ${next} mode`);
      button.setAttribute('aria-pressed', theme === 'dark' ? 'true' : 'false');
    }
  }

  // Apply before first paint as early as possible.
  applyTheme(preferredTheme(), false);

  function buildToggle() {
    if (document.querySelector('.theme-toggle')) return;
    const host = document.querySelector('.top-right') || document.querySelector('.topbar');
    if (!host) return;

    const button = document.createElement('button');
    button.type = 'button';
    button.className = 'theme-toggle';
    button.innerHTML = `
      <svg class="theme-sun" viewBox="0 0 24 24" aria-hidden="true"><circle cx="12" cy="12" r="4"></circle><path d="M12 2v2M12 20v2M4.93 4.93l1.41 1.41M17.66 17.66l1.41 1.41M2 12h2M20 12h2M4.93 19.07l1.41-1.41M17.66 6.34l1.41-1.41"></path></svg>
      <svg class="theme-moon" viewBox="0 0 24 24" aria-hidden="true"><path d="M21 12.79A9 9 0 1 1 11.21 3 7 7 0 0 0 21 12.79Z"></path></svg>`;
    button.addEventListener('click', function () {
      applyTheme(root.dataset.theme === 'dark' ? 'light' : 'dark', true);
    });
    host.prepend(button);
    applyTheme(root.dataset.theme || preferredTheme(), false);
  }

  function buildProfileShortcut() {
    if (document.querySelector('.profile-shortcut')) return;
    const host = document.querySelector('.top-right') || document.querySelector('.topbar');
    if (!host) return;

    const link = document.createElement('a');
    link.className = 'profile-shortcut';
    link.href = 'profile.html';
    link.setAttribute('aria-label', 'Profile');
    link.setAttribute('title', 'Profile');
    if (/\/profile\.html$/i.test(window.location.pathname)) link.setAttribute('aria-current', 'page');
    link.innerHTML = `
      <span class="profile-shortcut-avatar" aria-hidden="true">
        <svg viewBox="0 0 24 24"><circle cx="12" cy="8" r="3.5"></circle><path d="M5 20c.7-4 3.2-6 7-6s6.3 2 7 6"></path></svg>
      </span>
      <span class="profile-shortcut-label">Profile</span>`;
    host.append(link);
  }

  function buildSharedControls() {
    buildToggle();
    buildProfileShortcut();
  }

  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', buildSharedControls);
  else buildSharedControls();

  if (media && media.addEventListener) {
    media.addEventListener('change', function (event) {
      if (!savedTheme()) applyTheme(event.matches ? 'dark' : 'light', false);
    });
  }
})();
