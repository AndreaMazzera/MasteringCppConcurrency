// ── Custom Marked Renderer ────────────────────────────────────
const renderer = new marked.Renderer();

renderer.paragraph = (text) => {
  const videoMatch = text.match(/^::video\((.+?)\)(?:\s+"(.+?)")?$/);
  if (videoMatch) {
    const caption = videoMatch[2]
      ? `<div class="fig-caption">${videoMatch[2]}</div>` : '';
    return `<div><video src="${videoMatch[1]}" controls></video>${caption}</div>`;
  }
  return `<p>${text}</p>`;
};

marked.setOptions({ renderer });

// ── Main Render Function ──────────────────────────────────────
async function renderFileList(fileList) {
  try {
    const markdowns = await Promise.all(
      fileList.map(file => fetch(file).then(r => {
        if (!r.ok) throw new Error(`File non trovato: ${file}`);
        return r.text();
      }))
    );

    const fullMarkdown = markdowns.join('\n\n---pagebreak---\n\n');
    const contentDiv = document.getElementById('content');
    contentDiv.innerHTML = ''; // Pulisce il container

    // Divide il testo dove trova '---pagebreak---' o '---'
    const pagesContent = fullMarkdown.split(/(?:\r?\n){2}---(?:\r?\n){2}|---pagebreak---/);

    pagesContent.forEach((pageMd, index) => {
      const pageNum = index + 1;

      // Crea il foglio A4
      const pageDiv = document.createElement('div');
      pageDiv.className = 'a4-page';
      pageDiv.id = `page-${pageNum}`;
      pageDiv.setAttribute('data-page-number', pageNum);

      // Converte il Markdown di questa specifica pagina
      pageDiv.innerHTML = marked.parse(pageMd);

      // Aggiunge la pagina al container
      contentDiv.appendChild(pageDiv);
    });

    // Applica gli Header del codice e le formule KaTeX su tutto il container
    setupCodeBlocks(contentDiv);

    renderMathInElement(contentDiv, {
      delimiters: [
        { left: '$$', right: '$$', display: true  },
        { left: '$',  right: '$',  display: false }
      ],
      throwOnError: false
    });

  } catch (error) {
    console.error('Errore:', error);
    document.getElementById('content').innerHTML =
      `<p style="color:red;">${error.message}</p>`;
  }
}

// ── Bootstrap for Browser Testing ─────────────────────────────
async function loadFromConfig() {
  const response = await fetch('filelist.json');
  const config = await response.json();
  renderFileList(config.files);
}

window.addEventListener('DOMContentLoaded', loadFromConfig);