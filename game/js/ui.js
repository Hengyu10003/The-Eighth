/* ==========================================================================
   界面层：通用对话框（按原 Qt 对话框的像素坐标复刻）、文字输入、排行榜表格
   ========================================================================== */

const UI = {};

/* ---------- 通用对话框 ----------
   def = {
     w, h,                 // 原对话框客户区尺寸
     bg: "xxx.png",        // 背景图
     buttons: [ { x,y,w,h, img?, label?, size?, color?, on } ],
     drawExtra(ctx, s),    // 可选的额外绘制（在对话框坐标系内）
   }
*/
UI.dialog = function (def) {
  const s = Math.min(G.W / def.w, G.H / def.h);
  const ox = (G.W - def.w * s) / 2;
  const oy = (G.H - def.h * s) / 2;

  return {
    scale: s,
    ox: ox,
    oy: oy,
    hover: -1,

    toLocal: function (x, y) { return { x: (x - ox) / s, y: (y - oy) / s }; },

    update: function () { },

    draw: function (ctx) {
      ctx.fillStyle = "rgba(0,0,0,0.55)";
      ctx.fillRect(0, 0, G.W, G.H);

      ctx.save();
      ctx.translate(ox, oy);
      ctx.scale(s, s);

      G.draw(ctx, def.bg, 0, 0, def.w, def.h);

      if (def.drawExtra) def.drawExtra(ctx, s, this);

      if (def.buttons) {
        for (let i = 0; i < def.buttons.length; i++) {
          const b = def.buttons[i];
          if (b.hidden) continue;
          if (b.img && G.ready(b.img)) {
            ctx.drawImage(G.img(b.img), b.x, b.y, b.w, b.h);
            if (b.label) {
              G.text(ctx, b.label, b.x + b.w / 2, b.y + b.h / 2, b.size || 28, b.color || "#5a2418", "center");
            }
          } else {
            ctx.fillStyle = b.bg || "rgba(60,30,20,0.85)";
            ctx.fillRect(b.x, b.y, b.w, b.h);
            G.text(ctx, b.label || "", b.x + b.w / 2, b.y + b.h / 2, b.size || 28, b.color || "#ffe9c9", "center");
          }
          if (this.hover === i) {
            ctx.fillStyle = "rgba(255,255,255,0.18)";
            ctx.fillRect(b.x, b.y, b.w, b.h);
          }
        }
      }
      ctx.restore();
    },

    onMouseMove: function (x, y) {
      this.hover = -1;
      if (!def.buttons) return;
      const p = this.toLocal(x, y);
      for (let i = 0; i < def.buttons.length; i++) {
        const b = def.buttons[i];
        if (b.hidden) continue;
        if (G.hit(p.x, p.y, [b.x, b.y, b.w, b.h])) { this.hover = i; break; }
      }
    },

    onMouseDown: function (x, y) {
      if (!def.buttons) return;
      const p = this.toLocal(x, y);
      for (let i = 0; i < def.buttons.length; i++) {
        const b = def.buttons[i];
        if (b.hidden) continue;
        if (G.hit(p.x, p.y, [b.x, b.y, b.w, b.h])) {
          G.playBtn();
          if (b.on) b.on();
          return;
        }
      }
    },
  };
};

/* ---------- 排行榜表格（在原对话框中绘制） ---------- */
UI.drawLeaderboardTable = function (ctx, x, y, w, h, rows) {
  const colW = [60, 150, 120, 130];
  const heads = ["排名", "昵称", "碎片数", "完成时间"];
  const rowH = 34;

  ctx.save();
  ctx.fillStyle = "rgba(60,28,24,0.55)";
  ctx.fillRect(x, y, w, h);

  // 表头
  ctx.fillStyle = "rgba(40,16,12,0.75)";
  ctx.fillRect(x, y, w, rowH);
  let cx = x + 8;
  for (let i = 0; i < heads.length; i++) {
    G.text(ctx, heads[i], cx + colW[i] / 2, y + rowH / 2, 20, "#ffe3bd", "center");
    cx += colW[i];
  }
  ctx.strokeStyle = "rgba(255,220,180,0.35)";
  ctx.lineWidth = 1;
  for (let i = 1; i < heads.length; i++) {
    let lx = x + 8;
    for (let k = 0; k < i; k++) lx += colW[k];
    ctx.beginPath(); ctx.moveTo(lx, y); ctx.lineTo(lx, y + h); ctx.stroke();
  }

  if (!rows.length) {
    G.text(ctx, "还没有记录，快去闯关吧", x + w / 2, y + h / 2, 22, "#e8c9a0", "center");
  }

  const maxRows = Math.floor((h - rowH) / rowH);
  for (let r = 0; r < Math.min(rows.length, maxRows); r++) {
    const it = rows[r];
    const ry = y + rowH + r * rowH;
    if (r % 2 === 1) {
      ctx.fillStyle = "rgba(255,255,255,0.06)";
      ctx.fillRect(x, ry, w, rowH);
    }
    const vals = [String(r + 1), it.name, String(it.fragments), it.time + "秒"];
    let vx = x + 8;
    for (let i = 0; i < vals.length; i++) {
      G.text(ctx, vals[i], vx + colW[i] / 2, ry + rowH / 2, 19, "#ffe9c9", "center", G.FONT_SANS);
      vx += colW[i];
    }
  }
  ctx.restore();
};

/* ---------- 排行榜存档（localStorage 代替原 Qt 的 leaderboard.txt） ---------- */
UI.KEY = "the_eighth_leaderboard";

UI.loadScores = function () {
  try {
    const raw = localStorage.getItem(UI.KEY);
    const arr = raw ? JSON.parse(raw) : [];
    return Array.isArray(arr) ? arr : [];
  } catch (e) { return []; }
};

/* 同名玩家只在“碎片更多，或碎片相同但用时更短”时更新，然后按碎片降序、用时升序排列 */
UI.saveScore = function (name, fragments, time) {
  if (!name) return;
  const scores = UI.loadScores();
  let found = false;
  for (let i = 0; i < scores.length; i++) {
    if (scores[i].name === name) {
      if (fragments > scores[i].fragments || (fragments === scores[i].fragments && time < scores[i].time)) {
        scores[i].fragments = fragments;
        scores[i].time = time;
      }
      found = true;
      break;
    }
  }
  if (!found) scores.push({ name: name, fragments: fragments, time: time });
  scores.sort(function (a, b) {
    if (a.fragments !== b.fragments) return b.fragments - a.fragments;
    return a.time - b.time;
  });
  try { localStorage.setItem(UI.KEY, JSON.stringify(scores)); } catch (e) { }
};

/* ==========================================================================
   输入昵称（原 InputNameDialog：480x313）
   ========================================================================== */
UI.nicknameScene = function (onDone) {
  const DEF = {
    w: 480, h: 313, bg: "input_name_background.png",
    buttons: [
      { x: 80, y: 220, w: 131, h: 61, img: "yes.png", on: function () { confirm(); } },
      { x: 270, y: 220, w: 131, h: 61, img: "cancel.png", on: function () { onDone(null); } },
    ],
  };
  const dlg = UI.dialog(DEF);
  let text = "";

  function confirm() {
    const v = text.trim();
    if (!v) return;              // 与原版一致：昵称为空时点确定无效
    onDone(v);
  }

  dlg.enter = function () { };
  dlg.onKeyDown = function (k, e) {
    if (k === "Enter") { confirm(); return; }
    if (k === "Escape") { onDone(null); return; }
    if (k === "Backspace") { e.preventDefault(); text = text.slice(0, -1); }
  };
  /* 用原始字符，保留大小写（与 Qt 原版一致） */
  dlg.onText = function (ch) { text += ch; };

  const baseDraw = dlg.draw.bind(dlg);
  dlg.draw = function (ctx) {
    baseDraw(ctx);
    ctx.save();
    ctx.translate(dlg.ox, dlg.oy);
    ctx.scale(dlg.scale, dlg.scale);
    G.draw(ctx, "inputname1.png", 140, 40, 181, 71);
    G.draw(ctx, "inputname2.png", 70, 110, 341, 101);
    G.text(ctx, text, 240, 161, 40, "#5b3a1e", "center");
    if (!text) {
      G.text(ctx, "请输入昵称", 240, 161, 26, "rgba(120,90,50,0.55)", "center");
    }
    ctx.restore();
  };
  return dlg;
};

/* ==========================================================================
   背景简介（原 BackgroundDialog：689x876）
   ========================================================================== */
UI.backgroundScene = function (onBack) {
  const dlg = UI.dialog({
    w: 689, h: 876, bg: "background_introduct1.png",
    buttons: [
      { x: 270, y: 770, w: 131, h: 61, img: "inputname1.png", label: "返回", size: 30, color: "#5b3a1e", on: onBack },
    ],
    drawExtra: function (ctx) {
      G.draw(ctx, "background_introduct2.png", 60, 70, 561, 701);
    },
  });
  return dlg;
};

/* ==========================================================================
   排行榜（原 LeaderboardDialog：488x551）
   ========================================================================== */
UI.leaderboardScene = function (onBack) {
  const dlg = UI.dialog({
    w: 488, h: 551, bg: "paihangbang.png",
    buttons: [
      { x: 180, y: 470, w: 131, h: 51, img: "inputname1.png", label: "返回", size: 28, color: "#5b1a12", on: onBack },
    ],
    drawExtra: function (ctx) {
      G.draw(ctx, "paihang.png", 50, 60, 391, 401);
      UI.drawLeaderboardTable(ctx, 50, 60, 391, 401, UI.loadScores());
    },
  });
  return dlg;
};
