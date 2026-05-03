/**
 * Spring 2026, CSE 498 Sec 2 - Company C
 * WebPopup — modal messages using WebTextbox + WebButton on a thin DOM shell.
 *
 * Citation - LLM (OpenAI) was used to help generate parts of this file,
 * and maintain consistency with the project. The code was then reviewed
 * and heavily edited by the author to ensure correctness and suitability
 * for the project.
 * @author Prijam Khanal
 * Copyright (c) 2026 Prijam Khanal
 * SPDX-License-Identifier: MIT
 */
#include "tools/WebPopup.hpp"

#include <optional>
#include <string>

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

#include <memory>
#include <queue>
#include <utility>

#include "tools/WebButton.hpp"
#include "tools/WebTextbox.hpp"

namespace cse498::popup_detail {

std::queue<WebPopupRequest>         g_queue;
bool                                g_showing = false;
std::unique_ptr<WebTextbox>         g_text;
std::unique_ptr<WebButton>          g_ok;

void TryShowNextPopup();
void PopupDismiss();

EM_JS(void, cse498_popup_create_shell, (), {
  var old = document.getElementById('cse498_popup_overlay');
  if (old) old.remove();

  var overlay = document.createElement('div');
  overlay.id = 'cse498_popup_overlay';
  overlay.setAttribute('role', 'dialog');
  overlay.setAttribute('aria-modal', 'true');
  overlay.setAttribute('tabindex', '-1');
  overlay.style.cssText = [
    'position:fixed', 'inset:0', 'z-index:10000',
    'display:flex', 'align-items:center', 'justify-content:center',
    'padding:max(16px,env(safe-area-inset-top)) max(16px,env(safe-area-inset-right)) max(16px,env(safe-area-inset-bottom)) max(16px,env(safe-area-inset-left))',
    'box-sizing:border-box',
    'background:rgba(24,69,59,0.48)',
    'backdrop-filter:saturate(1.1) blur(6px)',
    '-webkit-backdrop-filter:saturate(1.1) blur(6px)',
    'font-family:system-ui,-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,sans-serif'
  ].join(';');

  var box = document.createElement('div');
  box.style.cssText = [
    'width:100%', 'max-width:min(420px,calc(100vw - 32px))',
    'border-radius:18px', 'overflow:hidden',
    'background:linear-gradient(165deg,#e8f5f0 0%,#c5e8dc 42%,#7ec9b8 100%)',
    'box-shadow:0 25px 50px -12px rgba(24,69,59,0.38),0 0 0 1px rgba(255,255,255,0.55) inset,0 0 0 1px rgba(24,69,59,0.12)'
  ].join(';');

  var accent = document.createElement('div');
  accent.style.cssText = [
    'height:5px', 'width:100%',
    'background:linear-gradient(90deg,#18453B 0%,#0f766e 55%,#14b8a6 100%)'
  ].join(';');

  var inner = document.createElement('div');
  inner.style.cssText = [
    'padding:26px 28px 22px', 'box-sizing:border-box',
    'display:flex', 'flex-direction:column', 'align-items:center', 'gap:22px'
  ].join(';');

  var msgHost = document.createElement('div');
  msgHost.id = 'cse498_popup_msg_host';
  msgHost.style.cssText = 'width:100%;box-sizing:border-box';

  var btnHost = document.createElement('div');
  btnHost.id = 'cse498_popup_btn_host';
  btnHost.style.cssText =
      'display:flex;justify-content:center;width:100%;box-sizing:border-box';

  inner.appendChild(msgHost);
  inner.appendChild(btnHost);
  box.appendChild(accent);
  box.appendChild(inner);
  overlay.appendChild(box);
  document.body.appendChild(overlay);

  window.__cse498PopupOverlayClick = function(ev) {
    if (ev.target === overlay) {
      Module.ccall('cse498_popup_dismiss', null, [], []);
    }
  };
  overlay.addEventListener('click', window.__cse498PopupOverlayClick);

  window.__cse498PopupKeydown = function(ev) {
    if (ev.key === 'Escape') {
      ev.preventDefault();
      Module.ccall('cse498_popup_dismiss', null, [], []);
    }
  };
  document.addEventListener('keydown', window.__cse498PopupKeydown);
});

EM_JS(void, cse498_popup_remove_shell, (), {
  if (window.__cse498PopupTimer) {
    clearTimeout(window.__cse498PopupTimer);
    window.__cse498PopupTimer = null;
  }
  var overlay = document.getElementById('cse498_popup_overlay');
  if (overlay && window.__cse498PopupOverlayClick) {
    overlay.removeEventListener('click', window.__cse498PopupOverlayClick);
    window.__cse498PopupOverlayClick = null;
  }
  if (window.__cse498PopupKeydown) {
    document.removeEventListener('keydown', window.__cse498PopupKeydown);
    window.__cse498PopupKeydown = null;
  }
  if (overlay) overlay.remove();
});

EM_JS(void, cse498_popup_focus_el, (const char* id_ptr), {
  var id = UTF8ToString(id_ptr);
  var el = document.getElementById(id);
  if (el) el.focus();
});

EM_JS(void, cse498_popup_focus_overlay, (), {
  var o = document.getElementById('cse498_popup_overlay');
  if (o) o.focus();
});

EM_JS(void, cse498_popup_btn_host_visible, (int show), {
  var h = document.getElementById('cse498_popup_btn_host');
  if (h) h.style.display = show ? 'flex' : 'none';
});

EM_JS(void, cse498_popup_arm_timer, (int ms), {
  if (window.__cse498PopupTimer) {
    clearTimeout(window.__cse498PopupTimer);
    window.__cse498PopupTimer = null;
  }
  if (ms <= 0) return;
  window.__cse498PopupTimer = setTimeout(function() {
    window.__cse498PopupTimer = null;
    Module.ccall('cse498_popup_dismiss', null, [], []);
  }, ms);
});

// Defer dismiss to the next event turn so WebButton is not destroyed during Click().
EM_JS(void, cse498_popup_schedule_dismiss, (), {
  setTimeout(function() {
    Module.ccall('cse498_popup_dismiss', null, [], []);
  }, 0);
});

WebPopupOptions NormalizePopupOptions(const WebPopupOptions& input) {
  WebPopupOptions opts = input;
  if (!opts.show_ok_button && !opts.auto_dismiss) {
    opts.show_ok_button = true;
  }
  return opts;
}

std::optional<int> ResolveAutoDismissDelayMs(const WebPopupOptions& opts) {
  if (!opts.auto_dismiss) return std::nullopt;
  if (!opts.auto_dismiss_ms.has_value()) return 2000;
  if (*opts.auto_dismiss_ms <= 0) return std::nullopt;
  return *opts.auto_dismiss_ms;
}

void BuildPopupMessage(std::string message) {
  g_text = std::make_unique<WebTextbox>();
  g_text->SetParentId("cse498_popup_msg_host");
  g_text->Create();
  g_text->SetText(std::move(message));
  g_text->ApplyFlowLayoutStyles();
  g_text->SetFontFamily(
      "system-ui, -apple-system, BlinkMacSystemFont, \"Segoe UI\", Roboto, "
      "sans-serif");
  g_text->SetFontSize(17);
  g_text->SetLineHeight(1.55);
  g_text->SetFontWeight("500");
  g_text->SetTextColor("#134e43");
  g_text->SetTextAlignment("center");
  g_text->SetWordWrap("break-word");
  g_text->SetBackgroundColor("transparent");
}

void BuildPopupDismissControl(const WebPopupOptions& opts) {
  if (opts.show_ok_button) {
    cse498_popup_btn_host_visible(1);
    g_ok = std::make_unique<WebButton>("OK");
    g_ok->SetBorderWidth(0);
    g_ok->SetBackgroundColor("#18453B");
    g_ok->SetTextColor("#ffffff");
    g_ok->SetBorderRadius(999);
    g_ok->SetSize(168, 46);
    g_ok->AppendTo("cse498_popup_btn_host");
    g_ok->SetOnClick([]() { cse498_popup_schedule_dismiss(); });
    cse498_popup_focus_el(g_ok->GetElementID().c_str());
  } else {
    cse498_popup_btn_host_visible(0);
    cse498_popup_focus_overlay();
  }
}

void ArmAutoDismissTimer(const WebPopupOptions& opts) {
  const std::optional<int> delay_ms = ResolveAutoDismissDelayMs(opts);
  if (!delay_ms.has_value()) return;
  cse498_popup_arm_timer(*delay_ms);
}

void ShowOnePopup(WebPopupRequest req) {
  WebPopupOptions opts = NormalizePopupOptions(req.options);
  g_showing = true;
  cse498_popup_create_shell();
  BuildPopupMessage(std::move(req.message));
  BuildPopupDismissControl(opts);
  ArmAutoDismissTimer(opts);
}

void PopupDismiss() {
  if (!g_showing) return;
  g_text.reset();
  g_ok.reset();
  cse498_popup_remove_shell();
  g_showing = false;
  TryShowNextPopup();
}

void TryShowNextPopup() {
  if (g_showing) return;
  if (g_queue.empty()) return;
  WebPopupRequest next = std::move(g_queue.front());
  g_queue.pop();
  ShowOnePopup(std::move(next));
}

void EnqueuePush(WebPopupRequest req) {
  g_queue.push(std::move(req));
  TryShowNextPopup();
}

}  // namespace cse498::popup_detail

extern "C" {

EMSCRIPTEN_KEEPALIVE
void cse498_popup_dismiss(void) {
  cse498::popup_detail::PopupDismiss();
}

}

#endif  // __EMSCRIPTEN__

namespace cse498 {

void EnqueueWebPopup(const std::string& message, const WebPopupOptions& options) {
#ifdef __EMSCRIPTEN__
  popup_detail::EnqueuePush(WebPopupRequest{message, options});
#else
  (void)message;
  (void)options;
#endif
}

}  // namespace cse498
