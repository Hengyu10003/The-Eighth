/* ==========================================================================
   核心引擎：素材加载、画布缩放、输入、场景调度、音频
   ========================================================================== */
"use strict";

const G = {
  W: 1201,            // 逻辑画布宽（等于原 Qt 窗口宽度）
  H: 676,             // 逻辑画布高
  images: {},         // 原始文件名 -> HTMLImageElement
  manifest: {},       // 原始文件名 -> 实际文件名（压缩后可能换了扩展名）
  keys: {},           // 当前按下的键（小写）
  scene: null,        // 当前场景
  mouse: { x: 0, y: 0, down: false },
  base: "assets/",
  time: 0,
  musicOn: true,
  audio: { bg: null, btn: null },
  ink: { r: 0, g: 0, b: 0, a: 0 },   // 上一帧的墨迹残留，由 ui.js 使用
};

/* ---------- 素材路径 ---------- */
G.imagePath = function (name) {
  return G.base + "images/" + (G.manifest[name] || name);
};
G.audioPath = function (name) {
  return G.base + "audio/" + (G.manifest[name] || name);
};

/* ---------- 加载全部素材 ---------- */
G.load = function (onProgress) {
  return fetch(G.base + "images/manifest.json")
    .then(r => r.json())
    .then(m => {
      G.manifest = m;
      const names = Object.keys(m).filter(k => k.indexOf("music/") !== 0);
      let done = 0;
      return Promise.all(names.map(function (n) {
        return new Promise(function (res) {
          const im = new Image();
          im.onload = im.onerror = function () {
            done++;
            if (onProgress) onProgress(done, names.length);
            res();
          };
          im.src = G.imagePath(n);
          G.images[n] = im;
        });
      }));
    });
};

/* ---------- 取图 / 判断是否就绪 ---------- */
G.img = function (name) { return G.images[name]; };
G.ready = function (name) {
  const im = G.images[name];
  return !!im && im.complete && im.naturalWidth > 0;
};

/* 只在图片就绪时绘制，避免素材缺失导致整帧报错 */
G.draw = function (ctx, name, x, y, w, h) {
  if (!G.ready(name)) return false;
  ctx.drawImage(G.images[name], x, y, w, h);
  return true;
};

/* 按原始尺寸绘制 */
G.drawRaw = function (ctx, name, x, y) {
  const im = G.images[name];
  if (!im || !im.complete || !im.naturalWidth) return false;
  ctx.drawImage(im, x, y);
  return true;
};

/* 等比缩放到能放进 (x,y,w,h) 并居中 */
G.drawFit = function (ctx, name, x, y, w, h) {
  const im = G.images[name];
  if (!im || !im.complete || !im.naturalWidth) return false;
  const s = Math.min(w / im.naturalWidth, h / im.naturalHeight);
  const dw = im.naturalWidth * s, dh = im.naturalHeight * s;
  ctx.drawImage(im, x + (w - dw) / 2, y + (h - dh) / 2, dw, dh);
  return true;
};

/* ---------- 音频 ---------- */
G.initAudio = function () {
  if (G.audio.bg) return;
  G.audio.bg = new Audio(G.audioPath("music/background.wav"));
  G.audio.bg.loop = true;
  G.audio.bg.volume = 0.5;
  G.audio.btn = new Audio(G.audioPath("music/button.wav"));
  G.audio.btn.volume = 0.7;
};
G.playMusic = function () {
  if (!G.audio.bg) return;
  G.audio.bg.play().catch(function () { /* 浏览器可能拦截自动播放，等用户交互后再试 */ });
};
G.playBtn = function () {
  if (!G.audio.btn) return;
  try { G.audio.btn.currentTime = 0; G.audio.btn.play(); } catch (e) { }
};

/* ---------- 场景切换 ---------- */
G.setScene = function (scene) {
  if (G.scene && G.scene.exit) G.scene.exit();
  G.scene = scene;
  if (scene && scene.enter) scene.enter();
};

/* ---------- 文字 ---------- */
G.FONT = '"华文隶书","STLiti","LiSu","楷体",KaiTi,"SimSun",serif';
G.FONT_SANS = '"Microsoft YaHei","PingFang SC",sans-serif';

G.text = function (ctx, str, x, y, size, color, align, font) {
  ctx.save();
  ctx.font = size + "px " + (font || G.FONT);
  ctx.fillStyle = color || "#2b1c12";
  ctx.textAlign = align || "left";
  ctx.textBaseline = "middle";
  ctx.fillText(str, x, y);
  ctx.restore();
};

/* ---------- 命中测试 ---------- */
G.hit = function (x, y, r) {
  return x >= r[0] && x <= r[0] + r[2] && y >= r[1] && y <= r[1] + r[3];
};

/* ==========================================================================
   启动
   ========================================================================== */
(function () {
  const canvas = document.getElementById("screen");
  const ctx = canvas.getContext("2d");
  const wrap = document.getElementById("stage");
  const loading = document.getElementById("loading");
  const bar = document.querySelector("#bar i");
  const loadText = document.getElementById("loadText");

  /* 画布等比缩放：CSS 控尺寸，鼠标坐标换算回逻辑坐标 */
  function toLogical(clientX, clientY) {
    const r = canvas.getBoundingClientRect();
    return {
      x: (clientX - r.left) * (G.W / r.width),
      y: (clientY - r.top) * (G.H / r.height),
    };
  }

  window.addEventListener("keydown", function (e) {
    const k = e.key.length === 1 ? e.key.toLowerCase() : e.key;
    if ([" ", "ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight"].indexOf(e.key) >= 0) e.preventDefault();
    G.keys[k] = true;
    if (G.scene && G.scene.onKeyDown) G.scene.onKeyDown(k, e);
    /* 原始字符单独回调一次，供需要区分大小写的场景（如输入昵称）使用 */
    if (e.key.length === 1 && G.scene && G.scene.onText) G.scene.onText(e.key, e);
  });
  window.addEventListener("keyup", function (e) {
    const k = e.key.length === 1 ? e.key.toLowerCase() : e.key;
    G.keys[k] = false;
    if (G.scene && G.scene.onKeyUp) G.scene.onKeyUp(k, e);
  });

  canvas.addEventListener("mousedown", function (e) {
    const p = toLogical(e.clientX, e.clientY);
    G.mouse.x = p.x; G.mouse.y = p.y; G.mouse.down = true;
    G.initAudio(); G.playMusic();
    if (G.scene && G.scene.onMouseDown) G.scene.onMouseDown(p.x, p.y);
  });
  canvas.addEventListener("mouseup", function (e) {
    const p = toLogical(e.clientX, e.clientY);
    G.mouse.x = p.x; G.mouse.y = p.y; G.mouse.down = false;
    if (G.scene && G.scene.onMouseUp) G.scene.onMouseUp(p.x, p.y);
  });
  canvas.addEventListener("mousemove", function (e) {
    const p = toLogical(e.clientX, e.clientY);
    G.mouse.x = p.x; G.mouse.y = p.y;
    if (G.scene && G.scene.onMouseMove) G.scene.onMouseMove(p.x, p.y);
  });
  canvas.addEventListener("contextmenu", function (e) { e.preventDefault(); });

  /* 首屏任意交互都尝试恢复音频（绕过自动播放限制） */
  function kickAudio() {
    G.initAudio();
    if (G.musicOn) G.playMusic();
    window.removeEventListener("pointerdown", kickAudio);
    window.removeEventListener("keydown", kickAudio);
  }
  window.addEventListener("pointerdown", kickAudio);
  window.addEventListener("keydown", kickAudio);

  let last = 0;
  function frame(ts) {
    if (!last) last = ts;
    let dt = (ts - last) / 1000;
    last = ts;
    if (dt > 0.1) dt = 0.1;          // 切标签页回来时避免物理穿透
    G.time += dt;

    ctx.clearRect(0, 0, G.W, G.H);
    if (G.scene) {
      if (G.scene.update) G.scene.update(dt);
      if (G.scene.draw) G.scene.draw(ctx);
    }
    requestAnimationFrame(frame);
  }

  G.load(function (done, total) {
    const p = Math.round(done / total * 100);
    bar.style.width = p + "%";
    loadText.textContent = "正在加载素材… " + p + "%";
  }).then(function () {
    loading.classList.add("hide");
    setTimeout(function () { loading.style.display = "none"; }, 500);
    if (typeof App !== "undefined" && App.start) App.start(ctx);
    requestAnimationFrame(frame);
  }).catch(function (err) {
    loadText.textContent = "素材加载失败：" + err;
  });
})();
