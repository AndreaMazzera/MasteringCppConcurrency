// ── Code Block Setup & Copy Logic ──────────────────────────────
function setupCodeBlocks(container) {
  const codeBlocks = container.querySelectorAll('pre code');

  codeBlocks.forEach((codeEl) => {
    const preEl = codeEl.parentElement;

    // Avoid re-processing already wrapped blocks
    if (preEl.parentElement.classList.contains('code-wrapper')) return;

    // Extract language name from class (e.g. language-cpp -> cpp)
    const langClass = Array.from(codeEl.classList).find(c => c.startsWith('language-'));
    const langName = langClass ? langClass.replace('language-', '') : 'code';

    // Build Header DOM
    const wrapper = document.createElement('div');
    wrapper.className = 'code-wrapper';

    const header = document.createElement('div');
    header.className = 'code-header';
    header.innerHTML = `
      <span class="code-lang">${langName}</span>
      <button class="copy-btn" type="button">
        <svg class="copy-icon" viewBox="0 0 24 24" width="14" height="14" stroke="currentColor" stroke-width="2" fill="none">
          <rect x="9" y="9" width="13" height="13" rx="2" ry="2"></rect>
          <path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"></path>
        </svg>
        <span class="copy-text">Copy</span>
      </button>
    `;

    // Insert wrapper into DOM
    preEl.parentNode.insertBefore(wrapper, preEl);
    wrapper.appendChild(header);
    wrapper.appendChild(preEl);

    // Copy event listener
    const copyBtn = header.querySelector('.copy-btn');
    copyBtn.addEventListener('click', () => {
      const textToCopy = codeEl.innerText;

      if (navigator.clipboard && window.isSecureContext) {
        navigator.clipboard.writeText(textToCopy).then(showCopiedState);
      } else {
        const textarea = document.createElement('textarea');
        textarea.value = textToCopy;
        textarea.style.position = 'fixed';
        textarea.style.opacity = '0';
        document.body.appendChild(textarea);
        textarea.select();
        try {
          document.execCommand('copy');
          showCopiedState();
        } catch (err) {
          console.error('Copy failed:', err);
        }
        document.body.removeChild(textarea);
      }

      function showCopiedState() {
        const copyText = copyBtn.querySelector('.copy-text');
        copyText.textContent = 'Copied!';
        copyBtn.classList.add('copied');

        setTimeout(() => {
          copyText.textContent = 'Copy';
          copyBtn.classList.remove('copied');
        }, 2000);
      }
    });
  });
}