/**
 * Spring 2026, CSE 498 Sec 2 - Company C
 * WebPopup implementation - for popup messages 
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

#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace cse498 {

#ifdef __EMSCRIPTEN__

EM_JS(void, cse498_popup_enqueue_js, (const char* msg_ptr), {
  var msg = UTF8ToString(msg_ptr);
  if (!window.__cse498Popup) {
    window.__cse498Popup = { queue: [], showing: false };
  }
  var st = window.__cse498Popup;
  st.queue.push(msg);

  function showNext() {
    if (st.showing) return;
    if (st.queue.length === 0) return;
    st.showing = true;
    var text = st.queue.shift();
    var old = document.getElementById('cse498_popup_overlay');
    if (old) old.remove();

    var overlay = document.createElement('div');
    overlay.id = 'cse498_popup_overlay';
    overlay.setAttribute('role', 'dialog');
    overlay.setAttribute('aria-modal', 'true');
    overlay.style.cssText = [
      'position:fixed', 'inset:0', 'background:rgba(0,0,0,0.45)',
      'display:flex', 'align-items:center', 'justify-content:center',
      'z-index:10000', 'font-family:system-ui,Arial,sans-serif'
    ].join(';');

    var box = document.createElement('div');
    box.style.cssText = [
      'background:#fff', 'border-radius:12px', 'padding:20px 24px',
      'max-width:min(420px,calc(100vw - 48px))',
      'box-shadow:0 10px 30px rgba(0,0,0,0.2)', 'border:1px solid #d1d5db'
    ].join(';');

    var p = document.createElement('div');
    p.style.cssText = 'margin:0 0 16px 0;font-size:16px;line-height:1.45;color:#111827;white-space:pre-wrap;';
    p.textContent = text;

    var btn = document.createElement('button');
    btn.type = 'button';
    btn.textContent = 'OK';
    btn.style.cssText = 'padding:8px 16px;border:1px solid #9ca3af;border-radius:8px;background:#fff;cursor:pointer;';

    var dismiss = function() {
      overlay.remove();
      st.showing = false;
      showNext();
    };
    btn.onclick = dismiss;
    overlay.onclick = function(ev) {
      if (ev.target === overlay) dismiss();
    };

    box.appendChild(p);
    box.appendChild(btn);
    overlay.appendChild(box);
    document.body.appendChild(overlay);
    btn.focus();
  }
  showNext();
});

#endif

void EnqueueWebPopup(const std::string& message) {
#ifdef __EMSCRIPTEN__
  cse498_popup_enqueue_js(message.c_str());
#else
  (void)message;
#endif
}

}  // namespace cse498
