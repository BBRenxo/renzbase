// RenzBase UI - frontend wiring
//
// Responsibilities:
//   - load Monaco from CDN (text/plain language; Luau highlighting TBD)
//   - tabs: array of {id, name, content, dirty, path?}
//   - sidebar: tree fetched from Rust `list_scripts`
//   - attach: call Rust `spawn_injector`
//   - execute: call Rust `execute_script` with current editor content
//   - save: call Rust `save_script` to persist into %APPDATA%\RenzBase\scripts
//   - console: stream Rust "console" events into the bottom panel

const { invoke } = window.__TAURI__.core;
const { listen } = window.__TAURI__.event;

// ---------- state ----------
const state = {
  tabs: [],                // [{id, name, content, dirty, path?}]
  activeTabId: null,
  nextTabId: 1,
  scripts: [],             // ScriptEntry[]
  filter: '',
};

let editor = null;

// ---------- helpers ----------
function uid() {
  return `t${state.nextTabId++}`;
}

function activeTab() {
  return state.tabs.find((t) => t.id === state.activeTabId) || null;
}

function log(level, msg) {
  const body = document.getElementById('console-body');
  if (!body) return;
  const line = document.createElement('div');
  line.className = `console-line ${level}`;
  const ts = new Date().toLocaleTimeString([], { hour12: false });
  line.innerHTML = `<span class="ts">[${ts}]</span>${escapeHtml(msg)}`;
  body.appendChild(line);
  body.scrollTop = body.scrollHeight;
}

function escapeHtml(s) {
  return String(s)
    .replace(/&/g, '&amp;')
    .replace(/</g, '&lt;')
    .replace(/>/g, '&gt;');
}

function setStatus(text) {
  const el = document.getElementById('status-text');
  if (el) el.textContent = text;
}

// ---------- tabs ----------
function renderTabs() {
  const list = document.getElementById('tabs-list');
  list.innerHTML = '';
  for (const tab of state.tabs) {
    const el = document.createElement('div');
    el.className = `tab ${tab.id === state.activeTabId ? 'active' : ''} ${tab.dirty ? 'dirty' : ''}`;
    el.title = tab.path || tab.name;

    const name = document.createElement('span');
    name.className = 'tab-name';
    name.textContent = tab.name;
    el.appendChild(name);

    const close = document.createElement('button');
    close.className = 'tab-close';
    close.textContent = '×';
    close.title = 'Close';
    close.addEventListener('click', (e) => {
      e.stopPropagation();
      closeTab(tab.id);
    });
    el.appendChild(close);

    el.addEventListener('click', () => activateTab(tab.id));
    list.appendChild(el);
  }
  const activeName = document.getElementById('active-tab-name');
  const a = activeTab();
  if (activeName) activeName.textContent = a ? (a.path || a.name) : '';
}

function addTab(name = 'untitled', content = '', path = null) {
  const tab = { id: uid(), name, content, dirty: false, path };
  state.tabs.push(tab);
  activateTab(tab.id);
  return tab;
}

function activateTab(id) {
  // flush current editor value into old active tab
  const cur = activeTab();
  if (cur && editor) cur.content = editor.getValue();

  state.activeTabId = id;
  const tab = activeTab();
  if (editor && tab) {
    editor.setValue(tab.content || '');
    editor.focus();
  }
  renderTabs();
}

function closeTab(id) {
  const idx = state.tabs.findIndex((t) => t.id === id);
  if (idx < 0) return;
  const wasActive = state.activeTabId === id;
  state.tabs.splice(idx, 1);
  if (state.tabs.length === 0) {
    // always keep at least one tab
    addTab('Start', '-- welcome to RenzBase\nprint("hello skid works")\n');
    return;
  }
  if (wasActive) {
    const next = state.tabs[Math.min(idx, state.tabs.length - 1)];
    activateTab(next.id);
  } else {
    renderTabs();
  }
}

// ---------- editor ----------
function initEditor() {
  require.config({ paths: { vs: 'https://cdn.jsdelivr.net/npm/monaco-editor@0.45.0/min/vs' } });

  require(['vs/editor/editor.main'], () => {
    // Register a minimal "luau" language so .luau files don't error.
    if (!monaco.languages.getLanguages().some((l) => l.id === 'luau')) {
      monaco.languages.register({ id: 'luau' });
      monaco.languages.setMonarchTokensProvider('luau', {
        tokenizer: {
          root: [
            [/--\[\[[\s\S]*?\]\]/, 'comment'],
            [/--.*$/, 'comment'],
            [/\b(local|function|end|if|then|else|elseif|for|while|do|return|true|false|nil|and|or|not|in|repeat|until|break)\b/, 'keyword'],
            [/\b(print|require|tonumber|tostring|type|pcall|xpcall|table|string|math|game|workspace|script)\b/, 'type'],
            [/"([^"\\]|\\.)*"/, 'string'],
            [/'([^'\\]|\\.)*'/, 'string'],
            [/\d+(\.\d+)?/, 'number'],
            [/--.*$/, 'comment'],
            [/[a-zA-Z_][a-zA-Z0-9_]*/, 'identifier'],
            [/[{}()\[\]]/, 'delimiter'],
            [/[+\-*/%=<>~^#]/, 'operator'],
          ],
        },
      });
    }

    editor = monaco.editor.create(document.getElementById('monaco-host'), {
      value: '-- welcome to RenzBase\nprint("hello skid works")\n',
      language: 'plaintext',
      theme: 'vs-dark',
      automaticLayout: true,
      minimap: { enabled: false },
      fontSize: 13,
      fontFamily: 'Consolas, Menlo, monospace',
      scrollBeyondLastLine: false,
      renderWhitespace: 'selection',
      tabSize: 2,
    });

    editor.onDidChangeModelContent(() => {
      const t = activeTab();
      if (t && editor.getValue() !== t.content) {
        t.content = editor.getValue();
        // mark dirty only if content differs from disk snapshot
        if (!t.savedSnapshot || t.content !== t.savedSnapshot) {
          if (!t.dirty) {
            t.dirty = true;
            renderTabs();
          }
        } else if (t.dirty) {
          t.dirty = false;
          renderTabs();
        }
      }
    });

    // Default tab now that editor exists.
    if (state.tabs.length === 0) {
      addTab('Start', '-- welcome to RenzBase\nprint("hello skid works")\n');
    } else {
      const t = activeTab();
      if (t && editor) editor.setValue(t.content);
    }
  });
}

// ---------- sidebar ----------
function renderSidebar() {
  const tree = document.getElementById('sidebar-tree');
  tree.innerHTML = '';

  const filter = state.filter.trim().toLowerCase();
  const matches = (name) => !filter || name.toLowerCase().includes(filter);

  const filtered = state.scripts
    .map((entry) => filterEntry(entry, matches))
    .filter(Boolean);

  if (filtered.length === 0) {
    const empty = document.createElement('div');
    empty.className = 'tree-empty';
    empty.textContent = 'no scripts yet — create one with the + button';
    tree.appendChild(empty);
    return;
  }

  for (const entry of filtered) {
    tree.appendChild(renderNode(entry, 0));
  }
}

function filterEntry(entry, matches) {
  if (entry.is_folder) {
    const kids = entry.children.map((c) => filterEntry(c, matches)).filter(Boolean);
    if (kids.length === 0 && !matches(entry.name)) return null;
    return { ...entry, children: kids };
  }
  if (!matches(entry.name)) return null;
  return entry;
}

function renderNode(entry, depth) {
  const wrap = document.createElement('div');

  const row = document.createElement('div');
  row.className = `tree-node ${entry.is_folder ? 'folder' : 'file'} ${entry.is_autoexecute ? 'autoexecute' : ''}`;
  row.style.paddingLeft = `${6 + depth * 12}px`;

  const twisty = document.createElement('span');
  twisty.className = 'twisty';
  twisty.textContent = entry.is_folder ? '▾' : '';
  row.appendChild(twisty);

  const icon = document.createElement('span');
  icon.className = 'icon';
  icon.textContent = entry.is_folder ? '▣' : '▤';
  row.appendChild(icon);

  const name = document.createElement('span');
  name.className = 'name';
  name.textContent = entry.name;
  row.appendChild(name);

  row.addEventListener('click', () => {
    if (entry.is_folder) return;
    openScript(entry);
    document.querySelectorAll('.tree-node.selected').forEach((n) => n.classList.remove('selected'));
    row.classList.add('selected');
  });

  wrap.appendChild(row);

  if (entry.is_folder && entry.children.length > 0) {
    const kids = document.createElement('div');
    kids.className = 'tree-children';
    for (const c of entry.children) {
      kids.appendChild(renderNode(c, depth + 1));
    }
    wrap.appendChild(kids);
  }

  return wrap;
}

function openScript(entry) {
  // load file content via fetch on file://? Not allowed in Tauri. Use a Rust command.
  // For now we trust that sidebar entries are leaf files; the Rust side reads them
  // through the save_script path. We'll add a `read_script` command if needed.
  // For the skeleton, we just open a tab and let the user paste/save.
  const tab = addTab(entry.name, `-- loaded: ${entry.name}\nprint("from ${entry.name}")\n`, entry.path);
  tab.savedSnapshot = tab.content;
  tab.dirty = false;
  renderTabs();
}

async function refreshScripts() {
  try {
    const list = await invoke('list_scripts');
    state.scripts = list || [];
    renderSidebar();
  } catch (e) {
    log('error', `list_scripts failed: ${e}`);
  }
}

// ---------- actions ----------
async function attach() {
  const btn = document.getElementById('attach-btn');
  btn.disabled = true;
  setStatus('attaching...');
  try {
    await invoke('spawn_injector');
    document.getElementById('attach-status').textContent = '1 client attached';
    document.getElementById('attach-status').className = 'status-pill status-attached';
    setStatus('attached');
    document.getElementById('status-right').textContent = 'no errors';
  } catch (e) {
    log('error', `attach failed: ${e}`);
    setStatus('attach failed');
  } finally {
    btn.disabled = false;
  }
}

async function executeCurrent() {
  if (!editor) return;
  const script = editor.getValue();
  if (!script.trim()) {
    log('dim', 'nothing to execute');
    return;
  }
  log('dim', `executing ${script.length} chars...`);
  try {
    await invoke('execute_script', { script });
    log('success', 'script sent to pipe');
  } catch (e) {
    log('error', `execute failed: ${e}`);
  }
}

async function saveCurrent() {
  const tab = activeTab();
  if (!tab || !editor) return;
  let name = tab.name;
  if (!name.endsWith('.luau') && !name.endsWith('.lua')) name += '.luau';
  try {
    const content = editor.getValue();
    const path = await invoke('save_script', { name, content });
    tab.name = name;
    tab.path = path;
    tab.savedSnapshot = content;
    tab.dirty = false;
    renderTabs();
    await refreshScripts();
    log('success', `saved ${name}`);
  } catch (e) {
    log('error', `save failed: ${e}`);
  }
}

async function newScript() {
  const name = window.prompt('New script name (e.g. foo.luau):', 'untitled.luau');
  if (!name) return;
  let n = name.trim();
  if (!n) return;
  if (!n.endsWith('.luau') && !n.endsWith('.lua')) n += '.luau';
  try {
    const path = await invoke('save_script', { name: n, content: '-- new script\n' });
    await refreshScripts();
    addTab(n, '-- new script\n', path).savedSnapshot = '-- new script\n';
    renderTabs();
    log('success', `created ${n}`);
  } catch (e) {
    log('error', `new script failed: ${e}`);
  }
}

// ---------- wire up ----------
window.addEventListener('DOMContentLoaded', () => {
  document.getElementById('tab-new').addEventListener('click', () => addTab('untitled', ''));
  document.getElementById('new-script-btn').addEventListener('click', newScript);
  document.getElementById('attach-btn').addEventListener('click', attach);
  document.getElementById('execute-btn').addEventListener('click', executeCurrent);
  document.getElementById('save-btn').addEventListener('click', saveCurrent);
  document.getElementById('console-clear').addEventListener('click', () => {
    document.getElementById('console-body').innerHTML = '';
  });

  document.getElementById('script-filter').addEventListener('input', (e) => {
    state.filter = e.target.value;
    renderSidebar();
  });

  // Ctrl+S to save, Ctrl+Enter to execute
  window.addEventListener('keydown', (e) => {
    if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 's') {
      e.preventDefault();
      saveCurrent();
    } else if ((e.ctrlKey || e.metaKey) && e.key === 'Enter') {
      e.preventDefault();
      executeCurrent();
    } else if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'n') {
      e.preventDefault();
      addTab('untitled', '');
    } else if ((e.ctrlKey || e.metaKey) && e.key.toLowerCase() === 'w') {
      e.preventDefault();
      const t = activeTab();
      if (t) closeTab(t.id);
    }
  });

  // Console event stream from Rust.
  listen('console', (event) => {
    const msg = String(event.payload || '');
    if (msg.startsWith('[error]')) log('error', msg.replace(/^\[error\]\s*/, ''));
    else if (msg.startsWith('[ok]')) log('success', msg.replace(/^\[ok\]\s*/, ''));
    else log('dim', msg);
  });

  initEditor();
  refreshScripts();
  log('dim', 'RenzBase UI ready');
  setStatus('ready');
});
